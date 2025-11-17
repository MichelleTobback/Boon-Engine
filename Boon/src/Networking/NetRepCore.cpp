#include "Networking/NetRepCore.h"
#include "Networking/NetScene.h"
#include "Networking/NetDriver.h"

#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Component/UUIDComponent.h"

namespace Boon
{
    NetRepCore::NetRepCore()
    {
        
    }

    void NetRepCore::ProcessPacket(NetScene& scene, NetPacket& pkt, NetConnection* sender)
    {
        auto& ser = pkt.GetSerializer();

        uint16_t objectCount = ser.Read<uint16_t>();

        NetRepRegistry& reg = NetRepRegistry::Get();

        for (int i = 0; i < objectCount; i++)
        {
            UUID uuid = ser.Read<UUID>();

            GameObject obj = scene.GetGameObjectByUUID(uuid);
            if (!obj.IsValid())
                continue;

            uint8_t compCount = ser.Read<uint8_t>();

            for (int c = 0; c < compCount; c++)
            {
                BClassID compId = ser.Read<BClassID>();
                uint32_t dirtyMask = ser.ReadBits(32);
                uint32_t blobSize = ser.Read<uint32_t>();

                // Raw byte blob of modified properties
                Buffer temp(blobSize);
                ser.ReadBytes(temp.Data(), blobSize);

                ReplicatedClass& comp = reg.GetClass(compId);

                if (!comp.cls->hasComponent(obj))
                {
                    comp.cls->addComponent(obj);
                }

                if (comp.serializer)
                {
                    BinarySerializer compSerializer{ temp };
                    comp.serializer->Deserialize(compSerializer, obj);
                    continue;
                }

                void* pInstance = comp.cls->getComponent(obj);

                uint8_t* cursor = temp.Data();

                for (auto& propSet : comp.fields)
                {
                    for (auto& [flag, repField] : propSet)
                    {
                        if (dirtyMask & flag)
                        {
                            uint8_t* dst = (uint8_t*)pInstance + repField.Offset();
                            std::memcpy(dst, cursor, repField.Size());
                            cursor += repField.Size();
                        }
                    }
                }
            }
        }
    }


    // ---------------------------------------------------------------------
    // Server-side: gather changes & send replication packets
    // ---------------------------------------------------------------------
    void NetRepCore::Update(NetScene& scene)
    {
        Scene& s = scene.GetScene();

        BinarySerializer ser;

        uint16_t objectCount = 0;
        ser.Write<uint16_t>(0); // placeholder, patch later

        size_t objectCountByteOffset = 0; // always 0 since write pos starts at 0

        NetRepRegistry& reg = NetRepRegistry::Get();

        s.ForeachGameObjectWith<NetIdentity>([&, this](GameObject obj)
            {
                auto& id = obj.GetComponent<NetIdentity>();
                if (!id.bReplicates || !id.IsAuthority())
                    return;

                ReplicationState state = m_StateCache[id.NetId];
                state.uuid = id.NetId;

                // -------------------------------------
                // Check all components first
                // -------------------------------------
                struct ComponentDelta
                {
                    BClassID clsId;
                    uint32_t dirtyMask;
                    Buffer   blob;
                };

                std::vector<ComponentDelta> dirtyComps;
                dirtyComps.reserve(4);

                reg.ForEach([&](ReplicatedClass& comp)
                    {
                        if (!obj.HasComponentByClass(comp.cls))
                            return;

                        if (comp.serializer && comp.serializer->IsDirty(obj))
                        {
                            BinarySerializer serializer{};
                            comp.serializer->Serialize(serializer, obj);
                            dirtyComps.push_back({ comp.cls->hash, 0, serializer.GetBuffer() });
                            return;
                        }

                        void* inst = comp.cls->getComponent(obj);
                        ReplicatedInstance& repInst = state.components[comp.cls];
                        if (!repInst.pObject)
                        {
                            repInst.pObject = &comp;
                            repInst.snapshot = Buffer(comp.size);
                        }

                        uint32_t dirtyMask = 0;
                        Buffer blob;
                        Buffer newSnapshot;
                        newSnapshot.Reserve(comp.size);

                        for (auto& propSet : comp.fields)
                        {
                            for (auto& [flag, rf] : propSet)
                            {
                                uint8_t* cur = (uint8_t*)inst + rf.Offset();
                                uint8_t* old = repInst.snapshot.DataAt(rf.Packedoffset);

                                newSnapshot.WriteRaw(cur, rf.Size());

                                if (memcmp(cur, old, rf.Size()) != 0)
                                {
                                    dirtyMask |= flag;
                                    blob.WriteRaw(cur, rf.Size());
                                }
                            }
                        }

                        if (dirtyMask != 0)
                        {
                            dirtyComps.push_back({ comp.cls->hash, dirtyMask, blob });
                            repInst.snapshot = newSnapshot;
                        }
                    });

                if (dirtyComps.empty())
                    return;  // do NOT write this object

                // -------------------------------------
                // Write object to stream
                // -------------------------------------
                ser.Write(id.NetId);

                uint8_t numComps = (uint8_t)dirtyComps.size();
                ser.Write<uint8_t>(numComps);

                for (auto& dc : dirtyComps)
                {
                    ser.Write<BClassID>(dc.clsId);
                    ser.WriteBits(dc.dirtyMask, 32);
                    uint32_t sz = dc.blob.Size();
                    ser.Write<uint32_t>(sz);
                    ser.WriteBytes(dc.blob.Data(), sz);
                }

                objectCount++;
            });

        // Patch objectCount
        *(uint16_t*)(ser.GetBuffer().Data() + objectCountByteOffset) = objectCount;

        if (objectCount == 0)
            return;

        NetPacket pkt(ENetPacketType::Replication);
        pkt.WriteBytes(ser.Data(), ser.Size());
        scene.GetDriver()->Broadcast(pkt, false);
    }
}
