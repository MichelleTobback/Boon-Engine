#include "ProjectTemplateBuilder.h"

#include <sstream>

namespace BoonBuild
{
    ProjectTemplateData ProjectTemplateBuilder::BuildProjectData(
        const ProjectRules& project,
        const std::vector<ModuleRules>& selectedEngineModules) const
    {
        ProjectTemplateData data;

        data.ProjectName = project.Name;
        data.OutputType = project.Output;

        data.ReflectionBlock = MakeReflectionBlock(project);
        data.ReflectionDependencyBlock = MakeReflectionDependencyBlock(project);

        data.CompileDefinitionsBlock = MakeCompileDefinitionsBlock(project, selectedEngineModules);
        data.IncludeDirectoriesBlock = MakeIncludeDirectoriesBlock(project, selectedEngineModules);
        data.LinkLibrariesBlock = MakeLinkLibrariesBlock(project, selectedEngineModules);
        data.AssetCopyBlock = MakeAssetCopyBlock(project);
        data.ProjectEngineModulesBlock = MakeProjectEngineModulesBlock(selectedEngineModules);

        const std::vector<std::string> staticModuleNames =
            MakeProjectStaticModuleNames(project, selectedEngineModules);

        data.ProjectStaticModuleDeclarations =
            MakeProjectStaticModuleDeclarations(staticModuleNames);

        data.ProjectStaticModuleRegisterCalls =
            MakeProjectStaticModuleRegisterCalls(staticModuleNames);

        data.GeneratedReflectionDeclarations = MakeGeneratedReflectionDeclarations(project);
        data.RegisterReflectionCall = MakeRegisterReflectionCall(project);
        data.UnregisterReflectionCall = MakeUnregisterReflectionCall(project);

        data.StaticModuleManifestEntries = MakeStaticModuleManifestEntries(project, selectedEngineModules);
        data.DynamicModuleManifestEntries = MakeDynamicModuleManifestEntries(project);

        data.CreateModuleInstanceBody = MakeCreateModuleInstanceBody(project);
        data.DestroyModuleInstanceBody = MakeDestroyModuleInstanceBody(project);

        return data;
    }

