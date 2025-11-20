#pragma once
#include <vector>
#include <functional>
#include <imgui.h>
#include <imgui_internal.h>

class DragDropRouter
{
public:
    using DropCallback = std::function<void(const ImGuiPayload*)>;

    struct Target
    {
        ImRect rect;
        DropCallback callback;
    };

    void RegisterTarget(const ImRect& rect, DropCallback cb)
    {
        m_Targets.push_back({ rect, cb });
    }

    void Clear()
    {
        m_Targets.clear();
    }

    void Process()
    {
        if (!ImGui::GetDragDropPayload())
            return;

        ImVec2 mouse = ImGui::GetMousePos();
        const ImGuiPayload* payload = ImGui::GetDragDropPayload();

        for (auto& t : m_Targets)
        {
            if (t.rect.Contains(mouse))
            {
                // Drop goes to the FIRST target under mouse
                t.callback(payload);
                break;
            }
        }

        Clear(); // clear after frame ends
    }

private:
    std::vector<Target> m_Targets;
};
