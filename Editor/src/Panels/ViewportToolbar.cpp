#include "Panels/ViewportToolbar.h"

#include <imgui.h>

BoonEditor::ViewportToolbar::ViewportToolbar(const std::string& name)
	: EditorPanel(name)
{
}

void BoonEditor::ViewportToolbar::OnRender(const glm::vec2& boundsMin, const glm::vec2& boundsMax)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    const float buttonWidth = 60.f;
    const float buttonHeight = 24.f;
    const float edgeOffset = 4.f;
    const float windowHeight = 32.f; // ImGui minimum enforced window height

    //--------------------------------------------------------------
    // LEFT TOOLBAR (Play / Stop)
    //--------------------------------------------------------------
    {
        const float backgroundWidth = buttonWidth; // exactly match button
        const float toolbarX = (boundsMin.x + boundsMax.x) * 0.5f - (backgroundWidth * 0.5f);
        const float toolbarY = boundsMin.y + edgeOffset + (windowHeight * 0.5f);

        // Shrink window to button size
        ImGui::SetNextWindowPos(ImVec2(toolbarX, toolbarY));
        ImGui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));

        ImGui::Begin("##viewportToolbar", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground);

        // Draw a background manually that matches the button bounds exactly
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 bgMin = winPos;
        ImVec2 bgMax = ImVec2(winPos.x + buttonWidth, winPos.y + windowHeight);
        drawList->AddRectFilled(bgMin, bgMax, IM_COL32(30, 30, 30, 200), 4.f); // rounded bg

        // Center button vertically
        ImGui::SetCursorPos(ImVec2(0, (windowHeight - buttonHeight) * 0.5f));

        const char* label = (m_PlayState == EditorPlayState::Edit) ? "Play" : "Stop";
        if (ImGui::Button(label, ImVec2(buttonWidth, buttonHeight)))
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
        const float backgroundWidth = buttonWidth; // same — exactly button width
        const float toolbarX = boundsMax.x - backgroundWidth - edgeOffset;
        const float toolbarY = boundsMin.y + edgeOffset + (windowHeight * 0.5f);

        ImGui::SetNextWindowPos(ImVec2(toolbarX, toolbarY));
        ImGui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));

        ImGui::Begin("##viewportToolbarSettings", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 bgMin = winPos;
        ImVec2 bgMax = ImVec2(winPos.x + buttonWidth, winPos.y + windowHeight);
        drawList->AddRectFilled(bgMin, bgMax, IM_COL32(30, 30, 30, 200), 4.f); // rounded bg

        ImGui::SetCursorPos(ImVec2(0, (windowHeight - buttonHeight) * 0.5f));

        bool cameraSetting = m_ActiveSetting == ViewportToolbarSetting::Camera;
        if (cameraSetting)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 1.0f, 1.0f)); // active color
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.4f, 0.9f, 1.0f));
        }

        if (ImGui::Button("Cam", ImVec2(buttonWidth, buttonHeight)))
        {
            m_ActiveSetting = cameraSetting
                ? ViewportToolbarSetting::None
                : ViewportToolbarSetting::Camera;
        }

        if (cameraSetting)
            ImGui::PopStyleColor(3);

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Camera Settings");

        ImGui::End();
    }

    ImGui::PopStyleVar(4);
    
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
