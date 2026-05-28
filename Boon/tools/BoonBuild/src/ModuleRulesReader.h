#pragma once

#include "ModuleRules.h"

#include <filesystem>
#include <vector>

namespace BoonBuild
{
    class ModuleRulesReader
    {
    public:
        bool ReadModuleRules(const std::filesystem::path& file, ModuleRules& outRules) const;
        bool ReadAllModules(const std::filesystem::path& modulesDir, std::vector<ModuleRules>& outModules) const;
    };
}