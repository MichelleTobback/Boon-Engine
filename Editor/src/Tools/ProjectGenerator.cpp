#include "Tools/ProjectGenerator.h"
#include "Project/ProjectLoader.h"
#include "Tools/TemplateProcessor.h"
#include "Utils/ProcessRunner.h"
#include "BoonDebug/Logger.h"

#include <algorithm>
#include <fstream>
#include <future>
#include <sstream>

using namespace Boon;

namespace BoonEditor
{
    namespace
    {
        static std::string Quote(const std::filesystem::path& path)
        {
            return "\"" + path.string() + "\"";
        }

        static std::filesystem::path GetRepoRootFromEngineRoot(const std::filesystem::path& engineRoot)
        {
            // If EngineRoot points to .../Boon-Engine/Boon, return .../Boon-Engine
            if (std::filesystem::exists(engineRoot / "tools" / "BClassGenerator" / "BoonReflection.cmake"))
                return engineRoot.parent_path();

            // If EngineRoot already points to .../Boon-Engine
            return engineRoot;
        }

        static std::filesystem::path GetBoonBuildExe(const std::filesystem::path& repoRoot)
        {
            return repoRoot / "bin" / "Debug" / "Tools" / "BoonBuild.exe";
        }
    }

    ProjectConfig ProjectGenerator::Generate(const ProjectGeneratorSettings& desc)
    {
        static std::future<void> sProjectBuildFuture;

        ProjectConfig project{};

        InitializeProject(project, desc);
        GenerateFilesFromTemplates(desc.TemplateFolder, project, desc);

        sProjectBuildFuture = std::async(std::launch::async, [project]()
            {
                std::string log{};

                if (!GenerateBoonBuildProject(project, log))
                {
                    BOON_LOG_ERROR("BoonBuild project generation failed!");
                    return;
                }

                if (!Configure(project, project.Runtime.ProjectRoot / "build", log))
                {
                    BOON_LOG_ERROR("CMake configure failed!");
                    return;
                }

                if (!Build(project.Runtime.ProjectRoot / "build", "Debug", log))
                {
                    BOON_LOG_ERROR("Build failed!");
                    return;
                }

                BOON_LOG("Build complete");
            });

        return project;
    }

    void ProjectGenerator::InitializeProject(ProjectConfig& project, const ProjectGeneratorSettings& desc)
    {
        BOON_LOG("Creating new Project ...");

        ProjectLoader::ApplyDefaults(project);

        project.Name = desc.Name;
        project.Runtime.GameModule = desc.Name;
        project.Runtime.EnabledModules = { desc.Name };
        project.Runtime.ProjectRoot = std::filesystem::path(desc.Location) / desc.Name;

        project.Runtime.Window.Title = desc.Name + " Project";

        const std::filesystem::path location = project.Runtime.ProjectRoot;

        std::filesystem::create_directories(location);
        std::filesystem::create_directories(location / "Assets");
        std::filesystem::create_directories(location / "src");
        std::filesystem::create_directories(location / "include");
        std::filesystem::create_directories(location / "generated");

        ProjectLoader::SaveToFile(location, project);
    }

