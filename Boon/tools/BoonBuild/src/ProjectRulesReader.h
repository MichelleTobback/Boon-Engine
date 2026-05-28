#pragma once

#include "ProjectRules.h"

#include <filesystem>

namespace BoonBuild
{
    class ProjectRulesReader
    {
    public:
        bool ReadProjectRules(const std::filesystem::path& file, ProjectRules& outRules) const;
    };
}