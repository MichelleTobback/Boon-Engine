#pragma once
#include "Module/ModuleContext.h"

namespace Boon
{
    class ModuleInstance;

    struct ModuleInfo
    {
        const char* Name = "";
        int VersionMajor = 1;
        int VersionMinor = 0;
    };

    struct ModuleRegistration
    {
        bool Success = false;
        ModuleInstance* Instance = nullptr;
    };

    using GetModuleInfoFn = const ModuleInfo* (*)();
    using RegisterModuleFn = ModuleRegistration(*)(ModuleContext*);
    using UnregisterModuleFn = void (*)(ModuleContext*, ModuleInstance*);
}

#ifdef _WIN32
    #define BOON_MODULE_EXPORT extern "C" __declspec(dllexport)
#else
    #define BOON_MODULE_EXPORT extern "C" __attribute__((visibility("default")))
#endif