    void ProjectGenerator::GenerateFilesFromTemplates(
        const std::filesystem::path& templatesLoc,
        const ProjectConfig& project,
        const ProjectGeneratorSettings& desc)
    {
        if (!std::filesystem::exists(templatesLoc))
            return;

        BOON_LOG("Generating project files ...");

        if (templatesLoc.has_extension())
        {
            ProcessTemplate(templatesLoc, project, desc);
            return;
        }

        for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(templatesLoc))
        {
            if (!dirEntry.is_regular_file())
                continue;

            ProcessTemplate(dirEntry.path(), project, desc);
        }
    }

    void ProjectGenerator::ProcessTemplate(
        const std::filesystem::path& from,
        const ProjectConfig& project,
        const ProjectGeneratorSettings& desc)
    {
        std::ifstream file(from, std::ios::binary);

        if (!file.is_open())
        {
            BOON_LOG_WARN("Failed to open template: {}, file skipped", from.string());
            return;
        }

        std::stringstream ss;
        ss << file.rdbuf();

        std::string content = ss.str();

        if (content.empty())
        {
            BOON_LOG_WARN("Template is empty: {}, file skipped", from.string());
            return;
        }

        std::string uppercaseName = desc.Name;
        std::transform(uppercaseName.begin(), uppercaseName.end(), uppercaseName.begin(), ::toupper);

        std::string lowercaseName = desc.Name;
        std::transform(lowercaseName.begin(), lowercaseName.end(), lowercaseName.begin(), ::tolower);

        const std::filesystem::path repoRoot = GetRepoRootFromEngineRoot(project.Runtime.EngineRoot);

        TemplateContext context{};
        context.Set("PROJECT_NAME", desc.Name);
        context.Set("PROJECT_NAME_UPPER", uppercaseName);
        context.Set("PROJECT_NAME_LOWER", lowercaseName);
        context.Set("PROJECT_ROOT", project.Runtime.ProjectRoot.string());
        context.Set("BOON_REPO_ROOT", repoRoot.string());
        context.Set("BOON_ENGINE_ROOT", (repoRoot / "Boon").string());
        context.Set("BOON_ENGINE_VERSION", std::to_string(project.Version));

        TemplateProcessor::ProcessTemplateBlocks(
            content,
            "@@",
            context,
            [project](const std::filesystem::path& path, std::string_view c) -> bool
            {
                const std::filesystem::path loc = project.Runtime.ProjectRoot / path;

                std::filesystem::create_directories(loc.parent_path());

                std::ofstream f(loc, std::ios::out | std::ios::binary | std::ios::trunc);

                if (!f.is_open())
                {
                    BOON_LOG_ERROR("Failed to write generated file: {}", loc.string());
                    return false;
                }

                f.write(c.data(), static_cast<std::streamsize>(c.size()));

                BOON_LOG("Generated {}", loc.string());

                return true;
            });
    }

    bool ProjectGenerator::GenerateBoonBuildProject(const ProjectConfig& project, std::string& outLog)
    {
        BOON_LOG("Running BoonBuild project generation ...");

        const std::filesystem::path repoRoot = GetRepoRootFromEngineRoot(project.Runtime.EngineRoot);
        const std::filesystem::path boonBuildExe = GetBoonBuildExe(repoRoot);

        if (!std::filesystem::exists(boonBuildExe))
        {
            BOON_LOG_ERROR("BoonBuild.exe not found: {}", boonBuildExe.string());
            return false;
        }

        const std::string command =
            Quote(boonBuildExe) + " " +
            Quote(project.Runtime.ProjectRoot);

        ProcessResult result = ProcessRunner::Run(
            command,
            [](const std::string& chunk)
            {
                BOON_LOG(chunk);
            });

        outLog += result.Output;

        if (result.ExitCode != 0)
        {
            BOON_LOG_ERROR(result.Output);
            return false;
        }

        return true;
    }

    bool ProjectGenerator::Configure(
        const ProjectConfig& project,
        const std::filesystem::path& buildDir,
        std::string& outLog)
    {
        BOON_LOG("Configuring project ...");

        const std::filesystem::path repoRoot = GetRepoRootFromEngineRoot(project.Runtime.EngineRoot);

        const std::string command =
            "cmake -S " + Quote(project.Runtime.ProjectRoot) +
            " -B " + Quote(buildDir) +
            " -G Ninja" +
            " -DCMAKE_BUILD_TYPE=Debug" +
            " -DBOON_REPO_ROOT:PATH=" + Quote(repoRoot);

        ProcessResult result = ProcessRunner::Run(
            command,
            [](const std::string& chunk)
            {
                BOON_LOG(chunk);
            });

        outLog += result.Output;

        if (result.ExitCode != 0)
        {
            BOON_LOG_ERROR(result.Output);
            return false;
        }

        return true;
    }

    bool ProjectGenerator::Build(
        const std::filesystem::path& buildDir,
        const std::string& config,
        std::string& outLog)
    {
        BOON_LOG("Building project ...");

        const std::string command =
            "cmake --build " + Quote(buildDir) +
            " --config " + config;

        ProcessResult result = ProcessRunner::Run(
            command,
            [](const std::string& chunk)
            {
                BOON_LOG(chunk);
            });

        outLog += result.Output;

        if (result.ExitCode != 0)
        {
            BOON_LOG_ERROR(result.Output);
            return false;
        }

        return true;
    }
}