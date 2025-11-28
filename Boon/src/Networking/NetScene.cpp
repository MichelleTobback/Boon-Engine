#include "Networking/NetScene.h"
#include "Networking/NetDriver.h"
#include "Networking/NetIdentity.h"
#include "Networking/NetConnection.h"
#include "Networking/NetRepCore.h"
#include "Networking/NetRPC.h"

#include "Core/ServiceLocator.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Component/UUIDComponent.h"

#include "Event/EventBus.h"
#include "Networking/Events/NetConnectionEvent.h"

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
            m_Scene->GetOnComponentAdded() += [this](GameObject  obj, const BClass* cls) {BroadcastComponent(obj.GetUUID(), cls->hash, true); };
            m_Scene->GetOnComponentRemoved() += [this](GameObject  obj, const BClass* cls) {BroadcastComponent(obj.GetUUID(), cls->hash, false); };

            ServiceLocator::Get<EventBus>().Subscribe<NetConnectionEvent>([this](const NetConnectionEvent& e)
                {
                    for (auto& [obj, id] : m_DynamicOwnership)
                    {
                        GameObject instance = m_Scene->GetGameObject(obj);
                        SendSpawnTo(m_Driver->GetConnection(e.ConnectionId), instance);
                    }
                });
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

        NetIdentity& ni = gameObject.GetOrAddComponent<NetIdentity>();
        ni.NetId = id;
        ni.Role = m_Driver->IsServer() ? ENetRole::Authority : ENetRole::SimulatedProxy;
        ni.OwnerConnectionId = m_Driver->GetLocalConnectionId();
        ni.pScene = this;
        ni.bSpawned = true;
        ni.m_bOwner = true;

        if (ni.onNetAwake) ni.onNetAwake(gameObject, &ni);

        // Static objects are not added to dynamic map
    }

    // -------------------------------------------------------------------------
    // Server: spawn dynamic object
    // -------------------------------------------------------------------------
    GameObject NetScene::RegisterDynamicGameObject(GameObject gameObject, uint64_t ownerConnection)
    {
        if (!m_bRegisterDynamicObject)
            return GameObject();

        if (m_Driver->GetMode() == ENetDriverMode::Client)
        {
            // client never registers dynamic objects
            return gameObject;
        }

        auto& id = gameObject.GetComponent<UUIDComponent>().Uuid;

        NetIdentity& ni = gameObject.GetOrAddComponent<NetIdentity>();
        ni.NetId = id;
        ni.OwnerConnectionId = ownerConnection;
        ni.m_bOwner = ni.OwnerConnectionId == m_Driver->GetLocalConnectionId();
        ni.Role = ENetRole::Authority;
        ni.pScene = this;
        if (ni.onNetAwake) ni.onNetAwake(gameObject, &ni);

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

        NetIdentity& ni = obj.GetOrAddComponent<NetIdentity>();
        ni.NetId = uuid;
        ni.OwnerConnectionId = ownerConnection;
        ni.m_bOwner = ni.OwnerConnectionId == m_Driver->GetLocalConnectionId();
        ni.Role = ni.m_bOwner ? ENetRole::AutonomousProxy : ENetRole::SimulatedProxy;
        ni.pScene = this;
        if (ni.onNetAwake) ni.onNetAwake(obj, &ni);

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
        case ENetPacketType::Component:
            HandleComponentPacket(sender, pkt); break;
        case ENetPacketType::LoadScene:
            HandleLoadScenePacket(sender, pkt); break;
        case ENetPacketType::Replication:
            m_Replication->ProcessPacket(*this, pkt, sender); break;
        case ENetPacketType::RPC:
            m_RPC->Process(pkt, m_Driver->IsServer()); break;
        default:
            break;
        }
    }

    uint64_t NetScene::GetLocalConnectionID() const
    {
        return m_Driver->GetLocalConnectionId();
    }


    GameObject NetScene::InstantiateGameObject(uint64_t connectionId, UUID uuid)
    {
        m_bRegisterDynamicObject = false;
        GameObject instance = m_Scene->Instantiate(uuid);
        m_bRegisterDynamicObject = true;
        
        RegisterDynamicGameObject(instance, connectionId);

        return instance;
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
    // Handle Server->Client Component add/remove
    // -------------------------------------------------------------------------
    void NetScene::HandleComponentPacket(NetConnection* sender, NetPacket& pkt)
    {
        auto& s = pkt.GetSerializer();
        UUID netId = s.Read<UUID>();
        BClassID compId = s.Read<uint32_t>();
        bool add = s.Read<bool>();

        GameObject obj = m_Scene->GetGameObject(netId);
        if (!obj.IsValid())
            return;

        BClass* cls = BClassRegistry::Get().Find(compId);
        if (!cls)
            return;

        if (add && !obj.HasComponentByClass(cls))
        {
            obj.AddComponentFromClass(cls);
        }
        else if (obj.HasComponentByClass(cls))
        {
            obj.RemoveComponentByClass(cls);
        }
    }

    // -------------------------------------------------------------------------
    // Handle Server->Client Load scene
    // -------------------------------------------------------------------------
    void NetScene::HandleLoadScenePacket(NetConnection* sender, NetPacket& pkt)
    {
        auto& s = pkt.GetSerializer();
        SceneID sceneId = s.Read<SceneID>();

        SceneManager& scenes = ServiceLocator::Get<SceneManager>();
        if (!scenes.IsLoaded(sceneId))
            return;

        scenes.SetActiveScene(sceneId);
    }

    // -------------------------------------------------------------------------
    // Create spawn packet and send
    // -------------------------------------------------------------------------
    void NetScene::SendSpawnTo(NetConnection* conn, const GameObject& obj)
    {
        if (!m_bRegisterDynamicObject)
            return;

        if (m_Driver->GetMode() == ENetDriverMode::Client)
        {
            return;
        }

        auto& id = obj.GetComponent<UUIDComponent>().Uuid;

        const NetIdentity& ni = obj.GetComponent<NetIdentity>();

        NetPacket pkt(ENetPacketType::Spawn);
        pkt.Write(id);
        pkt.Write(ni.OwnerConnectionId);

        m_Driver->Send(conn, pkt, true);
    }

    // -------------------------------------------------------------------------
    void NetScene::SendDespawnTo(NetConnection* conn, const UUID& uuid)
    {
        NetPacket pkt(ENetPacketType::Despawn);
        pkt.Write(uuid);

        m_Driver->Send(conn, pkt, true);
    }

    void NetScene::SendComponentTo(NetConnection* conn, const UUID& uuid, const BClassID& component, bool add)
    {
        NetPacket pkt(ENetPacketType::Component);
        pkt.Write(uuid);
        pkt.Write(component);
        pkt.Write(add);

        m_Driver->Send(conn, pkt, true);
    }

    void NetScene::SendLoadSceneTo(NetConnection* conn, const SceneID& sceneId)
    {
        NetPacket pkt(ENetPacketType::LoadScene);
        pkt.Write(sceneId);

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

    void NetScene::BroadcastComponent(const UUID& uuid, const BClassID& component, bool add)
    {
        NetPacket pkt(ENetPacketType::Component);
        pkt.Write(uuid);
        pkt.Write(component);
        pkt.Write(add);

        m_Driver->Broadcast(pkt, true);
    }

    void NetScene::BroadcastLoadScene(const SceneID& sceneId)
    {
        NetPacket pkt(ENetPacketType::LoadScene);
        pkt.Write(sceneId);

        m_Driver->Broadcast(pkt, true);
    }
}
