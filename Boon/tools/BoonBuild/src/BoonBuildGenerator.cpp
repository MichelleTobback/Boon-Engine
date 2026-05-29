#include "BoonBuildGenerator.h"

#include "ModuleRulesReader.h"
#include "ModuleTemplateBuilder.h"
#include "ProjectRulesReader.h"
#include "ProjectTemplateBuilder.h"
#include "TemplateFileGenerator.h"
#include "ModuleDependencyGraph.h"
#include "BuildPlatform.h"
#include "BuildProfileReader.h"
#include "CMakePresetsGenerator.h"

#include "Tools/TemplateProcessor.h"

#include <filesystem>
#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>

namespace BoonBuild
{
    namespace
    {
        bool LooksLikeRepoRoot(const std::filesystem::path& root)
        {
            return std::filesystem::exists(root / "Boon" / "CMakeLists.txt")
                && std::filesystem::exists(root / "Boon" / "modules");
        }

        bool LooksLikeProjectRoot(const std::filesystem::path& root)
        {
            return std::filesystem::exists(root / "BuildRules.json");
        }

        bool SupportsPlatform(const ModuleRules& module, BuildPlatform platform)
        {
            if (module.Platforms.empty())
                return true;

            const std::string platformName = ToString(platform);

            for (const std::string& value : module.Platforms)
            {
                if (value == "All" || value == platformName)
                    return true;
            }

            return false;
        }

        std::vector<ModuleRules> FilterModulesForPlatform(
            const std::vector<ModuleRules>& modules,
            BuildPlatform platform)
        {
            std::vector<ModuleRules> result;

            for (const ModuleRules& module : modules)
            {
                if (SupportsPlatform(module, platform))
                    result.push_back(module);
            }

            return result;
        }


        bool AddRequestedModuleWithDependencies(
            const std::string& moduleName,
            const std::unordered_map<std::string, ModuleRules>& byName,
            std::set<std::string>& visiting,
            std::set<std::string>& selected,
            std::string& error)
        {
            if (selected.contains(moduleName))
                return true;

            if (visiting.contains(moduleName))
            {
                error = "Circular module dependency while resolving: " + moduleName;
                return false;
            }

            const auto it = byName.find(moduleName);
            if (it == byName.end())
            {
                error = "Requested engine module was not found: " + moduleName;
                return false;
            }

            visiting.insert(moduleName);

            for (const std::string& dependency : it->second.ModuleDependencies)
            {
                if (!AddRequestedModuleWithDependencies(
                    dependency,
                    byName,
                    visiting,
                    selected,
                    error))
                {
                    return false;
                }
            }

            visiting.erase(moduleName);
            selected.insert(moduleName);
            return true;
        }

        bool SelectRequestedEngineModules(
            const std::vector<ModuleRules>& allModules,
            const std::vector<std::string>& requestedModules,
            BuildPlatform platform,
            std::vector<ModuleRules>& outModules,
            std::string& outError)
        {
            outModules.clear();

            std::unordered_map<std::string, ModuleRules> byName;
            for (const ModuleRules& module : allModules)
                byName[module.Name] = module;

            std::set<std::string> selected;
            std::set<std::string> visiting;

            for (const std::string& moduleName : requestedModules)
            {
                if (!AddRequestedModuleWithDependencies(
                    moduleName,
                    byName,
                    visiting,
                    selected,
                    outError))
                {
                    return false;
                }
            }

            for (const ModuleRules& module : allModules)
            {
                if (!selected.contains(module.Name))
                    continue;

                if (!SupportsPlatform(module, platform))
                {
                    outError = "Engine module '" + module.Name + "' does not support platform '" + ToString(platform) + "'.";
                    return false;
                }

                outModules.push_back(module);
            }

            return true;
        }

