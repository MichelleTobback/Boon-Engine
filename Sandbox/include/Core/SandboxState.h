#pragma once
#include <Core/AppState.h>

#include <Event/Event.h>

#include <Networking/NetworkSettings.h>
#include <Networking/NetPacket.h>

#include <memory>

namespace Boon
{
	class SceneRenderer;

	class Camera;
	class TransformComponent;
	class NetConnection;
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
		void StartNetwork();
		void StopNetwork();
		void OnConnected(NetConnection* pConnection);
		void OnDisconnected(NetConnection* pConnection);
		void OnPacketReceived(NetConnection* pConnection, NetPacket& packet);

		void OnRender();
		std::unique_ptr<SceneRenderer> m_pRenderer;

		Camera* m_pCamera;
		TransformComponent* m_pCameraTransform;

		EventListenerID m_WindowResizeEvent;
		EventListenerID m_SceneChangedEvent;
		EventListenerID m_BindNetSceneEvent;

		Boon::NetworkSettings m_NetworkSettings;
	};
}