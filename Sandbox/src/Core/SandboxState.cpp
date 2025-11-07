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
#include <Component/BoxCollider2D.h>

#include <Core/Application.h>
#include <Core/ServiceLocator.h>
#include <Core/Time.h>

#include <Input/Input.h>

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

Scene& CreateScene(const std::string& name, float playPosX, float playerPosY)
{
	Window& window{ Application::Get().GetWindow() };
	float aspect{ (float)window.GetWidth() / (float)window.GetHeight() };

	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
	Scene& scene = sceneManager.CreateScene(name);

	GameObject camera = scene.Instantiate({ 0.f, 0.f, 1.f });
	camera.AddComponent<CameraComponent>(Camera(8.f, aspect, 0.1f, 1.f)).Active = true;

	GameObject player = scene.Instantiate({ playPosX, playerPosY, 0.f });
	SpriteRendererComponent& sprite = player.AddComponent<SpriteRendererComponent>();
	sprite.SpriteAtlasHandle = Assets::Get().Load<SpriteAtlasAssetLoader>("game/Blue_witch/B_witch_atlas_compact.bsa");
	sprite.Sprite = 0;
	SpriteAnimatorComponent& animator = player.AddComponent<SpriteAnimatorComponent>();
	animator.Clip = 1;
	animator.Atlas = Assets::GetSpriteAtlas(sprite.SpriteAtlasHandle);
	animator.pRenderer = &sprite;
	player.AddComponent<PlayerController>();
	BoxCollider2D& col = player.AddComponent<BoxCollider2D>();
	col.Size = { 0.8f, 1.f };

	sceneManager.SetActiveScene(scene.GetID());

	return scene;
}

void Sandbox::SandboxState::OnEnter()
{
	Window& window{ Application::Get().GetWindow() };

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

	m_pRenderer->SetContext(&CreateScene("Scene1", 3.f, 0.f));
	//CreateScene("Scene2", -3.f, 0.f);
}

void Sandbox::SandboxState::OnUpdate()
{
	Time& time = Boon::Time::Get();
	SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();

	while (time.FixedStep())
	{
		sceneManager.FixedUpdate();
	}

	static float timer = 0.0f;
	static int frames = 0;

	frames++;
	timer += time.GetDeltaTime();

	if (timer >= 1.0f)
	{
		std::cout << "FPS: " << frames << '\n';
		frames = 0;
		timer -= 1.0f;
	}

	sceneManager.Update();
	sceneManager.LateUpdate();
	
	Input& input{ ServiceLocator::Get<Input>() };
	if (input.IsKeyPressed(Key::Q))
	{
		SceneManager& sceneManager{ ServiceLocator::Get<SceneManager>() };
		auto scenes = sceneManager.GetLoadedScenes();
		for (int i = 0; i < scenes.size(); ++i)
		{
			if (scenes[i]->GetID() == sceneManager.GetActiveScene().GetID())
			{
				if (i < scenes.size() - 1)
					sceneManager.SetActiveScene(scenes[i + 1]->GetID());
				else
					sceneManager.SetActiveScene(scenes[0]->GetID());
				break;
			}
		}
	}

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
