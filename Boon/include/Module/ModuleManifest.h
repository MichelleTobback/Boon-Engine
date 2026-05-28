#pragma once

#include <string>
#include <vector>

namespace Boon
{
    enum class ModuleBinaryType
    {
        Static,
        Shared
    };

    struct ModuleManifestEntry
    {
        std::string Name;
        ModuleBinaryType Type = ModuleBinaryType::Static;
        std::string Directory;
        std::string Subdirectory;
        bool LoadOnStartup = false;
    };

    struct ModuleManifest
    {
        std::string Project;
        ModuleManifestEntry GameModule;
        std::vector<ModuleManifestEntry> StaticModules;
        std::vector<ModuleManifestEntry> DynamicModules;
    };
}