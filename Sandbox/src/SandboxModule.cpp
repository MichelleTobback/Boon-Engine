#include "SandboxAPI.h"
#include "Module/ModuleAPI.h"
#include "Reflection/BClass.h"
#include "Networking/NetRepRegistry.h"
#include "Core/ServiceLocator.h"

#include "Reflection/BClassBase.h"

namespace Boon
{
    static ModuleInfo s_ModuleInfo{
        "SandboxGame",
        1,
        0
    };
}

extern "C"
{
    SANDBOX_API const Boon::ModuleInfo* Boon_GetModuleInfo()
    {
        return &Boon::s_ModuleInfo;
    }

    SANDBOX_API bool Boon_RegisterModule(Boon::ModuleContext* ctx)
    {
        if (!ctx || !ctx->BClasses || !ctx->NetReps)
            return false;

        Boon::ServiceLocator::SetRegistry(ctx->ServiceRegistry);
        Boon::BClassRegistry::SetRegistry(ctx->BClasses);
        Boon::NetRepRegistry::SetRegistry(ctx->NetReps);

        Boon::RegisterGeneratedClasses_SandboxGame(*ctx->BClasses, *ctx->NetReps);

        return true;
    }

    SANDBOX_API void Boon_UnregisterModule(Boon::ModuleContext* ctx)
    {
        if (!ctx || !ctx->BClasses || !ctx->NetReps)
            return;

        Boon::UnregisterGeneratedClasses_SandboxGame(*ctx->BClasses, *ctx->NetReps);

        Boon::ServiceLocator::SetRegistry(nullptr);
        Boon::BClassRegistry::SetRegistry(nullptr);
        Boon::NetRepRegistry::SetRegistry(nullptr);
    }
}