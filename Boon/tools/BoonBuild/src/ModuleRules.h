#pragma once

#include <string>
#include <vector>

namespace BoonBuild
{
    struct ModuleRules
    {
        std::string Name;
        std::string Type = "Runtime";
        std::string Output = "STATIC";
        std::string ModuleInstance;

        bool Reflection = true;

        std::vector<std::string> PublicDependencies;
        std::vector<std::string> PublicIncludePaths;
        std::vector<std::string> PrivateIncludePaths;
        std::vector<std::string> PublicDefinitions;
        std::vector<std::string> PrivateDefinitions;
        std::vector<std::string> SystemLibraries;
        std::vector<std::string> ThirdParty;
        std::vector<std::string> ModuleDependencies;
    };
}