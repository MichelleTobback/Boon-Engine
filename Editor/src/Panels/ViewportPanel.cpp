#include "Panels/ViewportPanel.h"
#include "UI/UI.h"

#include "DebugRenderer/DebugRenderer.h"

#include <Renderer/SceneRenderer.h>
#include <Renderer/Framebuffer.h>

#include <Core/EditorContext.h>

#include <Core/ServiceLocator.h>
#include <Input/Input.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cfloat>

using namespace BoonEditor;

BoonEditor::ViewportPanel::ViewportPanel(
    EditorContext* pContext,
    const std::string& name,
    SceneContext* pSceneContext,
    GameObjectContext* pSelection)
    : EditorPanel(pContext, name),
    m_pSceneContext(pSceneContext),
    m_pSelectionContext(pSelection),
    m_Camera(pContext, 0, 0)
{
    m_Camera.SetActive(true);

    SceneRendererCreateInfo rendererDesc{};
    rendererDesc.pScene = pSceneContext->Get();
    rendererDesc.AssetLib = pContext->GetEngineContext().AssetLib;
    m_pRenderer = std::make_unique<SceneRenderer>(rendererDesc);
    m_pDebugRenderer = std::make_unique<DebugRenderer>(
        pSceneContext->Get(),
        m_pRenderer->GetOutputTarget());

    m_pToolbar = std::make_unique<ViewportToolbar>(
        pContext,
        std::string(name).append("toolbar"));

    m_Settings.DebugRenderLayers |= DebugRenderLayer::Disabled;

    pSceneContext->AddOnContextChangedCallback(
        [this](Scene* pScene)
        {
            m_pRenderer->SetContext(pScene);
            m_pDebugRenderer->SetContext(pScene);
        });
}

BoonEditor::ViewportPanel::~ViewportPanel()
{
}

ViewportCanvasContext BoonEditor::ViewportPanel::CreateCanvasContext() const
{
    ViewportCanvasContext context{};
    context.Viewport = const_cast<ViewportPanel*>(this);
    context.Size = m_ViewportSize;
    context.MousePosition = m_MousePosition;
    context.Hovered = m_ViewportHovered;
    context.Focused = m_ViewportFocused;
    return context;
}

void BoonEditor::ViewportPanel::SetCanvasRenderer(IViewportCanvasRenderer* renderer)
{
    m_pCanvasRenderer = renderer;

    if (m_pCanvasRenderer)
        m_pCanvasRenderer->OnViewportCanvasResize(m_ViewportSize);
}

void BoonEditor::ViewportPanel::ClearCanvasRenderer(IViewportCanvasRenderer* renderer)
{
    if (!renderer || m_pCanvasRenderer == renderer)
        m_pCanvasRenderer = nullptr;
}

void BoonEditor::ViewportPanel::Update()
{
    if (m_ViewportHovered && !(m_pCanvasRenderer && m_pCanvasRenderer->CanRenderViewport()))
        m_Camera.Update();

    Input& input = *GetContext().GetEngineContext().Input;

    float mx = ImGui::GetMousePos().x;
    float my = ImGui::GetMousePos().y;

    mx -= m_ViewportImagePosition.x;
    my -= m_ViewportImagePosition.y;

    my = m_ViewportSize.y - my;

    int mouseX = static_cast<int>(mx);
    int mouseY = static_cast<int>(my);

    m_MousePosition.x = static_cast<float>(mouseX);
    m_MousePosition.y = static_cast<float>(mouseY);

    const bool mouseInsideViewport =
        mouseX >= 0 &&
        mouseY >= 0 &&
        mouseX < static_cast<int>(m_ViewportSize.x) &&
        mouseY < static_cast<int>(m_ViewportSize.y);

    if (!m_pCanvasRenderer && mouseInsideViewport)
    {
        int pixelData = m_pRenderer->GetOutputTarget()->ReadPixel(1, mouseX, mouseY);

        m_HoveredGameObject =
            pixelData < 0
            ? GameObject()
            : GameObject(static_cast<GameObjectID>(pixelData), m_pSceneContext->Get());

        if (input.IsMousePressed(Mouse::ButtonLeft))
            m_pSelectionContext->Set(m_HoveredGameObject);
    }

    if (m_pCanvasRenderer && m_pCanvasRenderer->CanRenderViewport())
    {
        m_pCanvasRenderer->OnViewportCanvasUpdate(CreateCanvasContext());
    }
    else
    {
        if (m_Camera.GetActive())
            m_pRenderer->Render(&m_Camera.GetCamera(), &m_Camera.GetTransform());
        else
            m_pRenderer->Render();

        m_pDebugRenderer->Render(m_Settings);
    }
}

