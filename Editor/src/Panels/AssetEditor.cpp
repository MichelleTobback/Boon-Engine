#include "Panels/AssetEditor.h"

void BoonEditor::AssetEditorBase::RenderUI()
{
    ImGui::BeginChild(m_Name.c_str());

    OnRenderUI();

    ImGui::EndChild();
}