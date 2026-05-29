#pragma once

#include "ModuleRules.h"

#include <string>
#include <vector>

namespace BoonBuild
{
    class ModuleDependencyGraph
    {
    public:
        static bool Sort(
            const std::vector<ModuleRules>& modules,
            std::vector<ModuleRules>& outSortedModules,
            std::string& outError);
    };
}