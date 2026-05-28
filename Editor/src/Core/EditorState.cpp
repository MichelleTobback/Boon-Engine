#pragma once
#include "Core/EditorState.h"
#include "Core/EditorCamera.h"
#include "Core/AppStateMachine.h"

#include "Panels/ScenePanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/NetworkPanel.h"
#include "Panels/ContentBrowser.h"
#include "Panels/TilemapEditorPanel.h"
#include "Panels/SpriteAtlasEditorPanel.h"
#include "Panels/MaterialEditorPanel.h"
#include "Panels/AssetEditorPanel.h"
#include "Panels/ConsolePanel.h"

#include <BoonDebug/Logger.h>

#include "Project/ProjectLoader.h"
#include "Tools/NewProjectDialog.h"
#include "Tools/PackageBuildDialog.h"

#include "UI/EditorRenderer.h"

#include <Asset/Assets.h>
#include <Asset/AssetRef.h>
#include <Assets/Importer/TextureImporter.h>
#include <Assets/Importer/ShaderImporter.h>
#include <Assets/Importer/SpriteAtlasImporter.h>
#include <Assets/Importer/SceneImporter.h>
#include <Assets/Importer/TilemapImporter.h>
#include <Assets/Importer/MaterialImporter.h>

#include "Assets/AssetDirectoryScanner.h"

#include <Core/Application.h>
#include <Core/ServiceLocator.h>
#include <Core/Time.h>
#include <Core/FileSystem.h>

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

#include <Networking/NetRepRegistry.h>
#include <Networking/NetworkingSubsystem.h>

#include <Module/ModuleLibrary.h>
#include <Module/ModuleManifest.h>

#include <Reflection/BClass.h>

#include <BoonDebug/DebugRenderer.h>

#include <Command/EditorCommandQueue.h>

#include <Input/Input.h>

#include <iostream>
#include <imgui.h>

using namespace BoonEditor;

BoonEditor::EditorState::EditorState(const ProjectConfig& project)
	: m_Context{}
{
	m_Context.m_CurrentProject = project;
}

EditorState::~EditorState() = default;