void BoonEditor::ViewportPanel::SetContext(SceneContext* pContext)
{
    m_pSceneContext = pContext;

    m_pRenderer->SetContext(pContext->Get());
    m_pDebugRenderer->SetContext(pContext->Get());
}

void BoonEditor::ViewportPanel::OnRenderUI()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    //ImGui::Begin("Viewport", nullptr, flags);

    constexpr float toolbarHeight = 22.0f;

    ImVec2 fullAvail = ImGui::GetContentRegionAvail();

    if (fullAvail.x < 100.0f || fullAvail.y < 100.0f)
    {
        ImGui::PopStyleVar();
        return;
    }

    ImVec2 toolbarPos = ImGui::GetCursorScreenPos();
    ImVec2 toolbarSize(fullAvail.x, toolbarHeight);

    ImGui::InvisibleButton(
        "##viewport_toolbar_strip_drag_area",
        toolbarSize
    );

    ImVec2 toolbarMin = toolbarPos;
    ImVec2 toolbarMax(
        toolbarPos.x + toolbarSize.x,
        toolbarPos.y + toolbarSize.y
    );

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec2 padding = {12.f, 5.f};
    drawList->AddRectFilled(
        { toolbarMin.x - padding.x, toolbarMin.y - padding.y },
        { toolbarMax.x + padding.x, toolbarMax.y + padding.y },
        ImGui::GetColorU32(ImGuiCol_ChildBg)
    );

    //drawList->AddLine(
    //    ImVec2(toolbarMin.x, toolbarMax.y - 1.0f),
    //    ImVec2(toolbarMax.x, toolbarMax.y - 1.0f),
    //    ImGui::GetColorU32(ImGuiCol_Border)
    //);

    m_pToolbar->OnRender(
        glm::vec2{ toolbarMin.x, toolbarMin.y },
        glm::vec2{ toolbarMax.x, toolbarMax.y }
    );

    // ─────────────────────────────────────────────
    // Viewport image/canvas area below toolbar
    // ─────────────────────────────────────────────

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    ImVec2 imagePos = ImGui::GetCursorScreenPos();

    glm::vec2 minBound{
        imagePos.x,
        imagePos.y
    };

    glm::vec2 maxBound{
        imagePos.x + viewportPanelSize.x,
        imagePos.y + viewportPanelSize.y
    };

    glm::vec2 bounds{
        glm::max(
            glm::vec2{ viewportPanelSize.x, viewportPanelSize.y },
            glm::vec2{ 100.0f, 100.0f }
        )
    };

    m_ViewportBounds[0] = minBound;
    m_ViewportBounds[1] = maxBound;

    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();

    if ((std::abs(bounds.x - m_ViewportSize.x) > FLT_EPSILON ||
        std::abs(bounds.y - m_ViewportSize.y) > FLT_EPSILON) &&
        !ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        m_pRenderer->SetViewport(bounds.x, bounds.y);
        m_Camera.Resize(bounds.x, bounds.y);

        m_ViewportSize = { bounds.x, bounds.y };

        if (m_pCanvasRenderer)
            m_pCanvasRenderer->OnViewportCanvasResize(m_ViewportSize);
    }

    m_ViewportImagePosition = { imagePos.x, imagePos.y };

    if (m_pCanvasRenderer && m_pCanvasRenderer->CanRenderViewport())
    {
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        ImGui::InvisibleButton(
            "##viewport_asset_canvas_host",
            ImVec2{ m_ViewportSize.x, m_ViewportSize.y }
        );

        ImGui::SetCursorScreenPos(canvasPos);

        m_pCanvasRenderer->OnViewportCanvasRenderUI(
            CreateCanvasContext()
        );

        ImGui::SetCursorScreenPos(
            ImVec2(
                canvasPos.x,
                canvasPos.y + m_ViewportSize.y
            )
        );
    }
    else
    {
        uint64_t textureID =
            m_pRenderer
            ->GetOutputTarget()
            ->GetColorAttachmentRendererID();

        ImGui::Image(
            reinterpret_cast<void*>(
                static_cast<uintptr_t>(textureID)
                ),
            ImVec2{ m_ViewportSize.x, m_ViewportSize.y },
            ImVec2{ 0.0f, 1.0f },
            ImVec2{ 1.0f, 0.0f }
        );
    }

    if (m_pToolbar->GetActiveSetting() == ViewportToolbarSetting::Camera)
    {
        const float panelWidth = 260.0f;
        const float panelGap = 6.0f;

        float panelX = maxBound.x - panelWidth - panelGap;
        float panelY = toolbarMax.y + panelGap;

        CameraSettings(panelX, panelY, panelWidth);
    }
    else if (m_pToolbar->GetActiveSetting() == ViewportToolbarSetting::Visibility)
    {
        const float panelWidth = 260.0f;
        const float panelGap = 6.0f;

        float panelX = maxBound.x - panelWidth - panelGap;
        float panelY = toolbarMax.y + panelGap;

        VisibilitySettings(panelX, panelY, panelWidth);
    }

    //ImGui::End();
    ImGui::PopStyleVar();
}

