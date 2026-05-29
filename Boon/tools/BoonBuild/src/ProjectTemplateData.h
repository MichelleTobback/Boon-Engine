#pragma once

#include <string>

namespace BoonBuild
{
    struct ProjectTemplateData
    {
        std::string ProjectName;
        std::string OutputType;

        std::string ReflectionBlock;
        std::string ReflectionDependencyBlock;

        std::string CompileDefinitionsBlock;
        std::string IncludeDirectoriesBlock;
        std::string LinkLibrariesBlock;
        std::string AssetCopyBlock;

        std::string ProjectEngineModulesBlock;

        std::string ProjectStaticModuleDeclarations;
        std::string ProjectStaticModuleRegisterCalls;

        std::string GeneratedReflectionDeclarations;
        std::string RegisterReflectionCall;
        std::string UnregisterReflectionCall;

        std::string StaticModuleManifestEntries;
        std::string DynamicModuleManifestEntries;

        std::string CreateModuleInstanceBody;
        std::string DestroyModuleInstanceBody;
    };
}
