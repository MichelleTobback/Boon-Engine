#include "Platform/Steam/SteamNetDriver.h"
#include "Networking/NetScene.h"

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingsockets.h>
#include <steam/steamnetworkingtypes.h>

#include <cstring>
#include <iostream>

namespace Boon
{
    SteamNetDriver* SteamNetDriver::s_Instance{nullptr};

    // -------------------------------------------------------------------------
    // Constructor / Destructor
    // -------------------------------------------------------------------------

    SteamNetDriver::SteamNetDriver()
    {
        
    }

    SteamNetDriver::~SteamNetDriver()
    {
        Shutdown();
    }

    // -------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------

    bool SteamNetDriver::Initialize(const NetworkSettings& settings)
    {
        m_Settings = settings;

        SteamDatagramErrMsg err;
        if (!GameNetworkingSockets_Init(nullptr, err))
        {
            std::cerr << "Failed to initialize GameNetworkingSockets: " << err << "\n";
        }

        m_Interface = SteamNetworkingSockets();

        // Create poll group
        m_PollGroup = m_Interface->CreatePollGroup();
        if (m_PollGroup == k_HSteamNetPollGroup_Invalid)
        {
            std::cerr << "Failed to create poll group\n";
            return false;
        }
        ENetDriverMode mode = m_Settings.NetMode;

        // Start server
        if (mode == ENetDriverMode::DedicatedServer || mode == ENetDriverMode::ListenServer)
        {
            m_LocalConnectionId = 1;

            SteamNetworkingIPAddr addr;
            addr.Clear();
            addr.m_port = m_Settings.Port;

            SteamNetworkingConfigValue_t opt;
            opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetDriver::StaticOnConnectionStatusChanged);

            m_ListenSocket = m_Interface->CreateListenSocketIP(addr, 1, &opt);
            if (m_ListenSocket == k_HSteamListenSocket_Invalid)
            {
                std::cerr << "Failed to create listen socket\n";
                return false;
            }

            std::cout << "Server listening on port " << addr.m_port << "\n";
        }

        s_Instance = this;

        if (m_OnStartup) m_OnStartup(this);