void EditorState::OnEnter()
{
	EngineContext& ctx = GetContext();

	const RuntimeConfig& config{ m_Context.m_CurrentProject.Runtime };
	m_Context.SetEngineContext(&ctx);

	Window& window{ *ctx.Window };
	AssetLibrary& assetLib{ *ctx.AssetLib };
	
	const std::filesystem::path generatedRoot = config.ProjectRoot / "generated/Assets";
	const std::filesystem::path gameRuntimeRoot = generatedRoot / "Game";
	const std::filesystem::path engineRuntimeRoot = generatedRoot / "Engine";
	const std::filesystem::path editorRuntimeRoot = generatedRoot / "Editor";

	assetLib.SetRuntimeAssetRoot(gameRuntimeRoot);
	assetLib.AddRuntimeAssetRoot(engineRuntimeRoot);
	assetLib.AddRuntimeAssetRoot(editorRuntimeRoot);

	assetLib.LoadManifest(gameRuntimeRoot / "AssetManifest.json");
	assetLib.LoadManifest(engineRuntimeRoot / "AssetManifest.json");
	assetLib.LoadManifest(editorRuntimeRoot / "AssetManifest.json");

	if (!ServiceLocator::Has<AssetImporterRegistry>())
		ServiceLocator::Register<AssetImporterRegistry>();

	AssetImporterRegistry& importer = ServiceLocator::Get<AssetImporterRegistry>();
	importer.ClearAssetRoots();
	importer.AddAssetRoot(config.AssetsRoot, gameRuntimeRoot);
	importer.AddAssetRoot(config.EngineContentRoot, engineRuntimeRoot);
	importer.AddAssetRoot(m_Context.m_CurrentProject.Editor.EditorResourcesRoot, editorRuntimeRoot);
	importer.BindToAssetLibrary(assetLib);

	importer.RegisterImporter<Texture2DImporter>();
	importer.RegisterImporter<ShaderImporter>();
	importer.RegisterImporter<SpriteAtlasImporter>();
	importer.RegisterImporter<SceneImporter>();
	importer.RegisterImporter<TilemapImporter>();
	importer.RegisterImporter<MaterialImporter>();

	AssetDatabase::Get().Init(assetLib);

	m_PRenderer = std::make_unique<EditorRenderer>(m_Context.m_CurrentProject, assetLib.Load<Texture2DAsset>("Resources/BoonEngine.png").Instance());
	m_PRenderer->SetMenuBarCallback(std::bind(&EditorState::RenderMenuBar, this));

	SceneManager& sceneManager = *ctx.Scenes;
	Scene& scene = sceneManager.CreateScene("Game");

	m_Context.CreateObject<AssetDirectoryScanner>(0, 1.f);
	m_Context.CreateObject<AssetDirectoryScanner>(1, 1.f);

	ViewportPanel& viewport = m_Context.CreateWidget<ViewportPanel>("Viewport", &m_SceneContext, &m_SelectionContext);
	viewport.GetToolbar()->BindOnPlayCallback(std::bind(&EditorState::OnBeginPlay, this));
	viewport.GetToolbar()->BindOnStopCallback(std::bind(&EditorState::OnStopPlay, this));

	AssetEditorPanel& assetEditor = m_Context.CreateWidget<AssetEditorPanel>("asset", &viewport);
	assetEditor.RegisterEditor(new TilemapEditorPanel(&m_Context, "tilemap"));
	assetEditor.RegisterEditor(new SpriteAtlasEditorPanel(&m_Context, "sprite atlas"));
	assetEditor.RegisterEditor(new MaterialEditorPanel(&m_Context, "material"));
	m_pSelectedAsset = &assetEditor.GetContext();

	m_Context.CreateWidget<ContentBrowser>("content", m_pSelectedAsset);
	m_Context.CreateWidget<PropertiesPanel>("properties", &m_SelectionContext);
	m_Context.CreateWidget<ScenePanel>("scene",  &m_SceneContext, &m_SelectionContext);
	m_Context.CreateWidget<NetworkPanel>("network");
	m_Context.CreateWidget<ConsolePanel>("console");

	m_Context.CreateWidget<NewProjectDialog>("NewProject");
	m_Context.CreateWidget<PackageBuildDialog>("PackageBuild");

	EventBus& eventBus = *ctx.EventBus;

	m_SceneContext.Set(&scene);
	m_pSelectedScene = &scene;

	m_SceneChangedEvent = eventBus.Subscribe<SceneChangedEvent>([&sceneManager, this](const SceneChangedEvent& e)
		{
			Scene& scene = sceneManager.GetActiveScene();
			m_SceneContext.Set(&scene);
			m_SelectionContext.Set(GameObject());
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

	m_SelectionContext.AddOnContextChangedCallback([this](GameObject obj)
		{
			if (obj.GetScene() == m_pSelectedScene)
				m_Context.GetWidget<ViewportPanel>("Viewport").SetContext(&m_SceneContext);
		});

	m_pModuleLib = std::make_unique<ModuleLibrary>(config.ProjectRoot);
	m_pModuleLib->LoadManifest(config.ProjectRoot / "generated" / "ModuleManifest.json");
	ModuleContext module{};
	module.BClasses = &BClassRegistry::Get();
	module.NetReps = &NetRepRegistry::Get();
	module.ServiceRegistry = ServiceLocator::GetRegistry();
	module.EngineContext = &ctx;
	m_pModuleLib->LoadStartupModules(module);

	ctx.Subsystems->InitAll(ctx);

	SceneSerializer serializer(scene);
	serializer.Deserialize(config.AssetsRoot / config.StartupScene);

	sceneManager.SetActiveScene(scene.GetID(), false);

	Application::Get().GetWindow().SetTitle(m_Context.m_CurrentProject.Runtime.Window.Title);
	BOON_LOG("Project loaded {} : {}", m_Context.m_CurrentProject.Name, m_Context.m_CurrentProject.Runtime.ProjectRoot.string());
}

void EditorState::OnUpdate()
{
	EngineContext& ctx = GetContext();
	Boon::DebugRenderer::Get().BeginFrame();

	Time& time = *ctx.Time;
	SceneManager& sceneManager = *ctx.Scenes;

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

	for (auto& pObject : m_Context.m_Objects)
	{
		pObject->Update();
	}

	Input& input = *ctx.Input;
	if (input.IsKeyHeld(Boon::Key::LeftControl)
		&& input.IsKeyPressed(Boon::Key::Z))
	{
		m_Context.GetCommandQueue()->RequestUndo();
	}

	OnRender();

	m_Context.GetCommandQueue()->Execute();
}

void EditorState::OnExit()
{
	EngineContext& ctx = GetContext();

	m_Context.Clear();

	EventBus& eventBus = *ctx.EventBus;
	eventBus.Unsubscribe<SceneChangedEvent>(m_SceneChangedEvent);
	eventBus.Unsubscribe<EditorPlayStateChangeEvent>(m_StateChangedEvent);

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

	AssetDatabase::Get().Clear();
}

bool TitlebarMenuButton(const char* label)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

	// invisible normal state
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

	// soft hover
	ImGui::PushStyleColor(
		ImGuiCol_ButtonHovered,
		ImVec4(1, 1, 1, 0.08f)
	);

	// active/open state
	ImGui::PushStyleColor(
		ImGuiCol_ButtonActive,
		ImVec4(0.58f, 0.36f, 0.78f, 0.22f)
	);

	bool pressed = ImGui::Button(label);

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);

	if (pressed)
		ImGui::OpenPopup(label);

	return ImGui::BeginPopup(label);
}

