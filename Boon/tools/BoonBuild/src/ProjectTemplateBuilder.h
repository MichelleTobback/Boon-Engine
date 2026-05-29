#pragma once

#include "ProjectRules.h"
#include "ProjectTemplateData.h"
#include "ModuleRules.h"

#include <string>
#include <string_view>
#include <vector>

namespace BoonBuild
{
    class ProjectTemplateBuilder
    {
    public:
        ProjectTemplateData BuildProjectData(
            const ProjectRules& project,
            const std::vector<ModuleRules>& selectedEngineModules) const;

    private:
        static std::string JoinIndented(const std::vector<std::string>& values, std::string_view indent = "        ");

        static std::string MakeReflectionBlock(const ProjectRules& project);
        static std::string MakeReflectionDependencyBlock(const ProjectRules& project);
        static std::string MakeCompileDefinitionsBlock(const ProjectRules& project);
        static std::string MakeIncludeDirectoriesBlock(const ProjectRules& project);
        static std::string MakeLinkLibrariesBlock(const ProjectRules& project);
        static std::string MakeAssetCopyBlock(const ProjectRules& project);
        static std::string MakeProjectEngineModulesBlock(const std::vector<ModuleRules>& selectedEngineModules);
        static std::vector<std::string> MakeProjectStaticModuleNames(
            const ProjectRules& project,
            const std::vector<ModuleRules>& selectedEngineModules);
        static std::string MakeProjectStaticModuleDeclarations(const std::vector<std::string>& moduleNames);
        static std::string MakeProjectStaticModuleRegisterCalls(const std::vector<std::string>& moduleNames);

        static std::string MakeGeneratedReflectionDeclarations(const ProjectRules& project);
        static std::string MakeRegisterReflectionCall(const ProjectRules& project);
        static std::string MakeUnregisterReflectionCall(const ProjectRules& project);

        static std::string MakeStaticModuleManifestEntries(
            const ProjectRules& project,
            const std::vector<ModuleRules>& selectedEngineModules);

        static std::string MakeDynamicModuleManifestEntries(const ProjectRules&);

        static std::string MakeCreateModuleInstanceBody(const ProjectRules& project);
        static std::string MakeDestroyModuleInstanceBody(const ProjectRules& project);
    };
}
