#include "Core/Application.h"
#include "Core/AppStateMachine.h"
#include "Core/Time.h"
#include "Core/ServiceLocator.h"

#include "Asset/AssetLibrary.h"

#include "Event/EventBus.h"

#include "Input/Input.h"

#include "Renderer/Renderer.h"

#include "Scene/SceneManager.h"

#include "BoonDebug/Logger.h"

#include <Reflection/BClassBase.h>
#include <Reflection/BClass.h>
#include <Networking/NetRepRegistry.h>

Boon::Application* Boon::Application::s_pInstance{ nullptr };

Boon::Application::Application(const AppDesc& desc)
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
	m_pWindow = std::make_unique<Window>(m_Desc.windowDesc);
	Renderer::Init();

	m_pServiceRegistry = std::make_unique<ServiceRegistry>();
	m_pClsRegistry = std::make_unique<BClassRegistry>();
	m_pNetRepRegistry = std::make_unique<NetRepRegistry>();

	Boon::ServiceLocator::SetRegistry(m_pServiceRegistry.get());
	Boon::BClassRegistry::SetRegistry(m_pClsRegistry.get());
	Boon::NetRepRegistry::SetRegistry(m_pNetRepRegistry.get());

	BOON_REGISTER_FN(BClassRegistry::Get(), NetRepRegistry::Get());

	ServiceLocator::Register(std::make_shared<AssetImporterRegistry>());
	ServiceLocator::Register(std::make_shared<AssetLibrary>("Assets/"));
	ServiceLocator::Register(std::make_shared<EventBus>());
	ServiceLocator::Register(std::make_shared<Input>());
	ServiceLocator::Register(std::make_shared<SceneManager>());

	BOON_INIT_LOGGER();

	m_pStateMachine->PushState(std::move(pState));
	pState = nullptr;

	bool quit{ false };
	Time& time{ Time::Get() };
	Input& input{ ServiceLocator::Get<Input>() };
	time.Start();
	while (!quit)
	{
		time.Step();
		quit = m_pWindow->Update();
		m_pStateMachine->Update();
		input.Update();
		m_pWindow->Present();
		time.Wait();
	}
	ServiceLocator::Get<SceneManager>().Shutdown();
	m_pStateMachine->Shutdown();
	ServiceLocator::Shutdown();
	Renderer::Shutdown();
	m_pWindow->Destroy();
}
