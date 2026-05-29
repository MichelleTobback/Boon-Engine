#include "Tools/ProjectGenerator.h"

#include "Project/ProjectLoader.h"
#include "Tools/TemplateProcessor.h"
#include "Process/ProcessRunner.h"
#include "BoonDebug/Logger.h"

#include <algorithm>
#include <cstdlib>
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

        static std::string QuoteArg(const std::string& value)
        {
            return "\"" + value + "\"";
        }

        static std::filesystem::path GetEnvPath(const char* name)
        {
            const char* value = std::getenv(name);

            if (!value || !*value)
                return {};

            return std::filesystem::path(value);
        }

        static bool IsSourceRepoRoot(const std::filesystem::path& root)
        {
            return std::filesystem::exists(root / "Boon" / "CMakeLists.txt")
                && std::filesystem::exists(root / "Boon" / "tools");
        }

        static bool IsInnerBoonRoot(const std::filesystem::path& root)
        {
            return std::filesystem::exists(root / "tools" / "BClassGenerator" / "BoonReflection.cmake");
        }

        static std::filesystem::path NormalizeSdkRoot(std::filesystem::path root)
        {
            if (root.empty())
                return {};

            root = std::filesystem::absolute(root);

            if (IsInnerBoonRoot(root))
                return root.parent_path();

            return root;
        }

        static std::filesystem::path ResolveSdkRoot(const ProjectConfig& project)
        {
            if (!project.Runtime.EngineRoot.empty())
                return NormalizeSdkRoot(project.Runtime.EngineRoot);

            if (auto sdkRoot = GetEnvPath("BOON_SDK_ROOT"); !sdkRoot.empty())
                return NormalizeSdkRoot(sdkRoot);

            if (auto engineRoot = GetEnvPath("BOON_ENGINE_ROOT"); !engineRoot.empty())
                return NormalizeSdkRoot(engineRoot);

            if (auto installRoot = GetEnvPath("BOON_INSTALL_ROOT"); !installRoot.empty())
            {
                const auto sdk = installRoot / "SDK";

                if (std::filesystem::exists(sdk / "Boon" / "CMakeLists.txt"))
                    return NormalizeSdkRoot(sdk);

                return NormalizeSdkRoot(installRoot);
            }

            return {};
        }

        static std::filesystem::path ResolveInstallRoot(const ProjectConfig& project)
        {
            if (auto installRoot = GetEnvPath("BOON_INSTALL_ROOT"); !installRoot.empty())
                return std::filesystem::absolute(installRoot);

            if (auto sdkRoot = GetEnvPath("BOON_SDK_ROOT"); !sdkRoot.empty())
                return std::filesystem::absolute(sdkRoot);

            if (auto engineRoot = GetEnvPath("BOON_ENGINE_ROOT"); !engineRoot.empty())
                return NormalizeSdkRoot(engineRoot);

            if (!project.Runtime.EngineRoot.empty())
                return NormalizeSdkRoot(project.Runtime.EngineRoot);

            return {};
        }

        static std::filesystem::path GetBoonBuildExe(const ProjectConfig& project)
        {
            const auto installRoot = ResolveInstallRoot(project);
            const auto sdkRoot = ResolveSdkRoot(project);

#ifdef _WIN32
            constexpr const char* exeName = "BoonBuild.exe";
#else
            constexpr const char* exeName = "BoonBuild";
#endif

            const std::filesystem::path installedTool =
                installRoot / "Tools" / exeName;

            if (std::filesystem::exists(installedTool))
                return installedTool;

            const std::filesystem::path sourceTool =
                sdkRoot / "bin" / "Debug" / "Tools" / exeName;

            if (std::filesystem::exists(sourceTool))
                return sourceTool;

            const std::filesystem::path fallbackTool =
                installRoot / "bin" / "Debug" / "Tools" / exeName;

            if (std::filesystem::exists(fallbackTool))
                return fallbackTool;

            return installedTool;
        }
    }

    ProjectConfig ProjectGenerator::Generate(const ProjectGeneratorSettings& desc)
    {
        static std::future<void> sProjectBuildFuture;

        ProjectConfig project{};

        InitializeProject(project, desc);
        GenerateFilesFromTemplates(desc.TemplateFolder, project, desc);

        if (desc.BuildAfterGenerate)
        {
            const std::string profile = desc.BuildProfile.empty()
                ? "Windows-Debug"
                : desc.BuildProfile;

            sProjectBuildFuture = std::async(std::launch::async, [project, profile]()
                {
                    std::string log{};

                    if (!GenerateBoonBuildProject(project, profile, log))
                    {
                        BOON_LOG_ERROR("BoonBuild project generation failed!");
                        return;
                    }

                    if (!Configure(project, profile, log))
                    {
                        BOON_LOG_ERROR("CMake configure failed!");
                        return;
                    }

                    if (!Build(project, profile, log))
                    {
                        BOON_LOG_ERROR("Build failed!");
                        return;
                    }

                    BOON_LOG("Build complete");
                });
        }

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

        const std::filesystem::path sdkRoot =
            desc.EngineRoot.empty()
            ? ResolveSdkRoot(project)
            : NormalizeSdkRoot(desc.EngineRoot);

        project.Runtime.EngineRoot = sdkRoot;
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
        std::transform(
            uppercaseName.begin(),
            uppercaseName.end(),
            uppercaseName.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

        std::string lowercaseName = desc.Name;
        std::transform(
            lowercaseName.begin(),
            lowercaseName.end(),
            lowercaseName.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        const std::filesystem::path sdkRoot = ResolveSdkRoot(project);
        const std::filesystem::path installRoot = ResolveInstallRoot(project);

        TemplateContext context{};
        context.Set("PROJECT_NAME", desc.Name);
        context.Set("PROJECT_NAME_UPPER", uppercaseName);
        context.Set("PROJECT_NAME_LOWER", lowercaseName);
        context.Set("PROJECT_ROOT", project.Runtime.ProjectRoot.string());

        context.Set("BOON_INSTALL_ROOT", installRoot.string());
        context.Set("BOON_SDK_ROOT", sdkRoot.string());

        // Backward-compatible names for current CMake/BoonBuild code.
        context.Set("BOON_REPO_ROOT", sdkRoot.string());
        context.Set("BOON_ENGINE_ROOT", (sdkRoot / "Boon").string());

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

    bool ProjectGenerator::GenerateBoonBuildProject(
        const ProjectConfig& project,
        const std::string& profile,
        std::string& outLog)
    {
        BOON_LOG("Running BoonBuild project generation ...");

        const std::filesystem::path sdkRoot = ResolveSdkRoot(project);
        const std::filesystem::path boonBuildExe = GetBoonBuildExe(project);

        if (sdkRoot.empty() || !std::filesystem::exists(sdkRoot / "Boon" / "CMakeLists.txt"))
        {
            BOON_LOG_ERROR("Invalid BOON_SDK_ROOT: {}", sdkRoot.string());
            return false;
        }

        if (!std::filesystem::exists(boonBuildExe))
        {
            BOON_LOG_ERROR("BoonBuild executable not found: {}", boonBuildExe.string());
            return false;
        }

        const std::string command =
            Quote(boonBuildExe) + " " +
            Quote(project.Runtime.ProjectRoot) +
            " --profile " + QuoteArg(profile);

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
        const std::string& profile,
        std::string& outLog)
    {
        BOON_LOG("Configuring project preset {} ...", profile);

        const std::string command =
            "cmake --preset " + QuoteArg(profile);

        ProcessResult result = ProcessRunner::Run(
            command,
            [](const std::string& chunk)
            {
                BOON_LOG(chunk);
            },
            project.Runtime.ProjectRoot);

        outLog += result.Output;

        if (result.ExitCode != 0)
        {
            BOON_LOG_ERROR(result.Output);
            return false;
        }

        return true;
    }

    bool ProjectGenerator::Build(
        const ProjectConfig& project,
        const std::string& profile,
        std::string& outLog)
    {
        BOON_LOG("Building project preset {} ...", profile);

        const std::string command =
            "cmake --build --preset " + QuoteArg(profile);

        ProcessResult result = ProcessRunner::Run(
            command,
            [](const std::string& chunk)
            {
                BOON_LOG(chunk);
            },
            project.Runtime.ProjectRoot);

        outLog += result.Output;

        if (result.ExitCode != 0)
        {
            BOON_LOG_ERROR(result.Output);
            return false;
        }

        return true;
    }
}