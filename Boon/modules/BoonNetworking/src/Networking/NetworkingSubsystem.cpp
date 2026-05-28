#include "Networking/NetworkingSubsystem.h"
#include "Core/EngineContext.h"
#include "Scene/SceneManager.h"
#include "Event/EventBus.h"
#include "Networking/Events/NetConnectionEvent.h"
#include "Networking/NetScene.h"

using namespace Boon;

void NetworkingSubsystem::OnInit(EngineContext& ctx)
{
    m_Driver = std::move(NetDriver::Create());
    m_pEngineContext = &ctx;
}

void NetworkingSubsystem::OnShutdown(EngineContext&)
{
    StopNetwork();

    m_Driver.reset();

    m_pEngineContext = nullptr;
}

void NetworkingSubsystem::StartNetwork(NetworkSettings settings, EngineContext& ctx)
{
    m_Settings = settings;
    m_Driver->Initialize(settings, ctx.EventBus);

	if (!m_Driver->IsStandalone())
	{
		m_Driver->BindOnConnectedCallback([this](NetConnection* pConnection) {OnConnected(pConnection); });
		m_Driver->BindOnDisconnectedCallback([this](NetConnection* pConnection) {OnDisconnected(pConnection); });
		m_Driver->BindOnPacketCallback([this](NetConnection* pConnection, NetPacket& packet) { OnPacketReceived(pConnection, packet); });

		m_BindNetSceneHandle = ctx.Scenes->BindOnSceneChanged([&ctx, this](Scene& e)
			{
				Scene& scene = ctx.Scenes->GetActiveScene();
				auto& driver = ctx.GetSubsystem<NetworkingSubsystem>().GetDriver();
				auto pScene{ std::make_shared<NetScene>(&scene, &driver , ctx.Scenes) };
				driver.BindScene(pScene);
			});

		if (m_Driver->IsClient())
		{
			m_Driver->Connect(m_Settings.Ip.c_str(), m_Settings.Port);
		}
	}
}

void NetworkingSubsystem::StopNetwork()
{
    if (m_BindNetSceneHandle.IsValid())
        m_pEngineContext->Scenes->UnbindOnSceneChanged(m_BindNetSceneHandle);

    if (m_Driver)
        m_Driver->Shutdown();
}

void NetworkingSubsystem::Update(EngineContext& ctx)
{
    if (m_Driver)
        m_Driver->Update();
}

void NetworkingSubsystem::OnConnected(NetConnection* pConnection)
{
	m_pEngineContext->EventBus->Post(Boon::NetConnectionEvent(pConnection->GetId(), Boon::ENetConnectionState::Connected));
}

void NetworkingSubsystem::OnDisconnected(NetConnection* pConnection)
{
	m_pEngineContext->EventBus->Post(Boon::NetConnectionEvent(pConnection->GetId(), Boon::ENetConnectionState::Disconnected));
}

void NetworkingSubsystem::OnPacketReceived(NetConnection* pConnection, NetPacket& packet)
{

}