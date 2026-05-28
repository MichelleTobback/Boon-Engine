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
#define BOON_IMPLEMENT_MODULE_INSTANCE(Type)                         \
    Boon::ModuleInstance* Boon_CreateUserModuleInstance()            \
    {                                                                \
        return new Type();                                           \
    }                                                                \
                                                                     \
    void Boon_DestroyUserModuleInstance(Boon::ModuleInstance* inst)  \
    {                                                                \
        delete inst;                                                 \
    }