void EditorState::RenderMenuBar()
{
	EngineContext& ctx = GetContext();
	ImGui::BeginGroup();

	if (TitlebarMenuButton("Project"))
	{
		if (ImGui::MenuItem("New Project"))
		{
			if (m_PlayState == EditorPlayState::Play)
			{
				OnStopPlay();
			}
			NewProjectDialog* pDialog = m_Context.TryGetWidget<NewProjectDialog>("NewProject");
			if (pDialog) pDialog->Open();
		}
		if (ImGui::MenuItem("Open Project"))
		{
			if (m_PlayState == EditorPlayState::Play)
			{
				OnStopPlay();
			}
			FileSystem::OpenDialogOptions options{};
			options.filters = { FileSystem::FileFilter{ L"Boon Project", L"*.bproj" } };
			options.title = L"Open Project";
			FileSystem::Path projPath = FileSystem::OpenFileDialog(options);

			if (!projPath.empty())
			{
				auto projectFile = ProjectLoader::LoadFromFile(projPath);
				Application::Get().GetStateMachine()->RequestStateChange(std::make_shared<EditorState>(projectFile.Value));
			}
		}
		ImGui::EndMenu();
	}

	ImGui::SameLine(0.0f, 4.0f);

	if (TitlebarMenuButton("File"))
	{
		if (ImGui::MenuItem("New scene"))
		{
			if (m_PlayState != EditorPlayState::Play)
			{
				SceneManager& sceneManager = *ctx.Scenes;
				m_SceneContext.Set(&sceneManager.CreateScene("New Scene"));
				m_pSelectedScene = m_SceneContext.Get();

				sceneManager.SetActiveScene(m_SceneContext.Get()->GetID());
			}
		}
		if (ImGui::MenuItem("Save scene"))
		{
			if (m_PlayState != EditorPlayState::Play)
			{
				FileSystem::OpenDialogOptions options{};
				options.filters = { FileSystem::FileFilter{ L"Boon Project", L"*.scene" } };
				options.title = L"Save Scene";
				options.initialPath = m_Context.m_CurrentProject.Runtime.AssetsRoot;
				FileSystem::Path scenPath = FileSystem::OpenFileDialog(options);

				if (!scenPath.empty())
				{
					SceneSerializer serializer(*m_pSelectedScene);
					serializer.Serialize(scenPath);
				}
			}
		}
		if (ImGui::MenuItem("Open scene"))
		{
			if (m_PlayState != EditorPlayState::Play)
			{
				FileSystem::OpenDialogOptions options{};
				options.filters = { FileSystem::FileFilter{ L"Boon Scene", L"*.scene" } };
				options.title = L"Open Scene";
				options.initialPath = m_Context.m_CurrentProject.Runtime.AssetsRoot;
				FileSystem::Path scenPath = FileSystem::OpenFileDialog(options);

				if (!scenPath.empty())
				{
					SceneManager& sceneManager = *ctx.Scenes;
					m_SceneContext.Set(&sceneManager.CreateScene("New Scene"));
					m_pSelectedScene = m_SceneContext.Get();

					SceneSerializer serializer(*m_SceneContext.Get());
					serializer.Deserialize(scenPath);

					sceneManager.SetActiveScene(m_SceneContext.Get()->GetID(), false);
				}
			}
		}
		ImGui::EndMenu();
	}
	ImGui::SameLine(0.0f, 4.0f);
	if (TitlebarMenuButton("Build"))
	{
		if (ImGui::MenuItem("Package Build"))
		{
			if (m_PlayState == EditorPlayState::Play)
			{
				OnStopPlay();
			}
			PackageBuildDialog* pDialog = m_Context.TryGetWidget<PackageBuildDialog>("PackageBuild");
			if (pDialog) pDialog->Open();
		}
		ImGui::EndMenu();
	}
	ImGui::EndGroup();
}

