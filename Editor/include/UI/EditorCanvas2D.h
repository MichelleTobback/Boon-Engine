#pragma once

#include <imgui.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>

namespace BoonEditor
{
    class EditorCanvas2D
    {
    public:
        void Begin(const char* id, ImVec2 size = ImVec2(0, 0))
        {
            ImGui::PushID(id);

            ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse;

            ImGui::BeginChild("##canvas_child", size, true, flags);

            m_DrawList = ImGui::GetWindowDrawList();
            m_Origin = ToVec2(ImGui::GetCursorScreenPos());

            ImVec2 avail = ImGui::GetContentRegionAvail();
            m_Size = { avail.x, avail.y };

            if (m_Size.x < 1.0f)
                m_Size.x = 1.0f;
            if (m_Size.y < 1.0f)
                m_Size.y = 1.0f;

            ImGui::InvisibleButton(
                "##canvas_button",
                ImVec2(m_Size.x, m_Size.y),
                ImGuiButtonFlags_MouseButtonLeft |
                ImGuiButtonFlags_MouseButtonMiddle |
                ImGuiButtonFlags_MouseButtonRight
            );

            m_Hovered = ImGui::IsItemHovered();
            m_Active = ImGui::IsItemActive();
            m_MouseScreen = ToVec2(ImGui::GetMousePos());

            m_DrawList->PushClipRect(
                ToImVec2(m_Origin),
                ToImVec2(m_Origin + m_Size),
                true
            );

            HandlePanZoom();
        }

        void End()
        {
            if (m_DrawList)
                m_DrawList->PopClipRect();

            ImGui::EndChild();
            ImGui::PopID();
        }

        glm::vec2 ScreenToCanvas(const glm::vec2& screen) const
        {
            return (screen - m_Origin - m_Pan) / m_Zoom;
        }

        glm::vec2 CanvasToScreen(const glm::vec2& canvas) const
        {
            return m_Origin + m_Pan + canvas * m_Zoom;
        }

        glm::vec2 GetMouseCanvas() const
        {
            return ScreenToCanvas(m_MouseScreen);
        }

        bool IsHovered() const { return m_Hovered; }
        bool IsActive() const { return m_Active; }

        float GetZoom() const { return m_Zoom; }
        glm::vec2 GetPan() const { return m_Pan; }
        glm::vec2 GetSize() const { return m_Size; }

        void FitContent(const glm::vec2& contentMin, const glm::vec2& contentSize, float padding = 32.0f)
        {
            if (contentSize.x <= 0.0f || contentSize.y <= 0.0f || m_Size.x <= 1.0f || m_Size.y <= 1.0f)
                return;

            const float availableX = std::max(1.0f, m_Size.x - padding * 2.0f);
            const float availableY = std::max(1.0f, m_Size.y - padding * 2.0f);

            const float scaleX = availableX / contentSize.x;
            const float scaleY = availableY / contentSize.y;

            m_Zoom = std::clamp(std::min(scaleX, scaleY), m_MinZoom, m_MaxZoom);

            const glm::vec2 contentCenter = contentMin + contentSize * 0.5f;
            const glm::vec2 viewCenter = m_Size * 0.5f;
            m_Pan = viewCenter - contentCenter * m_Zoom;
        }

        void SetZoom(float zoom)
        {
            m_Zoom = std::clamp(zoom, m_MinZoom, m_MaxZoom);
        }

        void SetPan(glm::vec2 pan)
        {
            m_Pan = pan;
        }

        void SetZoomLimits(float minZoom, float maxZoom)
        {
            m_MinZoom = minZoom;
            m_MaxZoom = maxZoom;
            m_Zoom = std::clamp(m_Zoom, m_MinZoom, m_MaxZoom);
        }

        void SetAllowPan(bool allow) { m_AllowPan = allow; }
        void SetAllowZoom(bool allow) { m_AllowZoom = allow; }

