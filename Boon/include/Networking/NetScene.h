#pragma once

#include "Core/UUID.h"
#include "Scene/GameObject.h"
#include "Networking/NetPacket.h"
#include "Networking/NetAuthority.h"

#include <memory>

namespace Boon
{
    class Scene;
    class NetDriver;
    class NetConnection;
    class NetRepCore;

    class NetScene
    {
    public:
        NetScene(Scene* scene, NetDriver* driver);
        ~NetScene();

        void Update();

        // ---------------------------------------------------------------------
        // Scene Registration
        // ---------------------------------------------------------------------

        // Called when loading a level (static objects)
        void RegisterStaticGameObject(GameObject gameObject);

        // Server-only: spawn runtime objects
        GameObject RegisterDynamicGameObject(GameObject gameObject, uint64_t ownerConnection);

        // Client-only: scene spawns object because server told us
        GameObject CreateReplicatedGameObject(const UUID& uuid, uint64_t ownerConnection);

        // Lookup
        GameObject GetGameObjectByUUID(const UUID& uuid);

        // ---------------------------------------------------------------------
        // Packet Routing (called by NetDriver)
        // ---------------------------------------------------------------------
        void ProcessPacket(NetConnection* sender, NetPacket& pkt);

        inline Scene& GetScene() { return *m_Scene; }
        inline NetDriver* GetDriver() { return m_Driver; }

    private:
        void HandleSpawnPacket(NetConnection* sender, NetPacket& pkt);
        void HandleDespawnPacket(NetConnection* sender, NetPacket& pkt);

        void SendSpawnTo(NetConnection* conn, const GameObject& obj);
        void SendDespawnTo(NetConnection* conn, const UUID& uuid);
        void BroadcastDespawn(const UUID& uuid);

    private:
        Scene* m_Scene = nullptr;
        NetDriver* m_Driver = nullptr;
        std::unique_ptr<NetRepCore> m_Replication = nullptr;

        // Objects that server spawned at runtime
        std::unordered_map<UUID, uint64_t> m_DynamicOwnership;
    };
}
