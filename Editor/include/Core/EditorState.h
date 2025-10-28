#pragma once
#include <Core/AppState.h>
#include <Renderer/SceneRenderer.h>

#include <memory>

using namespace Boon;

namespace BoonEditor
{
	class EditorState final : public AppState
	{
	public:
		EditorState() = default;
		~EditorState() = default;

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
	};
}