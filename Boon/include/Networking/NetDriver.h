#pragma once
#include <functional>
#include "NetPacket.h"
#include "NetConnection.h"
#include "NetAuthority.h"

namespace Boon
{
    class NetScene;

    using PacketCallback = std::function<void(NetConnection*, NetPacket&)>;
    using ConnectionCallback = std::function<void(NetConnection*)>;

    class NetDriver
    {
    public:
        virtual ~NetDriver() = default;

        // -----------------------------------------------------------------
        // Initialization
        // -----------------------------------------------------------------
        virtual bool Initialize(ENetDriverMode mode) = 0;
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

        // -----------------------------------------------------------------
        // Callback binding
        // -----------------------------------------------------------------
        virtual void BindOnPacketCallback(const PacketCallback& fn) = 0;
        virtual void BindOnConnectedCallback(const ConnectionCallback& fn) = 0;
        virtual void BindOnDisconnectedCallback(const ConnectionCallback& fn) = 0;

        // -----------------------------------------------------------------
        // Scene (owner)
        // -----------------------------------------------------------------
        virtual void BindScene(NetScene* scene) = 0;

        // -----------------------------------------------------------------
        // Connection lookup
        // -----------------------------------------------------------------
        virtual NetConnection* GetConnection(uint64_t id) = 0;

        // -----------------------------------------------------------------
        // Role
        // -----------------------------------------------------------------
        virtual ENetDriverMode GetMode() const = 0;
        virtual uint64_t GetLocalConnectionId() const = 0;
    };
}
