cmake_minimum_required(VERSION 3.20)
project(BoonWorkspace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
add_subdirectory(Sandbox)
add_subdirectory(Editor)
add_subdirectory(Runtime)