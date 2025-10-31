#include "Panels/ViewportPanel.h"

#include <Renderer/SceneRenderer.h>
#include <Renderer/Framebuffer.h>

#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace BoonEditor;

BoonEditor::ViewportPanel::ViewportPanel(const std::string& name, Scene* pContext)
	: EditorPanel(name), m_pContext{ pContext }
{
	m_Camera.SetActive(true);
	m_pRenderer = std::make_unique<SceneRenderer>(pContext);
}

BoonEditor::ViewportPanel::~ViewportPanel()
{
}

void BoonEditor::ViewportPanel::Update()
{
	if (m_ViewportHovered)
		m_Camera.Update();

	auto [mx, my] = ImGui::GetMousePos();
	mx -= m_ViewportBounds[0].x;
	my -= m_ViewportBounds[0].y;
	glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
	my = viewportSize.y - my;
	int mouseX = (int)mx;
	int mouseY = (int)my;

	if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
	{
		int pixelData = m_pRenderer->GetOutputTarget()->ReadPixel(1, mouseX, mouseY);
		m_HoveredGameObject = pixelData == -1 ? GameObject() : GameObject((GameObjectID)pixelData, m_pContext);
	}

	m_pRenderer->Render(&m_Camera.GetCamera(), &m_Camera.GetTransform());
}

void BoonEditor::ViewportPanel::OnRenderUI()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	ImGui::Begin("Viewport");
	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();
	m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	m_ViewportFocused = ImGui::IsWindowFocused();
	m_ViewportHovered = ImGui::IsWindowHovered();

	//Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportHovered);

	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

	if (std::abs(viewportPanelSize.x - m_ViewportSize.x) > FLT_EPSILON
		|| std::abs(viewportPanelSize.y - m_ViewportSize.y) > FLT_EPSILON)
	{
		float aspect{ viewportPanelSize.x / viewportPanelSize.y };
		m_Camera.GetCamera().SetSize(2.f * aspect, 2.f);
	}

	m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

	uint64_t textureID = m_pRenderer->GetOutputTarget()->GetColorAttachmentRendererID();
	ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

	ImGui::End();
	ImGui::PopStyleVar();
}
