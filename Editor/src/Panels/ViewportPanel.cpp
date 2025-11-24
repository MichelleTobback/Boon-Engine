#include "Panels/ViewportPanel.h"
#include "UI/UI.h"

#include "DebugRenderer/DebugRenderer.h"

#include <Renderer/SceneRenderer.h>
#include <Renderer/Framebuffer.h>

#include <Core/ServiceLocator.h>
#include <Input/Input.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

using namespace BoonEditor;

BoonEditor::ViewportPanel::ViewportPanel(const std::string& name, DragDropRouter* pRouter, SceneContext* pContext, GameObjectContext* pSelection)
	: EditorPanel(name, pRouter), m_pContext{ pContext }, m_pSelectionContext{pSelection}
{
	m_Camera.SetActive(true);
	m_pRenderer = std::make_unique<SceneRenderer>(pContext->Get());
    m_pDebugRenderer = std::make_unique<DebugRenderer>(pContext->Get(), m_pRenderer->GetOutputTarget());

    m_pToolbar = std::make_unique<ViewportToolbar>(std::string(name).append("toolbar"), m_pRouter);

    pContext->AddOnContextChangedCallback([this](Scene* pScene)
        {
            m_pRenderer->SetContext(pScene);
            m_pDebugRenderer->SetContext(pScene);
        });

    m_Settings.DebugRenderLayers |= DebugRenderLayer::Disabled;
}

BoonEditor::ViewportPanel::~ViewportPanel()
{
    
}

void BoonEditor::ViewportPanel::Update()
{
	if (m_ViewportHovered)
		m_Camera.Update();

    Input& input = ServiceLocator::Get<Input>();

    float mx = ImGui::GetMousePos().x;
    float my = ImGui::GetMousePos().y;
	
    mx -= m_ViewportImagePosition.x;
    my -= m_ViewportImagePosition.y;
    glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
    my = m_ViewportSize.y - my;
    int mouseX = (int)mx;
    int mouseY = (int)my;

    m_MousePosition.x = mouseX;
    m_MousePosition.y = mouseY;

	if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)m_ViewportSize.x && mouseY < (int)m_ViewportSize.y)
	{
		int pixelData = m_pRenderer->GetOutputTarget()->ReadPixel(1, mouseX, mouseY);
		m_HoveredGameObject = pixelData < 0 ? GameObject() : GameObject((GameObjectID)pixelData, m_pContext->Get());

        if (input.IsMousePressed(Mouse::ButtonLeft))
        {
            m_pSelectionContext->Set(m_HoveredGameObject);
        }
	}

    if (m_Camera.GetActive())
	    m_pRenderer->Render(&m_Camera.GetCamera(), &m_Camera.GetTransform());
    else
        m_pRenderer->Render();

    m_pDebugRenderer->Render(m_Settings);
}

