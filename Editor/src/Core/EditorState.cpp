#include "Core/EditorState.h"
#include "Core/EditorCamera.h"

#include "Panels/PropertiesPanel.h"
#include "Panels/ViewportPanel.h"

#include "UI/EditorRenderer.h"

#include <Core/Application.h>
#include <Scene/Scene.h>
#include <Renderer/Renderer.h>
#include <Scene/GameObject.h>

#include <Component/SpriteRendererComponent.h>

using namespace BoonEditor;

EditorState::EditorState() = default;
EditorState::~EditorState() = default;

void EditorState::OnEnter()
{
	Window& window{ Application::Get().GetWindow() };

	m_PRenderer = std::make_unique<EditorRenderer>();

	m_pScene = std::make_unique<Scene>();

	CreatePanel<ViewportPanel>("Viewport", m_pScene.get());
	CreatePanel<PropertiesPanel>("Properties");
	CreatePanel<PropertiesPanel>("Scene");

	GameObject quad = m_pScene->Instantiate();
	SpriteRendererComponent& sprite = quad.AddComponent<SpriteRendererComponent>();
	sprite.Color = { 0.9f, 0.1f, 0.3f, 1.f };
}

void EditorState::OnUpdate()
{
	m_pScene->Update();
	m_pScene->EndUpdate();

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
	m_PRenderer->BeginFrame();
	
	for (auto& pPanel : m_Panels)
	{
		pPanel->RenderUI();
	}
	
	m_PRenderer->EndFrame();
}