#pragma once

#include "Core/UUID.h"
#include "Scene/GameObject.h"
#include "Networking/NetPacket.h"
#include "Networking/NetAuthority.h"
#include "Reflection/BClass.h"
#include "Event/Event.h"

#include <memory>

namespace Boon
{
    class Scene;
    class NetDriver;
    class NetConnection;
    class NetRepCore;
    class NetRPC;

    class NetScene
    {
    public:
        /**
         * @brief Network-aware wrapper around a Scene instance.
         *
         * Manages replication, RPCs and routing of network packets for a
         * specific scene on a NetDriver.
         * @param scene Scene instance this NetScene manages.
         * @param driver Network driver used to send/receive packets.
         */
        NetScene(Scene* scene, NetDriver* driver);

        /**
         * @brief Destroy the NetScene and cleanup networking resources.
         */
        ~NetScene();

        /**
         * @brief Per-frame network update (process pending replication, RPCs).
         */
        void Update();

        // ---------------------------------------------------------------------
        // Scene Registration
        // ---------------------------------------------------------------------

        // ---------------------------------------------------------------------
        // Scene Registration
        // ---------------------------------------------------------------------

        /**
         * @brief Register a static (level) game object that exists on load.
         * @param gameObject GameObject to register as static.
         */
        void RegisterStaticGameObject(GameObject gameObject);

        /**
         * @brief Server-only: spawn a dynamic (runtime) game object.
         * @param gameObject GameObject data to spawn.
         * @param ownerConnection Owner connection id for the spawned object.
         * @return The spawned GameObject with assigned network identity.
         */
        GameObject RegisterDynamicGameObject(GameObject gameObject, uint64_t ownerConnection);

        /**
         * @brief Client-only: create a replicated game object because the server requested it.
         * @param uuid Network UUID assigned to the new object.
         * @param ownerConnection Owner connection id.
         * @return Created GameObject instance.
         */
        GameObject CreateReplicatedGameObject(const UUID& uuid, uint64_t ownerConnection);

        /**
         * @brief Lookup a game object by its network UUID.
         * @param uuid UUID of the requested object.
         * @return GameObject instance if found, otherwise an invalid/default GameObject.
         */
        GameObject GetGameObjectByUUID(const UUID& uuid);

        // ---------------------------------------------------------------------
        // Packet Routing (called by NetDriver)
        // ---------------------------------------------------------------------

        /**
         * @brief Process an incoming network packet for this scene.
         * @param sender Connection that sent the packet.
         * @param pkt Packet data to process.
         */
        void ProcessPacket(NetConnection* sender, NetPacket& pkt);


        /**
         * @brief Get the local connection identifier used by this NetScene.
         * @return Local connection id.
         */
        uint64_t GetLocalConnectionID() const;

        /**
         * @brief Access the underlying Scene managed by this NetScene.
         * @return Reference to the Scene.
         */
        inline Scene& GetScene() { return *m_Scene; }

        /**
         * @brief Access the NetDriver used by this NetScene.
         * @return Pointer to the NetDriver.
         */
        inline NetDriver* GetDriver() { return m_Driver; }

        /**
         * @brief Access the NetRPC subsystem for issuing RPCs.
         * @return Pointer to the NetRPC instance or nullptr if not available.
         */
        inline NetRPC* GetRPC() { return m_RPC.get(); }

        /**
         * @brief Spawn a networked GameObject and assign ownership.
         * @param connectionId Connection id that will own the instantiated object.
         * @param uuid Optional UUID to assign (generated if not provided).
         * @return The instantiated GameObject.
         */
        GameObject InstantiateGameObject(uint64_t connectionId, UUID uuid = UUID());

    private:
        void HandleSpawnPacket(NetConnection* sender, NetPacket& pkt);
        void HandleDespawnPacket(NetConnection* sender, NetPacket& pkt);
        void HandleComponentPacket(NetConnection* sender, NetPacket& pkt);
        void HandleLoadScenePacket(NetConnection* sender, NetPacket& pkt);
        void HandleClientSceneInitPacket(NetConnection* sender, NetPacket& pkt);

        void SendSpawnTo(NetConnection* conn, const GameObject& obj);
        void SendDespawnTo(NetConnection* conn, const UUID& uuid);
        void SendComponentTo(NetConnection* conn, const UUID& uuid, const BClassID& component, bool add);
        void SendLoadSceneTo(NetConnection* conn, const SceneID& sceneId);
        void BroadcastDespawn(const UUID& uuid);
        void BroadcastComponent(const UUID& uuid, const BClassID& component, bool add);
        void BroadcastLoadScene(const SceneID& sceneId);

        void InitClientScene(NetConnection* conn);

    private:
        Scene* m_Scene = nullptr;
        NetDriver* m_Driver = nullptr;
        std::unique_ptr<NetRPC> m_RPC = nullptr;
        std::unique_ptr<NetRepCore> m_Replication = nullptr;

        // Objects that server spawned at runtime
        std::unordered_map<UUID, uint64_t> m_DynamicOwnership;
        bool m_bRegisterDynamicObject{ true };

        EventListenerID m_ClientConnectedEvent;
    };
}
