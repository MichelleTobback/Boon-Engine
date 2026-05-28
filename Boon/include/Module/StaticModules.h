#pragma once

#include "Module/ModuleAPI.h"

namespace Boon
{
    bool RegisterStaticModules(ModuleContext& context);
    void UnregisterStaticModules(ModuleContext& context);
}