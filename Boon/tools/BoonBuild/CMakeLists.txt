cmake_minimum_required(VERSION 3.20)

project(BoonBuild LANGUAGES CXX)

# --------------------------------------------------
# Workspace root
# Boon/Tools/BoonBuild -> ../../.. -> Boon-Engine
# --------------------------------------------------
get_filename_component(BOON_WORKSPACE_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../../.."
    ABSOLUTE
)

# --------------------------------------------------
# Only place executables in workspace bin
# --------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${BOON_WORKSPACE_ROOT}/bin/$<CONFIG>/Tools"
)

# --------------------------------------------------
# Shared output helper fallback
# --------------------------------------------------
if(NOT COMMAND boon_set_output_dirs)
    function(boon_set_output_dirs target subdir)
        set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${BOON_WORKSPACE_ROOT}/bin/$<CONFIG>/${subdir}"
        )
    endfunction()
endif()

# --------------------------------------------------
# Shared tool code
# --------------------------------------------------
if(NOT TARGET BoonToolsCommon)
    add_subdirectory(
        "${CMAKE_CURRENT_LIST_DIR}/../Common"
        "${CMAKE_BINARY_DIR}/BoonToolsCommon"
    )
endif()

# --------------------------------------------------
# BoonBuild
# --------------------------------------------------
add_executable(BoonBuild
    src/main.cpp

    src/ModuleRulesReader.cpp
    src/ModuleTemplateBuilder.cpp
    src/TemplateFileGenerator.cpp
    src/BoonBuildGenerator.cpp

    src/ProjectRulesReader.cpp
    src/ProjectTemplateBuilder.cpp
    src/ModuleDependencyGraph.cpp

    src/ModuleRules.h
    src/ModuleRulesReader.h
    src/ModuleTemplateData.h
    src/ModuleTemplateBuilder.h

    src/ProjectRules.h
    src/ProjectRulesReader.h
    src/ProjectTemplateData.h
    src/ProjectTemplateBuilder.h

    src/TemplateFileGenerator.h
    src/BoonBuildGenerator.h
    src/BuildLog.h
    src/ModuleDependencyGraph.h
 "src/ModuleDependencyGraph.h" "src/ModuleDependencyGraph.cpp")

set_target_properties(BoonBuild PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BOON_WORKSPACE_ROOT}/bin/$<CONFIG>/Tools"
    PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/DebugFiles"
    COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/DebugFiles"
)

if(MSVC)
    target_link_options(BoonBuild PRIVATE
        "/ILK:${CMAKE_CURRENT_BINARY_DIR}/DebugFiles/BoonBuild.ilk"
    )
endif()

target_compile_features(BoonBuild PRIVATE cxx_std_20)

target_include_directories(BoonBuild
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/src"
        "${CMAKE_CURRENT_LIST_DIR}/../../external/json/single_include"
)

target_link_libraries(BoonBuild
    PRIVATE
        BoonToolsCommon
)

boon_set_output_dirs(BoonBuild Tools)