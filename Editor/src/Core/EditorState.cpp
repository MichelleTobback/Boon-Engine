#include "Core/EditorState.h"
#include "Core/EditorCamera.h"

#include "Panels/ScenePanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/NetworkPanel.h"
#include "Panels/ContentBrowser.h"
#include "Panels/TilemapEditorPanel.h"
#include "Panels/SpriteAtlasEditorPanel.h"
#include "Panels/AssetEditorPanel.h"

#include "UI/EditorRenderer.h"

#include <Asset/Assets.h>
#include <Asset/AssetRef.h>
#include <Asset/Importer/TextureImporter.h>
#include <Asset/Importer/ShaderImporter.h>
#include <Asset/Importer/SpriteAtlasImporter.h>
#include <Asset/Importer/SceneImporter.h>
#include <Asset/Importer/TilemapImporter.h>

#include "Assets/AssetDirectoryScanner.h"

#include <Core/Application.h>
#include <Core/ServiceLocator.h>
#include <Core/Time.h>

#include <Event/EventBus.h>
#include <Event/SceneEvents.h>
#include "Event/EditorEvent.h"

#include <Renderer/Renderer.h>

#include <Scene/GameObject.h>
#include <Scene/SceneSerializer.h>

#include <Component/NameComponent.h>
#include <Component/CameraComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/SpriteAnimatorComponent.h>
#include <Component/BoxCollider2D.h>
#include <Component/Rigidbody2D.h>

#include <Networking/Components/NetRigidbody2D.h>
#include <Networking/Components/NetTransform.h>
#include <Networking/Events/NetConnectionEvent.h>
#include <Networking/NetIdentity.h>
#include <Networking/NetDriver.h>
#include <Networking/NetScene.h>
#include <Platform/Steam/SteamNetDriver.h>

#include <Reflection/BClass.h>

#include <BoonDebug/DebugRenderer.h>

#include <iostream>
#include <imgui.h>

using namespace BoonEditor;

EditorState::EditorState() = default;
EditorState::~EditorState() = default;

void EditorState::OnEnter()
{
	Window& window{ Application::Get().GetWindow() };
	AssetLibrary& assetLib{ Assets::Get() };
	AssetImporterRegistry& importer = AssetImporterRegistry::Get();
	importer.RegisterImporter<Texture2DImporter>();
	importer.RegisterImporter<ShaderImporter>();
	importer.RegisterImporter<SpriteAtlasImporter>();
	importer.RegisterImporter<SceneImporter>();
	importer.RegisterImporter<TilemapImporter>();

	m_NetworkSettings.NetMode = Application::Get().GetDescriptor().netDriverMode;

	m_PRenderer = std::make_unique<EditorRenderer>();

	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	Scene& scene = sceneManager.CreateScene("Game");

	ViewportPanel& viewport = CreatePanel<ViewportPanel>("Viewport", &m_DragDrop, &m_SceneContext, &m_SelectionContext);
	viewport.GetToolbar()->BindOnPlayCallback(std::bind(&EditorState::OnBeginPlay, this));
	viewport.GetToolbar()->BindOnStopCallback(std::bind(&EditorState::OnStopPlay, this));

	AssetEditorPanel& assetEditor = CreatePanel<AssetEditorPanel>("asset", &m_DragDrop, &viewport);
	assetEditor.RegisterEditor(new TilemapEditorPanel("tilemap", &m_DragDrop));
	assetEditor.RegisterEditor(new SpriteAtlasEditorPanel("sprite atlas", &m_DragDrop));
	m_pSelectedAsset = &assetEditor.GetContext();

	CreatePanel<ContentBrowser>("content", &m_DragDrop, m_pSelectedAsset);
	CreatePanel<PropertiesPanel>("properties", &m_DragDrop, &m_SelectionContext);
	CreatePanel<ScenePanel>("scene", &m_DragDrop,  &m_SceneContext, &m_SelectionContext);
	CreatePanel<NetworkPanel>("network", &m_DragDrop, m_NetworkSettings);

	CreateObject<AssetDirectoryScanner>("Assets/", 1.f);

	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	std::shared_ptr<NetDriver> network = std::make_shared<SteamNetDriver>();
	ServiceLocator::Register<NetDriver>(network);

	m_SceneContext.Set(&scene);
	m_pSelectedScene = &scene;
	
	AssetRef<SpriteAtlasAsset> atlas = assetLib.Import<SpriteAtlasAsset>("game/Blue_witch/B_witch_atlas_compact.bsa");
	assetLib.Import<TilemapAsset>("game/Arena/Arena-tilemap.btm");
	assetLib.Import<SpriteAtlasAsset>("game/Witch/Witch-combined.bsa");

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

	SceneSerializer serializer(scene);
	//serializer.Serialize("Assets/scenes/Test.scene");
	serializer.Clear();
	serializer.Deserialize("Assets/scenes/Test.scene");

	sceneManager.SetActiveScene(scene.GetID(), false);

	m_SelectionContext.AddOnContextChangedCallback([this](GameObject obj)
		{
			if (obj.GetScene() == m_pSelectedScene)
				GetPanel<ViewportPanel>("Viewport").SetContext(&m_SceneContext);
		});
}

