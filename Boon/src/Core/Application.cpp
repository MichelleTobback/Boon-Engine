#include "Core/Application.h"
#include "Core/AppStateMachine.h"
#include "Core/Time.h"
#include "Renderer/Renderer.h"

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
	m_pStateMachine->PushState(std::move(pState));
	pState = nullptr;

	bool quit{ false };
	Time& time{ Time::Get() };
	time.Start();
	while (!quit)
	{
		time.Step();
		quit = m_pWindow->Update();
		m_pStateMachine->Update();
		time.Wait();
		m_pWindow->Present();
	}
	m_pStateMachine->Shutdown();
	Renderer::Shutdown();
	m_pWindow->Destroy();
}