void EditorState::OnRender()
{
	m_PRenderer->BeginFrame();

	m_DragDrop.Clear();
	
	for (auto& [name, pPanel] : m_Context.m_Widgets)
	{
		pPanel->RenderUI();
	}
	
	m_DragDrop.Process();
	m_PRenderer->EndFrame();
}

void EditorState::OnBeginPlay()
{
	EngineContext& ctx = GetContext();

	StartNetwork();

	m_PlayState = EditorPlayState::Play;

	m_Context.GetWidget<ViewportPanel>("Viewport").SetContext(&m_SceneContext);

	m_SceneContext.Set(&ctx.Scenes->CreateScene("PlayScene"));
	SceneSerializer serializer(*m_SceneContext.Get());
	serializer.Copy(*m_pSelectedScene);
	ctx.Scenes->SetActiveScene(m_SceneContext.Get()->GetID());
	
	ctx.EventBus->Post(EditorPlayStateChangeEvent(m_PlayState));
}

void EditorState::OnStopPlay()
{
	EngineContext& ctx = GetContext();

	StopNetwork();

	m_PlayState = EditorPlayState::Edit;

	SceneID sceneId = m_SceneContext.Get()->GetID();
	m_SceneContext.Set(m_pSelectedScene);
	ctx.Scenes->SetActiveScene(m_SceneContext.Get()->GetID(), false);
	ctx.Scenes->UnloadScene(sceneId);

	ctx.EventBus->Post(EditorPlayStateChangeEvent(m_PlayState));
}

void EditorState::StartNetwork()
{
	EngineContext& ctx = GetContext();
	if (NetworkingSubsystem* network = ctx.TryGetSubsystem<NetworkingSubsystem>())
	{
		network->StartNetwork(Application::Get().GetDescriptor().Network, ctx);
	}
}

void EditorState::StopNetwork()
{
	EngineContext& ctx = GetContext();
	if (NetworkingSubsystem* network = ctx.TryGetSubsystem<NetworkingSubsystem>())
	{
		network->StopNetwork();
	}
}