#include "Core/RuntimeState.h"

#include <Scene/SceneManager.h>
#include <Scene/SceneSerializer.h>

#include <Asset/Assets.h>

#include <Renderer/SceneRenderer.h>

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
#include <Networking/NetRepRegistry.h>

#include <Module/ModuleLibrary.h>

#include "Reflection/BClass.h"
#include <iostream>

#include <Asset/Importer/SceneImporter.h>
#include <Asset/Importer/TilemapImporter.h>
#include <Asset/Importer/SpriteAtlasImporter.h>
#include <Asset/Importer/TextureImporter.h>
#include <Asset/Importer/ShaderImporter.h>

using namespace Boon;

Runtime::RuntimeState::RuntimeState()
{
	
}

Runtime::RuntimeState::~RuntimeState()
{
}

void Runtime::RuntimeState::OnEnter()
{
	const RuntimeConfig& config{ Application::Get().GetDescriptor() };
	Window& window{ Application::Get().GetWindow() };

	m_NetworkSettings = config.Network;
	std::shared_ptr<NetDriver> network = std::make_shared<SteamNetDriver>();
	ServiceLocator::Register<NetDriver>(network);
	StartNetwork();

	AssetImporterRegistry& importer = ServiceLocator::Get<AssetImporterRegistry>();
	importer.RegisterImporter<Texture2DImporter>();
	importer.RegisterImporter<ShaderImporter>();
	importer.RegisterImporter<SpriteAtlasImporter>();
	importer.RegisterImporter<SceneImporter>();
	importer.RegisterImporter<TilemapImporter>();

	AssetLibrary& assets = Assets::Get();
	assets.AddRoot(config.EngineRoot / config.AssetsRoot); // engine assets
	assets.Import<TilemapAsset>("game/Arena/Arena-tilemap.btm");
	assets.Import<SpriteAtlasAsset>("game/Witch/Witch-combined.bsa");

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

	std::shared_ptr<ModuleLibrary> moduleLib = std::make_shared<ModuleLibrary>();
	ServiceLocator::Register<ModuleLibrary>(moduleLib);
	ModuleContext ctx{};
	ctx.BClasses = &BClassRegistry::Get();
	ctx.NetReps = &NetRepRegistry::Get();
	ctx.ServiceRegistry = ServiceLocator::GetRegistry();
	moduleLib->LoadModule(config.ProjectRoot / config.IntermediateRoot / config.GameModule / (config.GameModule + ".dll"), ctx);

	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	Scene& scene = sceneManager.CreateScene("scene");
	SceneSerializer serializer(scene);
	serializer.Deserialize(config.AssetsRoot / config.StartupScene);
	m_pRenderer->SetContext(&scene);

	sceneManager.SetActiveScene(scene.GetID(), true);
}

void Runtime::RuntimeState::OnUpdate()
{
	ServiceLocator::Get<NetDriver>().Update();

	Time& time = Boon::Time::Get();
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();

	while (time.FixedStep())
	{
		sceneManager.FixedUpdate();
	}

	sceneManager.Update();

	OnRender();
}

void Runtime::RuntimeState::OnExit()
{
	StopNetwork();

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Unsubscribe<WindowResizeEvent>(m_WindowResizeEvent);
	eventBus.Unsubscribe<SceneChangedEvent>(m_SceneChangedEvent);

	ModuleContext ctx{};
	ctx.BClasses = &BClassRegistry::Get();
	ctx.NetReps = &NetRepRegistry::Get();
	ctx.ServiceRegistry = ServiceLocator::GetRegistry();
	ServiceLocator::Get<ModuleLibrary>().UnloadAll(ctx);
}

void Runtime::RuntimeState::StartNetwork()
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

void Runtime::RuntimeState::StopNetwork()
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Unsubscribe<SceneChangedEvent>(m_BindNetSceneEvent);
	NetDriver& network = ServiceLocator::Get<NetDriver>();
	network.Shutdown();
}

void Runtime::RuntimeState::OnConnected(NetConnection* pConnection)
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(Boon::NetConnectionEvent(pConnection->GetId(), Boon::ENetConnectionState::Connected));
}

void Runtime::RuntimeState::OnDisconnected(NetConnection* pConnection)
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(Boon::NetConnectionEvent(pConnection->GetId(), Boon::ENetConnectionState::Disconnected));
}

void Runtime::RuntimeState::OnPacketReceived(NetConnection* pConnection, NetPacket& packet)
{

}

void Runtime::RuntimeState::OnRender()
{
	m_pRenderer->Render();
}
