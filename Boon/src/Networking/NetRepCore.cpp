#include "Networking/NetRepCore.h"
#include "Networking/NetScene.h"
#include "Networking/NetDriver.h"
#include "Reflection/BClass.h"

#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Component/UUIDComponent.h"

namespace Boon
{
    NetRepCore::NetRepCore()
    {
        BClassRegistry::Get().ForEach([this](BClass& cls)
            {
                if (cls.HasMeta("Replicated"))
                    m_ReplicatedClasses.push_back(&cls);
            });
    }

    void NetRepCore::ProcessPacket(NetScene& scene, NetPacket& pkt)
    {
        auto& s = pkt.GetSerializer();

        // ---------------------------------------------------------
        // Basic structure:
        //
        //  UUID netId
        //  uint32_t numFields
        //  For each field:
        //      uint16_t fieldNameLen
        //      char[fieldNameLen]
        //      uint16_t fieldSize
        //      uint8_t[fieldSize]
        // ---------------------------------------------------------

        UUID netId = s.Read<UUID>();
        uint32_t fieldCount = s.Read<uint32_t>();

        GameObject obj = scene.GetGameObjectByUUID(netId);
        if (!obj.IsValid())
            return;

        for (uint32_t i = 0; i < fieldCount; i++)
        {
            uint16_t nameLen = s.Read<uint16_t>();
            std::string fieldName(nameLen, '\0');
            s.ReadBytes(fieldName.data(), nameLen);

            uint16_t fieldSize = s.Read<uint16_t>();

            std::vector<uint8_t> value(fieldSize);
            s.ReadBytes(value.data(), fieldSize);

            
        }
    }

    // ---------------------------------------------------------------------
    // Server-side: gather changes & send replication packets
    // ---------------------------------------------------------------------
    void NetRepCore::Update(NetScene& scene)
    {
        Scene& s = scene.GetScene();

        static Buffer tempSnapshot;

        s.ForeachGameObjectWith<NetIdentity>([&](GameObject obj)
            {
                auto& id = obj.GetComponent<NetIdentity>();
                if (!id.bReplicates || !id.IsAuthority())
                    return;

                for (BClass* pClass : m_ReplicatedClasses)
                {
                    RegisterReplicatedObject(obj, pClass);

                    ReplicationState& st = m_StateCache[id.NetId];

                    for (auto& [cls, comp] : st.components)
                    {
                        if (IsComponentDirty(comp, obj, tempSnapshot))
                        {
                            SendComponentReplication(scene, st, comp, tempSnapshot);
                            CommitComponentSnapshot(comp, tempSnapshot);
                        }
                    }
                }
            });
    }

    void NetRepCore::RegisterReplicatedObject(GameObject obj, BClass* cls)
    {
        NetIdentity& id = obj.GetComponent<NetIdentity>();
        UUID uuid = id.NetId;

        if (m_StateCache.find(uuid) != m_StateCache.end())
            return;

        ReplicationState st;
        st.uuid = uuid;
        st.gameObject = obj;

        ReplicatedObject comp = BuildComponentLayout(obj, cls);
        st.components[cls] = std::move(comp);

        // Build initial snapshots
        for (auto& [clsKey, compObj] : st.components)
            PackComponentSnapshot(compObj, obj, compObj.snapshot);

        m_StateCache[uuid] = std::move(st);
    }


    NetRepCore::ReplicatedObject NetRepCore::BuildComponentLayout(GameObject obj, const BClass* cls)
    {
        ReplicatedObject comp;
        comp.cls = cls;

        size_t totalSize = 0;

        for (auto& prop : cls->GetProperties())
        {
            if (!prop.HasMeta("Replicated"))
                continue;

            comp.fields.push_back({ (BProperty*)&prop });
            totalSize += prop.size;
        }

        // Allocate snapshot buffer
        comp.snapshot = Buffer(totalSize);

        return comp;
    }

    void NetRepCore::PackComponentSnapshot(const ReplicatedObject& comp, GameObject obj, Buffer& outSnapshot)
    {
        uint8_t* basePtr = (uint8_t*)comp.cls->getComponent(obj);

        outSnapshot = Buffer(comp.snapshot.Size());

        uint8_t* dst = outSnapshot.Data();

        size_t writeHead = 0;

        for (auto& f : comp.fields)
        {
            size_t offset = f.Offset();
            size_t size = f.Size();

            std::memcpy(dst + writeHead, basePtr + offset, size);
            writeHead += size;
        }
    }

    void NetRepCore::BuildInitialSnapshots(ReplicationState& state)
    {
        for (auto& pair : state.components)
        {
            Buffer buffer = pair.second.snapshot;
            PackComponentSnapshot(pair.second, state.gameObject, buffer);
        }
    }

    bool NetRepCore::IsComponentDirty(const ReplicatedObject& comp, GameObject obj, Buffer& tempBuffer)
    {
        PackComponentSnapshot(comp, obj, tempBuffer);

        return std::memcmp(comp.snapshot.Data(), tempBuffer.Data(), comp.snapshot.Size()) != 0;
    }

    void NetRepCore::CommitComponentSnapshot(ReplicatedObject& comp, Buffer& temp)
    {
        std::memcpy(comp.snapshot.Data(), temp.Data(), comp.snapshot.Size());
    }

    void NetRepCore::SendComponentReplication(NetScene& scene, ReplicationState& state, ReplicatedObject& comp, Buffer& tempSnapshot)
    {
        NetPacket pkt(ENetPacketType::Replication);

        // GAME OBJECT UUID
        pkt.Write(state.uuid);

        // COMPONENT TYPE (reflective BClass* turned into a stable ID)
        uint32_t compId = comp.cls->hash;
        pkt.Write<uint32_t>(compId);

        // SNAPSHOT SIZE
        uint32_t size = (uint32_t)tempSnapshot.Size();
        pkt.Write<uint32_t>(size);

        // RAW DATA
        pkt.WriteBytes(tempSnapshot.Data(), size);

        // Send to all clients
        scene.GetDriver()->Broadcast(pkt, false);
    }
}
