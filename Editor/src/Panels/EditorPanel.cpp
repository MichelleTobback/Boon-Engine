#include "Panels/EditorPanel.h"

#include <imgui.h>
#include <imgui_Internal.h>

using namespace BoonEditor;

BoonEditor::EditorPanel::EditorPanel(const std::string& name)
	: m_Name{ name }
{

}

void BoonEditor::EditorPanel::RenderUI()
{
	ImGui::Begin(m_Name.c_str());

	OnRenderUI();

	ImGui::End();
}

bool BoonEditor::EditorPanel::RenderFloat3Control(const std::string& label, glm::vec3& vector, float resetValue, float columnWidth)
{
	bool result{ false };

	ImGuiIO& io{ ImGui::GetIO() };
	auto boldFont{ io.Fonts->Fonts[0] };

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });

	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize))
	{
		vector.x = resetValue;
		result = true;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	if (ImGui::DragFloat("##X", &vector.x, 0.1f, 0.0f, 0.0f, "%.2f"))
	{
		result = true;
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });

	ImGui::PushFont(boldFont);

	if (ImGui::Button("Y", buttonSize))
	{
		vector.y = resetValue;
		result = true;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	if (ImGui::DragFloat("##Y", &vector.y, 0.1f, 0.0f, 0.0f, "%.2f"))
	{
		result = true;
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });

	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize))
	{
		vector.z = resetValue;
		result = true;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	if (ImGui::DragFloat("##Z", &vector.z, 0.1f, 0.0f, 0.0f, "%.2f"))
	{
		result = true;
	}
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);

	ImGui::PopID();

	return result;
}