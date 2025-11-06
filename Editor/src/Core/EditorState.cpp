#include "Core/EditorState.h"
#include "Core/EditorCamera.h"

#include "Panels/ScenePanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/ViewportPanel.h"

#include "UI/EditorRenderer.h"

#include <Asset/Assets.h>

#include <Core/Application.h>
#include <Core/ServiceLocator.h>

#include <Event/EventBus.h>
#include <Event/SceneEvents.h>

#include <Renderer/Renderer.h>

#include <Scene/SceneManager.h>
#include <Scene/GameObject.h>

#include <Component/CameraComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/SpriteAnimatorComponent.h>

#include <Reflection/BClass.h>

using namespace BoonEditor;

EditorState::EditorState() = default;
EditorState::~EditorState() = default;

void EditorState::OnEnter()
{
	Window& window{ Application::Get().GetWindow() };
	AssetLibrary& assetLib{ Assets::Get() };

	m_PRenderer = std::make_unique<EditorRenderer>();

	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	Scene& scene = sceneManager.CreateScene("Game");

	ViewportPanel& viewport = CreatePanel<ViewportPanel>("Viewport", &m_SceneContext, &m_SelectionContext);
	CreatePanel<PropertiesPanel>(&m_SelectionContext);
	CreatePanel<ScenePanel>(&m_SceneContext, &m_SelectionContext);

	GameObject camera = scene.Instantiate({ 0.f, 0.f, 1.f });
	camera.AddComponent<CameraComponent>(Camera(2.f, 2.f, 0.1f, 1.f), false).Active = true;
	m_SelectionContext.Set(camera);
	m_SceneContext.Set(&scene);

	GameObject quad = scene.Instantiate();
	SpriteRendererComponent& sprite = quad.AddComponent<SpriteRendererComponent>();
	sprite.SpriteAtlasHandle = assetLib.Load<SpriteAtlasAssetLoader>("game/Blue_witch/B_witch_idle.bsa");
	sprite.Sprite = 0;
	SpriteAnimatorComponent& animator = quad.AddComponent<SpriteAnimatorComponent>();
	auto atlas = assetLib.GetAsset<SpriteAtlasAsset>(sprite.SpriteAtlasHandle);
	animator.Clip = 0;
	animator.Atlas = atlas;
	animator.pRenderer = &sprite;

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	m_SceneChangedEvent = eventBus.Subscribe<SceneChangedEvent>([this](const SceneChangedEvent& e)
		{
			SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
			Scene& scene = sceneManager.GetActiveScene();
			m_SceneContext.Set(&scene);
		});

	sceneManager.SetActiveScene(scene.GetID());
}

void EditorState::OnUpdate()
{
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	sceneManager.Update();
	sceneManager.LateUpdate();

	for (auto& pObject : m_Objects)
	{
		pObject->Update();
	}

	OnRender();
}

void EditorState::OnExit()
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Unsubscribe<SceneChangedEvent>(m_SceneChangedEvent);
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