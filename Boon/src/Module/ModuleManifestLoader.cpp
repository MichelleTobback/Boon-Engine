#include "Module/ModuleManifestLoader.h"

#include <nlohmann/json.hpp>

#include <fstream>

namespace Boon
{
    namespace
    {
        using json = nlohmann::json;

        ModuleBinaryType ParseType(const std::string& value)
        {
            if (value == "SHARED")
                return ModuleBinaryType::Shared;

            return ModuleBinaryType::Static;
        }

        ModuleManifestEntry ParseEntry(const json& j)
        {
            ModuleManifestEntry entry;

            entry.Name = j.value("name", "");
            entry.Type = ParseType(j.value("type", "STATIC"));
            entry.Directory = j.value("directory", "");
            entry.Subdirectory = j.value("subdirectory", "");
            entry.LoadOnStartup = j.value("loadOnStartup", false);

            return entry;
        }
    }

    bool ModuleManifestLoader::Load(const std::filesystem::path& path, ModuleManifest& outManifest)
    {
        std::ifstream in(path);

        if (!in.is_open())
            return false;

        json j;
        in >> j;

        outManifest.Project = j.value("project", "");

        if (j.contains("gameModule"))
            outManifest.GameModule = ParseEntry(j.at("gameModule"));

        if (j.contains("staticModules"))
        {
            for (const auto& item : j.at("staticModules"))
                outManifest.StaticModules.push_back(ParseEntry(item));
        }

        if (j.contains("dynamicModules"))
        {
            for (const auto& item : j.at("dynamicModules"))
                outManifest.DynamicModules.push_back(ParseEntry(item));
        }

        return true;
    }
}