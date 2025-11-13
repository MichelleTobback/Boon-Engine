#pragma once

#include "Core/UUID.h"
#include "Scene/GameObject.h"
#include "Networking/NetPacket.h"
#include "Networking/NetAuthority.h"

namespace Boon
{
    class Scene;
    class NetDriver;
    class NetConnection;

    class NetScene
    {
    public:
        NetScene(Scene* scene, NetDriver* driver);

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

    private:
        void HandleSpawnPacket(NetConnection* sender, NetPacket& pkt);
        void HandleDespawnPacket(NetConnection* sender, NetPacket& pkt);

        void SendSpawnTo(NetConnection* conn, const GameObject& obj);
        void SendDespawnTo(NetConnection* conn, const UUID& uuid);

    private:
        Scene* m_Scene = nullptr;
        NetDriver* m_Driver = nullptr;

        // Objects that server spawned at runtime
        std::unordered_map<UUID, uint64_t> m_DynamicOwnership;
    };
}
