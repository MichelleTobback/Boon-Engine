#pragma once

#include <string>

namespace BoonBuild
{
    struct GlobalModuleTemplateData
    {
        std::string ModuleIncludes;
        std::string StaticModuleDeclarations;
        std::string StaticModuleRegisterCalls;
        std::string StaticModuleUnregisterCalls;
    };

    struct ModuleTemplateData
    {
        std::string ModuleName;
        std::string OutputType;

        std::string ReflectionBlock;
        std::string ReflectionDependencyBlock;
        std::string CompileDefinitionsBlock;
        std::string IncludeDirectoriesBlock;
        std::string PublicDependenciesBlock;
        std::string ThirdPartyBlock;
        std::string SystemLibrariesBlock;

        std::string GeneratedReflectionDeclarations;
        std::string RegisterReflectionCall;
        std::string UnregisterReflectionCall;

        std::string CreateModuleInstanceBody;
        std::string DestroyModuleInstanceBody;
    };
}