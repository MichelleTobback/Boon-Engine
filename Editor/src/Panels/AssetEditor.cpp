#include "Panels/AssetEditor.h"
#include "Panels/ViewportPanel.h"

void BoonEditor::AssetEditorBase::OnRender()
{
	ImGui::BeginChild(m_Name.c_str());

	OnRenderUI();

	ImGui::EndChild();
}

glm::vec3 BoonEditor::AssetEditorBase::ScreenToWorld(const glm::vec2& mousePos)
{
	float vw = m_pViewport->GetSize().x;
	float vh = m_pViewport->GetSize().y;

	glm::vec2 ndc;
	ndc.x = (mousePos.x / vw) * 2.0f - 1.f;
	ndc.y = (mousePos.y / vh) * 2.0f - 1.f;

	glm::vec4 clip = glm::vec4(ndc, 0.0f, 1.0f);

	glm::mat4 view = glm::inverse(m_pViewport->GetCamera().GetTransform().GetWorld());
	glm::mat4 proj = m_pViewport->GetCamera().GetCamera().GetProjection();

	glm::mat4 invVP = glm::inverse(proj * view);
	glm::vec4 world = invVP * clip;
	return glm::vec3(world) / world.w;
}
