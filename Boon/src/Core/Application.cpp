#include "Core/Application.h"
#include "Core/AppStateMachine.h"
#include "Core/Time.h"
#include "Core/ServiceLocator.h"
#include <Core/SubsystemRegistry.h>

#include "Asset/AssetLibrary.h"

#include "Event/EventBus.h"

#include "Input/Input.h"

#include "Renderer/Renderer.h"

#include "Scene/SceneManager.h"

#include "Module/StaticModules.h"

#include "BoonDebug/Logger.h"

#include <Reflection/BClassBase.h>
#include <Reflection/BClass.h>
#include <Networking/NetRepRegistry.h>

Boon::Application* Boon::Application::s_pInstance{ nullptr };

Boon::Application::Application(const RuntimeConfig& desc)
	: m_Desc{desc}
	, m_pStateMachine{std::make_unique<AppStateMachine>()}
{
	s_pInstance = this;
}

Boon::Application::~Application()
{
	s_pInstance = nullptr;
}

void Boon::Application::Run(std::shared_ptr<AppState>&& pState)
{
	m_pInput = std::make_unique<Input>();
	m_pEventBus = std::make_unique<EventBus>();
	m_pTime = std::make_unique<Time>();
	m_pSubsystems = std::make_unique<SubsystemRegistry>();

	Window::WindowDesc windowDesc{};
	windowDesc.name = m_Desc.Window.Title;
	windowDesc.width = m_Desc.Window.Width;
	windowDesc.height = m_Desc.Window.Height;
	windowDesc.pEventBus = m_pEventBus.get();
	windowDesc.pInput = m_pInput.get();
	windowDesc.icon = m_Desc.EngineRoot / "Assets/Resources/BoonEngine.png";

	uint32_t windowFlags = (uint32_t)windowDesc.flags;
	if (m_Desc.Render.bVSync)
		windowFlags |= (uint32_t)Window::WinConfigFlag::Vsync;
	if (m_Desc.Window.bBorderless)
		windowFlags |= (uint32_t)Window::WinConfigFlag::Borderless;
	if (m_Desc.Window.bFullscreen)
		windowFlags |= (uint32_t)Window::WinConfigFlag::FullScreen;
	if (m_Desc.Window.bResizable)
		windowFlags |= (uint32_t)Window::WinConfigFlag::Resizable;

	windowDesc.flags = (Window::WinConfigFlag)windowFlags;
	m_pWindow = std::make_unique<Window>(windowDesc);
	Renderer::Init();

	m_pServiceRegistry = std::make_unique<ServiceRegistry>();
	m_pClsRegistry = std::make_unique<BClassRegistry>();
	m_pNetRepRegistry = std::make_unique<NetRepRegistry>();

	Boon::ServiceLocator::SetRegistry(m_pServiceRegistry.get());
	Boon::BClassRegistry::SetRegistry(m_pClsRegistry.get());
	Boon::NetRepRegistry::SetRegistry(m_pNetRepRegistry.get());

	BOON_REGISTER_FN(BClassRegistry::Get(), NetRepRegistry::Get());

	m_pAssets = std::make_unique<AssetLibrary>(m_Desc.AssetsRoot);
	m_pAssets->LoadManifest("AssetManifest.json");
	m_pScenes = std::make_unique<SceneManager>(&m_Context);

	m_Context.AssetLib = m_pAssets.get();
	m_Context.Input = m_pInput.get();
	m_Context.Scenes = m_pScenes.get();
	m_Context.EventBus = m_pEventBus.get();
	m_Context.Window = m_pWindow.get();
	m_Context.Time = m_pTime.get();
	m_Context.ProjectConfig = &m_Desc;
	m_Context.Subsystems = m_pSubsystems.get();

	BOON_INIT_LOGGER();

	ModuleContext moduleContext{};
	moduleContext.BClasses = m_pClsRegistry.get();
	moduleContext.NetReps = m_pNetRepRegistry.get();
	moduleContext.ServiceRegistry = m_pServiceRegistry.get();
	moduleContext.EngineContext = &m_Context;
	RegisterStaticModules(moduleContext);

	m_pSubsystems->InitAll(m_Context);

	m_pStateMachine->PushState(std::move(pState), m_Context);
	pState = nullptr;

	StartStaticModules(moduleContext);

	bool quit{ false };
	Time& time{ *m_pTime };
	time.Start();
	while (!quit)
	{
		time.Step();
		quit = m_pWindow->Update();
		m_pSubsystems->UpdateAll(m_Context);
		m_pStateMachine->Update();
		m_pInput->Update();
		m_pWindow->Present();
		time.Wait();
		m_pStateMachine->EndUpdate(m_Context);
	}
	StopStaticModules(moduleContext);

	m_pSubsystems->ShutdownAll(m_Context);
	m_pScenes->Shutdown();
	m_pStateMachine->Shutdown();

	UnregisterStaticModules(moduleContext);

	ServiceLocator::Shutdown();
	Renderer::Shutdown();
	m_pWindow->Destroy();
}
