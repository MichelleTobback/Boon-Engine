#pragma once

#include "Module/ModuleContext.h"

namespace Boon
{
	bool RegisterStaticModules(ModuleContext& context);
	void StartStaticModules(ModuleContext& context);
	void StopStaticModules(ModuleContext& context);
	void UnregisterStaticModules(ModuleContext& context);
}