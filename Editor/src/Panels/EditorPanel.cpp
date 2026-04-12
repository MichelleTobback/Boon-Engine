#include "Panels/EditorPanel.h"

#include <Core/BitFlag.h>

#include <imgui.h>
#include <imgui_Internal.h>

using namespace Boon;
using namespace BoonEditor;

BoonEditor::EditorPanel::EditorPanel(const std::string& name, EditorContext* pContext)
	: EditorWidget(name, pContext)
{

}

void BoonEditor::EditorPanel::RenderUI()
{
	ImGui::Begin(m_Name.c_str());

	BitFlag::Set(m_Flags, EditorPanelFlag::Hovered, ImGui::IsWindowHovered());

	OnRenderUI();

	ImGui::End();
}

bool BoonEditor::EditorPanel::IsHovered() const
{
	return BitFlag::IsSet(m_Flags, EditorPanelFlag::Hovered);
}