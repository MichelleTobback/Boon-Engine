# Boon/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(BoonEngine LANGUAGES CXX)

# --------------------------------------------------
# Reflection for engine module
# --------------------------------------------------
set(BOON_GENERATED_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/Generated_Components.cpp
)

boon_add_reflected_module(
    NAME BoonEngine
    OUTPUT ${BOON_GENERATED_CPP}
    SCAN_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_source_files_properties(${BOON_GENERATED_CPP} PROPERTIES GENERATED TRUE)

# --------------------------------------------------
# Sources
# --------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
    include/*.h
    include/*.hpp
)

# --------------------------------------------------
# Engine library
# --------------------------------------------------
add_library(BoonEngine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
    ${BOON_GENERATED_CPP}
)

add_dependencies(BoonEngine GenerateReflection)

target_compile_features(BoonEngine PUBLIC cxx_std_20)

target_compile_definitions(BoonEngine PRIVATE BOON_MODULE_NAME=BoonEngine)

target_include_directories(BoonEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# --------------------------------------------------
# Header-only dependencies
# --------------------------------------------------
target_include_directories(BoonEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/external/json/single_include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/box2d/include
)

# --------------------------------------------------
# External dependencies
# --------------------------------------------------
add_subdirectory(external/glad)
add_subdirectory(external/glfw)
add_subdirectory(external/entt)
add_subdirectory(external/box2d)

find_package(OpenGL REQUIRED)

target_link_libraries(BoonEngine
    PUBLIC
        glad
        glfw
        EnTT::EnTT
        OpenGL::GL
    PRIVATE
        box2d
)

# --------------------------------------------------
# Platform defines
# --------------------------------------------------
if (WIN32)
    target_compile_definitions(BoonEngine PUBLIC BOON_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(BoonEngine PUBLIC BOON_PLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(BoonEngine PUBLIC BOON_PLATFORM_LINUX)
endif()

# --------------------------------------------------
# Warnings
# --------------------------------------------------
if (MSVC)
    target_compile_options(BoonEngine PRIVATE /W4 /permissive-)
else()
    target_compile_options(BoonEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()

# --------------------------------------------------
# Output dirs
# --------------------------------------------------
boon_set_output_dirs(BoonEngine Boon)

# --------------------------------------------------
# Copy assets/resources
# --------------------------------------------------
add_custom_command(TARGET BoonEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:BoonEngine>/Assets
    COMMENT "Copying Boon Engine Resources..."
)

function(boon_add_engine_modules)
    if(NOT TARGET BoonStaticModules)
        add_subdirectory(
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/modules/generated"
            "${CMAKE_BINARY_DIR}/BoonModules"
        )
    endif()
endfunction()