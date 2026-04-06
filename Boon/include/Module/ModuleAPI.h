#pragma once

#include <string>

namespace Boon
{
    class BClassRegistry;
    class NetRepRegistry;
    class AssetImporterRegistry;
    class ServiceRegistry;

    struct ModuleContext
    {
        BClassRegistry* BClasses = nullptr;
        NetRepRegistry* NetReps = nullptr;
        ServiceRegistry* ServiceRegistry = nullptr;
    };

    struct ModuleInfo
    {
        const char* Name = "";
        int VersionMajor = 1;
        int VersionMinor = 0;
    };

    using GetModuleInfoFn = const ModuleInfo* (*)();
    using RegisterModuleFn = bool (*)(ModuleContext*);
    using UnregisterModuleFn = void (*)(ModuleContext*);
}