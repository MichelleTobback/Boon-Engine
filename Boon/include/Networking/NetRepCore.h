#pragma once
#include "Core/UUID.h"
#include "NetPacket.h"
#include "NetIdentity.h"

#include <unordered_map>
#include <vector>

namespace Boon
{
    class NetScene;

    // -------------------------------------------------------------
    // Base replication core
    // -------------------------------------------------------------
    class NetRepCore
    {
    public:
        NetRepCore() = default;

        // Called by NetScene when a replication packet arrives
        void ProcessPacket(NetScene& scene, NetPacket& pkt);

        // Called every frame by NetScene or by the world tick
        // to gather changed replicated properties and send them.
        void Update(NetScene& scene);

    private:
        // Stores last known replicated state per object.
        // Useful for delta-compression or change detection later.
        struct ReplicationState
        {
            std::vector<uint8_t> LastState;
        };

        // Cache: UUID -> state
        std::unordered_map<UUID, ReplicationState> m_StateCache;
    };
}
