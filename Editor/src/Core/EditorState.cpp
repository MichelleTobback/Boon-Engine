#include "Core/EditorState.h"

#include <Renderer/Renderer.h>

using namespace BoonEditor;

void EditorState::OnEnter()
{

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

	Renderer::EndFrame();
}