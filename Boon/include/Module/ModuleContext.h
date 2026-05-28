#pragma once

namespace Boon
{
    class BClassRegistry;
    class NetRepRegistry;
    class ServiceRegistry;
    struct EngineContext;

    struct ModuleContext
    {
        BClassRegistry* BClasses = nullptr;
        NetRepRegistry* NetReps = nullptr;
        ServiceRegistry* ServiceRegistry = nullptr;

        EngineContext* EngineContext = nullptr;
    };
}