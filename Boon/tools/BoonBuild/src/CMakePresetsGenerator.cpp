#include "CMakePresetsGenerator.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>

namespace BoonBuild
{
    namespace
    {
        std::string ToCMakeBuildType(const std::string& configuration)
        {
            if (configuration == "Release")
                return "Release";

            if (configuration == "RelWithDebInfo")
                return "RelWithDebInfo";

            if (configuration == "MinSizeRel")
                return "MinSizeRel";

            return "Debug";
        }

        std::string DefaultGeneratorForPlatform(BuildPlatform platform)
        {
            switch (platform)
            {
            case BuildPlatform::Windows:
                return "Ninja";

            case BuildPlatform::Linux:
                return "Ninja";

            case BuildPlatform::Web:
                return "Ninja";

            case BuildPlatform::Android:
                return "Ninja";
            }

            return "Ninja";
        }
    }

    bool CMakePresetsGenerator::Generate(
        const std::filesystem::path& root,
        const std::filesystem::path& repoRoot,
        const std::vector<BuildProfile>& profiles) const
    {
        nlohmann::json configurePresets = nlohmann::json::array();
        nlohmann::json buildPresets = nlohmann::json::array();

        for (const BuildProfile& profile : profiles)
        {
            const std::string generator = profile.Generator.empty()
                ? DefaultGeneratorForPlatform(profile.Platform)
                : profile.Generator;

            nlohmann::json configure;
            configure["name"] = profile.Name;
            configure["displayName"] = profile.Name;
            configure["generator"] = generator;
            configure["binaryDir"] = "${sourceDir}/out/build/" + profile.Name;

            configure["cacheVariables"] = {
                { "CMAKE_BUILD_TYPE", ToCMakeBuildType(profile.Configuration) },
                { "BOON_REPO_ROOT", repoRoot.generic_string() },
                { "BOON_BUILD_PROFILE", profile.Name }
            };

            if (profile.Platform == BuildPlatform::Web)
            {
                configure["cacheVariables"]["BOON_PLATFORM_WEB"] = "ON";
            }

            if (profile.Platform == BuildPlatform::Android)
            {
                configure["cacheVariables"]["BOON_PLATFORM_ANDROID"] = "ON";
            }

            configurePresets.push_back(configure);

            nlohmann::json build;
            build["name"] = profile.Name;
            build["configurePreset"] = profile.Name;
            build["configuration"] = ToCMakeBuildType(profile.Configuration);

            buildPresets.push_back(build);
        }

        nlohmann::json rootJson;
        rootJson["version"] = 6;
        rootJson["configurePresets"] = configurePresets;
        rootJson["buildPresets"] = buildPresets;

        const std::filesystem::path output = root / "CMakePresets.json";

        std::ofstream out(output, std::ios::binary | std::ios::trunc);

        if (!out.is_open())
        {
            std::cerr << "Failed to write CMakePresets.json:\n"
                << output << "\n";
            return false;
        }

        out << rootJson.dump(4);
        out << "\n";

        std::cout << "Generated: " << output << "\n";
        return true;
    }
}