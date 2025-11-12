cmake_minimum_required(VERSION 3.15)
project(BoonEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================================
# === Find Engine Sources
# =========================================
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS include/*.h)

# =========================================
# === Build Engine Lib
# =========================================
add_library(BoonEngine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
    ${GENERATED_COMPONENTS}
)

add_dependencies(BoonEngine GenerateReflection)

target_include_directories(BoonEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# =========================================
# === Output directories
# =========================================
set_target_properties(BoonEngine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Boon/$<CONFIG>
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Boon/$<CONFIG>
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Boon/$<CONFIG>
)

# =========================================
# === External dependencies
# =========================================

# glm
target_include_directories(BoonEngine PUBLIC external/glm)

# glad
add_subdirectory(external/glad)
target_link_libraries(BoonEngine PUBLIC glad)

# glfw
add_subdirectory(external/glfw)
target_link_libraries(BoonEngine PUBLIC glfw)

# stb
target_include_directories(BoonEngine PUBLIC external/stb)

# json (nlohmann)
target_include_directories(BoonEngine PUBLIC external/json/single_include)

# EnTT
add_subdirectory(external/entt)
target_link_libraries(BoonEngine PUBLIC EnTT::EnTT)

# Box2D
add_subdirectory(external/box2d)
target_include_directories(BoonEngine PUBLIC external/box2d/include)
target_link_libraries(BoonEngine PRIVATE box2d)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(BoonEngine PUBLIC OpenGL::GL)

# SteamNetworkingSockets prebuilt import
set(STEAMNET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/GameNetworkingSockets")
set(STEAMNET_INCLUDE "${STEAMNET_DIR}/include")
set(STEAMNET_BIN_DIR "${STEAMNET_DIR}/bin")

add_library(SteamNetworkingSockets STATIC IMPORTED GLOBAL)

# Tell CMake where to find each configuration’s .lib file
set_target_properties(SteamNetworkingSockets PROPERTIES
    IMPORTED_LOCATION_DEBUG   "${STEAMNET_BIN_DIR}/Debug/GameNetworkingSockets.lib"
    IMPORTED_LOCATION_RELEASE "${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${STEAMNET_INCLUDE}"
)

# (Optional, in case the headers use platform macros)
if (WIN32)
    target_compile_definitions(SteamNetworkingSockets INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

target_link_libraries(BoonEngine
    PUBLIC
        SteamNetworkingSockets
        ws2_32     # required by SteamNetworkingSockets on Windows
        crypt32
)

# =========================================
# === Platform Defines
# =========================================
if (WIN32)
    target_compile_definitions(BoonEngine PRIVATE BOON_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(BoonEngine PRIVATE BOON_PLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(BoonEngine PRIVATE BOON_PLATFORM_LINUX)
endif()

# =========================================
# === Warnings
# =========================================
if (MSVC)
    target_compile_options(BoonEngine PRIVATE /W4)
else()
    target_compile_options(BoonEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (WIN32)
    add_custom_command(TARGET BoonEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<IF:$<CONFIG:Debug>,${STEAMNET_BIN_DIR}/Debug/GameNetworkingSockets.dll,${STEAMNET_BIN_DIR}/Release/GameNetworkingSockets.dll>"
        "$<TARGET_FILE_DIR:BoonEngine>"
    )
endif()
