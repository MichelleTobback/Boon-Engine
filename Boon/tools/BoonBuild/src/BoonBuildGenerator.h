#pragma once

#include <filesystem>

namespace BoonBuild
{
    class BoonBuildGenerator
    {
    public:
        bool Generate(const std::filesystem::path& root) const;

    private:
        bool GenerateEngineModules(const std::filesystem::path& repoRoot) const;
        bool GenerateProject(const std::filesystem::path& projectRoot) const;
    };
}