#include "Module/ModuleInstance.h"
#include "Networking/NetworkingSubsystem.h"
#include "Core/EngineContext.h"
#include "Module/ModuleLibrary.h"
#include "BoonDebug/Logger.h"

namespace Boon
{
    class BoonNetworkModule final : public ModuleInstance
    {
    public:
        bool OnInitialize(ModuleContext& ctx) override
        {
            BOON_LOG("BoonNetworkModule::OnInitialize");
            ctx.EngineContext->Subsystems->Register<NetworkingSubsystem>(NetworkSettings{});

            return true;
        }

        void OnShutdown(ModuleContext& ctx) override
        {
            ctx.EngineContext->Subsystems->Unregister<NetworkingSubsystem>(*ctx.EngineContext);
        }
    };
}

BOON_IMPLEMENT_MODULE_INSTANCE(Boon::BoonNetworkModule)