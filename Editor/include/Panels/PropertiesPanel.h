#pragma once
#include "EditorPanel.h"
#include "Core/BoonEditor.h"
#include "UI/UI.h"
#include <UI/IconsFontAwesome7.h>
#include <Core/EditorContext.h>
#include <Reflection/BClass.h>

#include <Command/EditorCommandQueue.h>
#include <Command/PropertyCommand.h>

#include <memory>
#include <type_traits>
#include <imgui.h>

namespace BoonEditor
{
	class PropertiesPanel final : public EditorPanel
	{
	public:
		PropertiesPanel(EditorContext* pContext, const std::string& name, GameObjectContext* pGameObject);
		virtual ~PropertiesPanel() = default;

		virtual void Update() override {}

	protected:
		virtual void OnRenderUI() override;

	private:
		template <typename T>
		void RenderComponentNode(const std::string& name, bool canBeRemoved, const std::function<void(T&)>& fn, const char* icon = ICON_FA_CUBE);
        void RenderComponentNode(BClass* cls, const std::function<void()>& fn = nullptr);
		void RenderComponentNode_Internal(
			const std::string& id,
			const std::string& name,
			const char* icon,
			bool canExpand,
			bool canBeRemoved,
			bool& open,
			const std::function<void()>& drawBody,
			const std::function<void()>& onRemove);

	private:
		GameObjectContext* m_pGameObjectContext;
	};

	template<typename T>
	inline void PropertiesPanel::RenderComponentNode(
		const std::string& displayName,
		bool canBeRemoved,
		const std::function<void(T&)>& fn,
		const char* icon)
	{
		GameObject& owner = m_pGameObjectContext->Get();

		if (!owner.HasComponent<T>())
			return;

		static std::unordered_map<std::string, bool> s_OpenStates;

		const std::string id = displayName + std::to_string(owner.GetUUID());
		const std::string name = displayName.empty() ? "Component" : displayName;

		bool& open = s_OpenStates[id];
		if (s_OpenStates.find(id) == s_OpenStates.end())
			open = true;

		const bool canExpand = static_cast<bool>(fn);

		RenderComponentNode_Internal(
			id,
			name,
			icon,
			canExpand,
			canBeRemoved,
			open,
			[&]()
			{
				T& component = owner.GetComponent<T>();

				if (fn)
					fn(component);
			},
			[&]()
			{
				owner.RemoveComponent<T>();
			});
	}
}