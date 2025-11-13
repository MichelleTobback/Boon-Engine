#pragma once

namespace Boon
{
    enum class ENetDriverMode
    {
        Standalone,        // no networking
        DedicatedServer,   // server with no local player
        ListenServer,      // server hosting + local player
        Client             // remote client
    };

    enum class ENetRole
    {
        Authority,          // Server owns this object
        AutonomousProxy,    // Owned by this client
        SimulatedProxy,     // Replicated from server
        None                // Not networked yet
    };
}