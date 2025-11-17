#include "Networking/NetRPC.h"
#include "Networking/NetScene.h"
#include "Networking/NetDriver.h"
#include "Networking/NetRepRegistry.h"
#include "Reflection/BFunction.h"
#include "Reflection/BClass.h"

namespace Boon
{
    NetRPC::NetRPC(NetScene* scene)
        : m_Scene(scene)
    {
    }

    // ============================================
    // Write parameter using reflection type
    // ============================================
    static void WriteParam(BinarySerializer& ser, const Variant& v, BTypeId type)
    {
        switch (type)
        {
        case BTypeId::Int:      ser.Write<int>(v.As<int>()); break;
        case BTypeId::Float:    ser.Write<float>(v.As<float>()); break;
        case BTypeId::Bool:     ser.Write<bool>(v.As<bool>()); break;
        case BTypeId::String:   ser.WriteString(v.As<std::string>()); break;
        case BTypeId::Int64:    ser.Write<int64_t>(v.As<int64_t>()); break;

        case BTypeId::Float2:   ser.Write(v.As<glm::vec2>()); break;
        case BTypeId::Float3:   ser.Write(v.As<glm::vec3>()); break;
        case BTypeId::Float4:   ser.Write(v.As<glm::vec4>()); break;

        default: break;
        }
    }

    // ============================================
    // Read parameter using reflection type
    // ============================================
    static Variant ReadParam(BinarySerializer& ser, BTypeId type)
    {
        switch (type)
        {
        case BTypeId::Int:      return Variant(ser.Read<int>());
        case BTypeId::Float:    return Variant(ser.Read<float>());
        case BTypeId::Bool:     return Variant(ser.Read<bool>());
        case BTypeId::String:   return Variant(ser.ReadString());
        case BTypeId::Int64:    return Variant(ser.Read<int64_t>());

        case BTypeId::Float2:   return Variant(ser.Read<glm::vec2>());
        case BTypeId::Float3:   return Variant(ser.Read<glm::vec3>());
        case BTypeId::Float4:   return Variant(ser.Read<glm::vec4>());

        default: return Variant();
        }
    }

    // ============================================
    // SEND TO SERVER
    // ============================================
    void NetRPC::CallServer(uint32_t classId, uint32_t fnId, const UUID& uuid, const std::vector<Variant>& args)
    {
        NetPacket pkt(ENetPacketType::RPC);
        auto& ser = pkt.GetSerializer();

        ser.Write<uint32_t>(classId);
        ser.Write<uint32_t>(fnId);
        ser.Write<UUID>(uuid);

        // Arguments
        ser.Write<uint32_t>((uint32_t)args.size());

        const auto& rep = NetRepRegistry::Get().GetClass(classId);
        const BFunction* fn = rep.FindServerRPC(fnId);
        if (!fn) return;

        for (size_t i = 0; i < args.size(); i++)
            WriteParam(ser, args[i], fn->params[i].typeId);

        m_Scene->GetDriver()->SendToServer(pkt);
    }

    // ============================================
    // SEND TO SPECIFIC CLIENT
    // ============================================
    void NetRPC::CallClient(uint64_t clientId, uint32_t classId, uint32_t fnId, const UUID& uuid, const std::vector<Variant>& args)
    {
        NetPacket pkt(ENetPacketType::RPC);
        auto& ser = pkt.GetSerializer();

        ser.Write<uint32_t>(classId);
        ser.Write<uint32_t>(fnId);
        ser.Write<UUID>(uuid);

        ser.Write<uint32_t>((uint32_t)args.size());

        const auto& rep = NetRepRegistry::Get().GetClass(classId);
        const BFunction* fn = rep.FindClientRPC(fnId);
        if (!fn) return;

        for (size_t i = 0; i < args.size(); i++)
            WriteParam(ser, args[i], fn->params[i].typeId);

        m_Scene->GetDriver()->Send(m_Scene->GetDriver()->GetConnection(clientId), pkt);
    }

    // ============================================
    // RECEIVED RPC (Server OR Client)
    // ============================================
    void NetRPC::Process(const NetPacket& pkt, bool isServerSide)
    {
        auto ser = pkt.GetSerializer();

        uint32_t classId = ser.Read<uint32_t>();
        uint32_t fnId = ser.Read<uint32_t>();
        UUID uuid = ser.Read<UUID>();

        auto& rep = NetRepRegistry::Get().GetClass(classId);

        const BFunction* fn =
            isServerSide ? rep.FindServerRPC(fnId)
            : rep.FindClientRPC(fnId);

        if (!fn)
            return;

        uint32_t argCount = ser.Read<uint32_t>();
        Variant args[16];

        for (uint32_t i = 0; i < argCount; i++)
        {
            BTypeId type = fn->params[i].typeId;
            args[i] = ReadParam(ser, type);
        }

        GameObject obj = m_Scene->GetGameObjectByUUID(uuid);
        if (!obj.IsValid())
            return;

        void* component = obj.GetComponentByClass(rep.cls);
        fn->thunk(component, args, argCount);
    }

}
