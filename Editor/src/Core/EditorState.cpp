#include "Core/EditorState.h"

#include "Core/EditorCamera.h"

#include <Core/Application.h>
#include <Renderer/Renderer.h>

using namespace BoonEditor;

EditorState::EditorState() = default;
EditorState::~EditorState() = default;

void EditorState::OnEnter()
{
	Window& window{ Application::Get().GetWindow() };

	m_pSceneRenderer = std::make_unique<SceneRenderer>();

	float aspect = (float)window.GetWidth() / (float)window.GetHeight();
	m_pEditorCamera = std::make_unique<EditorCamera>(2.f * aspect, 2.f);
	m_pEditorCamera->SetActive(true);
}

void EditorState::OnUpdate()
{
	m_pEditorCamera->Update();
	OnRender();
}

void EditorState::OnExit()
{
	
}

void EditorState::OnRender()
{
	Renderer::BeginFrame();

	m_pSceneRenderer->Render(&m_pEditorCamera->GetCamera(), &m_pEditorCamera->GetTransform());

	Renderer::EndFrame();
}