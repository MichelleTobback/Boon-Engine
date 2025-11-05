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

# =========================================
# === OpenGL
# =========================================
find_package(OpenGL REQUIRED)
target_link_libraries(BoonEngine PUBLIC OpenGL::GL)

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
