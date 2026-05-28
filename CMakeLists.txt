# root CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(BoonWorkspace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------------------
# Boon paths
# ----------------------------------------

if(NOT BOON_REPO_ROOT)
    set(BOON_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}")
endif()

if(EXISTS "${BOON_REPO_ROOT}/CMakeLists.txt" AND
   EXISTS "${BOON_REPO_ROOT}/external" AND
   EXISTS "${BOON_REPO_ROOT}/tools")
    get_filename_component(BOON_REPO_ROOT
        "${BOON_REPO_ROOT}"
        DIRECTORY
    )
endif()

set(BOON_REPO_ROOT
    "${BOON_REPO_ROOT}"
    CACHE PATH
    "Path to Boon repository root"
    FORCE
)

if(NOT EXISTS "${BOON_REPO_ROOT}/Boon/CMakeLists.txt")
    message(FATAL_ERROR
        "Boon repository not found.\n"
        "BOON_REPO_ROOT = ${BOON_REPO_ROOT}\n"
        "Expected: ${BOON_REPO_ROOT}/Boon/CMakeLists.txt"
    )
endif()

set(BOON_ENGINE_ROOT
    "${BOON_REPO_ROOT}/Boon"
    CACHE PATH
    "Path to Boon engine source root"
    FORCE
)

set(BOON_CMAKE_DIR
    "${BOON_ENGINE_ROOT}/cmake"
    CACHE PATH
    "Path to Boon CMake helper directory"
    FORCE
)

message(STATUS "BOON_REPO_ROOT   = ${BOON_REPO_ROOT}")
message(STATUS "BOON_ENGINE_ROOT = ${BOON_ENGINE_ROOT}")
message(STATUS "BOON_CMAKE_DIR   = ${BOON_CMAKE_DIR}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# --------------------------------------------------
# Shared output helper
# --------------------------------------------------
function(boon_set_output_dirs target subdir)
    set_target_properties(${target} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>/${subdir}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>/${subdir}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>/${subdir}"
    )
endfunction()

# --------------------------------------------------
# Build tools first
# --------------------------------------------------
add_subdirectory(Boon/tools/BClassGenerator)

# --------------------------------------------------
# Global reflection umbrella target
# --------------------------------------------------
add_custom_target(GenerateReflection)

# Reflection helper
include(${CMAKE_SOURCE_DIR}/Boon/tools/BClassGenerator/BoonReflection.cmake)

# --------------------------------------------------
# Add modules
# --------------------------------------------------
add_subdirectory(Boon)
add_subdirectory(Boon/tools/Common)
boon_add_engine_modules()
add_subdirectory(Editor)
add_subdirectory(Runtime)