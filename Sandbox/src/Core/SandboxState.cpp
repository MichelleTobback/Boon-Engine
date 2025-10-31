#include "Core/SandboxState.h"

#include <Scene/Scene.h>
#include <Scene/GameObject.h>
#include <Renderer/SceneRenderer.h>
#include <Renderer/Framebuffer.h>

#include <Renderer/Camera.h>
#include <Component/TransformComponent.h>
#include <Component/SpriteRendererComponent.h>

#include <Core/Application.h>
#include <Core/ServiceLocator.h>

#include <Event/EventBus.h>
#include <Event/WindowEvents.h>

using namespace Boon;

Sandbox::SandboxState::SandboxState()
{
	
}

Sandbox::SandboxState::~SandboxState()
{
}

void Sandbox::SandboxState::OnEnter()
{
	Window& window{ Application::Get().GetWindow() };
	float aspect{ (float)window.GetWidth() / (float)window.GetHeight() };

	m_pScene = std::make_unique<Scene>();
	m_pRenderer = std::make_unique<SceneRenderer>(m_pScene.get(), window.GetWidth(), window.GetHeight(), true);

	m_pCamera = std::make_unique<Camera>(3.f * aspect, 3.f);
	m_pCameraTransform = std::make_unique<TransformComponent>(nullptr);
	m_pCameraTransform->SetLocalPosition(0.f, 0.f, 1.f);

	GameObject quad = m_pScene->Instantiate();
	SpriteRendererComponent& sprite = quad.AddComponent<SpriteRendererComponent>();
	sprite.Color = { 0.9f, 0.1f, 0.3f, 1.f };

	m_WindowResizeEvent = ServiceLocator::Get<EventBus>().Subscribe<WindowResizeEvent>([this](const WindowResizeEvent& e)
		{
			m_pRenderer->SetViewport(e.Width, e.Height);
		});
}

void Sandbox::SandboxState::OnUpdate()
{
	m_pScene->Update();
	m_pScene->EndUpdate();

	OnRender();
}

void Sandbox::SandboxState::OnExit()
{
	ServiceLocator::Get<EventBus>().Unsubscribe<WindowResizeEvent>(m_WindowResizeEvent);
}

void Sandbox::SandboxState::OnRender()
{
	m_pRenderer->Render(m_pCamera.get(), m_pCameraTransform.get());
}
