#include "Core/EditorState.h"
#include "Core/EditorCamera.h"

#include "Panels/PropertiesPanel.h"

#include "UI/EditorRenderer.h"

#include <Core/Application.h>
#include <Renderer/Renderer.h>

using namespace BoonEditor;

EditorState::EditorState() = default;
EditorState::~EditorState() = default;

void EditorState::OnEnter()
{
	Window& window{ Application::Get().GetWindow() };

	m_PRenderer = std::make_unique<EditorRenderer>();
	m_pSceneRenderer = std::make_unique<SceneRenderer>();

	float aspect = (float)window.GetWidth() / (float)window.GetHeight();
	m_pEditorCamera = &CreateObject<EditorCamera>(2.f * aspect, 2.f);
	m_pEditorCamera->SetActive(true);

	CreatePanel<PropertiesPanel>("Properties");
	CreatePanel<PropertiesPanel>("Viewport");
	CreatePanel<PropertiesPanel>("Scene");
}

void EditorState::OnUpdate()
{
	for (auto& pObject : m_Objects)
	{
		pObject->Update();
	}

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

	m_PRenderer->BeginFrame();

	for (auto& pPanel : m_Panels)
	{
		pPanel->Render();
	}
	
	m_PRenderer->EndFrame();
}