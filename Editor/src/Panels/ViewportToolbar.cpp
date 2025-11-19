#include "Panels/ViewportToolbar.h"

#include <Core/ServiceLocator.h>
#include <Asset/AssetLibrary.h>
#include <Asset/TextureAsset.h>

#include <math.h>
#include <imgui.h>

BoonEditor::ViewportToolbar::ViewportToolbar(const std::string& name)
	: EditorPanel(name)
{
    AssetLibrary& assetLib = ServiceLocator::Get<AssetLibrary>();

    {
        auto handle = assetLib.Import<Texture2DAsset>("Icons/PlayButton.png");
        m_pPlayIcon = handle->GetInstance();
    }

    {
        auto handle = assetLib.Import<Texture2DAsset>("Icons/StopButton.png");
        m_pStopIcon = handle->GetInstance();
    }

    {
        auto handle = assetLib.Import<Texture2DAsset>("Icons/CameraSettingIcon.png");
        m_pCameraIcon = handle->GetInstance();
    }

    {
        auto handle = assetLib.Import<Texture2DAsset>("Icons/VisibilityButton.png");
        m_pVisibilityIcon = handle->GetInstance();
    }
}

void BoonEditor::ViewportToolbar::OnRender(const glm::vec2& boundsMin, const glm::vec2& boundsMax)
{
    const float buttonSize = 16.f;
    const float edgeOffset = 8.f;
    const float spacing = 4.f;

    const float backgroundHeight = spacing * 2.f + buttonSize;
    const float toolbarY = boundsMin.y + edgeOffset + backgroundHeight;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(spacing, spacing));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));

    //--------------------------------------------------------------
    // MIDDLE TOOLBAR (Play / Stop)
    //--------------------------------------------------------------
    {
        const float numberOfButtons{ 1.f };
        const float backgroundWidth{ spacing * (numberOfButtons + 2.f) + buttonSize * numberOfButtons + spacing * (numberOfButtons - 1.f) * 2.0f };
        const float toolbarX{ (boundsMin.x + boundsMax.x) / 2.0f };

        ImGui::SetNextWindowPos(ImVec2(toolbarX - (backgroundWidth / 2.0f), boundsMin.y + edgeOffset + buttonSize));
        ImGui::SetNextWindowBgAlpha(0.55f);

        ImGui::Begin("##viewportToolbar", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);

        std::shared_ptr<Texture2D>& icon = (m_PlayState == EditorPlayState::Edit) ? m_pPlayIcon : m_pStopIcon;

        if (ImGui::ImageButton("##playbutton", (ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(buttonSize, buttonSize)))
        {
            if (m_PlayState == EditorPlayState::Edit)
                OnPlay();
            else
                OnStop();
        }

        ImGui::End();
    }

    //--------------------------------------------------------------
    // RIGHT TOOLBAR (Camera Settings)
    //--------------------------------------------------------------
    {
        const float numberOfButtons{ 2.f };
        const float backgroundWidth{ spacing * (numberOfButtons + 2.f) + buttonSize * numberOfButtons + spacing * (numberOfButtons - 1.f) * 2.0f };
        const float toolbarX = boundsMax.x - backgroundWidth - edgeOffset * 2.f;

        ImGui::SetNextWindowPos(ImVec2(toolbarX, boundsMin.y + edgeOffset + buttonSize));
        ImGui::SetNextWindowBgAlpha(0.55f);

        ImGui::Begin("##viewportToolbarSettings", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);

        auto drawSettingsButton = [this, buttonSize](const std::string& name, ViewportToolbarSetting toolbarSetting, Texture2D* pTexture)
            {
                bool buttonEnabled = m_ActiveSetting == toolbarSetting;
                if (buttonEnabled)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 1.0f, 1.0f)); // active color
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.4f, 0.9f, 1.0f));
                }

                if (ImGui::ImageButton(name.c_str(), (ImTextureID)(uint64_t)pTexture->GetRendererID(), ImVec2(buttonSize, buttonSize)))
                {
                    m_ActiveSetting = buttonEnabled
                        ? ViewportToolbarSetting::None
                        : toolbarSetting;
                }

                if (buttonEnabled)
                    ImGui::PopStyleColor(3);
            };

        drawSettingsButton("##visibilitybutton", ViewportToolbarSetting::Visibility, m_pVisibilityIcon.get());
        ImGui::SameLine();
        drawSettingsButton("##camerabutton", ViewportToolbarSetting::Camera, m_pCameraIcon.get());

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Camera Settings");

        ImGui::End();
    }

    ImGui::PopStyleVar(5);
    
}

void BoonEditor::ViewportToolbar::BindOnPlayCallback(const std::function<void()>& fn)
{
	m_fnOnPlayCallback = fn;
}

void BoonEditor::ViewportToolbar::BindOnPauseCallback(const std::function<void()>& fn)
{
	m_fnOnPauseCallback = fn;
}

void BoonEditor::ViewportToolbar::BindOnStopCallback(const std::function<void()>& fn)
{
	m_fnOnStopCallback = fn;
}

void BoonEditor::ViewportToolbar::OnPlay()
{
	m_PlayState = EditorPlayState::Play;
	if (m_fnOnPlayCallback)
		m_fnOnPlayCallback();
}

void BoonEditor::ViewportToolbar::OnPause()
{
	if (m_fnOnPauseCallback)
		m_fnOnPauseCallback();
}

void BoonEditor::ViewportToolbar::OnStop()
{
	m_PlayState = EditorPlayState::Edit;
	if (m_fnOnStopCallback)
		m_fnOnStopCallback();
}
