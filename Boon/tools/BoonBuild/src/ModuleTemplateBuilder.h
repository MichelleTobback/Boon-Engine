#pragma once

#include "ModuleRules.h"
#include "ModuleTemplateData.h"

#include <vector>

namespace BoonBuild
{
    class ModuleTemplateBuilder
    {
    public:
        GlobalModuleTemplateData BuildGlobalData(const std::vector<ModuleRules>& modules) const;
        ModuleTemplateData BuildModuleData(const ModuleRules& module) const;

    private:
        static std::string JoinIndented(const std::vector<std::string>& values, std::string_view indent = "        ");

        static std::string MakeModuleIncludes(const std::vector<ModuleRules>& modules);
        static std::string MakeStaticModuleDeclarations(const std::vector<ModuleRules>& modules);
        static std::string MakeStaticModuleRegisterCalls(const std::vector<ModuleRules>& modules);
        static std::string MakeStaticModuleUnregisterCalls(const std::vector<ModuleRules>& modules);

        static std::string MakeReflectionBlock(const ModuleRules& module);
        static std::string MakeReflectionDependencyBlock(const ModuleRules& module);
        static std::string MakeCompileDefinitionsBlock(const ModuleRules& module);
        static std::string MakeIncludeDirectoriesBlock(const ModuleRules& module);
        static std::string MakePublicDependenciesBlock(const ModuleRules& module);
        static std::string MakeThirdPartyBlock(const ModuleRules& module);
        static std::string MakeSystemLibrariesBlock(const ModuleRules& module);

        static std::string MakeGeneratedReflectionDeclarations(const ModuleRules& module);
        static std::string MakeRegisterReflectionCall(const ModuleRules& module);
        static std::string MakeUnregisterReflectionCall(const ModuleRules& module);
    };
}