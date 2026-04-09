#include "Panels/EditorDialog.h"

#include <imgui.h>

BoonEditor::EditorDialog::EditorDialog(const std::string& name, EditorContext* pContext)
    : EditorWidget(name, pContext) { }

void BoonEditor::EditorDialog::Open()
{
    m_ShouldOpen = true;
    m_IsOpen = true;
    OnOpen();
}

void BoonEditor::EditorDialog::Close()
{
    m_IsOpen = false;
    OnClose();
}

bool BoonEditor::EditorDialog::IsOpen() const { return m_IsOpen; }

void BoonEditor::EditorDialog::RenderUI()
{
    if (m_ShouldOpen)
    {
        ImGui::OpenPopup(m_Name.c_str());
        m_ShouldOpen = false;
    }

    if (!m_IsOpen)
        return;

    if (ImGui::BeginPopupModal(m_Name.c_str()))
    {
        RenderDialog();
        ImGui::EndPopup();
    }
}