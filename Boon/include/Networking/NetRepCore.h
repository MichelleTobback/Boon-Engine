#pragma once
#include "Core/UUID.h"
#include "Core/Memory/Buffer.h"
#include "NetPacket.h"
#include "NetIdentity.h"
#include "Reflection/BProperty.h"
#include "Reflection/BClass.h"
#include "NetRepRegistry.h"

#include <unordered_map>
#include <vector>

namespace Boon
{
    class NetConnection;
    class NetScene;
    class NetRepCore
    {
    public:
        NetRepCore();

        void ProcessPacket(NetScene& scene, NetPacket& pkt, NetConnection* sender);
        void Update(NetScene& scene);

    private:
        struct ReplicatedInstance
        {
            ReplicatedClass* pObject;
            Buffer snapshot;
        };

        struct ReplicationState
        {
            UUID uuid;
            std::unordered_map<const BClass*, ReplicatedInstance> components;
        };

        // Cache: UUID -> state
        std::unordered_map<UUID, ReplicationState> m_StateCache;
    };
}
