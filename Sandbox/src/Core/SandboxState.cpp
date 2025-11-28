#include "Core/SandboxState.h"

#include <Scene/GameObject.h>
#include <Scene/SceneManager.h>
#include <Scene/SceneSerializer.h>

#include <Asset/Assets.h>

#include <Renderer/SceneRenderer.h>
#include <Renderer/Framebuffer.h>
#include <Renderer/Camera.h>

#include <Component/CameraComponent.h>
#include <Component/TransformComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/SpriteAnimatorComponent.h>
#include <Component/BoxCollider2D.h>

#include <Core/Application.h>
#include <Core/ServiceLocator.h>
#include <Core/Time.h>

#include <Input/Input.h>

#include <Event/EventBus.h>
#include <Event/WindowEvents.h>
#include <Event/SceneEvents.h>

#include <Platform/Steam/SteamNetDriver.h>
#include <Networking/NetScene.h>
#include <Networking/Events/NetConnectionEvent.h>

#include "Reflection/BClass.h"
#include <iostream>

#include "Game/PlayerController.h"
#include "Game/PlayerSpawn.h"

#include <Asset/Importer/SceneImporter.h>
#include <Asset/Importer/TilemapImporter.h>
#include <Asset/Importer/SpriteAtlasImporter.h>
#include <Asset/Importer/TextureImporter.h>
#include <Asset/Importer/ShaderImporter.h>

using namespace Boon;

Sandbox::SandboxState::SandboxState()
{
	
}

Sandbox::SandboxState::~SandboxState()
{
}

Scene& CreateScene(const std::string& name, float playPosX, float playerPosY)
{
	Window& window{ Application::Get().GetWindow() };
	float aspect{ (float)window.GetWidth() / (float)window.GetHeight() };

	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	Scene& scene = sceneManager.CreateScene(name);

	GameObject camera = scene.Instantiate({ 0.f, 0.f, 1.f });
	camera.AddComponent<CameraComponent>(Camera(8.f, aspect, 0.1f, 1.f)).Active = true;

	GameObject player = scene.Instantiate({ playPosX, playerPosY, 0.f });
	SpriteRendererComponent& sprite = player.AddComponent<SpriteRendererComponent>();
	sprite.SpriteAtlasHandle = Assets::Get().Import<SpriteAtlasAsset>("game/Blue_witch/B_witch_atlas_compact.bsa")->GetHandle();
	sprite.Sprite = 0;
	SpriteAnimatorComponent& animator = player.AddComponent<SpriteAnimatorComponent>();
	animator.Clip = 1;
	animator.Atlas = Assets::GetSpriteAtlas(sprite.SpriteAtlasHandle).Instance();
	animator.pRenderer = player;
	player.AddComponent<PlayerController>();
	BoxCollider2D& col = player.AddComponent<BoxCollider2D>();
	col.Size = { 0.8f, 1.f };

	sceneManager.SetActiveScene(scene.GetID());

	return scene;
}

void Sandbox::SandboxState::OnEnter()
{
	Window& window{ Application::Get().GetWindow() };

	m_NetworkSettings.NetMode = Application::Get().GetDescriptor().netDriverMode;
	std::shared_ptr<NetDriver> network = std::make_shared<SteamNetDriver>();
	ServiceLocator::Register<NetDriver>(network);
	StartNetwork();

	AssetImporterRegistry& importer = AssetImporterRegistry::Get();
	importer.RegisterImporter<Texture2DImporter>();
	importer.RegisterImporter<ShaderImporter>();
	importer.RegisterImporter<SpriteAtlasImporter>();
	importer.RegisterImporter<SceneImporter>();
	importer.RegisterImporter<TilemapImporter>();

	AssetLibrary& assets = Assets::Get();
	assets.Import<TilemapAsset>("game/Tilemap.btm");
	assets.Import<SpriteAtlasAsset>("game/Blue_witch/B_witch_atlas_compact.bsa");

	m_pRenderer = std::make_unique<SceneRenderer>(nullptr, window.GetWidth(), window.GetHeight(), true);

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	m_WindowResizeEvent = eventBus.Subscribe<WindowResizeEvent>([this](const WindowResizeEvent& e)
		{
			m_pRenderer->SetViewport(e.Width, e.Height);
		});

	m_SceneChangedEvent = eventBus.Subscribe<SceneChangedEvent>([this](const SceneChangedEvent& e)
		{
			SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
			Scene& scene = sceneManager.GetActiveScene();
			m_pRenderer->SetContext(&scene);
		});

	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	Scene& scene = sceneManager.CreateScene("scene");
	SceneSerializer serializer(scene);
	serializer.Deserialize("Assets/scenes/Test.scene");
	m_pRenderer->SetContext(&scene);

	GameObject spawnerObj = scene.Instantiate();
	spawnerObj.AddComponent<PlayerSpawn>();
	spawnerObj.AddComponent<NetIdentity>();
	sceneManager.SetActiveScene(scene.GetID(), true);

	//m_pRenderer->SetContext(&CreateScene("Scene1", 3.f, 0.f));
}