void EditorState::OnUpdate()
{
	Boon::DebugRenderer::Get().BeginFrame();

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

	ServiceLocator::Get<NetDriver>().Shutdown();
}

void EditorState::OnRender()
{
	m_PRenderer->BeginFrame();

	m_DragDrop.Clear();

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New scene"))
		{
			if (m_PlayState != EditorPlayState::Play)
			{
				SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
				m_SceneContext.Set(&sceneManager.CreateScene("New Scene"));
				m_pSelectedScene = m_SceneContext.Get();

				sceneManager.SetActiveScene(m_SceneContext.Get()->GetID());
			}
		}
		if (ImGui::MenuItem("Save scene"))
		{
			if (m_PlayState != EditorPlayState::Play)
			{
				SceneSerializer serializer(*m_pSelectedScene);
				serializer.Serialize("Assets/scenes/Test.scene");
			}
		}
		if (ImGui::MenuItem("Open scene"))
		{
			if (m_PlayState != EditorPlayState::Play)
			{
				SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
				m_SceneContext.Set(&sceneManager.CreateScene("New Scene"));
				m_pSelectedScene = m_SceneContext.Get();

				SceneSerializer serializer(*m_SceneContext.Get());
				serializer.Deserialize("Assets/scenes/Test.scene");

				sceneManager.SetActiveScene(m_SceneContext.Get()->GetID(), false);
			}
		}
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
	
	for (auto& [name, pPanel] : m_Panels)
	{
		pPanel->RenderUI();
	}
	
	m_DragDrop.Process();
	m_PRenderer->EndFrame();
}

void EditorState::OnBeginPlay()
{
	StartNetwork();

	m_PlayState = EditorPlayState::Play;
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();

	GetPanel<ViewportPanel>("Viewport").SetContext(&m_SceneContext);

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

	SceneID sceneId = m_SceneContext.Get()->GetID();
	m_SceneContext.Set(m_pSelectedScene);
	sceneManager.SetActiveScene(m_SceneContext.Get()->GetID(), false);
	sceneManager.UnloadScene(sceneId);

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

		m_BindNetSceneHandle = ServiceLocator::Get<SceneManager>().BindOnSceneChanged([](Scene& e)
			{
				Scene& scene = ServiceLocator::Get<SceneManager>().GetActiveScene();
				auto& network = ServiceLocator::Get<NetDriver>();
				auto pScene{ std::make_shared<NetScene>(&scene, &network) };
				network.BindScene(pScene);
			});

		//m_BindNetSceneEvent = eventBus.Subscribe<SceneChangedEvent>([](Scene& e)
		//	{
		//		Scene& scene = ServiceLocator::Get<SceneManager>().GetActiveScene();
		//		auto& network = ServiceLocator::Get<NetDriver>();
		//		auto pScene{ std::make_shared<NetScene>(&scene, &network) };
		//		network.BindScene(pScene);
		//	});

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

	ServiceLocator::Get<SceneManager>().UnbindOnSceneChanged(m_BindNetSceneHandle);
	
	NetDriver& network = ServiceLocator::Get<NetDriver>();
	network.Shutdown();
}

void EditorState::OnConnected(NetConnection* pConnection)
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(Boon::NetConnectionEvent(pConnection->GetId(), Boon::ENetConnectionState::Connected));
}

void EditorState::OnDisconnected(NetConnection* pConnection)
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Post(Boon::NetConnectionEvent(pConnection->GetId(), Boon::ENetConnectionState::Disconnected));
}

void EditorState::OnPacketReceived(NetConnection* pConnection, NetPacket& packet)
{

}