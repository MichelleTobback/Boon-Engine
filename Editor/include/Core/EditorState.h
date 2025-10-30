#pragma once
#include <Core/AppState.h>
#include <Renderer/SceneRenderer.h>

#include <memory>

using namespace Boon;

namespace BoonEditor
{
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

	private:
		void OnRender();

		std::unique_ptr<SceneRenderer> m_pSceneRenderer;

		std::unique_ptr<EditorCamera> m_pEditorCamera;
	};
}