#pragma once

#include "Module/ModuleAPI.h"

namespace Boon
{
    class ModuleInstance
    {
    public:
        virtual ~ModuleInstance() = default;

        virtual bool OnInitialize(ModuleContext& context)
        {
            return true;
        }

        virtual void OnShutdown(ModuleContext& context)
        {
        }

        virtual void OnStart(ModuleContext& context)
        {
        }

        virtual void OnStop(ModuleContext& context)
        {
        }

        virtual void OnReload(ModuleContext& context)
        {
        }
    };
}

// TODO: later replace with reflection discovery.
#define BOON_IMPLEMENT_MODULE_INSTANCE(ModuleName, Type)                  \
    Boon::ModuleInstance* ModuleName##_CreateUserModuleInstance()         \
    {                                                                     \
        return new Type();                                                \
    }                                                                     \
                                                                          \
    void ModuleName##_DestroyUserModuleInstance(Boon::ModuleInstance* inst)\
    {                                                                     \
        delete inst;                                                      \
    }