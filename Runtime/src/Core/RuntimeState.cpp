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

#include <Networking/NetworkingSubsystem.h>
#include <Networking/NetScene.h>
#include <Networking/Events/NetConnectionEvent.h>
#include <Networking/NetRepRegistry.h>

#include <Module/ModuleLibrary.h>

#include "Reflection/BClass.h"
#include <iostream>

using namespace Boon;

Runtime::RuntimeState::RuntimeState()
{
	
}

Runtime::RuntimeState::~RuntimeState()
{
}

void Runtime::RuntimeState::OnEnter()
{
	EngineContext& ctx = GetContext();

	const RuntimeConfig& config{ Application::Get().GetDescriptor() };
	Window& window{ *ctx.Window };

	m_NetworkSettings = config.Network;
	ctx.Subsystems->Register<NetworkingSubsystem>(m_NetworkSettings);
	StartNetwork();

	AssetLibrary& assetLib = *ctx.AssetLib;
	const std::filesystem::path assetRoot = config.ProjectRoot / "generated/Assets";
	const std::filesystem::path gameRuntimeRoot = assetRoot / "Game";
	const std::filesystem::path engineRuntimeRoot = assetRoot / "Engine";

	assetLib.SetRuntimeAssetRoot(gameRuntimeRoot);
	assetLib.AddRuntimeAssetRoot(engineRuntimeRoot);

	assetLib.LoadManifest(gameRuntimeRoot / "AssetManifest.json");
	assetLib.LoadManifest(engineRuntimeRoot / "AssetManifest.json");

	SceneRendererCreateInfo desc{};
	desc.Width = window.GetWidth();
	desc.Height = window.GetHeight();
	desc.bIsSwapchainTarget = true;
	desc.AssetLib = ctx.AssetLib;
	m_pRenderer = std::make_unique<SceneRenderer>(desc);

	EventBus& eventBus = *ctx.EventBus;
	SceneManager& sceneManager = *ctx.Scenes;
	m_WindowResizeEvent = eventBus.Subscribe<WindowResizeEvent>([this](const WindowResizeEvent& e)
		{
			m_pRenderer->SetViewport(e.Width, e.Height);
		});

	m_SceneChangedEvent = eventBus.Subscribe<SceneChangedEvent>([&sceneManager, this](const SceneChangedEvent& e)
		{
			Scene& scene = sceneManager.GetActiveScene();
			m_pRenderer->SetContext(&scene);
		});

	ctx.Subsystems->InitAll(ctx);

	m_pModuleLib = std::make_unique<ModuleLibrary>();
	ModuleContext module{};
	module.BClasses = &BClassRegistry::Get();
	module.NetReps = &NetRepRegistry::Get();
	module.ServiceRegistry = ServiceLocator::GetRegistry();
	module.EngineContext = &ctx;
	m_pModuleLib->LoadModule(config.ProjectRoot / config.IntermediateRoot / config.GameModule / (config.GameModule + ".dll"), module);

	Scene& scene = sceneManager.CreateScene("scene");
	SceneSerializer serializer(scene);
	serializer.Deserialize(config.AssetsRoot / config.StartupScene);
	m_pRenderer->SetContext(&scene);

	sceneManager.SetActiveScene(scene.GetID(), true);
}

void Runtime::RuntimeState::OnUpdate()
{
	EngineContext& ctx = GetContext();

	ctx.GetSubsystem<NetworkingSubsystem>().Update();

	Time& time = *ctx.Time;
	SceneManager& sceneManager = *ctx.Scenes;

	while (time.FixedStep())
	{
		sceneManager.FixedUpdate();
	}

	sceneManager.Update();

	OnRender();
}

void Runtime::RuntimeState::OnExit()
{
	EngineContext& ctx = GetContext();

	EventBus& eventBus = *ctx.EventBus;
	eventBus.Unsubscribe<WindowResizeEvent>(m_WindowResizeEvent);
	eventBus.Unsubscribe<SceneChangedEvent>(m_SceneChangedEvent);

	StopNetwork();

	SceneManager& sceneManager = *ctx.Scenes;
	sceneManager.UnloadAll();

	ModuleContext module{};
	module.BClasses = &BClassRegistry::Get();
	module.NetReps = &NetRepRegistry::Get();
	module.ServiceRegistry = ServiceLocator::GetRegistry();
	module.EngineContext = &ctx;
	m_pModuleLib->UnloadAll(module);
	m_pModuleLib = nullptr;

	AssetLibrary& assetLib = *ctx.AssetLib;
	assetLib.ClearCache();
	assetLib.ClearRegistry();
}

void Runtime::RuntimeState::StartNetwork()
{
	EngineContext& ctx = GetContext();
	NetworkingSubsystem& network = ctx.GetSubsystem<NetworkingSubsystem>();
	NetDriver& driver = network.GetDriver();

	network.StartNetwork(m_NetworkSettings, ctx);
	if (!driver.IsStandalone())
	{
		driver.BindOnConnectedCallback([this](NetConnection* pConnection) {OnConnected(pConnection); });
		driver.BindOnDisconnectedCallback([this](NetConnection* pConnection) {OnDisconnected(pConnection); });
		driver.BindOnPacketCallback([this](NetConnection* pConnection, NetPacket& packet) { OnPacketReceived(pConnection, packet); });

		m_BindNetSceneHandle = ctx.Scenes->BindOnSceneChanged([&ctx, this](Scene& e)
			{
				Scene& scene = ctx.Scenes->GetActiveScene();
				auto& driver = ctx.GetSubsystem<NetworkingSubsystem>().GetDriver();
				auto pScene{ std::make_shared<NetScene>(&scene, &driver , ctx.Scenes) };
				driver.BindScene(pScene);
			});

		if (driver.IsClient())
		{
			driver.Connect(m_NetworkSettings.Ip.c_str(), m_NetworkSettings.Port);
		}
	}
}

void Runtime::RuntimeState::StopNetwork()
{
	EngineContext& ctx = GetContext();

	if (m_BindNetSceneHandle.IsValid())
		GetContext().Scenes->UnbindOnSceneChanged(m_BindNetSceneHandle);

	ctx.GetSubsystem<NetworkingSubsystem>().StopNetwork();
}

void Runtime::RuntimeState::OnConnected(NetConnection* pConnection)
{
	GetContext().EventBus->Post(Boon::NetConnectionEvent(pConnection->GetId(), Boon::ENetConnectionState::Connected));
}

void Runtime::RuntimeState::OnDisconnected(NetConnection* pConnection)
{
	GetContext().EventBus->Post(Boon::NetConnectionEvent(pConnection->GetId(), Boon::ENetConnectionState::Disconnected));
}

void Runtime::RuntimeState::OnPacketReceived(NetConnection* pConnection, NetPacket& packet)
{

}

void Runtime::RuntimeState::OnRender()
{
	m_pRenderer->Render();
}