void Sandbox::SandboxState::OnUpdate()
{
	ServiceLocator::Get<NetDriver>().Update();

	Time& time = Boon::Time::Get();
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();

	while (time.FixedStep())
	{
		sceneManager.FixedUpdate();
	}

	sceneManager.Update();
	
	Input& input{ ServiceLocator::Get<Input>() };
	if (input.IsKeyPressed(Key::Q))
	{
		SceneManager& sceneManager{ ServiceLocator::Get<SceneManager>() };
		auto scenes = sceneManager.GetLoadedScenes();
		for (int i = 0; i < scenes.size(); ++i)
		{
			if (scenes[i]->GetID() == sceneManager.GetActiveScene().GetID())
			{
				if (i < scenes.size() - 1)
					sceneManager.SetActiveScene(scenes[i + 1]->GetID());
				else
					sceneManager.SetActiveScene(scenes[0]->GetID());
				break;
			}
		}
	}

	OnRender();
}

void Sandbox::SandboxState::OnExit()
{
	StopNetwork();

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Unsubscribe<WindowResizeEvent>(m_WindowResizeEvent);
	eventBus.Unsubscribe<SceneChangedEvent>(m_SceneChangedEvent);
}

void Sandbox::SandboxState::StartNetwork()
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	NetDriver& network = ServiceLocator::Get<NetDriver>();

	network.Initialize(m_NetworkSettings);
	if (!network.IsStandalone())
	{
		network.BindOnConnectedCallback([this](NetConnection* pConnection) {OnConnected(pConnection); });
		network.BindOnDisconnectedCallback([this](NetConnection* pConnection) {OnDisconnected(pConnection); });
		network.BindOnPacketCallback([this](NetConnection* pConnection, NetPacket& packet) { OnPacketReceived(pConnection, packet); });

		ServiceLocator::Get<SceneManager>().BindOnSceneChanged([](Scene& scene)
			{
				auto& network = ServiceLocator::Get<NetDriver>();
				auto pScene{ std::make_shared<NetScene>(&scene, &network) };
				network.BindScene(pScene);
			});

		if (network.IsClient())
		{
			network.Connect(m_NetworkSettings.Ip.c_str(), m_NetworkSettings.Port);
		}
	}
}

void Sandbox::SandboxState::StopNetwork()
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Unsubscribe<SceneChangedEvent>(m_BindNetSceneEvent);
	NetDriver& network = ServiceLocator::Get<NetDriver>();
	network.Shutdown();
}

void Sandbox::SandboxState::OnConnected(NetConnection* pConnection)
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(Boon::NetConnectionEvent(pConnection->GetId(), Boon::ENetConnectionState::Connected));

	std::cout << "Connected\n";
}

void Sandbox::SandboxState::OnDisconnected(NetConnection* pConnection)
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(Boon::NetConnectionEvent(pConnection->GetId(), Boon::ENetConnectionState::Disconnected));

	std::cout << "Disconnected\n";
}

void Sandbox::SandboxState::OnPacketReceived(NetConnection* pConnection, NetPacket& packet)
{

}

void Sandbox::SandboxState::OnRender()
{
	m_pRenderer->Render();
}