        bool GenerateEngineModuleFiles(
            const std::filesystem::path& repoRoot,
            const std::vector<ModuleRules>& inputModules,
            BuildPlatform platform)
        {
            const std::filesystem::path modulesDir =
                repoRoot / "Boon" / "modules";

            const std::filesystem::path generatedDir =
                modulesDir / "generated";

            const std::filesystem::path templatesDir =
                repoRoot / "Boon" / "tools" / "BoonBuild" / "Templates";

            ModuleTemplateBuilder templateBuilder;
            TemplateFileGenerator templateGenerator;

            std::vector<ModuleRules> modules = inputModules;
            std::vector<ModuleRules> sortedModules;
            std::string dependencyError;

            if (!ModuleDependencyGraph::Sort(
                modules,
                sortedModules,
                dependencyError))
            {
                std::cerr << dependencyError << "\n";
                return false;
            }

            modules = std::move(sortedModules);

            std::filesystem::create_directories(generatedDir);

            {
                const GlobalModuleTemplateData data =
                    templateBuilder.BuildGlobalData(modules);

                Boon::TemplateContext context;
                context.Set("MODULE_INCLUDES", data.ModuleIncludes);
                context.Set("STATIC_MODULE_DECLARATIONS", data.StaticModuleDeclarations);
                context.Set("STATIC_MODULE_REGISTER_CALLS", data.StaticModuleRegisterCalls);
                context.Set("STATIC_MODULE_UNREGISTER_CALLS", data.StaticModuleUnregisterCalls);

                if (!templateGenerator.Generate(
                    templatesDir / "Modules.btemplate",
                    generatedDir,
                    context))
                {
                    return false;
                }
            }

            for (const ModuleRules& module : modules)
            {
                const ModuleTemplateData data =
                    templateBuilder.BuildModuleData(module);

                Boon::TemplateContext context;
                context.Set("MODULE_NAME", data.ModuleName);
                context.Set("OUTPUT_TYPE", data.OutputType);
                context.Set("REFLECTION_BLOCK", data.ReflectionBlock);
                context.Set("REFLECTION_DEPENDENCY_BLOCK", data.ReflectionDependencyBlock);
                context.Set("COMPILE_DEFINITIONS_BLOCK", data.CompileDefinitionsBlock);
                context.Set("INCLUDE_DIRECTORIES_BLOCK", data.IncludeDirectoriesBlock);
                context.Set("PUBLIC_DEPENDENCIES_BLOCK", data.PublicDependenciesBlock);
                context.Set("THIRD_PARTY_BLOCK", data.ThirdPartyBlock);
                context.Set("SYSTEM_LIBRARIES_BLOCK", data.SystemLibrariesBlock);
                context.Set("GENERATED_REFLECTION_DECLARATIONS", data.GeneratedReflectionDeclarations);
                context.Set("REGISTER_REFLECTION_CALL", data.RegisterReflectionCall);
                context.Set("UNREGISTER_REFLECTION_CALL", data.UnregisterReflectionCall);
                context.Set("CREATE_MODULE_INSTANCE_BODY", data.CreateModuleInstanceBody);
                context.Set("DESTROY_MODULE_INSTANCE_BODY", data.DestroyModuleInstanceBody);
                context.Set("PLATFORM_COMPILE_DEFINITION", ToCompileDefinition(platform));

                if (!templateGenerator.Generate(
                    templatesDir / "Module.btemplate",
                    generatedDir,
                    context))
                {
                    return false;
                }
            }

            return true;
        }
    }

    bool BoonBuildGenerator::Generate(
        const std::filesystem::path& root,
        const std::string& profileName) const
    {
        const std::filesystem::path absoluteRoot =
            std::filesystem::absolute(root);

        if (LooksLikeRepoRoot(absoluteRoot))
            return GenerateEngineModules(absoluteRoot, profileName);

        if (LooksLikeProjectRoot(absoluteRoot))
            return GenerateProject(absoluteRoot, profileName);

        std::cerr << "Unknown BoonBuild root:\n" << absoluteRoot << "\n";
        std::cerr << "Expected either:\n";
        std::cerr << "  - Boon repo root with Boon/CMakeLists.txt\n";
        std::cerr << "  - Game project root with BuildRules.json\n";

        return false;
    }

