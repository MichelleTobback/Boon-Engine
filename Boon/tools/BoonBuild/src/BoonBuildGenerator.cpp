#include "BoonBuildGenerator.h"

#include "ModuleRulesReader.h"
#include "ModuleTemplateBuilder.h"
#include "ProjectRulesReader.h"
#include "ProjectTemplateBuilder.h"
#include "TemplateFileGenerator.h"
#include "ModuleDependencyGraph.h"

#include "Tools/TemplateProcessor.h"

#include <filesystem>
#include <iostream>
#include <vector>

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
    }

    bool BoonBuildGenerator::Generate(const std::filesystem::path& root) const
    {
        const std::filesystem::path absoluteRoot = std::filesystem::absolute(root);

        if (LooksLikeRepoRoot(absoluteRoot))
            return GenerateEngineModules(absoluteRoot);

        if (LooksLikeProjectRoot(absoluteRoot))
            return GenerateProject(absoluteRoot);

        std::cerr << "Unknown BoonBuild root:\n" << absoluteRoot << "\n";
        std::cerr << "Expected either:\n";
        std::cerr << "  - Boon repo root with Boon/CMakeLists.txt\n";
        std::cerr << "  - Game project root with BuildRules.json\n";

        return false;
    }

    bool BoonBuildGenerator::GenerateEngineModules(const std::filesystem::path& repoRoot) const
    {
        const std::filesystem::path modulesDir = repoRoot / "Boon" / "modules";
        const std::filesystem::path generatedDir = modulesDir / "generated";
        const std::filesystem::path templatesDir = repoRoot / "Boon" / "tools" / "BoonBuild" / "Templates";

        ModuleRulesReader rulesReader;
        ModuleTemplateBuilder templateBuilder;
        TemplateFileGenerator templateGenerator;

        std::vector<ModuleRules> modules;

        if (!rulesReader.ReadAllModules(modulesDir, modules))
            return false;

        std::vector<ModuleRules> sortedModules;
        std::string dependencyError;

        if (!ModuleDependencyGraph::Sort(modules, sortedModules, dependencyError))
        {
            std::cerr << dependencyError << "\n";
            return false;
        }

        modules = std::move(sortedModules);

        std::filesystem::create_directories(generatedDir);

        {
            const GlobalModuleTemplateData data = templateBuilder.BuildGlobalData(modules);

            Boon::TemplateContext context;
            context.Set("MODULE_INCLUDES", data.ModuleIncludes);
            context.Set("STATIC_MODULE_DECLARATIONS", data.StaticModuleDeclarations);
            context.Set("STATIC_MODULE_REGISTER_CALLS", data.StaticModuleRegisterCalls);
            context.Set("STATIC_MODULE_UNREGISTER_CALLS", data.StaticModuleUnregisterCalls);

            if (!templateGenerator.Generate(templatesDir / "Modules.btemplate", generatedDir, context))
                return false;
        }

        for (const ModuleRules& module : modules)
        {
            const ModuleTemplateData data = templateBuilder.BuildModuleData(module);

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

            if (!templateGenerator.Generate(templatesDir / "Module.btemplate", generatedDir, context))
                return false;
        }

        std::cout << "Generated " << modules.size() << " engine module(s).\n";
        return true;
    }

    bool BoonBuildGenerator::GenerateProject(const std::filesystem::path& projectRoot) const
    {
        const char* repoRootEnv = std::getenv("BOON_REPO_ROOT");
        const char* engineRootEnv = std::getenv("BOON_ENGINE_ROOT");

        std::filesystem::path repoRoot;

        if (repoRootEnv && *repoRootEnv)
            repoRoot = repoRootEnv;
        else if (engineRootEnv && *engineRootEnv)
            repoRoot = engineRootEnv;
        else
        {
            std::cerr << "BOON_REPO_ROOT is not set.\n";
            std::cerr << "Set it to your Boon repository root, for example:\n";
            std::cerr << "  D:/Projects/BoonEngine/Boon-Engine\n";
            return false;
        }

        repoRoot = std::filesystem::absolute(repoRoot);

        if (std::filesystem::exists(repoRoot / "tools" / "BClassGenerator" / "BoonReflection.cmake"))
            repoRoot = repoRoot.parent_path();

        if (!LooksLikeRepoRoot(repoRoot))
        {
            std::cerr << "Invalid BOON_REPO_ROOT:\n" << repoRoot << "\n";
            return false;
        }

        const std::filesystem::path templatesDir = repoRoot / "Boon" / "tools" / "BoonBuild" / "Templates";

        ProjectRulesReader rulesReader;
        ProjectTemplateBuilder templateBuilder;
        TemplateFileGenerator templateGenerator;

        ProjectRules project;

        if (!rulesReader.ReadProjectRules(projectRoot / "BuildRules.json", project))
            return false;

        const ProjectTemplateData data = templateBuilder.BuildProjectData(project);

        Boon::TemplateContext context;
        context.Set("PROJECT_NAME", data.ProjectName);
        context.Set("OUTPUT_TYPE", data.OutputType);
        context.Set("REFLECTION_BLOCK", data.ReflectionBlock);
        context.Set("REFLECTION_DEPENDENCY_BLOCK", data.ReflectionDependencyBlock);
        context.Set("COMPILE_DEFINITIONS_BLOCK", data.CompileDefinitionsBlock);
        context.Set("INCLUDE_DIRECTORIES_BLOCK", data.IncludeDirectoriesBlock);
        context.Set("LINK_LIBRARIES_BLOCK", data.LinkLibrariesBlock);
        context.Set("ASSET_COPY_BLOCK", data.AssetCopyBlock);
        context.Set("GENERATED_REFLECTION_DECLARATIONS", data.GeneratedReflectionDeclarations);
        context.Set("REGISTER_REFLECTION_CALL", data.RegisterReflectionCall);
        context.Set("UNREGISTER_REFLECTION_CALL", data.UnregisterReflectionCall);
        context.Set("STATIC_MODULE_MANIFEST_ENTRIES", data.StaticModuleManifestEntries);
        context.Set("DYNAMIC_MODULE_MANIFEST_ENTRIES", data.DynamicModuleManifestEntries);
        context.Set("CREATE_MODULE_INSTANCE_BODY", data.CreateModuleInstanceBody);
        context.Set("DESTROY_MODULE_INSTANCE_BODY", data.DestroyModuleInstanceBody);

        if (!templateGenerator.Generate(templatesDir / "Project.btemplate", projectRoot, context))
            return false;

        std::cout << "Generated project: " << project.Name << "\n";
        return true;
    }
}