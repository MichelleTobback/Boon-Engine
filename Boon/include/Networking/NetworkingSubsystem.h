#pragma once
#include <Core/ISubsystem.h>
#include <Platform/Steam/SteamNetDriver.h>

namespace Boon
{
    class NetworkingSubsystem final : public ISubsystem
    {
    public:
        explicit NetworkingSubsystem(NetworkSettings settings)
            : m_Settings(settings) {}

        void OnInit(EngineContext& ctx) override
        {
            m_Driver = std::make_unique<SteamNetDriver>();
        }

        void OnShutdown(EngineContext&) override
        {
            StopNetwork();

            m_Driver.reset();
        }

        void StartNetwork(NetworkSettings settings, EngineContext& ctx)
        {
            m_Settings = settings;
            m_Driver->Initialize(settings, ctx.EventBus);
        }

        void StopNetwork()
        {
            if (m_Driver)
                m_Driver->Shutdown();
        }

        void Update()
        {
            if (m_Driver)
                m_Driver->Update();
        }

        NetDriver& GetDriver()
        {
            return *m_Driver;
        }

    private:
        NetworkSettings m_Settings;
        std::unique_ptr<NetDriver> m_Driver;
    };
}