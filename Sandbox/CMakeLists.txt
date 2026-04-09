cmake_minimum_required(VERSION 3.20)
project(SandboxGame LANGUAGES CXX)

# --------------------------------------------------
# Reflection for sandbox module
# --------------------------------------------------
set(SANDBOX_GENERATED_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/Generated_Components.cpp
)

boon_add_reflected_module(
    NAME SandboxGame
    OUTPUT ${SANDBOX_GENERATED_CPP}
    SCAN_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    MINIMAL
)

set_source_files_properties(${SANDBOX_GENERATED_CPP} PROPERTIES GENERATED TRUE)

# --------------------------------------------------
# Sources
# --------------------------------------------------
file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
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
    ${SANDBOX_GENERATED_CPP}
  )

add_dependencies(SandboxGame GenerateReflection)

target_compile_features(SandboxGame PUBLIC cxx_std_20)

target_compile_definitions(SandboxGame PRIVATE BOON_MODULE_NAME=SandboxGame)

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

add_custom_command(TARGET SandboxGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE_DIR:SandboxGame>/SandboxGame.dll"
        "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>/Editor/SandboxGame.dll"
    VERBATIM
)
