#pragma once

#include "ProjectRules.h"
#include "ProjectTemplateData.h"

#include <string>
#include <string_view>
#include <vector>

namespace BoonBuild
{
    class ProjectTemplateBuilder
    {
    public:
        ProjectTemplateData BuildProjectData(const ProjectRules& project) const;

    private:
        static std::string JoinIndented(const std::vector<std::string>& values, std::string_view indent = "        ");

        static std::string MakeReflectionBlock(const ProjectRules& project);
        static std::string MakeReflectionDependencyBlock(const ProjectRules& project);
        static std::string MakeCompileDefinitionsBlock(const ProjectRules& project);
        static std::string MakeIncludeDirectoriesBlock(const ProjectRules& project);
        static std::string MakeLinkLibrariesBlock(const ProjectRules& project);
        static std::string MakeAssetCopyBlock(const ProjectRules& project);

        static std::string MakeGeneratedReflectionDeclarations(const ProjectRules& project);
        static std::string MakeRegisterReflectionCall(const ProjectRules& project);
        static std::string MakeUnregisterReflectionCall(const ProjectRules& project);

        static std::string MakeStaticModuleManifestEntries(const ProjectRules& project);
        static std::string MakeDynamicModuleManifestEntries(const ProjectRules&);

        static std::string MakeCreateModuleInstanceBody(const ProjectRules& project);
        static std::string MakeDestroyModuleInstanceBody(const ProjectRules& project);
    };
}