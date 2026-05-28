#include "ProjectRulesReader.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>

namespace BoonBuild
{
    namespace
    {
        using json = nlohmann::json;

        std::vector<std::string> ReadStringArray(const json& j, const char* key)
        {
            std::vector<std::string> result;

            if (!j.contains(key))
                return result;

            for (const auto& value : j.at(key))
                result.push_back(value.get<std::string>());

            return result;
        }
    }

    bool ProjectRulesReader::ReadProjectRules(const std::filesystem::path& file, ProjectRules& outRules) const
    {
        std::ifstream in(file);

        if (!in.is_open())
        {
            std::cerr << "Failed to open project rules file:\n" << file << "\n";
            return false;
        }

        json j;
        in >> j;

        outRules.Name = j.value("name", "");
        outRules.Output = j.value("output", "SHARED");
        outRules.SourceDir = j.value("sourceDir", ".");
        outRules.Reflection = j.value("reflection", true);
        outRules.MinimalReflection = j.value("minimalReflection", true);
        outRules.ModuleInstance = j.value("moduleInstance", "");

        outRules.EngineModules = ReadStringArray(j, "engineModules");
        outRules.GameModules = ReadStringArray(j, "gameModules");

        outRules.PublicIncludePaths = ReadStringArray(j, "publicIncludePaths");
        outRules.PrivateIncludePaths = ReadStringArray(j, "privateIncludePaths");

        outRules.PublicDefinitions = ReadStringArray(j, "publicDefinitions");
        outRules.PrivateDefinitions = ReadStringArray(j, "privateDefinitions");

        outRules.PublicDependencies = ReadStringArray(j, "publicDependencies");
        outRules.PrivateDependencies = ReadStringArray(j, "privateDependencies");

        outRules.AssetDirectories = ReadStringArray(j, "assetDirectories");

        if (outRules.Name.empty())
        {
            std::cerr << "Project rules missing required field: name\n";
            return false;
        }

        return true;
    }
}