#include "Core/EditorState.h"

#include <Renderer/Renderer.h>

using namespace BoonEditor;

void EditorState::OnEnter()
{
	m_pSceneRenderer = std::make_unique<SceneRenderer>();
}

void EditorState::OnUpdate()
{
	OnRender();
}

void EditorState::OnExit()
{

}

void EditorState::OnRender()
{
	Renderer::BeginFrame();

	m_pSceneRenderer->Render();

	Renderer::EndFrame();
}