        return true;
    }

    // -------------------------------------------------------------------------
    void SteamNetDriver::Shutdown()
    {
        if (!m_Interface)
            return;

        if (m_OnShutdown) m_OnShutdown(this);

        for (auto& [id, dc] : m_Connections)
        {
            m_Interface->CloseConnection(dc.hConn, 0, "Shutdown", false);
        }
        m_Connections.clear();
        m_ReverseLookup.clear();

        if (m_ListenSocket != k_HSteamListenSocket_Invalid)
        {
            m_Interface->CloseListenSocket(m_ListenSocket);
            m_ListenSocket = k_HSteamListenSocket_Invalid;
        }

        if (m_PollGroup != k_HSteamNetPollGroup_Invalid)
        {
            m_Interface->DestroyPollGroup(m_PollGroup);
            m_PollGroup = k_HSteamNetPollGroup_Invalid;
        }

        GameNetworkingSockets_Kill();

        m_LocalConnectionId = 0;
        s_Instance = nullptr;
    }

    // -------------------------------------------------------------------------
    void SteamNetDriver::Update()
    {
        if (!s_Instance)
            return;

        m_Interface->RunCallbacks();
        ProcessIncomingMessages();
        UpdateConnections();

        if (m_Scene)
            m_Scene->Update();
    }

    // -------------------------------------------------------------------------
    // Client connect
    // -------------------------------------------------------------------------

    bool SteamNetDriver::Connect(const char* host, uint16_t port)
    {
        SteamNetworkingIPAddr addr;
        std::string ip = std::string(host);
        if (ip == "localhost") ip = "127.0.0.1";
        addr.ParseString((ip + ":" + std::to_string(port)).c_str());

        SteamNetworkingConfigValue_t opt;
        opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetDriver::StaticOnConnectionStatusChanged);

        HSteamNetConnection hConn =
            m_Interface->ConnectByIPAddress(addr, 1, &opt);

        if (hConn == k_HSteamNetConnection_Invalid)
        {
            std::cerr << "Failed to connect to server\n";
            return false;
        }

        std::cout << "Connecting to server at port" << m_Settings.Port << " ...\n";
        return true;
    }

    // -------------------------------------------------------------------------
    // Message Processing
    // -------------------------------------------------------------------------

    void SteamNetDriver::ProcessIncomingMessages()
    {
        ISteamNetworkingMessage* msgs[32];

        while (true)
        {
            int msgCount = m_Interface->ReceiveMessagesOnPollGroup(m_PollGroup, msgs, 32);
            if (msgCount == 0)
                break;

            if (msgCount < 0)
            {
                std::cerr << "Error in ReceiveMessagesOnPollGroup\n";
                break;
            }

            for (int i = 0; i < msgCount; i++)
            {
                ISteamNetworkingMessage* msg = msgs[i];
                HSteamNetConnection hConn = msg->m_conn;

                // Wrap in NetPacket
                NetPacket pkt((uint8_t*)msg->m_pData, msg->m_cbSize);

                if (pkt.GetType() == ENetPacketType::AssignID)
                {
                    uint64_t connId = pkt.Read<uint64_t>();

                    DriverConnection dc;
                    dc.hConn = hConn;
                    dc.conn = std::make_unique<NetConnection>(connId, this);
                    dc.conn->SetState(ENetConnectionState::Connected);

                    m_LocalConnectionId = connId;

                    m_Connections[connId] = std::move(dc);
                    m_ReverseLookup[hConn] = connId;

                    std::cout << "Client " << connId << " Connected to server\n";

                    if (m_OnConnected)
                        m_OnConnected(m_Connections[connId].conn.get());
                }
                else
                {
                    if (!m_ReverseLookup.count(hConn))
                    {
                        std::cerr << "Message from unknown connection!\n";
                        msg->Release();
                        continue;
                    }

                    uint64_t connId = m_ReverseLookup[hConn];
                    NetConnection* conn = m_Connections[connId].conn.get();

                    if (m_Scene)
                        m_Scene->ProcessPacket(conn, pkt);

                    if (m_OnPacket)
                        m_OnPacket(conn, pkt);
                }

                msg->Release();
            }
        }
    }

    // -------------------------------------------------------------------------
    // Accept new client connection
    // -------------------------------------------------------------------------

    void SteamNetDriver::AcceptConnection(HSteamNetConnection hConn)
    {
        m_Interface->AcceptConnection(hConn);
        m_Interface->SetConnectionPollGroup(hConn, m_PollGroup);
    }

    void SteamNetDriver::OnConnected(HSteamNetConnection hConn)
    {
        // Ensure messages go through the poll group (clients need this!)
        m_Interface->SetConnectionPollGroup(hConn, m_PollGroup);

        if (!IsServer())
            return;

        uint64_t connId = ++m_LocalConnectionId;

        DriverConnection dc;
        dc.hConn = hConn;
        dc.conn = std::make_unique<NetConnection>(connId, this);
        dc.conn->SetState(ENetConnectionState::Connected);

        m_Connections[connId] = std::move(dc);
        m_ReverseLookup[hConn] = connId;

        std::cout << "Client connected: " << connId << "\n";

        NetPacket assignIdPkt{ ENetPacketType::AssignID };
        assignIdPkt.Write(connId);
        Send(hConn, assignIdPkt);

        if (m_OnConnected)
            m_OnConnected(m_Connections[connId].conn.get());
    }

    void SteamNetDriver::OnDisconnected(HSteamNetConnection hConn)
    {
        if (m_ReverseLookup.count(hConn))
        {
            uint64_t id = m_ReverseLookup[hConn];
            m_Connections[id].conn->SetState(ENetConnectionState::Disconnected);

            if (m_OnDisconnected)
                m_OnDisconnected(m_Connections[id].conn.get());

            m_Connections.erase(id);
            m_ReverseLookup.erase(hConn);

            if (IsServer())
                std::cout << "Client " << id << " disconnected" << "\n";
        }
        if (IsClient())
            std::cout << "Disconnected from server " << "\n";

        m_Interface->CloseConnection(hConn, 0, nullptr, false);
    }

    void SteamNetDriver::Send(HSteamNetConnection hConn, NetPacket& pkt, bool reliable)
    {
        m_Interface->SendMessageToConnection(
            hConn,
            pkt.RawData(),
            pkt.RawSize(),
            reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable,
            nullptr
        );
    }

    // -------------------------------------------------------------------------
    // Static → Instance callback dispatch
    // -------------------------------------------------------------------------

    void SteamNetDriver::StaticOnConnectionStatusChanged(
        SteamNetConnectionStatusChangedCallback_t* info)
    {
        s_Instance->HandleConnectionStatusChanged(info);
    }

    // -------------------------------------------------------------------------

    void SteamNetDriver::HandleConnectionStatusChanged(
        const SteamNetConnectionStatusChangedCallback_t* info)
    {
        HSteamNetConnection hConn = info->m_hConn;

        switch (info->m_info.m_eState)
        {
        case k_ESteamNetworkingConnectionState_Connecting:
            if (!IsClient())
                AcceptConnection(hConn);
            break;

        case k_ESteamNetworkingConnectionState_Connected:
        {
            OnConnected(hConn);
            break;
        }

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        case k_ESteamNetworkingConnectionState_Dead:
        {
            OnDisconnected(hConn);
            break;
        }

        default:
            break;
        }
    }

    // -------------------------------------------------------------------------
    // Send & Broadcast
    // -------------------------------------------------------------------------

    void SteamNetDriver::Send(NetConnection* conn, NetPacket& pkt, bool reliable)
    {
        auto it = m_Connections.find(conn->GetId());
        if (it == m_Connections.end())
            return;

        Send(it->second.hConn, pkt, reliable);
    }

    void SteamNetDriver::Broadcast(NetPacket& pkt, bool reliable)
    {
        for (auto& [id, dc] : m_Connections)
        {
            m_Interface->SendMessageToConnection(
                dc.hConn,
                pkt.RawData(),
                pkt.RawSize(),
                reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable,
                nullptr
            );
        }
    }

    void SteamNetDriver::SendToServer(NetPacket& pkt, bool reliable)
    {
        if (!IsClient())
            return;

        auto it = m_Connections.find(m_LocalConnectionId);
        if (it == m_Connections.end())
            return;

        m_Interface->SendMessageToConnection(
            it->second.hConn,
            pkt.RawData(),
            pkt.RawSize(),
            reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable,
            nullptr
        );
    }

    // -------------------------------------------------------------------------
    // Lookup
    // -------------------------------------------------------------------------

    NetConnection* SteamNetDriver::GetConnection(uint64_t id)
    {
        auto it = m_Connections.find(id);
        if (it == m_Connections.end())
            return nullptr;
        return it->second.conn.get();
    }

    void SteamNetDriver::ForeachConnection(const std::function<void(NetConnection*)>& fn)
    {
        for (auto& c : m_Connections)
        {
            fn(c.second.conn.get());
        }
    }

    uint32_t SteamNetDriver::GetConnectionCount() const
    {
        return m_Connections.size();
    }

    void SteamNetDriver::UpdateConnections()
    {
        for (auto& connection : m_Connections)
        {
            SteamNetConnectionRealTimeStatus_t status;

            if (m_Interface->GetConnectionRealTimeStatus(connection.second.hConn, &status, 0, nullptr) == k_EResultOK)
            {
                int ping = status.m_nPing;
                connection.second.conn->SetPing(ping);
            }
        }
    }
}
