#include "Core/SandboxState.h"

#include <Scene/Scene.h>
#include <Scene/GameObject.h>

#include <Asset/Assets.h>

#include <Renderer/SceneRenderer.h>
#include <Renderer/Framebuffer.h>
#include <Renderer/Camera.h>

#include <Component/CameraComponent.h>
#include <Component/TransformComponent.h>
#include <Component/SpriteRendererComponent.h>
#include <Component/SpriteAnimatorComponent.h>

#include <Core/Application.h>
#include <Core/ServiceLocator.h>

#include <Event/EventBus.h>
#include <Event/WindowEvents.h>

#include "Reflection/BClass.h"
#include <iostream>

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

	GameObject camera = m_pScene->Instantiate({ 0.f, 0.f, 1.f });
	camera.AddComponent<CameraComponent>(Camera(2.f, aspect, 0.1f, 1.f)).Active = true;

	GameObject quad = m_pScene->Instantiate();
	SpriteRendererComponent& sprite = quad.AddComponent<SpriteRendererComponent>();
	sprite.SpriteAtlasHandle = Assets::Get().Load<SpriteAtlasAssetLoader>("game/Blue_witch/B_witch_idle.bsa");
	sprite.Sprite = 0;
	SpriteAnimatorComponent& animator = quad.AddComponent<SpriteAnimatorComponent>();
	animator.Clip = 0;
	animator.Atlas = Assets::GetSpriteAtlas(sprite.SpriteAtlasHandle);
	animator.pRenderer = &sprite;

	m_WindowResizeEvent = ServiceLocator::Get<EventBus>().Subscribe<WindowResizeEvent>([this](const WindowResizeEvent& e)
		{
			m_pRenderer->SetViewport(e.Width, e.Height);
		});
}

void Sandbox::SandboxState::OnUpdate()
{
	m_pScene->Update();
	m_pScene->LateUpdate();
	m_pScene->EndUpdate();

	OnRender();
}

void Sandbox::SandboxState::OnExit()
{
	ServiceLocator::Get<EventBus>().Unsubscribe<WindowResizeEvent>(m_WindowResizeEvent);
}

void Sandbox::SandboxState::OnRender()
{
	m_pRenderer->Render();
}