void BoonEditor::ViewportPanel::CameraSettings(float posX, float posY, float maxWidth)
{
    ImVec2 panelPos = ImVec2(posX, posY);

    ImGui::SetNextWindowPos(panelPos);
    ImGui::SetNextWindowSizeConstraints(ImVec2(maxWidth, 0), ImVec2(maxWidth, FLT_MAX));
    ImGui::SetNextWindowBgAlpha(0.55f);

    ImGui::Begin(
        "Camera Toolbar",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav);

    ImGui::TextUnformatted("Camera Settings");
    ImGui::Separator();

    Camera& camera = m_Camera.GetCamera();

    int projectionType = static_cast<int>(camera.GetProjectionType());
    const char* projectionTypes[] = { "Perspective", "Orthographic" };

    if (UI::Combo("Mode", projectionType, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
        m_Camera.SetMode(static_cast<Camera::ProjectionType>(projectionType));

    switch (camera.GetProjectionType())
    {
    case Camera::ProjectionType::Perspective:
    {
        float fov = camera.GetFov();

        if (UI::SliderFloat("FOV", fov, 1.0f, 180.0f))
            camera.SetFov(fov);

        break;
    }

    case Camera::ProjectionType::Orthographic:
    {
        float size = camera.GetSize();

        if (UI::SliderFloat("size", size, 1.0f, 100.0f))
            camera.SetSize(size);

        break;
    }
    }

    float value = camera.GetNear();
    if (UI::DragFloat("near", value))
        camera.SetNear(value);

    value = camera.GetFar();
    if (UI::DragFloat("far", value))
        camera.SetFar(value);

    ImGui::End();
}

void BoonEditor::ViewportPanel::VisibilitySettings(float posX, float posY, float maxWidth)
{
    ImVec2 panelPos = ImVec2(posX, posY);

    ImGui::SetNextWindowPos(panelPos);
    ImGui::SetNextWindowSizeConstraints(ImVec2(maxWidth, 0), ImVec2(maxWidth, FLT_MAX));
    ImGui::SetNextWindowBgAlpha(0.55f);

    ImGui::Begin(
        "Visibility Toolbar",
        nullptr,
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