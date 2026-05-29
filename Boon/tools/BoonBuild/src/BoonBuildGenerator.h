#pragma once

#include <filesystem>

namespace BoonBuild
{
    class BoonBuildGenerator
    {
    public:
        bool Generate(const std::filesystem::path& root, const std::string& profileName) const;

    private:
        bool GenerateEngineModules(const std::filesystem::path& repoRoot, const std::string& profileName) const;
        bool GenerateProject(const std::filesystem::path& projectRoot, const std::string& profileName) const;
    };
}