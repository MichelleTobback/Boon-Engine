#pragma once
#include "Core/UUID.h"
#include "NetAuthority.h"
#include "Core/Boon.h"

#include <functional>

namespace Boon
{
    // ----------------------------------------------------------------------------------------
    // NetIdentity Component
    // Attach this component to ANY GameObject that participates in networking.
    //
    // The server assigns a unique NetId AND assigns a role.
    //  - NetId            = Unique replication identifier
    //  - OwnerConnection  = Which client owns this object
    //  - Role             = How this GameObject behaves on this machine
    // ----------------------------------------------------------------------------------------
    class NetScene;

    BCLASS()
    struct NetIdentity
    {
        UUID NetId;

        uint64_t OwnerConnectionId = 0;

        ENetRole Role = ENetRole::None;

        bool bReplicates = true;
        bool bSpawned = false;

        NetScene* pScene;

        std::function<void(GameObject, NetIdentity*)> onNetAwake;

        bool IsAuthority() const { return Role == ENetRole::Authority; }
        bool IsAutonomousProxy() const { return Role == ENetRole::AutonomousProxy; }
        bool IsSimulatedProxy() const { return Role == ENetRole::SimulatedProxy; }

        bool IsOwnedBy(uint64_t conn) const { return OwnerConnectionId == conn; }
        bool IsOwner() const 
        { 
            return m_bOwner; 
        }

    private:
        friend class NetScene;
        bool m_bOwner;
    };
}
