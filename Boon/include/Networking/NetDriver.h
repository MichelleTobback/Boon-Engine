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
        /**
         * @brief Initialize the network driver with provided settings.
         *
         * @param settings Network configuration to apply.
         * @return true on success, false otherwise.
         */
        virtual bool Initialize(const NetworkSettings& settings) = 0;

        /**
         * @brief Shutdown the driver and release resources.
         */
        virtual void Shutdown() = 0;

        /**
         * @brief Connect to a remote host (client mode).
         *
         * @param host Hostname or IP string.
         * @param port Port number.
         * @return true if connection attempt was initiated/succeeded.
         */
        virtual bool Connect(const char* host, uint16_t port) = 0;

        // -----------------------------------------------------------------
        // Tick
        // -----------------------------------------------------------------
        /**
         * @brief Per-frame update for the network driver.
         */
        virtual void Update() = 0;

        // -----------------------------------------------------------------
        // Send
        // -----------------------------------------------------------------
        /**
         * @brief Send a packet to a specific connection.
         *
         * @param conn Destination connection.
         * @param pkt Packet to send.
         * @param reliable Whether to send reliably.
         */
        virtual void Send(NetConnection* conn, NetPacket& pkt, bool reliable = true) = 0;

        /**
         * @brief Broadcast a packet to all connected peers.
         */
        virtual void Broadcast(NetPacket& pkt, bool reliable = true) = 0;

        /**
         * @brief Send a packet to the connected server (client mode).
         */
        virtual void SendToServer(NetPacket& pkt, bool reliable = true) = 0;

        // -----------------------------------------------------------------
        // Callback binding
        // -----------------------------------------------------------------
        /**
         * @brief Bind a callback invoked when the driver starts up.
         */
        virtual void BindOnStartupCallback(const NetDriverCallback& fn) = 0;

        /**
         * @brief Bind a callback invoked on shutdown.
         */
        virtual void BindOnShutdownCallback(const NetDriverCallback& fn) = 0;

        /**
         * @brief Bind a callback invoked for incoming packets.
         */
        virtual void BindOnPacketCallback(const PacketCallback& fn) = 0;

        /**
         * @brief Bind a callback invoked when a connection is established.
         */
        virtual void BindOnConnectedCallback(const ConnectionCallback& fn) = 0;

        /**
         * @brief Bind a callback invoked when a connection is lost.
         */
        virtual void BindOnDisconnectedCallback(const ConnectionCallback& fn) = 0;

        // -----------------------------------------------------------------
        // Scene (owner)
        // -----------------------------------------------------------------
        /**
         * @brief Bind a NetScene to the driver for packet routing.
         */
        virtual void BindScene(const std::shared_ptr<NetScene>& scene) = 0;

        // -----------------------------------------------------------------
        // Connection lookup
        // -----------------------------------------------------------------
        /**
         * @brief Lookup a connection by its id.
         */
        virtual NetConnection* GetConnection(uint64_t id) = 0;

        /**
         * @brief Invoke a function for each active connection.
         */
        virtual void ForeachConnection(const std::function<void(NetConnection*)>& fn) = 0;

        /**
         * @brief Get the current number of active connections.
         */
        virtual uint32_t GetConnectionCount() const = 0;

        // -----------------------------------------------------------------
        // Role
        // -----------------------------------------------------------------
        /**
         * @brief Get the driver's operating mode (client/server/standalone).
         */
        virtual ENetDriverMode GetMode() const = 0;

        /**
         * @brief Get local connection identifier used by this driver.
         */
        virtual uint64_t GetLocalConnectionId() const = 0;

        virtual bool IsStandalone() const = 0;
        virtual bool IsClient() const = 0;
        virtual bool IsServer() const = 0;
    };
}
