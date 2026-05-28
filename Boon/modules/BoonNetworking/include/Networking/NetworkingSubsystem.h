#pragma once
#include <Core/ISubsystem.h>
#include <Networking/NetDriver.h>
#include <Core/Delegate.h>

namespace Boon
{
    class NetConnection;
    class Scene;
    struct EngineContext;

    class NetworkingSubsystem final : public ISubsystem
    {
    public:
        explicit NetworkingSubsystem(NetworkSettings settings)
            : m_Settings(settings) {}

        virtual void OnInit(EngineContext& ctx) override;
        virtual void OnShutdown(EngineContext&) override;
        virtual void Update(EngineContext& ctx) override;

        void StartNetwork(NetworkSettings settings, EngineContext& ctx);
        void StopNetwork();

        NetDriver& GetDriver()
        {
            return *m_Driver;
        }

        NetworkSettings& GetSettings() { return m_Settings; }
        const NetworkSettings& GetSettings() const { return m_Settings; }

    private:
        void OnConnected(Boon::NetConnection* pConnection);
        void OnDisconnected(Boon::NetConnection* pConnection);
        void OnPacketReceived(Boon::NetConnection* pConnection, Boon::NetPacket& packet);

        std::unique_ptr<NetDriver> m_Driver = nullptr;

        Boon::Delegate<void(Boon::Scene&)>::Handle m_BindNetSceneHandle;
        EngineContext* m_pEngineContext = nullptr;
        NetworkSettings m_Settings;
    };
}