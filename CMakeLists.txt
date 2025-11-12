cmake_minimum_required(VERSION 3.15)
project(BoonEngineWorkspace)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------
# 1) Add the Engine first
# ---------------------------
add_subdirectory(Boon)

# ---------------------------
# 2) Build the Generator
# ---------------------------
add_subdirectory(Boon/tools/BClassGenerator)

# ---------------------------
# 3) Reflection Code Generation
# ---------------------------

# Directory to scan for components
set(BOON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Boon/include)
set(EDITOR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Editor/include)
set(SANDBOX_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Sandbox/include)

# Output locations for generated files
set(GENERATED_EDITOR   ${CMAKE_SOURCE_DIR}/Editor/generated/Generated_Components.cpp)
set(GENERATED_SANDBOX  ${CMAKE_SOURCE_DIR}/Sandbox/generated/Generated_Components.cpp)

add_custom_command(
    OUTPUT ${GENERATED_EDITOR}
    DEPENDS BClassGenerator ${BOON_INCLUDE_DIR} ${EDITOR_INCLUDE_DIR}
    COMMAND BClassGenerator ${GENERATED_EDITOR} ${BOON_INCLUDE_DIR} ${EDITOR_INCLUDE_DIR}
    COMMENT "Generating BClass reflection for Editor ..."
)

add_custom_command(
    OUTPUT ${GENERATED_SANDBOX}
    DEPENDS BClassGenerator ${BOON_INCLUDE_DIR} ${SANDBOX_INCLUDE_DIR}
    COMMAND $<TARGET_FILE:BClassGenerator> "--minimal" ${GENERATED_SANDBOX} ${BOON_INCLUDE_DIR} ${SANDBOX_INCLUDE_DIR}
    COMMENT "Generating BClass reflection for Sandbox ..."
)

add_custom_target(GenerateReflection
    DEPENDS ${GENERATED_EDITOR} ${GENERATED_SANDBOX}
)

# ---------------------------
# 4) Add Editor + Sandbox
# ---------------------------
add_subdirectory(Editor)
add_subdirectory(Sandbox)
