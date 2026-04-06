#pragma once
#include <Core/AppState.h>

#include <Event/Event.h>

#include <Networking/NetworkSettings.h>
#include <Networking/NetPacket.h>

#include <memory>

namespace Boon
{
	class SceneRenderer;
	class NetConnection;
}

using namespace Boon;

namespace Runtime
{
	class RuntimeState final : public AppState
	{
	public:
		RuntimeState();
		~RuntimeState();

		RuntimeState(const RuntimeState& other) = default;
		RuntimeState(RuntimeState&& other) = default;
		RuntimeState& operator=(const RuntimeState& other) = default;
		RuntimeState& operator=(RuntimeState&& other) = default;

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

		EventListenerID m_WindowResizeEvent;
		EventListenerID m_SceneChangedEvent;
		EventListenerID m_BindNetSceneEvent;

		Boon::NetworkSettings m_NetworkSettings;
	};
}