    std::string ProjectTemplateBuilder::JoinIndented(const std::vector<std::string>& values, std::string_view indent)
    {
        std::stringstream ss;

        for (const auto& value : values)
            ss << indent << value << "\n";

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeReflectionBlock(const ProjectRules& project)
    {
        if (!project.Reflection)
            return {};

        std::stringstream ss;

        ss << "boon_add_reflected_module(\n";
        ss << "    NAME ${GAME_TARGET}\n";
        ss << "    OUTPUT ${GAME_GENERATED_COMPONENTS_CPP}\n";
        ss << "    SCAN_DIRS\n";
        ss << "        \"${GAME_SOURCE_DIR}/src\"\n";

        if (project.MinimalReflection)
            ss << "    MINIMAL\n";

        ss << ")\n\n";

        ss << "set_source_files_properties(\n";
        ss << "    ${GAME_GENERATED_COMPONENTS_CPP}\n";
        ss << "    PROPERTIES GENERATED TRUE\n";
        ss << ")";

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeReflectionDependencyBlock(const ProjectRules& project)
    {
        if (!project.Reflection)
            return {};

        return "add_dependencies(${GAME_TARGET} GenerateReflection_${GAME_TARGET})";
    }

    std::string ProjectTemplateBuilder::MakeCompileDefinitionsBlock(
        const ProjectRules& project,
        const std::vector<ModuleRules>& selectedEngineModules)
    {
        std::vector<std::string> publicDefinitions = project.PublicDefinitions;

        for (const ModuleRules& module : selectedEngineModules)
        {
            for (const std::string& definition : module.PublicDefinitions)
                publicDefinitions.push_back(definition);
        }

        std::stringstream ss;

        ss << "target_compile_definitions(${GAME_TARGET}\n";
        ss << "    PRIVATE\n";
        ss << "        BOON_MODULE_NAME=${GAME_TARGET}\n";

        if (!publicDefinitions.empty())
        {
            ss << "    PUBLIC\n";
            ss << JoinIndented(publicDefinitions);
        }

        if (!project.PrivateDefinitions.empty())
        {
            ss << "    PRIVATE\n";
            ss << JoinIndented(project.PrivateDefinitions);
        }

        ss << ")";

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeIncludeDirectoriesBlock(
        const ProjectRules& project,
        const std::vector<ModuleRules>& selectedEngineModules)
    {
        std::stringstream ss;

        ss << "target_include_directories(${GAME_TARGET}\n";

        if (!project.PublicIncludePaths.empty())
        {
            ss << "    PUBLIC\n";

            for (const auto& path : project.PublicIncludePaths)
                ss << "        \"${GAME_SOURCE_DIR}/" << path << "\"\n";
        }

        ss << "    PRIVATE\n";

        for (const auto& path : project.PrivateIncludePaths)
            ss << "        \"${GAME_SOURCE_DIR}/" << path << "\"\n";

        ss << "        \"${GAME_GENERATED_DIR}\"\n";

        for (const ModuleRules& module : selectedEngineModules)
        {
            for (const std::string& path : module.PublicIncludePaths)
            {
                ss << "        \"${BOON_ENGINE_ROOT}/modules/"
                    << module.Name << "/" << path << "\"\n";
            }
        }

        ss << ")";

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeLinkLibrariesBlock(
        const ProjectRules& project,
        const std::vector<ModuleRules>& selectedEngineModules)
    {
        std::vector<std::string> publicDependencies = project.PublicDependencies;

        for (const ModuleRules& module : selectedEngineModules)
        {
            for (const std::string& dependency : module.PublicDependencies)
                publicDependencies.push_back(dependency);
        }

        std::stringstream ss;

        ss << "set(GAME_STATIC_MODULE_TARGETS)\n\n";
        ss << "list(APPEND GAME_STATIC_MODULE_TARGETS ${BOON_PROJECT_ENGINE_MODULES})\n";

        for (const auto& module : project.GameModules)
            ss << "list(APPEND GAME_STATIC_MODULE_TARGETS " << module << ")\n";

        ss << "\n";

        ss << "target_link_libraries(${GAME_TARGET}\n";
        ss << "    PRIVATE\n";
        ss << "        BoonEngine\n";
        ss << "        ${GAME_STATIC_MODULE_TARGETS}\n";

        for (const auto& dependency : project.PrivateDependencies)
            ss << "        " << dependency << "\n";

        if (!publicDependencies.empty())
        {
            ss << "    PUBLIC\n";
            ss << JoinIndented(publicDependencies);
        }

        ss << ")";

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeAssetCopyBlock(const ProjectRules& project)
    {
        if (project.AssetDirectories.empty())
            return {};

        std::stringstream ss;

        for (const auto& dir : project.AssetDirectories)
        {
            ss << "if(EXISTS \"${GAME_SOURCE_DIR}/" << dir << "\")\n";
            ss << "    add_custom_command(TARGET ${GAME_TARGET} POST_BUILD\n";
            ss << "        COMMAND ${CMAKE_COMMAND} -E copy_directory\n";
            ss << "            \"${GAME_SOURCE_DIR}/" << dir << "\"\n";
            ss << "            \"$<TARGET_FILE_DIR:" << project.Name << ">/" << dir << "\"\n";
            ss << "        COMMENT \"Copying " << dir << " for " << project.Name << "\"\n";
            ss << "        VERBATIM\n";
            ss << "    )\n";
            ss << "endif()\n\n";
        }

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeProjectEngineModulesBlock(const std::vector<ModuleRules>& selectedEngineModules)
    {
        std::stringstream ss;

        ss << "set(BOON_PROJECT_ENGINE_MODULES\n";

        for (const ModuleRules& module : selectedEngineModules)
            ss << "    " << module.Name << "\n";

        ss << ")";

        return ss.str();
    }

    std::vector<std::string> ProjectTemplateBuilder::MakeProjectStaticModuleNames(
        const ProjectRules& project,
        const std::vector<ModuleRules>& selectedEngineModules)
    {
        std::vector<std::string> names;

        for (const ModuleRules& module : selectedEngineModules)
        {
            if (module.Output == "STATIC")
                names.push_back(module.Name);
        }

        for (const std::string& module : project.GameModules)
            names.push_back(module);

        names.push_back(project.Name);

        return names;
    }

    std::string ProjectTemplateBuilder::MakeProjectStaticModuleDeclarations(
        const std::vector<std::string>& moduleNames)
    {
        std::stringstream ss;

        for (const std::string& moduleName : moduleNames)
        {
            ss << "    const Boon::ModuleInfo* " << moduleName << "_GetModuleInfo();\n";
            ss << "    Boon::ModuleRegistration " << moduleName
                << "_RegisterModule(Boon::ModuleContext*);\n";
            ss << "    void " << moduleName
                << "_UnregisterModule(Boon::ModuleContext*, Boon::ModuleInstance*);\n\n";
        }

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeProjectStaticModuleRegisterCalls(
        const std::vector<std::string>& moduleNames)
    {
        std::stringstream ss;

        for (const std::string& moduleName : moduleNames)
        {
            ss << "        {\n";
            ss << "            const Boon::ModuleInfo* info = "
                << moduleName << "_GetModuleInfo();\n";
            ss << "            Boon::ModuleRegistration registration = "
                << moduleName << "_RegisterModule(&context);\n\n";
            ss << "            if (!registration.Success)\n";
            ss << "            {\n";
            ss << "                result = false;\n";
            ss << "            }\n";
            ss << "            else\n";
            ss << "            {\n";
            ss << "                s_StaticModules.push_back({\n";
            ss << "                    info,\n";
            ss << "                    registration.Instance,\n";
            ss << "                    &" << moduleName << "_UnregisterModule\n";
            ss << "                });\n";
            ss << "            }\n";
            ss << "        }\n\n";
        }

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeGeneratedReflectionDeclarations(const ProjectRules& project)
    {
        if (!project.Reflection)
            return {};

        std::stringstream ss;

        ss << "    void RegisterGeneratedClasses_" << project.Name << "(\n";
        ss << "        BClassRegistry& classRegistry,\n";
        ss << "        NetRepRegistry& netRegistry);\n\n";

        ss << "    void UnregisterGeneratedClasses_" << project.Name << "(\n";
        ss << "        BClassRegistry& classRegistry,\n";
        ss << "        NetRepRegistry& netRegistry);";

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeRegisterReflectionCall(const ProjectRules& project)
    {
        if (!project.Reflection)
            return {};

        std::stringstream ss;

        ss << "        if (context->BClasses && context->NetReps)\n";
        ss << "        {\n";
        ss << "            Boon::RegisterGeneratedClasses_" << project.Name << "(\n";
        ss << "                *context->BClasses,\n";
        ss << "                *context->NetReps\n";
        ss << "            );\n";
        ss << "        }";

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeUnregisterReflectionCall(const ProjectRules& project)
    {
        if (!project.Reflection)
            return {};

        std::stringstream ss;

        ss << "        if (context->BClasses && context->NetReps)\n";
        ss << "        {\n";
        ss << "            Boon::UnregisterGeneratedClasses_" << project.Name << "(\n";
        ss << "                *context->BClasses,\n";
        ss << "                *context->NetReps\n";
        ss << "            );\n";
        ss << "        }";

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeStaticModuleManifestEntries(
        const ProjectRules& project,
        const std::vector<ModuleRules>& selectedEngineModules)
    {
        std::stringstream ss;

        bool first = true;

        auto addModule = [&](const std::string& name)
            {
                if (!first)
                    ss << ",\n";

                ss << "        {\n";
                ss << "            \"name\": \"" << name << "\",\n";
                ss << "            \"type\": \"STATIC\"\n";
                ss << "        }";

                first = false;
            };

        for (const ModuleRules& module : selectedEngineModules)
            addModule(module.Name);

        for (const auto& module : project.GameModules)
            addModule(module);

        return ss.str();
    }

    std::string ProjectTemplateBuilder::MakeDynamicModuleManifestEntries(const ProjectRules&)
    {
        return {};
    }

    std::string ProjectTemplateBuilder::MakeCreateModuleInstanceBody(const ProjectRules& project)
    {
        if (project.ModuleInstance.empty())
            return "    return nullptr;";

        return "    return " + project.Name + "_CreateUserModuleInstance();";
    }

    std::string ProjectTemplateBuilder::MakeDestroyModuleInstanceBody(const ProjectRules& project)
    {
        if (project.ModuleInstance.empty())
            return "    delete instance;";

        return "    " + project.Name + "_DestroyUserModuleInstance(instance);";
    }
}