    bool BoonBuildGenerator::GenerateEngineModules(
        const std::filesystem::path& repoRoot,
        const std::string& profileName) const
    {
        const std::filesystem::path profilesFile =
            repoRoot / "BuildProfiles.json";

        BuildProfileReader profileReader;
        BuildProfile profile;

        if (!profileReader.ReadProfile(profilesFile, profileName, profile))
            return false;

        std::vector<BuildProfile> allProfiles;

        if (!profileReader.ReadAllProfiles(profilesFile, allProfiles))
            return false;

        CMakePresetsGenerator presetGenerator;

        if (!presetGenerator.Generate(repoRoot, repoRoot, allProfiles))
            return false;

        const std::filesystem::path modulesDir =
            repoRoot / "Boon" / "modules";

        ModuleRulesReader rulesReader;
        std::vector<ModuleRules> modules;

        if (!rulesReader.ReadAllModules(modulesDir, modules))
            return false;

        modules = FilterModulesForPlatform(
            modules,
            profile.Platform);

        if (!GenerateEngineModuleFiles(repoRoot, modules, profile.Platform))
            return false;

        std::cout << "Generated "
            << modules.size()
            << " engine module(s) for profile "
            << profile.Name
            << ".\n";

        return true;
    }

    bool BoonBuildGenerator::GenerateProject(
        const std::filesystem::path& projectRoot,
        const std::string& profileName) const
    {
        std::filesystem::path sdkRoot = ResolveSdkRoot();

        if (sdkRoot.empty())
        {
            std::cerr << "BOON_SDK_ROOT is not set.\n";
            return false;
        }

        sdkRoot = std::filesystem::absolute(sdkRoot);

        if (std::filesystem::exists(
            sdkRoot / "tools" / "BClassGenerator" / "BoonReflection.cmake"))
        {
            sdkRoot = sdkRoot.parent_path();
        }

        if (!std::filesystem::exists(sdkRoot / "Boon" / "CMakeLists.txt"))
        {
            std::cerr << "Invalid BOON_SDK_ROOT:\n"
                << sdkRoot << "\n";
            std::cerr << "Expected:\n"
                << (sdkRoot / "Boon" / "CMakeLists.txt") << "\n";
            return false;
        }

        const std::filesystem::path profilesFile =
            projectRoot / "BuildProfiles.json";

        BuildProfileReader profileReader;
        BuildProfile profile;

        if (!profileReader.ReadProfile(profilesFile, profileName, profile))
            return false;

        std::vector<BuildProfile> allProfiles;

        if (!profileReader.ReadAllProfiles(profilesFile, allProfiles))
            return false;

        CMakePresetsGenerator presetGenerator;

        if (!presetGenerator.Generate(projectRoot, sdkRoot, allProfiles))
            return false;

        const std::filesystem::path templatesDir =
            sdkRoot / "Boon" / "tools" / "BoonBuild" / "Templates";

        if (!std::filesystem::exists(templatesDir / "Project.btemplate"))
        {
            std::cerr << "Project template not found:\n"
                << (templatesDir / "Project.btemplate") << "\n";
            return false;
        }

        ProjectRulesReader rulesReader;
        ProjectTemplateBuilder templateBuilder;
        TemplateFileGenerator templateGenerator;

        ProjectRules project;

        if (!rulesReader.ReadProjectRules(
            projectRoot / "BuildRules.json",
            project))
        {
            return false;
        }

        if (!project.Platform.empty() && project.Platform != ToString(profile.Platform))
        {
            std::cerr << "BuildRules.json platform is '"
                << project.Platform
                << "' but selected profile '"
                << profile.Name
                << "' targets '"
                << ToString(profile.Platform)
                << "'.\n";
            return false;
        }

        ModuleRulesReader moduleReader;
        std::vector<ModuleRules> allEngineModules;

        if (!moduleReader.ReadAllModules(
            sdkRoot / "Boon" / "modules",
            allEngineModules))
        {
            return false;
        }

        std::vector<ModuleRules> selectedEngineModules;
        std::string moduleError;

        if (!SelectRequestedEngineModules(
            allEngineModules,
            project.EngineModules,
            profile.Platform,
            selectedEngineModules,
            moduleError))
        {
            std::cerr << moduleError << "\n";
            return false;
        }

        std::vector<ModuleRules> sortedEngineModules;

        if (!ModuleDependencyGraph::Sort(
            selectedEngineModules,
            sortedEngineModules,
            moduleError))
        {
            std::cerr << moduleError << "\n";
            return false;
        }

        selectedEngineModules = std::move(sortedEngineModules);

        const ProjectTemplateData data =
            templateBuilder.BuildProjectData(project, selectedEngineModules);

        Boon::TemplateContext context;
        context.Set("PROJECT_NAME", data.ProjectName);
        context.Set("OUTPUT_TYPE", data.OutputType);
        context.Set("REFLECTION_BLOCK", data.ReflectionBlock);
        context.Set("REFLECTION_DEPENDENCY_BLOCK", data.ReflectionDependencyBlock);
        context.Set("COMPILE_DEFINITIONS_BLOCK", data.CompileDefinitionsBlock);
        context.Set("INCLUDE_DIRECTORIES_BLOCK", data.IncludeDirectoriesBlock);
        context.Set("LINK_LIBRARIES_BLOCK", data.LinkLibrariesBlock);
        context.Set("ASSET_COPY_BLOCK", data.AssetCopyBlock);
        context.Set("PROJECT_ENGINE_MODULES_BLOCK", data.ProjectEngineModulesBlock);
        context.Set("PROJECT_STATIC_MODULE_DECLARATIONS", data.ProjectStaticModuleDeclarations);
        context.Set("PROJECT_STATIC_MODULE_REGISTER_CALLS", data.ProjectStaticModuleRegisterCalls);

        context.Set("GENERATED_REFLECTION_DECLARATIONS", data.GeneratedReflectionDeclarations);
        context.Set("REGISTER_REFLECTION_CALL", data.RegisterReflectionCall);
        context.Set("UNREGISTER_REFLECTION_CALL", data.UnregisterReflectionCall);

        context.Set("STATIC_MODULE_MANIFEST_ENTRIES", data.StaticModuleManifestEntries);
        context.Set("DYNAMIC_MODULE_MANIFEST_ENTRIES", data.DynamicModuleManifestEntries);
        context.Set("CREATE_MODULE_INSTANCE_BODY", data.CreateModuleInstanceBody);
        context.Set("DESTROY_MODULE_INSTANCE_BODY", data.DestroyModuleInstanceBody);

        context.Set("BUILD_PROFILE_NAME", profile.Name);
        context.Set("BUILD_PLATFORM", ToString(profile.Platform));
        context.Set("BUILD_CONFIGURATION", profile.Configuration);
        context.Set("BUILD_GENERATOR", profile.Generator);
        context.Set("PLATFORM_COMPILE_DEFINITION", ToCompileDefinition(profile.Platform));

        context.Set("BOON_SDK_ROOT", sdkRoot.string());
        context.Set("BOON_REPO_ROOT", sdkRoot.string());
        context.Set("BOON_ENGINE_ROOT", (sdkRoot / "Boon").string());

        if (!templateGenerator.Generate(
            templatesDir / "Project.btemplate",
            projectRoot,
            context))
        {
            return false;
        }

        std::cout << "Generated project: "
            << project.Name
            << " for profile "
            << profile.Name
            << ".\n";

        return true;
    }

    std::filesystem::path BoonBuildGenerator::ResolveSdkRoot()
    {
        if (const char* sdk = std::getenv("BOON_SDK_ROOT"))
        {
            if (*sdk)
                return std::filesystem::path(sdk);
        }

        if (const char* engine = std::getenv("BOON_ENGINE_ROOT"))
        {
            if (*engine)
                return std::filesystem::path(engine);
        }

        if (const char* repo = std::getenv("BOON_REPO_ROOT"))
        {
            if (*repo)
                return std::filesystem::path(repo);
        }

        return {};
    }
}