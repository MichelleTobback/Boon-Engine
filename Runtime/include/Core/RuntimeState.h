#pragma once
#include <Core/AppState.h>
#include <Core/EngineContext.h>
#include <Core/Delegate.h>
#include <Event/Event.h>
#include <Scene/Scene.h>

#include <Networking/NetworkSettings.h>
#include <Networking/NetPacket.h>

#include <memory>

namespace Boon
{
	class SceneRenderer;
	class NetConnection;
	class ModuleLibrary;
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

	protected:
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

		Delegate<void(Scene&)>::Handle m_BindNetSceneHandle;

		Boon::NetworkSettings m_NetworkSettings;
		std::unique_ptr<ModuleLibrary> m_pModuleLib;
	};
}