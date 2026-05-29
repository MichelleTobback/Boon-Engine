#include "BuildProfileReader.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>

namespace BoonBuild
{
    bool BuildProfileReader::ReadAllProfiles(const std::filesystem::path& file, std::vector<BuildProfile>& outProfiles) const
    {
        outProfiles.clear();

        std::ifstream in(file);

        if (!in.is_open())
        {
            std::cerr << "Failed to open BuildProfiles.json:\n" << file << "\n";
            return false;
        }

        nlohmann::json j;
        in >> j;

        if (!j.contains("profiles"))
        {
            std::cerr << "BuildProfiles.json missing 'profiles'\n";
            return false;
        }

        for (auto it = j["profiles"].begin(); it != j["profiles"].end(); ++it)
        {
            BuildProfile profile{};
            profile.Name = it.key();

            const auto& value = it.value();

            profile.Configuration = value.value("configuration", "Debug");
            profile.Generator = value.value("generator", "Ninja");

            const std::string platformText = value.value("platform", "Windows");

            if (!TryParsePlatform(platformText, profile.Platform))
            {
                std::cerr << "Unknown build platform: " << platformText << "\n";
                return false;
            }

            outProfiles.push_back(profile);
        }

        return true;
    }

    bool BuildProfileReader::ReadProfile(const std::filesystem::path& file, const std::string& requestedProfile, BuildProfile& outProfile) const
    {
        std::ifstream in(file);

        if (!in.is_open())
        {
            std::cerr << "Failed to open BuildProfiles.json:\n" << file << "\n";
            return false;
        }

        nlohmann::json j;
        in >> j;

        const std::string profileName =
            requestedProfile.empty()
            ? j.value("defaultProfile", "Windows-Debug")
            : requestedProfile;

        if (!j.contains("profiles") || !j["profiles"].contains(profileName))
        {
            std::cerr << "Build profile not found: " << profileName << "\n";
            return false;
        }

        const auto& profile = j["profiles"][profileName];

        outProfile.Name = profileName;
        outProfile.Configuration = profile.value("configuration", "Debug");
        outProfile.Generator = profile.value("generator", "Ninja");

        const std::string platformText = profile.value("platform", "Windows");

        if (!TryParsePlatform(platformText, outProfile.Platform))
        {
            std::cerr << "Unknown build platform: " << platformText << "\n";
            return false;
        }

        return true;
    }
}