#include "CMakePresetsGenerator.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace Boon;
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

        nlohmann::json MakePlatformCacheVariables(BuildPlatform platform)
        {
            nlohmann::json vars = nlohmann::json::object();

            vars["BOON_PLATFORM_WINDOWS"] = "OFF";
            vars["BOON_PLATFORM_LINUX"] = "OFF";
            vars["BOON_PLATFORM_WEB"] = "OFF";
            vars["BOON_PLATFORM_ANDROID"] = "OFF";

            switch (platform)
            {
            case BuildPlatform::Windows:
                vars["BOON_PLATFORM_WINDOWS"] = "ON";
                break;

            case BuildPlatform::Linux:
                vars["BOON_PLATFORM_LINUX"] = "ON";
                break;

            case BuildPlatform::Web:
                vars["BOON_PLATFORM_WEB"] = "ON";
                break;

            case BuildPlatform::Android:
                vars["BOON_PLATFORM_ANDROID"] = "ON";
                break;
            }

            return vars;
        }
    }

    bool CMakePresetsGenerator::Generate(const std::filesystem::path& root, const std::filesystem::path& sdkRoot, const std::vector<BuildProfile>& profiles) const
    {
        nlohmann::json configurePresets = nlohmann::json::array();
        nlohmann::json buildPresets = nlohmann::json::array();

        const std::filesystem::path absoluteSdkRoot =
            std::filesystem::absolute(sdkRoot);

        for (const BuildProfile& profile : profiles)
        {
            const std::string generator =
                profile.Generator.empty()
                ? DefaultGeneratorForPlatform(profile.Platform)
                : profile.Generator;

            nlohmann::json cacheVariables = nlohmann::json::object();

            cacheVariables["CMAKE_BUILD_TYPE"] =
                ToCMakeBuildType(profile.Configuration);

            cacheVariables["BOON_SDK_ROOT"] =
                absoluteSdkRoot.generic_string();

            // Backwards-compatible while the rest of the CMake code still uses BOON_REPO_ROOT.
            cacheVariables["BOON_REPO_ROOT"] =
                absoluteSdkRoot.generic_string();

            cacheVariables["BOON_BUILD_PROFILE"] =
                profile.Name;

            nlohmann::json platformVars =
                MakePlatformCacheVariables(profile.Platform);

            for (auto it = platformVars.begin(); it != platformVars.end(); ++it)
                cacheVariables[it.key()] = it.value();

            nlohmann::json configure;
            configure["name"] = profile.Name;
            configure["displayName"] = profile.Name;
            configure["generator"] = generator;
            configure["binaryDir"] = "${sourceDir}/out/build/" + profile.Name;
            configure["cacheVariables"] = cacheVariables;

            configurePresets.push_back(configure);

            nlohmann::json build;
            build["name"] = profile.Name;
            build["configurePreset"] = profile.Name;
            build["configuration"] = ToCMakeBuildType(profile.Configuration);

            buildPresets.push_back(build);
        }

        nlohmann::json output;
        output["version"] = 6;
        output["configurePresets"] = configurePresets;
        output["buildPresets"] = buildPresets;

        const std::filesystem::path outputPath =
            root / "CMakePresets.json";

        std::ofstream out(
            outputPath,
            std::ios::binary | std::ios::trunc);

        if (!out.is_open())
        {
            std::cerr << "Failed to write CMakePresets.json:\n"
                << outputPath << "\n";
            return false;
        }

        out << output.dump(4);
        out << "\n";

        std::cout << "Generated: "
            << outputPath << "\n";

        return true;
    }
}