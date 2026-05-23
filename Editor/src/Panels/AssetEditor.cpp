#include "Panels/AssetEditor.h"

void BoonEditor::AssetEditorBase::RenderUI()
{
    ImGui::BeginChild(
        m_Name.c_str(),
        ImVec2(0.0f, 0.0f),
        false,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse
    );

    OnRenderUI();

    ImGui::EndChild();
}