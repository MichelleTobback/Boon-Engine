#include "Core/EditorState.h"
#include "Core/EditorCamera.h"

#include "Panels/ScenePanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/ViewportPanel.h"

#include "UI/EditorRenderer.h"

#include <Asset/Assets.h>

#include <Core/Application.h>
#include <Core/ServiceLocator.h>
#include <Core/Time.h>

#include <Event/EventBus.h>
#include <Event/SceneEvents.h>
#include "Event/EditorEvent.h"

#include <Renderer/Renderer.h>

#include <Scene/SceneManager.h>
#include <Scene/GameObject.h>

#include <Component/CameraComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/SpriteAnimatorComponent.h>
#include <Component/BoxCollider2D.h>
#include <Component/Rigidbody2D.h>

#include "Game/PlayerController.h"

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
	viewport.GetToolbar()->BindOnPlayCallback(std::bind(&EditorState::OnBeginPlay, this));
	viewport.GetToolbar()->BindOnStopCallback(std::bind(&EditorState::OnStopPlay, this));

	CreatePanel<PropertiesPanel>(&m_SelectionContext);
	CreatePanel<ScenePanel>(&m_SceneContext, &m_SelectionContext);

	GameObject camera = scene.Instantiate({ 0.f, 0.f, 1.f });
	camera.AddComponent<CameraComponent>(Camera(4.f, 2.f, 0.1f, 1.f), false).Active = true;
	m_SelectionContext.Set(camera);
	m_SceneContext.Set(&scene);

	//player
	{
		GameObject quad = scene.Instantiate();
		SpriteRendererComponent& sprite = quad.AddComponent<SpriteRendererComponent>();
		sprite.SpriteAtlasHandle = assetLib.Load<SpriteAtlasAssetLoader>("game/Blue_witch/B_witch_atlas_compact.bsa");
		sprite.Sprite = 0;

		SpriteAnimatorComponent& animator = quad.AddComponent<SpriteAnimatorComponent>();
		auto atlas = assetLib.GetAsset<SpriteAtlasAsset>(sprite.SpriteAtlasHandle);
		animator.Clip = 0;
		animator.Atlas = atlas;
		animator.pRenderer = &sprite;

		BoxCollider2D& col = quad.AddComponent<BoxCollider2D>();
		col.Size = { 0.8f, 1.f };

		Rigidbody2D& rb = quad.AddComponent<Rigidbody2D>();
		rb.Type = Boon::Rigidbody2D::BodyType::Dynamic;

		quad.AddComponent<PlayerController>();
	}

	//floor
	{
		GameObject floor = scene.Instantiate({ 0.f, -1.f, 0.f });
	
		BoxCollider2D& floorCol = floor.AddComponent<BoxCollider2D>();
		floorCol.Size = { 10.f, 0.2f };
	
		Rigidbody2D& floorRb = floor.AddComponent<Rigidbody2D>();
		floorRb.Type = Boon::Rigidbody2D::BodyType::Static;
	}

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	m_SceneChangedEvent = eventBus.Subscribe<SceneChangedEvent>([this](const SceneChangedEvent& e)
		{
			SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
			Scene& scene = sceneManager.GetActiveScene();
			m_SceneContext.Set(&scene);
		});

	m_StateChangedEvent = eventBus.Subscribe<EditorPlayStateChangeEvent>([&viewport](const EditorPlayStateChangeEvent& e)
		{
			switch (e.State)
			{
			case EditorPlayState::Play:
				viewport.GetCamera().SetActive(false);
				break;
			case EditorPlayState::Edit:
				viewport.GetCamera().SetActive(true);
				break;
			}
		});

	sceneManager.SetActiveScene(scene.GetID());
}

void EditorState::OnUpdate()
{
	Time& time = Boon::Time::Get();
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();

	switch (m_PlayState)
	{
		case EditorPlayState::Play:
		{
			while (time.FixedStep())
			{
				sceneManager.FixedUpdate();
			}

			sceneManager.Update();
		}
	}

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
	eventBus.Unsubscribe<EditorPlayStateChangeEvent>(m_StateChangedEvent);
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

void EditorState::OnBeginPlay()
{
	m_PlayState = EditorPlayState::Play;
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	sceneManager.GetActiveScene().Awake();

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(EditorPlayStateChangeEvent(m_PlayState));
}

void EditorState::OnStopPlay()
{
	m_PlayState = EditorPlayState::Edit;
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	sceneManager.GetActiveScene().Sleep();

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(EditorPlayStateChangeEvent(m_PlayState));
}