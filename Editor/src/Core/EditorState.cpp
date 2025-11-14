#include "Core/EditorState.h"
#include "Core/EditorCamera.h"

#include "Panels/ScenePanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/NetworkPanel.h"

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

#include <Networking/NetIdentity.h>
#include <Networking/NetDriver.h>
#include <Networking/NetScene.h>
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

	m_NetworkSettings.NetMode = Application::Get().GetDescriptor().netDriverMode;

	m_PRenderer = std::make_unique<EditorRenderer>();

	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	Scene& scene = sceneManager.CreateScene("Game");

	ViewportPanel& viewport = CreatePanel<ViewportPanel>("Viewport", &m_SceneContext, &m_SelectionContext);
	viewport.GetToolbar()->BindOnPlayCallback(std::bind(&EditorState::OnBeginPlay, this));
	viewport.GetToolbar()->BindOnStopCallback(std::bind(&EditorState::OnStopPlay, this));

	CreatePanel<PropertiesPanel>("properties", &m_SelectionContext);
	CreatePanel<ScenePanel>("scene", &m_SceneContext, &m_SelectionContext);
	CreatePanel<NetworkPanel>("network", m_NetworkSettings);

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	std::shared_ptr<NetDriver> network = std::make_shared<SteamNetDriver>();
	ServiceLocator::Register<NetDriver>(network);

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

		quad.AddComponent<NetIdentity>();
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
	
	for (auto& [name, pPanel] : m_Panels)
	{
		pPanel->RenderUI();
	}
	
	m_PRenderer->EndFrame();
}

void EditorState::OnBeginPlay()
{
	StartNetwork();

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
	StopNetwork();

	m_PlayState = EditorPlayState::Edit;
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();

	sceneManager.UnloadScene(m_SceneContext.Get()->GetID());
	m_SceneContext.Set(m_pSelectedScene);
	sceneManager.SetActiveScene(m_SceneContext.Get()->GetID(), false);

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(EditorPlayStateChangeEvent(m_PlayState));
}

void EditorState::StartNetwork()
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	NetDriver& network = ServiceLocator::Get<NetDriver>();

	network.Initialize(m_NetworkSettings);
	if (!network.IsStandalone())
	{
		network.BindOnConnectedCallback([this](NetConnection* pConnection) {OnConnected(pConnection); });
		network.BindOnDisconnectedCallback([this](NetConnection* pConnection) {OnDisconnected(pConnection); });
		network.BindOnPacketCallback([this](NetConnection* pConnection, NetPacket& packet) { OnPacketReceived(pConnection, packet); });

		m_BindNetSceneEvent = eventBus.Subscribe<SceneChangedEvent>([](const SceneChangedEvent& e)
			{
				Scene& scene = ServiceLocator::Get<SceneManager>().GetActiveScene();
				auto& network = ServiceLocator::Get<NetDriver>();
				auto pScene{ std::make_shared<NetScene>(&scene, &network) };
				network.BindScene(pScene);
			});

		if (network.IsClient())
		{
			network.Connect(m_NetworkSettings.Ip.c_str(), m_NetworkSettings.Port);
		}

		NetworkPanel& net = GetPanel<NetworkPanel>("network");
		net.SetDriver(&network);
	}
}

void EditorState::StopNetwork()
{
	NetworkPanel& net = GetPanel<NetworkPanel>("network");
	net.SetDriver(nullptr);

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Unsubscribe<SceneChangedEvent>(m_BindNetSceneEvent);
	NetDriver& network = ServiceLocator::Get<NetDriver>();
	network.Shutdown();
}

void EditorState::OnConnected(NetConnection* pConnection)
{
	
}

void EditorState::OnDisconnected(NetConnection* pConnection)
{

}

void EditorState::OnPacketReceived(NetConnection* pConnection, NetPacket& packet)
{

}