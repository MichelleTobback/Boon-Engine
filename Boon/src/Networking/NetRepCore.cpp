#include "Networking/NetRepCore.h"
#include "Networking/NetScene.h"

#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Component/UUIDComponent.h"

namespace Boon
{
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

            // ----------------------------------------------
            // TODO:
            //   Use your reflection system to apply:
            //     obj.SetFieldValue(fieldName, value)
            // ----------------------------------------------

            // Example placeholder:
            // Reflection::SetProperty(obj, fieldName, value);
        }
    }

    // ---------------------------------------------------------------------
    // Server-side: gather changes & send replication packets
    // ---------------------------------------------------------------------
    void NetRepCore::Update(NetScene& scene)
    {
        // TODO:
        // - Iterate all NetIdentity objects with bReplicates=true
        // - Use reflection to gather replicated properties
        // - Compare with cached state
        // - If changed, send NetPacket(ENetPacketType::Replication)
        // - Update cache

        // This is intentionally left empty until you provide:
        //  - your reflection API for reading fields
        //  - list of fields marked as Replicated
    }
}
