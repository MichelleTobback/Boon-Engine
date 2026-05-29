#pragma once

#include <Project/ProjectConfig.h>

#include <filesystem>
#include <string>

using namespace Boon;

namespace BoonEditor
{
    struct ProjectGeneratorSettings
    {
        std::string Name;
        std::string Location;
        std::string TemplateFolder;
        std::filesystem::path EngineRoot;

        std::string BuildProfile = "Windows-Debug";
        bool BuildAfterGenerate = true;
    };

    class ProjectGenerator final
    {
    public:
        static ProjectConfig Generate(const ProjectGeneratorSettings& desc);

    private:
        static void InitializeProject(ProjectConfig& project, const ProjectGeneratorSettings& desc);

        static void GenerateFilesFromTemplates(
            const std::filesystem::path& templatesLoc,
            const ProjectConfig& project,
            const ProjectGeneratorSettings& desc);

        static void ProcessTemplate(
            const std::filesystem::path& from,
            const ProjectConfig& project,
            const ProjectGeneratorSettings& desc);

        static bool GenerateBoonBuildProject(
            const ProjectConfig& project,
            const std::string& profile,
            std::string& outLog);

        static bool Configure(
            const ProjectConfig& project,
            const std::string& profile,
            std::string& outLog);

        static bool Build(
            const ProjectConfig& project,
            const std::string& profile,
            std::string& outLog);
    };
}