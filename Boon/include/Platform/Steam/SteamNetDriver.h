#pragma once

#include "Networking/NetDriver.h"
#include "Networking/NetPacket.h"
#include "Networking/NetConnection.h"
#include "Networking/NetworkSettings.h"

#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <unordered_map>
#include <functional>

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
        bool Initialize(const NetworkSettings& settings) override;
        void Shutdown() override;

        void Update() override;

        void Send(NetConnection* conn, NetPacket& pkt, bool reliable = true) override;
        void Broadcast(NetPacket& pkt, bool reliable = true) override;
        virtual void SendToServer(NetPacket& pkt, bool reliable = true) override;

        virtual void BindOnStartupCallback(const NetDriverCallback& fn) override { m_OnStartup = fn; }
        virtual void BindOnShutdownCallback(const NetDriverCallback& fn) override { m_OnShutdown = fn; }
        virtual void BindOnPacketCallback(const PacketCallback& fn) override { m_OnPacket = fn; }
        virtual void BindOnConnectedCallback(const ConnectionCallback& fn) override { m_OnConnected = fn; }
        virtual void BindOnDisconnectedCallback(const ConnectionCallback& fn) override { m_OnDisconnected = fn; }

        void BindScene(const std::shared_ptr<NetScene>& scene) override { m_Scene = scene; }

        NetConnection* GetConnection(uint64_t id) override;
        virtual void ForeachConnection(const std::function<void(NetConnection*)>& fn) override;
        virtual uint32_t GetConnectionCount() const override;

        inline virtual bool IsStandalone() const override { return m_Settings.NetMode == ENetDriverMode::Standalone; }
        inline virtual bool IsClient() const override { return m_Settings.NetMode == ENetDriverMode::Client; }
        inline virtual bool IsServer() const override { return m_Settings.NetMode == ENetDriverMode::DedicatedServer || m_Settings.NetMode == ENetDriverMode::ListenServer; }

        ENetDriverMode GetMode() const override { return m_Settings.NetMode; }
        uint64_t GetLocalConnectionId() const override { return IsServer() ? 1 : m_LocalConnectionId; }

        const NetworkSettings& GetSettings() const { return m_Settings; }

        // Client connect
        virtual bool Connect(const char* host, uint16_t port) override;

        // ---------------------------------------------------------
        // Callback Dispatchers (Static entrypoint → instance)
        // ---------------------------------------------------------
        static void StaticOnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);
        void HandleConnectionStatusChanged(const SteamNetConnectionStatusChangedCallback_t* info);

    private:
        NetworkSettings m_Settings{};

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
        std::shared_ptr<NetScene> m_Scene = nullptr;

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
        NetDriverCallback m_OnStartup;
        NetDriverCallback m_OnShutdown;

        // ---------------------------------------------------------
        // Helpers
        // ---------------------------------------------------------
        void ProcessIncomingMessages();
        void AcceptConnection(HSteamNetConnection hConn);
        void OnConnected(HSteamNetConnection hConn);
        void OnDisconnected(HSteamNetConnection hConn);

        void Send(HSteamNetConnection hConn, NetPacket& pkt, bool reliable = true);
        void UpdateConnections();
    };
}
