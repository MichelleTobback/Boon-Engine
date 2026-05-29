#pragma once

#include "BuildProfile.h"

#include <filesystem>
#include <vector>

namespace BoonBuild
{
    class CMakePresetsGenerator
    {
    public:
        bool Generate(const std::filesystem::path& root, const std::filesystem::path& sdkRoot, const std::vector<BuildProfile>& profiles) const;
    };
}