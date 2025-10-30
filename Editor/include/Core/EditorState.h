#pragma once
#include "Panels/EditorPanel.h"

#include <Core/AppState.h>
#include <Renderer/SceneRenderer.h>

#include <memory>
#include <type_traits>
#include <vector>

using namespace Boon;

namespace BoonEditor
{
	class EditorRenderer;
	class EditorCamera;
	class EditorState final : public AppState
	{
	public:
		EditorState();
		~EditorState();

		EditorState(const EditorState& other) = default;
		EditorState(EditorState&& other) = default;
		EditorState& operator=(const EditorState& other) = default;
		EditorState& operator=(EditorState&& other) = default;

		virtual void OnEnter() override;
		virtual void OnUpdate() override;
		virtual void OnExit() override;

		template <typename T, typename ...TArgs>
		T& CreateObject(TArgs&& ... args)
		{
			static_assert(std::is_base_of<EditorObject, T>::value, "T must derive from Object");

			auto pInstance = std::make_unique<T>(std::forward<TArgs>(args)...);
			T& ref = *pInstance;
			m_Objects.push_back(std::move(pInstance));
			return ref;
		}

		template <typename T, typename ...TArgs>
		T& CreatePanel(TArgs&& ... args)
		{
			static_assert(std::is_base_of<EditorPanel, T>::value, "T must derive from Panel");

			T& ref = CreateObject<T>(std::forward<TArgs>(args)...);
			m_Panels.push_back(&ref);
			return ref;
		}


	private:
		void OnRender();

		std::unique_ptr<SceneRenderer> m_pSceneRenderer;
		std::unique_ptr<EditorRenderer> m_PRenderer;
		EditorCamera* m_pEditorCamera;

		std::vector<std::unique_ptr<EditorObject>> m_Objects;
		std::vector<EditorPanel*> m_Panels;
	};
}