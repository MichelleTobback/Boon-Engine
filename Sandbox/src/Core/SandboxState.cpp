#include "Core/SandboxState.h"

#include <Scene/GameObject.h>
#include <Scene/SceneManager.h>

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
#include <Event/SceneEvents.h>

#include "Reflection/BClass.h"
#include <iostream>

#include "Game/PlayerController.h"

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

	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	Scene& scene = sceneManager.CreateScene("Game");
	sceneManager.SetActiveScene(scene.GetID());
	m_pRenderer = std::make_unique<SceneRenderer>(&scene, window.GetWidth(), window.GetHeight(), true);

	GameObject camera = scene.Instantiate({ 0.f, 0.f, 1.f });
	camera.AddComponent<CameraComponent>(Camera(2.f, aspect, 0.1f, 1.f)).Active = true;

	GameObject quad = scene.Instantiate();
	SpriteRendererComponent& sprite = quad.AddComponent<SpriteRendererComponent>();
	sprite.SpriteAtlasHandle = Assets::Get().Load<SpriteAtlasAssetLoader>("game/Blue_witch/B_witch_idle.bsa");
	sprite.Sprite = 0;
	SpriteAnimatorComponent& animator = quad.AddComponent<SpriteAnimatorComponent>();
	animator.Clip = 0;
	animator.Atlas = Assets::GetSpriteAtlas(sprite.SpriteAtlasHandle);
	animator.pRenderer = &sprite;
	quad.AddComponent<PlayerController>();

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
}

void Sandbox::SandboxState::OnUpdate()
{
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	sceneManager.Update();
	sceneManager.LateUpdate();

	OnRender();
}

void Sandbox::SandboxState::OnExit()
{
	EventBus& eventBus = ServiceLocator::Get<EventBus>();
	eventBus.Unsubscribe<WindowResizeEvent>(m_WindowResizeEvent);
	eventBus.Unsubscribe<SceneChangedEvent>(m_SceneChangedEvent);
}

void Sandbox::SandboxState::OnRender()
{
	m_pRenderer->Render();
}
