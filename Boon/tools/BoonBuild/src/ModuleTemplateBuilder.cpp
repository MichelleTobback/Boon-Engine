#include "ModuleTemplateBuilder.h"

#include <sstream>

namespace BoonBuild
{
    GlobalModuleTemplateData ModuleTemplateBuilder::BuildGlobalData(const std::vector<ModuleRules>& modules) const
    {
        GlobalModuleTemplateData data;
        data.ModuleIncludes = MakeModuleIncludes(modules);
        data.StaticModuleDeclarations = MakeStaticModuleDeclarations(modules);
        data.StaticModuleRegisterCalls = MakeStaticModuleRegisterCalls(modules);
        data.StaticModuleUnregisterCalls = MakeStaticModuleUnregisterCalls(modules);
        return data;
    }

    ModuleTemplateData ModuleTemplateBuilder::BuildModuleData(const ModuleRules& module) const
    {
        ModuleTemplateData data;
        data.ModuleName = module.Name;
        data.OutputType = module.Output;
        data.ReflectionBlock = MakeReflectionBlock(module);
        data.ReflectionDependencyBlock = MakeReflectionDependencyBlock(module);
        data.CompileDefinitionsBlock = MakeCompileDefinitionsBlock(module);
        data.IncludeDirectoriesBlock = MakeIncludeDirectoriesBlock(module);
        data.PublicDependenciesBlock = MakePublicDependenciesBlock(module);
        data.ThirdPartyBlock = MakeThirdPartyBlock(module);
        data.SystemLibrariesBlock = MakeSystemLibrariesBlock(module);
        data.GeneratedReflectionDeclarations = MakeGeneratedReflectionDeclarations(module);
        data.RegisterReflectionCall = MakeRegisterReflectionCall(module);
        data.UnregisterReflectionCall = MakeUnregisterReflectionCall(module);
        return data;
    }

