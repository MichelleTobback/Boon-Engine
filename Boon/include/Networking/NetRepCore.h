#pragma once
#include "Core/UUID.h"
#include "Core/Memory/Buffer.h"
#include "NetPacket.h"
#include "NetIdentity.h"
#include "Reflection/BProperty.h"

#include <unordered_map>
#include <vector>

namespace Boon
{
    class NetScene;
    struct BClass;
    // -------------------------------------------------------------
    // Base replication core
    // -------------------------------------------------------------
    class NetRepCore
    {
    public:
        NetRepCore();

        // Called by NetScene when a replication packet arrives
        void ProcessPacket(NetScene& scene, NetPacket& pkt);

        // Called every frame by NetScene or by the world tick
        // to gather changed replicated properties and send them.
        void Update(NetScene& scene);

        void RegisterReplicatedObject(GameObject owner, BClass* cls);

    private:
        // Stores last known replicated state per object.
        // Useful for delta-compression or change detection later.
        struct ReplicatedField
        {
            BProperty* pProp;
            uint32_t Offset() const { return pProp->offset; }
            uint32_t Size() const { return pProp->size; }
        };

        struct ReplicatedObject
        {
            const BClass* cls;

            // Static: reflected replicated fields
            std::vector<ReplicatedField> fields;

            // Dynamic: flat byte snapshot for this component
            Buffer snapshot;
        };

        struct ReplicationState
        {
            UUID uuid;
            GameObject gameObject;

            // Each component snapshot is keyed by its BClass reflection type.
            std::unordered_map<const BClass*, ReplicatedObject> components;
        };

        ReplicatedObject BuildComponentLayout(GameObject obj, const BClass* cls);
        void PackComponentSnapshot(const ReplicatedObject& comp, GameObject obj, Buffer& outSnapshot);
        void BuildInitialSnapshots(ReplicationState& state);
        bool IsComponentDirty(const ReplicatedObject& comp, GameObject obj, Buffer& tempBuffer);
        void CommitComponentSnapshot(ReplicatedObject& comp, Buffer& temp);
        void SendComponentReplication(NetScene& scene, ReplicationState& state, ReplicatedObject& comp, Buffer& tempSnapshot);

        // Cache: UUID -> state
        std::unordered_map<UUID, ReplicationState> m_StateCache;
        std::vector<BClass*> m_ReplicatedClasses;
    };
}
