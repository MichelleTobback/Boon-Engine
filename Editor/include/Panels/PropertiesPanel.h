#pragma once
#include "EditorPanel.h"
#include "Core/BoonEditor.h"
#include "UI/UI.h"

#include <Reflection/BClass.h>

#include <memory>

#include <imgui.h>

namespace BoonEditor
{
	class PropertiesPanel final : public EditorPanel
	{
	public:
		PropertiesPanel(const std::string& name, GameObjectContext* pContext);
		virtual ~PropertiesPanel() = default;

		virtual void Update() override {}

	protected:
		virtual void OnRenderUI() override;

	private:
		template <typename T>
		void RenderComponentNode(const std::string& name, const std::function<void(T&)>& fn);
        void RenderComponentNode(BClass* cls, const std::function<void()>& fn = nullptr);

		GameObjectContext* m_pContext;
	};

    template<typename T>
    inline void PropertiesPanel::RenderComponentNode(const std::string& name, const std::function<void(T&)>& fn)
    {
        // Compatibility fallback for older Dear ImGui versions
#ifndef ImGuiTreeNodeFlags_AllowItemOverlap
#define ImGuiTreeNodeFlags_AllowItemOverlap (1 << 20)
#endif

        const ImGuiTreeNodeFlags treeNodeFlags =
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_Framed |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_FramePadding |
            ImGuiTreeNodeFlags_AllowItemOverlap;

        GameObject& owner = m_pContext->Get();
        if (owner.HasComponent<T>())
        {
            T& pComponent = owner.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });

            // Use version-safe way to get line height
            float lineHeight = ImGui::GetTextLineHeight();

            ImGui::Separator();

            // Use the owner UUID correctly
            std::string id = name + std::to_string(owner.GetUUID());
            bool open = ImGui::TreeNodeEx(id.c_str(), treeNodeFlags, "%s", name.c_str());

            ImGui::PopStyleVar();

            // Position the "+" button on the far right
            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
            if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
            {
                ImGui::OpenPopup("ComponentSettings");
            }

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSettings"))
            {
                if (ImGui::MenuItem("Remove component"))
                    removeComponent = true;

                ImGui::EndPopup();
            }

            if (open)
            {
                BClass* cls = BClassRegistry::Get().Find<T>();
                if (cls)
                {
                    cls->ForEachProperty([&pComponent](const BProperty& prop)
                        {
                            UI::Property(prop, &pComponent);
                        });
                }

                fn(pComponent);
                ImGui::TreePop();
            }

            if (removeComponent)
                owner.RemoveComponent<T>();
        }
    }
}