#pragma once
#include <functional>
#include "NetPacket.h"
#include "NetConnection.h"
#include "NetAuthority.h"
#include "Networking/NetworkSettings.h"

#include <memory>

namespace Boon
{
    class NetScene;

    using PacketCallback = std::function<void(NetConnection*, NetPacket&)>;
    using ConnectionCallback = std::function<void(NetConnection*)>;
    using NetDriverCallback = std::function<void(NetDriver*)>;

    class NetDriver
    {
    public:
        virtual ~NetDriver() = default;

        // -----------------------------------------------------------------
        // Initialization
        // -----------------------------------------------------------------
        virtual bool Initialize(const NetworkSettings& settings) = 0;
        virtual void Shutdown() = 0;

        virtual bool Connect(const char* host, uint16_t port) = 0;

        // -----------------------------------------------------------------
        // Tick
        // -----------------------------------------------------------------
        virtual void Update() = 0;

        // -----------------------------------------------------------------
        // Send
        // -----------------------------------------------------------------
        virtual void Send(NetConnection* conn, NetPacket& pkt, bool reliable = true) = 0;
        virtual void Broadcast(NetPacket& pkt, bool reliable = true) = 0;
        virtual void SendToServer(NetPacket& pkt, bool reliable = true) = 0;

        // -----------------------------------------------------------------
        // Callback binding
        // -----------------------------------------------------------------
        virtual void BindOnStartupCallback(const NetDriverCallback& fn) = 0;
        virtual void BindOnShutdownCallback(const NetDriverCallback& fn) = 0;

        virtual void BindOnPacketCallback(const PacketCallback& fn) = 0;
        virtual void BindOnConnectedCallback(const ConnectionCallback& fn) = 0;
        virtual void BindOnDisconnectedCallback(const ConnectionCallback& fn) = 0;

        // -----------------------------------------------------------------
        // Scene (owner)
        // -----------------------------------------------------------------
        virtual void BindScene(const std::shared_ptr<NetScene>& scene) = 0;

        // -----------------------------------------------------------------
        // Connection lookup
        // -----------------------------------------------------------------
        virtual NetConnection* GetConnection(uint64_t id) = 0;
        virtual void ForeachConnection(const std::function<void(NetConnection*)>& fn) = 0;
        virtual uint32_t GetConnectionCount() const = 0;

        // -----------------------------------------------------------------
        // Role
        // -----------------------------------------------------------------
        virtual ENetDriverMode GetMode() const = 0;
        virtual uint64_t GetLocalConnectionId() const = 0;

        virtual bool IsStandalone() const = 0;
        virtual bool IsClient() const = 0;
        virtual bool IsServer() const = 0;
    };
}