        void DrawImage(
            ImTextureID texture,
            const glm::vec2& pos,
            const glm::vec2& size,
            const glm::vec2& uv0 = { 0.0f, 0.0f },
            const glm::vec2& uv1 = { 1.0f, 1.0f },
            ImU32 tint = IM_COL32_WHITE)
        {
            ImVec2 p0 = ToImVec2(CanvasToScreen(pos));
            ImVec2 p1 = ToImVec2(CanvasToScreen(pos + size));

            m_DrawList->AddImage(
                texture,
                p0,
                p1,
                ToImVec2(uv0),
                ToImVec2(uv1),
                tint
            );
        }

        void DrawRect(
            const glm::vec2& pos,
            const glm::vec2& size,
            ImU32 color,
            float thickness = 1.0f)
        {
            ImVec2 p0 = ToImVec2(CanvasToScreen(pos));
            ImVec2 p1 = ToImVec2(CanvasToScreen(pos + size));

            m_DrawList->AddRect(p0, p1, color, 0.0f, 0, thickness);
        }

        void DrawFilledRect(
            const glm::vec2& pos,
            const glm::vec2& size,
            ImU32 color)
        {
            ImVec2 p0 = ToImVec2(CanvasToScreen(pos));
            ImVec2 p1 = ToImVec2(CanvasToScreen(pos + size));

            m_DrawList->AddRectFilled(p0, p1, color);
        }

        void DrawLine(
            const glm::vec2& a,
            const glm::vec2& b,
            ImU32 color,
            float thickness = 1.0f)
        {
            m_DrawList->AddLine(
                ToImVec2(CanvasToScreen(a)),
                ToImVec2(CanvasToScreen(b)),
                color,
                thickness
            );
        }

        void DrawGrid(float spacing, ImU32 color)
        {
            if (spacing <= 0.0f)
                return;

            glm::vec2 topLeft = ScreenToCanvas(m_Origin);
            glm::vec2 bottomRight = ScreenToCanvas(m_Origin + m_Size);

            int startX = static_cast<int>(std::floor(topLeft.x / spacing));
            int endX = static_cast<int>(std::ceil(bottomRight.x / spacing));
            int startY = static_cast<int>(std::floor(topLeft.y / spacing));
            int endY = static_cast<int>(std::ceil(bottomRight.y / spacing));

            for (int x = startX; x <= endX; ++x)
            {
                float px = x * spacing;
                DrawLine({ px, topLeft.y }, { px, bottomRight.y }, color);
            }

            for (int y = startY; y <= endY; ++y)
            {
                float py = y * spacing;
                DrawLine({ topLeft.x, py }, { bottomRight.x, py }, color);
            }
        }

    private:
        void HandlePanZoom()
        {
            if (!m_Hovered)
                return;

            ImGuiIO& io = ImGui::GetIO();

            if (m_AllowPan && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
            {
                m_Pan.x += io.MouseDelta.x;
                m_Pan.y += io.MouseDelta.y;
            }

            if (m_AllowZoom && io.MouseWheel != 0.0f)
            {
                glm::vec2 mouseBefore = ScreenToCanvas(m_MouseScreen);

                float factor = io.MouseWheel > 0.0f ? 1.12f : 0.88f;
                m_Zoom = std::clamp(m_Zoom * factor, std::max(0.001f, m_MinZoom), m_MaxZoom);

                m_Pan = m_MouseScreen - m_Origin - mouseBefore * m_Zoom;
            }
        }

        static glm::vec2 ToVec2(const ImVec2& v)
        {
            return { v.x, v.y };
        }

        static ImVec2 ToImVec2(const glm::vec2& v)
        {
            return { v.x, v.y };
        }

    private:
        ImDrawList* m_DrawList = nullptr;

        glm::vec2 m_Origin{ 0.0f };
        glm::vec2 m_Size{ 0.0f };
        glm::vec2 m_MouseScreen{ 0.0f };

        glm::vec2 m_Pan{ 24.0f, 24.0f };
        float m_Zoom = 1.0f;
        float m_MinZoom = 0.05f;
        float m_MaxZoom = 32.0f;

        bool m_Hovered = false;
        bool m_Active = false;

        bool m_AllowPan = true;
        bool m_AllowZoom = true;
    };
}