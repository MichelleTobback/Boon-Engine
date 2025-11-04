#include "Core/EditorState.h"
#include "Core/EditorCamera.h"

#include "Panels/ScenePanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/ViewportPanel.h"

#include "UI/EditorRenderer.h"

#include <Asset/AssetLibrary.h>
#include <Asset/TextureAsset.h>
#include <Core/ServiceLocator.h>
#include <Core/Application.h>
#include <Scene/Scene.h>
#include <Renderer/Renderer.h>
#include <Scene/GameObject.h>

#include <Component/CameraComponent.h>
#include <Component/SpriteRendererComponent.h>

using namespace BoonEditor;

EditorState::EditorState() = default;
EditorState::~EditorState() = default;

void EditorState::OnEnter()
{
	Window& window{ Application::Get().GetWindow() };
	AssetLibrary& assetLib{ ServiceLocator::Get<AssetLibrary>() };

	m_PRenderer = std::make_unique<EditorRenderer>();

	m_pScene = std::make_unique<Scene>();

	CreatePanel<ViewportPanel>("Viewport", &m_SceneContext, &m_SelectionContext);
	CreatePanel<PropertiesPanel>(&m_SelectionContext);
	CreatePanel<ScenePanel>(&m_SceneContext, &m_SelectionContext);

	GameObject camera = m_pScene->Instantiate({ 0.f, 0.f, 1.f });
	camera.AddComponent<CameraComponent>(Camera(2.f, 2.f, 0.1f, 1.f), false).Active = true;
	m_SelectionContext.Set(camera);
	m_SceneContext.Set(m_pScene.get());

	GameObject quad = m_pScene->Instantiate();
	SpriteRendererComponent& sprite = quad.AddComponent<SpriteRendererComponent>();
	sprite.TextureHandle = assetLib.Load<Texture2DAssetLoader>("game/Blue_witch/B_witch_idle.png");
	auto tex = assetLib.GetAsset<Texture2DAsset>(sprite.TextureHandle);
	sprite.TexRect.w = tex->GetHeight() / 6.f / tex->GetHeight();
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