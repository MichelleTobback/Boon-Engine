#pragma once

#include "Networking/NetDriver.h"
#include "Networking/NetPacket.h"
#include "Networking/NetConnection.h"

#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <unordered_map>
#include <functional>
#include <memory>

namespace Boon
{
    class NetScene;

    class SteamNetDriver : public NetDriver
    {
    public:
        SteamNetDriver();
        ~SteamNetDriver();

        // ---------------------------------------------------------
        // Interface Implementation
        // ---------------------------------------------------------
        bool Initialize(ENetDriverMode mode) override;
        void Shutdown() override;

        void Update() override;

        void Send(NetConnection* conn, NetPacket& pkt, bool reliable = true) override;
        void Broadcast(NetPacket& pkt, bool reliable = true) override;

        void BindOnPacketCallback(const PacketCallback& fn) override { m_OnPacket = fn; }
        void BindOnConnectedCallback(const ConnectionCallback& fn) override { m_OnConnected = fn; }
        void BindOnDisconnectedCallback(const ConnectionCallback& fn) override { m_OnDisconnected = fn; }

        bool IsServer() const { return m_Mode == ENetDriverMode::DedicatedServer || m_Mode == ENetDriverMode::ListenServer; }
        bool IsClient() const { return m_Mode == ENetDriverMode::Client; }
        bool IsListenServer() const { return m_Mode == ENetDriverMode::ListenServer; }

        void BindScene(NetScene* scene) override { m_Scene = scene; }

        NetConnection* GetConnection(uint64_t id) override;

        ENetDriverMode GetMode() const override { return m_Mode; }
        uint64_t GetLocalConnectionId() const override { return m_LocalConnectionId; }

        // Client connect
        virtual bool Connect(const char* host, uint16_t port) override;

        // ---------------------------------------------------------
        // Callback Dispatchers (Static entrypoint → instance)
        // ---------------------------------------------------------
        static void StaticOnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);
        void HandleConnectionStatusChanged(const SteamNetConnectionStatusChangedCallback_t* info);

    private:
        // ---------------------------------------------------------
        // Steam Networking Handles
        // ---------------------------------------------------------
        ISteamNetworkingSockets* m_Interface = nullptr;
        HSteamListenSocket m_ListenSocket = k_HSteamListenSocket_Invalid;
        HSteamNetPollGroup m_PollGroup = k_HSteamNetPollGroup_Invalid;
        static SteamNetDriver* s_Instance;

        // ---------------------------------------------------------
        // Driver state
        // ---------------------------------------------------------
        ENetDriverMode m_Mode = ENetDriverMode::Standalone;
        NetScene* m_Scene = nullptr;

        uint64_t m_LocalConnectionId = 0;

        // ---------------------------------------------------------
        // Connections
        // ---------------------------------------------------------
        struct DriverConnection
        {
            HSteamNetConnection hConn = k_HSteamNetConnection_Invalid;
            std::unique_ptr<NetConnection> conn;
        };

        std::unordered_map<uint64_t, DriverConnection> m_Connections;
        std::unordered_map<HSteamNetConnection, uint64_t> m_ReverseLookup;

        // ---------------------------------------------------------
        // Callbacks
        // ---------------------------------------------------------
        PacketCallback m_OnPacket;
        ConnectionCallback m_OnConnected;
        ConnectionCallback m_OnDisconnected;

        // ---------------------------------------------------------
        // Helpers
        // ---------------------------------------------------------
        void ProcessIncomingMessages();
        void AcceptConnection(HSteamNetConnection hConn);
        void OnConnected(HSteamNetConnection hConn);
        void OnDisconnected(HSteamNetConnection hConn);

        void UpdateConnections();
    };
}
