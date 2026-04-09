cmake_minimum_required(VERSION 3.20)
project(BoonEngine LANGUAGES CXX)

# --------------------------------------------------
# Reflection for engine module
# --------------------------------------------------
boon_add_reflected_module(
    NAME BoonEngine
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/generated/Generated_Components.cpp
    SCAN_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --------------------------------------------------
# Sources
# --------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    generated/*.cpp
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
# External dependencies
# --------------------------------------------------

# Header-only
target_include_directories(BoonEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/external/json/single_include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/box2d/include
)

# glad
add_subdirectory(external/glad)

# glfw
add_subdirectory(external/glfw)

# EnTT
add_subdirectory(external/entt)

# Box2D
add_subdirectory(external/box2d)

# OpenGL
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
# SteamNetworkingSockets prebuilt import
# --------------------------------------------------
set(STEAMNET_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/external/GameNetworkingSockets")
set(STEAMNET_INCLUDE "${STEAMNET_DIR}/include")
set(STEAMNET_BIN_DIR "${STEAMNET_DIR}/bin")

add_library(SteamNetworkingSockets STATIC IMPORTED GLOBAL)

set_target_properties(SteamNetworkingSockets PROPERTIES
    IMPORTED_LOCATION_DEBUG          "${STEAMNET_BIN_DIR}/Debug/GameNetworkingSockets.lib"
    IMPORTED_LOCATION_RELEASE        "${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.lib"
    IMPORTED_LOCATION_RELWITHDEBINFO "${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.lib"
    IMPORTED_LOCATION_MINSIZEREL     "${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.lib"
    INTERFACE_INCLUDE_DIRECTORIES    "${STEAMNET_INCLUDE}"
)

if (WIN32)
    target_compile_definitions(SteamNetworkingSockets INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

target_link_libraries(BoonEngine PUBLIC SteamNetworkingSockets)

if (WIN32)
    target_link_libraries(BoonEngine PUBLIC ws2_32 crypt32)
endif()


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

# --------------------------------------------------
# Copy SteamNetworkingSockets runtime dll
# --------------------------------------------------
if (WIN32)
    add_custom_command(TARGET BoonEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<IF:$<CONFIG:Debug>,${STEAMNET_BIN_DIR}/Debug/GameNetworkingSockets.dll,${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.dll>"
            "$<TARGET_FILE_DIR:BoonEngine>"
        COMMENT "Copying SteamNetworkingSockets.dll"
    )
endif()