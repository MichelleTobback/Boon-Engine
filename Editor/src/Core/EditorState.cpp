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
#include <Scene/SceneSerializer.h>

#include <Component/NameComponent.h>
#include <Component/CameraComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/SpriteAnimatorComponent.h>
#include <Component/BoxCollider2D.h>
#include <Component/Rigidbody2D.h>

#include <Networking/NetDriver.h>
#include <Platform/Steam/SteamNetDriver.h>

#include "Game/PlayerController.h"

#include <Reflection/BClass.h>

#include <iostream>

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

	std::shared_ptr<NetDriver> network = std::make_shared<SteamNetDriver>();
	ServiceLocator::Register<NetDriver>(network);
	network->Initialize(Application::Get().GetDescriptor().netDriverMode);

	if (network->GetMode() == ENetDriverMode::Client)
	{
		network->Connect("127.0.0.1", 27020);

		network->BindOnConnectedCallback([](NetConnection* pConnection)
			{
				NetPacket packet{ ENetPacketType::Ping };
				packet.WriteString("hello server");
				pConnection->GetDriver()->Send(pConnection, packet);
			});

		network->BindOnPacketCallback([](NetConnection* pConnection, NetPacket& packet)
			{
				if (packet.GetType() == ENetPacketType::Pong)
				{
					std::string result = packet.ReadString();
					std::cout << result << '\n';
				}
			});
	}
	else
	{
		network->BindOnPacketCallback([](NetConnection* pConnection, NetPacket& packet)
			{
				if (packet.GetType() == ENetPacketType::Ping)
				{
					std::string result = packet.ReadString();
					std::cout << result << '\n';

					NetPacket reply{ ENetPacketType::Pong };
					reply.WriteString("hello client");
					pConnection->GetDriver()->Send(pConnection, reply);
				}

				if (packet.GetType() == ENetPacketType::Pong)
				{
					std::string result = packet.ReadString();
					std::cout << result << '\n';
				}
			});

		network->BindOnConnectedCallback([](NetConnection* pConnection)
			{
				NetPacket packet{ ENetPacketType::Pong };
				packet.WriteString("this is a broadcast message");
				pConnection->GetDriver()->Broadcast(packet);
			});
	}

	ViewportPanel& viewport = CreatePanel<ViewportPanel>("Viewport", &m_SceneContext, &m_SelectionContext);
	viewport.GetToolbar()->BindOnPlayCallback(std::bind(&EditorState::OnBeginPlay, this));
	viewport.GetToolbar()->BindOnStopCallback(std::bind(&EditorState::OnStopPlay, this));

	CreatePanel<PropertiesPanel>(&m_SelectionContext);
	CreatePanel<ScenePanel>(&m_SceneContext, &m_SelectionContext);

	GameObject camera = scene.Instantiate({ 0.f, 0.f, 1.f });
	camera.AddComponent<CameraComponent>(Camera(4.f, 2.f, 0.1f, 1.f), false).Active = true;
	camera.GetComponent<NameComponent>().Name = "Camera";
	m_SelectionContext.Set(camera);
	m_SceneContext.Set(&scene);
	m_pSelectedScene = &scene;

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
		quad.GetComponent<NameComponent>().Name = "Player";
	}

	//floor
	{
		GameObject floor = scene.Instantiate({ 0.f, -1.f, 0.f });
	
		BoxCollider2D& floorCol = floor.AddComponent<BoxCollider2D>();
		floorCol.Size = { 10.f, 0.2f };
	
		Rigidbody2D& floorRb = floor.AddComponent<Rigidbody2D>();
		floorRb.Type = Boon::Rigidbody2D::BodyType::Static;

		floor.GetComponent<NameComponent>().Name = "Floor";
	}

	//trigger
	{
		GameObject trigger = scene.Instantiate({ -1.f, 0.f, 0.f });

		BoxCollider2D& floorCol = trigger.AddComponent<BoxCollider2D>();
		floorCol.Size = { 1.f, 1.f };
		floorCol.IsTrigger = true;

		Rigidbody2D& floorRb = trigger.AddComponent<Rigidbody2D>();
		floorRb.Type = Boon::Rigidbody2D::BodyType::Static;

		trigger.GetComponent<NameComponent>().Name = "Trigger";
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

	sceneManager.SetActiveScene(scene.GetID(), false);

}

void EditorState::OnUpdate()
{
	ServiceLocator::Get<NetDriver>().Update();

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
		}
	}

	sceneManager.Update();

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

	m_SceneContext.Set(&sceneManager.CreateScene("PlayScene"));
	SceneSerializer serializer(*m_SceneContext.Get());
	serializer.Copy(*m_pSelectedScene);
	sceneManager.SetActiveScene(m_SceneContext.Get()->GetID());

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(EditorPlayStateChangeEvent(m_PlayState));
}

void EditorState::OnStopPlay()
{
	m_PlayState = EditorPlayState::Edit;
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();

	sceneManager.UnloadScene(m_SceneContext.Get()->GetID());
	m_SceneContext.Set(m_pSelectedScene);
	sceneManager.SetActiveScene(m_SceneContext.Get()->GetID(), false);

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(EditorPlayStateChangeEvent(m_PlayState));
}