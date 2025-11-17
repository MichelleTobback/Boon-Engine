#include "Networking/NetScene.h"
#include "Networking/NetDriver.h"
#include "Networking/NetIdentity.h"
#include "Networking/NetConnection.h"
#include "Networking/NetRepCore.h"
#include "Networking/NetRPC.h"

#include "Scene/Scene.h"
#include "Component/UUIDComponent.h"

namespace Boon
{
    NetScene::NetScene(Scene* scene, NetDriver* driver)
        : m_Scene(scene), m_Driver(driver), m_Replication(std::make_unique<NetRepCore>()), m_RPC(std::make_unique<NetRPC>(this))
    {
        scene->ForeachGameObjectWith<NetIdentity>([this](GameObject obj){RegisterStaticGameObject(obj); });

        if (m_Driver->GetMode() != ENetDriverMode::Client)
        {
            m_Scene->GetOnGameObjectSpawned() += [this](GameObject obj) { RegisterDynamicGameObject(obj, 1); };
            m_Scene->GetOnGameObjectDestroyed() += [this](GameObject obj) { BroadcastDespawn(obj.GetUUID()); };
        }
    }

    NetScene::~NetScene()
    {

    }

    void NetScene::Update()
    {
        if (m_Driver->IsServer())
            m_Replication->Update(*this);
    }

    // -------------------------------------------------------------------------
    // Registration of static objects (level-placed)
    // -------------------------------------------------------------------------
    void NetScene::RegisterStaticGameObject(GameObject gameObject)
    {
        auto& id = gameObject.GetComponent<UUIDComponent>().Uuid;

        if (!gameObject.HasComponent<NetIdentity>())
            gameObject.AddComponent<NetIdentity>();

        NetIdentity& ni = gameObject.GetComponent<NetIdentity>();
        ni.NetId = id;
        ni.Role = m_Driver->IsServer() ? ENetRole::Authority : ENetRole::SimulatedProxy;
        ni.OwnerConnectionId = 0;
        ni.pScene = this;

        // Static objects are not added to dynamic map
    }

    // -------------------------------------------------------------------------
    // Server: spawn dynamic object
    // -------------------------------------------------------------------------
    GameObject NetScene::RegisterDynamicGameObject(GameObject gameObject, uint64_t ownerConnection)
    {
        if (m_Driver->GetMode() == ENetDriverMode::Client)
        {
            // client never registers dynamic objects
            return gameObject;
        }

        auto& id = gameObject.GetComponent<UUIDComponent>().Uuid;

        if (!gameObject.HasComponent<NetIdentity>())
            gameObject.AddComponent<NetIdentity>();

        NetIdentity& ni = gameObject.GetComponent<NetIdentity>();
        ni.NetId = id;
        ni.Role = ENetRole::Authority;
        ni.OwnerConnectionId = ownerConnection;
        ni.pScene = this;

        m_DynamicOwnership[id] = ownerConnection;

        // Broadcast spawn packet
        NetPacket pkt(ENetPacketType::Spawn);
        pkt.Write(id);
        pkt.Write(ownerConnection);
        m_Driver->Broadcast(pkt, true);

        return gameObject;
    }

    // -------------------------------------------------------------------------
    // Client: replicate object spawned by server
    // -------------------------------------------------------------------------
    GameObject NetScene::CreateReplicatedGameObject(const UUID& uuid, uint64_t ownerConnection)
    {
        GameObject obj = m_Scene->Instantiate(uuid);
        obj.GetComponent<UUIDComponent>().Uuid = uuid;

        if (!obj.HasComponent<NetIdentity>())
            obj.AddComponent<NetIdentity>();

        NetIdentity& ni = obj.GetComponent<NetIdentity>();
        ni.NetId = uuid;
        ni.Role = ENetRole::SimulatedProxy;
        ni.OwnerConnectionId = ownerConnection;
        ni.pScene = this;

        m_DynamicOwnership[uuid] = ownerConnection;

        return obj;
    }

    // -------------------------------------------------------------------------
    GameObject NetScene::GetGameObjectByUUID(const UUID& uuid)
    {
        return m_Scene->GetGameObject(uuid);
    }

    // -------------------------------------------------------------------------
    // Packet Routing
    // -------------------------------------------------------------------------
    void NetScene::ProcessPacket(NetConnection* sender, NetPacket& pkt)
    {
        switch (pkt.GetType())
        {
        case ENetPacketType::Spawn:    
            HandleSpawnPacket(sender, pkt); break;
        case ENetPacketType::Despawn:  
            HandleDespawnPacket(sender, pkt); break;
        case ENetPacketType::Replication:
            m_Replication->ProcessPacket(*this, pkt, sender); break;
        case ENetPacketType::RPC:
            m_RPC->Process(pkt, m_Driver->IsServer()); break;
        default:
            break;
        }
    }

    // -------------------------------------------------------------------------
    // Handle Server->Client Spawn
    // -------------------------------------------------------------------------
    void NetScene::HandleSpawnPacket(NetConnection* sender, NetPacket& pkt)
    {
        auto& s = pkt.GetSerializer();

        UUID netId = s.Read<UUID>();
        uint64_t owner = s.Read<uint64_t>();

        GameObject obj = CreateReplicatedGameObject(netId, owner);

        // Additional replicated fields can follow later
    }

    // -------------------------------------------------------------------------
    // Handle Server->Client Despawn
    // -------------------------------------------------------------------------
    void NetScene::HandleDespawnPacket(NetConnection* sender, NetPacket& pkt)
    {
        auto& s = pkt.GetSerializer();
        UUID netId = s.Read<UUID>();

        if (m_DynamicOwnership.find(netId) == m_DynamicOwnership.end())
            return;
        m_DynamicOwnership.erase(netId);

        GameObject obj = m_Scene->GetGameObject(netId);
        if (obj.IsValid())
            m_Scene->DestroyGameObject(obj);
    }

    // -------------------------------------------------------------------------
    // Create spawn packet and send
    // -------------------------------------------------------------------------
    void NetScene::SendSpawnTo(NetConnection* conn, const GameObject& obj)
    {
        const UUID& uuid = obj.GetComponent<UUIDComponent>().Uuid;
        uint64_t owner = 0;

        if (obj.HasComponent<NetIdentity>())
            owner = obj.GetComponent<NetIdentity>().OwnerConnectionId;

        NetPacket pkt(ENetPacketType::Spawn);
        pkt.Write(uuid);
        pkt.Write(owner);

        m_Driver->Send(conn, pkt, true);
    }

    // -------------------------------------------------------------------------
    void NetScene::SendDespawnTo(NetConnection* conn, const UUID& uuid)
    {
        NetPacket pkt(ENetPacketType::Despawn);
        pkt.Write(uuid);

        m_Driver->Send(conn, pkt, true);
    }

    void NetScene::BroadcastDespawn(const UUID& uuid)
    {
        if (m_DynamicOwnership.find(uuid) == m_DynamicOwnership.end())
            return;

        NetPacket pkt(ENetPacketType::Despawn);
        pkt.Write(uuid);

        m_Driver->Broadcast(pkt, true);

        m_DynamicOwnership.erase(uuid);
    }
}