void BoonEditor::ViewportPanel::SetContext(SceneContext* pContext)
{
    m_pContext = pContext;
    pContext->AddOnContextChangedCallback([this](Scene* pScene)
        {
            m_pRenderer->SetContext(pScene);
            m_pDebugRenderer->SetContext(pScene);
        });
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

    m_ViewportBounds[0] = minBound;
    m_ViewportBounds[1] = maxBound;

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

    ImVec2 imagePos = ImGui::GetCursorScreenPos();
    m_ViewportImagePosition = { imagePos.x, imagePos.y };

    // === Render the viewport image ===
    uint64_t textureID = m_pRenderer->GetOutputTarget()->GetColorAttachmentRendererID();
    ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });

    // === Overlay Toolbar (collapsible) ===
    if (m_pToolbar->GetActiveSetting() == ViewportToolbarSetting::Camera)
    {
        const float panelWidth = 260.0f;
        const float panelGap = 6.0f;
        const float topMargin = 20.0f;
        const float toolbarHeight = 36.0f;

        float panelX = maxBound.x - panelWidth - panelGap;
        float panelY = m_ViewportBounds[0].y + topMargin + toolbarHeight + panelGap;

        CameraSettings(panelX, panelY, panelWidth);
    }
    else if (m_pToolbar->GetActiveSetting() == ViewportToolbarSetting::Visibility)
    {
        const float panelWidth = 260.0f;
        const float panelGap = 6.0f;
        const float topMargin = 20.0f;
        const float toolbarHeight = 36.0f;
    
        float panelX = maxBound.x - panelWidth - panelGap;
        float panelY = m_ViewportBounds[0].y + topMargin + toolbarHeight + panelGap;
    
        VisibilitySettings(panelX, panelY, panelWidth);
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

    ImGui::TextUnformatted("Camera Settings");
    ImGui::Separator();

    Camera& camera = m_Camera.GetCamera();
    int projectionType = (int)camera.GetProjectionType();
    const char* projectionTypes[] = { "Perspective", "Orthographic" };

    if (UI::Combo("Mode", projectionType, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
    {
        m_Camera.SetMode((Camera::ProjectionType)projectionType);
    }

    switch (camera.GetProjectionType())
    {
    case Camera::ProjectionType::Perspective:
    {
        float fov = camera.GetFov();
        if (UI::SliderFloat("FOV", fov, 1.0f, 180.0f))
        {
            camera.SetFov(fov);
        }
    }
    break;
    case Camera::ProjectionType::Orthographic:
    {
        float size = camera.GetSize();
        if (UI::SliderFloat("size", size, 1.0f, 100.0f))
        {
            camera.SetSize(size);
        }
    }
    break;
    }

    float value = camera.GetNear();
    if (UI::DragFloat("near", value))
    {
        camera.SetNear(value);
    }
    value = camera.GetFar();
    if (UI::DragFloat("far", value))
    {
        camera.SetFar(value);
    }

    ImGui::End();
}

void BoonEditor::ViewportPanel::VisibilitySettings(float posX, float posY, float maxWidth)
{
    ImVec2 panelPos = ImVec2(posX, posY);

    ImGui::SetNextWindowPos(panelPos);
    ImGui::SetNextWindowSizeConstraints(ImVec2(maxWidth, 0), ImVec2(maxWidth, FLT_MAX));
    ImGui::SetNextWindowBgAlpha(0.55f);

    ImGui::Begin("Visibility Toolbar", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav);

    ImGui::TextUnformatted("Visibility Settings");
    ImGui::Separator();
    ImGui::TextUnformatted("Layers");

    bool isSet = !(m_Settings.DebugRenderLayers & DebugRenderLayer::Disabled);
    if (UI::Checkbox("Enabled", isSet))
    {
        if (isSet)
            m_Settings.DebugRenderLayers &= ~DebugRenderLayer::Disabled;
        else
            m_Settings.DebugRenderLayers |= DebugRenderLayer::Disabled;
    }

    isSet = m_Settings.DebugRenderLayers & DebugRenderLayer::Default;
    if (UI::Checkbox("Default", isSet))
    {
        if (isSet)
            m_Settings.DebugRenderLayers |= DebugRenderLayer::Default;
        else
            m_Settings.DebugRenderLayers &= ~DebugRenderLayer::Default;
    }

    isSet = m_Settings.DebugRenderLayers & DebugRenderLayer::Collision;
    if (UI::Checkbox("Collision", isSet))
    {
        if (isSet)
            m_Settings.DebugRenderLayers |= DebugRenderLayer::Collision;
        else
            m_Settings.DebugRenderLayers &= ~DebugRenderLayer::Collision;
    }

    isSet = m_Settings.DebugRenderLayers & DebugRenderLayer::Gizmos;
    if (UI::Checkbox("Gizmos", isSet))
    {
        if (isSet)
            m_Settings.DebugRenderLayers |= DebugRenderLayer::Gizmos;
        else
            m_Settings.DebugRenderLayers &= ~DebugRenderLayer::Gizmos;
    }

    ImGui::End();
}
