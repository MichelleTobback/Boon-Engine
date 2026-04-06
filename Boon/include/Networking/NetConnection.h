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

        /**
         * @brief Update per-connection runtime state.
         */
        void Update();

        /**
         * @brief Get the engine-level connection identifier.
         */
        uint64_t GetId() const { return m_Id; }

        /**
         * @brief Get the NetDriver that owns this connection.
         */
        NetDriver* GetDriver() const { return m_Driver; }

        /**
         * @brief Get the current connection state.
         */
        ENetConnectionState GetState() const { return m_State; }

        /**
         * @brief Set the current connection state.
         */
        void SetState(ENetConnectionState state) { m_State = state; }

        // Transport-specific data (e.g. SteamNetworkingSockets)
        /**
         * @brief Set a transport-specific handle associated with this connection.
         */
        void SetTransportHandle(uint64_t handle)
        {
            m_TransportHandle = handle;
        }

        /**
         * @brief Get the transport-specific handle.
         */
        uint64_t GetTransportHandle() const
        {
            return m_TransportHandle;
        }

        // Future: ping, player data, replication filters
        /**
         * @brief Get the last measured ping value for the connection.
         */
        float GetPing() const { return m_Ping; }

        /**
         * @brief Set the ping value for the connection.
         */
        void SetPing(float ping) { m_Ping = ping; }

    private:
        uint64_t m_Id;           // Engine-level connection ID
        uint64_t m_TransportHandle = 0; // transport-specific handle (Steam)
        NetDriver* m_Driver;

        float m_Ping = 0.0f;

        ENetConnectionState m_State = ENetConnectionState::Connecting;
    };
}