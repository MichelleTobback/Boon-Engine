#pragma once

#include "BuildProfile.h"

#include <filesystem>
#include <string>
#include <vector>

namespace BoonBuild
{
    class BuildProfileReader
    {
    public:
        bool ReadProfile(
            const std::filesystem::path& file,
            const std::string& requestedProfile,
            BuildProfile& outProfile) const;

        bool ReadAllProfiles(
            const std::filesystem::path& file,
            std::vector<BuildProfile>& outProfiles) const;
    };
}