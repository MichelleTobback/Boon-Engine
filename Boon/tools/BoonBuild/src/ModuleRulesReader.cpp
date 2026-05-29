#include "ModuleRulesReader.h"

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

    bool ModuleRulesReader::ReadModuleRules(const std::filesystem::path& file, ModuleRules& outRules) const
    {
        std::ifstream in(file);

        if (!in.is_open())
            return false;

        json j;
        in >> j;

        outRules.Name = j.value("name", "");
        outRules.Type = j.value("type", "Runtime");
        outRules.Output = j.value("output", "STATIC");
        outRules.Reflection = j.value("reflection", true);
        outRules.ModuleInstance = j.value("moduleInstance", "");

        outRules.PublicDependencies = ReadStringArray(j, "publicDependencies");
        outRules.PublicIncludePaths = ReadStringArray(j, "publicIncludePaths");
        outRules.PrivateIncludePaths = ReadStringArray(j, "privateIncludePaths");
        outRules.PublicDefinitions = ReadStringArray(j, "publicDefinitions");
        outRules.PrivateDefinitions = ReadStringArray(j, "privateDefinitions");
        outRules.SystemLibraries = ReadStringArray(j, "systemLibraries");
        outRules.ThirdParty = ReadStringArray(j, "thirdParty");
        outRules.ModuleDependencies = ReadStringArray(j, "moduleDependencies");
        outRules.Platforms = ReadStringArray(j, "platforms");

        return !outRules.Name.empty();
    }

    bool ModuleRulesReader::ReadAllModules(const std::filesystem::path& modulesDir, std::vector<ModuleRules>& outModules) const
    {
        if (!std::filesystem::exists(modulesDir))
        {
            std::cerr << "Modules directory not found:\n" << modulesDir << "\n";
            return false;
        }

        for (const auto& entry : std::filesystem::directory_iterator(modulesDir))
        {
            if (!entry.is_directory())
                continue;

            if (entry.path().filename() == "generated")
                continue;

            const std::filesystem::path rulesFile = entry.path() / "BoonModule.json";

            if (!std::filesystem::exists(rulesFile))
                continue;

            ModuleRules rules;

            if (!ReadModuleRules(rulesFile, rules))
            {
                std::cerr << "Failed to read module rules:\n" << rulesFile << "\n";
                return false;
            }

            outModules.push_back(std::move(rules));
        }

        return true;
    }
}