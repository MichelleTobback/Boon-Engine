#pragma once
#include "NetAuthority.h"
#include <cstdint>

namespace Boon
{
    class NetDriver;

    enum class ENetConnectionState : uint8_t
    {
        Connecting = 0,
        Connected,
        Disconnected
    };

    class NetConnection
    {
    public:
        NetConnection(uint64_t id, NetDriver* driver)
            : m_Id(id), m_Driver(driver)
        {}

        void Update();

        uint64_t GetId() const { return m_Id; }
        NetDriver* GetDriver() const { return m_Driver; }

        ENetConnectionState GetState() const { return m_State; }
        void SetState(ENetConnectionState state) { m_State = state; }

        // Transport-specific data (e.g. SteamNetworkingSockets)
        void SetTransportHandle(uint64_t handle)
        {
            m_TransportHandle = handle;
        }

        uint64_t GetTransportHandle() const
        {
            return m_TransportHandle;
        }

        // Future: ping, player data, replication filters
        float GetPing() const { return m_Ping; }
        void SetPing(float ping) { m_Ping = ping; }

    private:
        uint64_t m_Id;           // Engine-level connection ID
        uint64_t m_TransportHandle = 0; // transport-specific handle (Steam)
        NetDriver* m_Driver;

        float m_Ping = 0.0f;

        ENetConnectionState m_State = ENetConnectionState::Connecting;
    };
}