    std::string ModuleTemplateBuilder::JoinIndented(const std::vector<std::string>& values, std::string_view indent)
    {
        std::stringstream ss;

        for (const auto& value : values)
            ss << indent << value << "\n";

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeModuleIncludes(const std::vector<ModuleRules>& modules)
    {
        std::stringstream ss;

        for (const auto& module : modules)
        {
            ss << "include(\"${CMAKE_CURRENT_LIST_DIR}/"
                << module.Name << "/" << module.Name << ".cmake\")\n";

            if (module.Output == "STATIC")
            {
                ss << "list(APPEND BOON_STATIC_MODULE_TARGETS "
                    << module.Name << ")\n";
            }
            else if (module.Output == "SHARED")
            {
                ss << "list(APPEND BOON_DYNAMIC_MODULE_TARGETS "
                    << module.Name << ")\n";
            }

            ss << "\n";
        }

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeStaticModuleDeclarations(const std::vector<ModuleRules>& modules)
    {
        std::stringstream ss;

        for (const auto& module : modules)
        {
            if (module.Output != "STATIC")
                continue;

            ss << "    const Boon::ModuleInfo* " << module.Name << "_GetModuleInfo();\n";
            ss << "    Boon::ModuleRegistration " << module.Name << "_RegisterModule(Boon::ModuleContext*);\n";
            ss << "    void " << module.Name << "_UnregisterModule(Boon::ModuleContext*, Boon::ModuleInstance*);\n\n";
        }

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeStaticModuleRegisterCalls(const std::vector<ModuleRules>& modules)
    {
        std::stringstream ss;

        for (const auto& module : modules)
        {
            if (module.Output != "STATIC")
                continue;

            ss << "        {\n";
            ss << "            const Boon::ModuleInfo* info = " << module.Name << "_GetModuleInfo();\n";
            ss << "            Boon::ModuleRegistration registration = " << module.Name << "_RegisterModule(&context);\n\n";
            ss << "            if (!registration.Success)\n";
            ss << "            {\n";
            ss << "                result = false;\n";
            ss << "            }\n";
            ss << "            else\n";
            ss << "            {\n";
            ss << "                s_StaticModules.push_back({\n";
            ss << "                    info,\n";
            ss << "                    registration.Instance,\n";
            ss << "                    &" << module.Name << "_UnregisterModule\n";
            ss << "                });\n";
            ss << "            }\n";
            ss << "        }\n\n";
        }

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeStaticModuleUnregisterCalls(const std::vector<ModuleRules>& modules)
    {
        return {};
    }

    std::string ModuleTemplateBuilder::MakeReflectionBlock(const ModuleRules& module)
    {
        if (!module.Reflection)
            return {};

        std::stringstream ss;

        ss << "boon_add_reflected_module(\n";
        ss << "    NAME ${MODULE_TARGET}\n";
        ss << "    OUTPUT ${MODULE_GENERATED_COMPONENTS_CPP}\n";
        ss << "    SCAN_DIRS\n";
        ss << "        \"${MODULE_SOURCE_DIR}/include\"\n";
        ss << ")\n\n";

        ss << "set_source_files_properties(\n";
        ss << "    ${MODULE_GENERATED_COMPONENTS_CPP}\n";
        ss << "    PROPERTIES GENERATED TRUE\n";
        ss << ")";

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeReflectionDependencyBlock(const ModuleRules& module)
    {
        if (!module.Reflection)
            return {};

        std::stringstream ss;

        ss << "add_dependencies(${MODULE_TARGET} GenerateReflection_${MODULE_TARGET})";

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeCompileDefinitionsBlock(const ModuleRules& module)
    {
        std::stringstream ss;

        ss << "target_compile_definitions(${MODULE_TARGET}\n";
        ss << "    PRIVATE\n";
        ss << "        BOON_MODULE_NAME=${MODULE_TARGET}\n";

        if (!module.PublicDefinitions.empty())
        {
            ss << "    PUBLIC\n";
            ss << JoinIndented(module.PublicDefinitions);
        }

        if (!module.PrivateDefinitions.empty())
        {
            ss << "    PRIVATE\n";
            ss << JoinIndented(module.PrivateDefinitions);
        }

        ss << ")";

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeIncludeDirectoriesBlock(const ModuleRules& module)
    {
        std::stringstream ss;

        ss << "target_include_directories(${MODULE_TARGET}\n";

        if (!module.PublicIncludePaths.empty())
        {
            ss << "    PUBLIC\n";

            for (const auto& path : module.PublicIncludePaths)
                ss << "        \"${MODULE_SOURCE_DIR}/" << path << "\"\n";
        }

        ss << "    PRIVATE\n";

        for (const auto& path : module.PrivateIncludePaths)
            ss << "        \"${MODULE_SOURCE_DIR}/" << path << "\"\n";

        ss << "        \"${MODULE_GENERATED_DIR}\"\n";

        ss << ")";

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakePublicDependenciesBlock(const ModuleRules& module)
    {
        if (module.PublicDependencies.empty())
            return {};

        std::stringstream ss;

        ss << "target_link_libraries(${MODULE_TARGET}\n";
        ss << "    PUBLIC\n";
        ss << JoinIndented(module.PublicDependencies);
        ss << ")";

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeThirdPartyBlock(const ModuleRules& module)
    {
        if (module.ThirdParty.empty())
            return {};

        std::stringstream ss;

        for (const auto& thirdParty : module.ThirdParty)
        {
            ss << "include(\"${BOON_CMAKE_DIR}/ThirdParty/"
                << thirdParty << ".cmake\")\n";

            ss << "boon_use_" << thirdParty
                << "(${MODULE_TARGET})\n\n";
        }

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeSystemLibrariesBlock(const ModuleRules& module)
    {
        if (module.SystemLibraries.empty())
            return {};

        std::stringstream ss;

        ss << "if(WIN32)\n";
        ss << "    target_link_libraries(${MODULE_TARGET}\n";
        ss << "        PRIVATE\n";
        ss << JoinIndented(module.SystemLibraries);
        ss << "    )\n";
        ss << "endif()";

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeGeneratedReflectionDeclarations(const ModuleRules& module)
    {
        if (!module.Reflection)
            return {};

        std::stringstream ss;

        ss << "    void RegisterGeneratedClasses_" << module.Name << "(\n";
        ss << "        BClassRegistry& classRegistry,\n";
        ss << "        NetRepRegistry& netRegistry);\n\n";

        ss << "    void UnregisterGeneratedClasses_" << module.Name << "(\n";
        ss << "        BClassRegistry& classRegistry,\n";
        ss << "        NetRepRegistry& netRegistry);";

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeRegisterReflectionCall(const ModuleRules& module)
    {
        if (!module.Reflection)
            return {};

        std::stringstream ss;

        ss << "        if (context->BClasses && context->NetReps)\n";
        ss << "        {\n";
        ss << "            Boon::RegisterGeneratedClasses_" << module.Name << "(\n";
        ss << "                *context->BClasses,\n";
        ss << "                *context->NetReps\n";
        ss << "            );\n";
        ss << "        }";

        return ss.str();
    }

    std::string ModuleTemplateBuilder::MakeUnregisterReflectionCall(const ModuleRules& module)
    {
        if (!module.Reflection)
            return {};

        std::stringstream ss;

        ss << "        if (context->BClasses && context->NetReps)\n";
        ss << "        {\n";
        ss << "            Boon::UnregisterGeneratedClasses_" << module.Name << "(\n";
        ss << "                *context->BClasses,\n";
        ss << "                *context->NetReps\n";
        ss << "            );\n";
        ss << "        }";

        return ss.str();
    }
}