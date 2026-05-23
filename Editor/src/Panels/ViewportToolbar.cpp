#include "Panels/ViewportToolbar.h"
#include <UI/IconsFontAwesome7.h>

#include <Core/ServiceLocator.h>
#include <Asset/AssetLibrary.h>
#include <Asset/TextureAsset.h>
#include <Assets/AssetDatabase.h>

#include <math.h>
#include <imgui.h>

BoonEditor::ViewportToolbar::ViewportToolbar(
    const std::string& name,
    EditorContext* pContext)
    : EditorPanel(name, pContext)
{
}

bool BoonEditor::ViewportToolbar::OnRender(const glm::vec2& boundsMin, const glm::vec2& boundsMax)
{
    const float toolbarHeight = boundsMax.y - boundsMin.y;

    const float edgeOffset = 0.0f;
    const float spacing = 8.0f;
    const float padding = 2.0f;

    const float buttonSize = std::max(toolbarHeight - padding * 2.0f, 16.0f);

    const float backgroundHeight = buttonSize + padding * 2.0f;

    const float toolbarY = boundsMin.y + (toolbarHeight - backgroundHeight) * 0.5f;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 1.0f, 0.0f));

    auto toolbarFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    //--------------------------------------------------------------
    // MIDDLE TOOLBAR (Play / Stop)
    //--------------------------------------------------------------
    {
        const float numberOfButtons = 1.0f;

        const float backgroundWidth =
            buttonSize * numberOfButtons +
            spacing * (numberOfButtons - 1.0f) +
            padding * 2.0f;

        const float toolbarX =
            (boundsMin.x + boundsMax.x) * 0.5f -
            backgroundWidth * 0.5f;

        ImGui::SetNextWindowPos(ImVec2(toolbarX, toolbarY));
        ImGui::SetNextWindowSize(ImVec2(backgroundWidth, backgroundHeight));
        ImGui::SetNextWindowBgAlpha(0.f);

        ImGui::Begin("##viewportToolbar", nullptr, toolbarFlags);

        const char* icon =
            (m_PlayState == EditorPlayState::Edit)
            ? ICON_FA_PLAY
            : ICON_FA_STOP;

        ImGui::PushFont(ImGui::GetFont());

        if (ImGui::Button(icon, ImVec2(buttonSize, buttonSize)))
        {
            if (m_PlayState == EditorPlayState::Edit)
                OnPlay();
            else
                OnStop();
        }

        ImGui::PopFont();

        ImGui::End();
    }

    //--------------------------------------------------------------
    // RIGHT TOOLBAR (Camera Settings)
    //--------------------------------------------------------------
    {
        const float numberOfButtons = 2.0f;

        const float backgroundWidth =
            buttonSize * numberOfButtons +
            spacing * (numberOfButtons - 1.0f) +
            padding * 2.0f;

        const float toolbarX =
            boundsMax.x - backgroundWidth - edgeOffset;

        ImGui::SetNextWindowPos(ImVec2(toolbarX, toolbarY));
        ImGui::SetNextWindowSize(ImVec2(backgroundWidth, backgroundHeight));
        ImGui::SetNextWindowBgAlpha(0.f);

        ImGui::Begin("##viewportToolbarSettings", nullptr, toolbarFlags);

        auto drawSettingsButton =
            [this, buttonSize](
                const std::string& name,
                const char* icon,
                ViewportToolbarSetting toolbarSetting)
            {
                bool buttonEnabled =
                    m_ActiveSetting == toolbarSetting;

                if (buttonEnabled)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 1.0f, 0.25f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 1.0f, 0.45f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.4f, 0.9f, 0.65f));
                }

                if (ImGui::Button(icon, ImVec2(buttonSize, buttonSize)))
                {
                    m_ActiveSetting =
                        buttonEnabled
                        ? ViewportToolbarSetting::None
                        : toolbarSetting;
                }

                if (buttonEnabled)
                    ImGui::PopStyleColor(3);
            };

        drawSettingsButton("##visibilitybutton", ICON_FA_EYE, ViewportToolbarSetting::Visibility);

        ImGui::SameLine(0.0f, spacing);

        drawSettingsButton("##camerabutton", ICON_FA_CAMERA, ViewportToolbarSetting::Camera);

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Camera Settings");

        ImGui::End();
    }

    ImGui::PopStyleVar(7);

    ImGui::PopStyleColor();

    return false;
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
