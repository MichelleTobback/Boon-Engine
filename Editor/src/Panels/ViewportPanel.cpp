#include "Panels/ViewportPanel.h"

#include <Renderer/SceneRenderer.h>
#include <Renderer/Framebuffer.h>

#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace BoonEditor;

BoonEditor::ViewportPanel::ViewportPanel(const std::string& name, SceneContext* pContext, GameObjectContext* pSelection)
	: EditorPanel(name), m_pContext{ pContext }, m_pSelectionContext{pSelection}
{
	m_Camera.SetActive(true);
	m_pRenderer = std::make_unique<SceneRenderer>(pContext->Get());
    m_pToolbar = std::make_unique<ViewportToolbar>(std::string(name).append("toolbar"));

    pContext->AddOnContextChangedCallback([this](Scene* pScene)
        {
            m_pRenderer->SetContext(pScene);
        });
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
		m_HoveredGameObject = pixelData == -1 ? GameObject() : GameObject((GameObjectID)pixelData, m_pContext->Get());

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_HoveredGameObject.IsValid())
        {
            m_pSelectionContext->Set(m_HoveredGameObject);
        }
	}

	//m_pRenderer->Render();
	m_pRenderer->Render(&m_Camera.GetCamera(), &m_Camera.GetTransform());
}

void BoonEditor::ViewportPanel::OnRenderUI()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    ImGui::Begin("Viewport");

    const ImVec2 viewportOffset{ ImGui::GetCursorPos() };
    const ImVec2 windowSize{ ImGui::GetWindowSize() };

    glm::vec2 minBound{ ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
    minBound.x -= viewportOffset.x;
    minBound.y -= viewportOffset.y;
    const glm::vec2 maxBound{ minBound.x + windowSize.x, minBound.y + windowSize.y };
    m_pToolbar->OnRender(minBound, maxBound);

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    glm::vec2 bounds{ glm::max(glm::vec2{viewportPanelSize.x, viewportPanelSize.y}, {100.0f, 100.0f}) };

    // === Viewport bounds ===
    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();

    m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
    m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();

    if ((std::abs(bounds.x - m_ViewportSize.x) > FLT_EPSILON
        || std::abs(bounds.y - m_ViewportSize.y) > FLT_EPSILON)
        && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        m_pRenderer->SetViewport(bounds.x, bounds.y);
        m_Camera.Resize(bounds.x, bounds.y);
        m_ViewportSize = { bounds.x, bounds.y };
    }

    // === Render the viewport image ===
    uint64_t textureID = m_pRenderer->GetOutputTarget()->GetColorAttachmentRendererID();
    ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });

    // === Overlay Toolbar (collapsible) ===
    if (m_pToolbar->GetActiveSetting() == ViewportToolbarSetting::Camera)
    {
        const float panelWidth = 260.0f;
        const float panelGap = 6.0f;
        const float topMargin = 20.0f;
        const float toolbarHeight = 64.0f;

        const float vpLeft = m_ViewportBounds[0].x;
        const float vpTop = m_ViewportBounds[0].y;
        const float vpRight = m_ViewportBounds[1].x;
        const float vpBottom = m_ViewportBounds[1].y;

        float panelX = vpRight + panelGap;
        float panelY = vpTop + topMargin + toolbarHeight;

        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        if (panelX + panelWidth > displaySize.x - 4.0f) 
        {
            // fallback: place inside viewport, flush to right edge (so it's still visible)
            panelX = vpRight - panelWidth - panelGap;
            // if that would go past left edge as well, clamp to a safe value
            if (panelX < vpLeft + 4.0f)
                panelX = vpLeft + 4.0f;
        }

        // call your CameraSettings with the computed X,Y and known width
        CameraSettings(panelX, panelY, panelWidth);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void BoonEditor::ViewportPanel::CameraSettings(float posX, float posY, float maxWidth)
{
    ImVec2 panelPos = ImVec2(posX, posY);

    ImGui::SetNextWindowPos(panelPos);
    ImGui::SetNextWindowSizeConstraints(ImVec2(maxWidth, 0), ImVec2(maxWidth, FLT_MAX));
    ImGui::SetNextWindowBgAlpha(0.55f);

    ImGui::Begin("Camera Toolbar", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav);

    Camera& camera = m_Camera.GetCamera();
    int projectionType = (int)m_Camera.GetMode();
    const char* projectionTypes[] = { "Perspective", "Orthographic" };

    ImGui::TextUnformatted("Camera Settings");
    ImGui::Separator();

    ImGui::SetNextItemWidth(150);
    if (ImGui::Combo("Mode", &projectionType, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
        m_Camera.SetMode((Camera::ProjectionType)projectionType);

    ImGui::SetNextItemWidth(120);
    switch (m_Camera.GetMode())
    {
    case Camera::ProjectionType::Perspective:
    {
        float fov = camera.GetFov();
        if (ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f))
            camera.SetFov(fov);
    }
    break;

    case Camera::ProjectionType::Orthographic:
    {
        float size = camera.GetSize();
        if (ImGui::SliderFloat("Zoom", &size, 1.0f, 100.0f))
            camera.SetSize(size);
    }
    break;
    }

    float nearVal = camera.GetNear();
    if (ImGui::DragFloat("Near", &nearVal, 0.01f))
        camera.SetNear(nearVal);

    float farVal = camera.GetFar();
    if (ImGui::DragFloat("Far", &farVal, 1.0f))
        camera.SetFar(farVal);

    ImGui::End();
}
