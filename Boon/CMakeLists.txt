# Boon/CMakeLists.txt

cmake_minimum_required(VERSION 3.15)
project(BoonEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================================
# === Source and header files
# =========================================
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS include/*.h)

add_library(BoonEngine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

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

# --- glm (header-only)
target_include_directories(BoonEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
)

# GLAD (OpenGL loader)
add_subdirectory(external/glad)
target_link_libraries(BoonEngine PUBLIC glad)

# --- glfw (window + input)
add_subdirectory(external/glfw)
target_link_libraries(BoonEngine PUBLIC glfw)

# --- stb_image (header-only image loader)
target_include_directories(BoonEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
)

# --- EnTT (ecs)
add_subdirectory(external/entt)
target_link_libraries(${PROJECT_NAME} PUBLIC EnTT::EnTT)

# =========================================
# === Platform-specific settings
# =========================================
if (WIN32)
    target_compile_definitions(BoonEngine PRIVATE BOON_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(BoonEngine PRIVATE BOON_PLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(BoonEngine PRIVATE BOON_PLATFORM_LINUX)
endif()

# =========================================
# === Link core libraries
# =========================================
find_package(OpenGL REQUIRED)
target_link_libraries(BoonEngine PUBLIC OpenGL::GL)

# =========================================
# === Misc compiler options
# =========================================
if (MSVC)
    target_compile_options(BoonEngine PRIVATE /W4)
else()
    target_compile_options(BoonEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()
