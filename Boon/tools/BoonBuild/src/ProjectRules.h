#pragma once

#include <string>
#include <vector>

namespace BoonBuild
{
    struct ProjectRules
    {
        std::string Name;
        std::string Output = "SHARED";
        std::string SourceDir = ".";
        std::string ModuleInstance;

        bool Reflection = true;
        bool MinimalReflection = true;

        std::vector<std::string> EngineModules;
        std::vector<std::string> GameModules;

        std::vector<std::string> PublicIncludePaths;
        std::vector<std::string> PrivateIncludePaths;

        std::vector<std::string> PublicDefinitions;
        std::vector<std::string> PrivateDefinitions;

        std::vector<std::string> PublicDependencies;
        std::vector<std::string> PrivateDependencies;

        std::vector<std::string> AssetDirectories;
    };
}