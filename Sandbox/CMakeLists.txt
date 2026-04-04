cmake_minimum_required(VERSION 3.20)
project(SandboxGame LANGUAGES CXX)

# --------------------------------------------------
# Reflection for sandbox module
# --------------------------------------------------
boon_add_reflected_module(
    NAME SandboxGame
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/generated/Generated_Components.cpp
    SCAN_DIRS
        ${CMAKE_SOURCE_DIR}/Boon/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    MINIMAL
)

# --------------------------------------------------
# Sources
# --------------------------------------------------
file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    generated/*.cpp
)

file(GLOB_RECURSE SANDBOX_HEADERS CONFIGURE_DEPENDS
    include/*.h
    include/*.hpp
)

# --------------------------------------------------
# Game module
# --------------------------------------------------
add_library(SandboxGame SHARED
    ${SANDBOX_SOURCES}
    ${SANDBOX_HEADERS}
  )

add_dependencies(SandboxGame GenerateReflection)

target_compile_features(SandboxGame PUBLIC cxx_std_20)

target_link_libraries(SandboxGame
    PRIVATE
        BoonEngine
)

target_include_directories(SandboxGame
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(SandboxGame
    PRIVATE
        BOON_GAME_BUILD
)

boon_set_output_dirs(SandboxGame Sandbox)

# --------------------------------------------------
# Copy assets
# --------------------------------------------------
add_custom_command(TARGET SandboxGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Boon/Assets
        $<TARGET_FILE_DIR:SandboxGame>/Assets
    COMMENT "Copying Engine assets"
)

add_custom_command(TARGET SandboxGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:SandboxGame>/Assets
    COMMENT "Copying Sandbox assets"
)

add_custom_command(TARGET SandboxGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>/Runtime"
        "$<TARGET_FILE_DIR:SandboxGame>"
    VERBATIM
)
