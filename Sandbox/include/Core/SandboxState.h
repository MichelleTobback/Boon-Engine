#pragma once
#include <Core/AppState.h>
#include <Event/Event.h>

#include <memory>

namespace Boon
{
	class Scene;
	class SceneRenderer;

	class Camera;
	class TransformComponent;
}

using namespace Boon;

namespace Sandbox
{
	class SandboxState final : public AppState
	{
	public:
		SandboxState();
		~SandboxState();

		SandboxState(const SandboxState& other) = default;
		SandboxState(SandboxState&& other) = default;
		SandboxState& operator=(const SandboxState& other) = default;
		SandboxState& operator=(SandboxState&& other) = default;

		virtual void OnEnter() override;
		virtual void OnUpdate() override;
		virtual void OnExit() override;

	private:
		void OnRender();

		std::unique_ptr<Scene> m_pScene;
		std::unique_ptr<SceneRenderer> m_pRenderer;

		std::unique_ptr<Camera> m_pCamera;
		std::unique_ptr<TransformComponent> m_pCameraTransform;

		EventListenerID m_WindowResizeEvent;
	};
}