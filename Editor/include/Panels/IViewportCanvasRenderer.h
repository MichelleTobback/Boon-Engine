#pragma once

#include <glm/glm.hpp>

namespace BoonEditor
{
    class ViewportPanel;

    struct ViewportCanvasContext
    {
        ViewportPanel* Viewport = nullptr;

        glm::vec2 Size{ 0.0f, 0.0f };
        glm::vec2 MousePosition{ 0.0f, 0.0f };

        bool Hovered = false;
        bool Focused = false;
    };

    class IViewportCanvasRenderer
    {
    public:
        virtual ~IViewportCanvasRenderer() = default;

        virtual void OnViewportCanvasResize(const glm::vec2& size) {}
        virtual void OnViewportCanvasUpdate(const ViewportCanvasContext& context) {}
        virtual bool OnViewportCanvasRenderUI(const ViewportCanvasContext& context) { return false; }
        virtual bool CanRenderViewport() const { return true; }
    };
}