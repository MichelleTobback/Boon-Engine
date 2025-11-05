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
set(COMPONENT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Boon/include)

# Output locations for generated files
set(GENERATED_EDITOR   ${CMAKE_SOURCE_DIR}/Editor/generated/Generated_Components.cpp)
set(GENERATED_SANDBOX  ${CMAKE_SOURCE_DIR}/Sandbox/generated/Generated_Components.cpp)

# Ensure initial files exist (IDE indexing)
file(WRITE ${GENERATED_EDITOR}  "// Generated - will be overwritten\n")
file(WRITE ${GENERATED_SANDBOX} "// Generated - will be overwritten\n")

# Run generator
add_custom_command(
    OUTPUT ${GENERATED_EDITOR} ${GENERATED_SANDBOX}
    COMMENT "Generating BClass reflection (Editor)"
    COMMAND BClassGenerator ${COMPONENT_INCLUDE_DIR} ${GENERATED_EDITOR}
    COMMENT "Generating BClass reflection (Sandbox)"
    COMMAND BClassGenerator ${COMPONENT_INCLUDE_DIR} ${GENERATED_SANDBOX}
    DEPENDS BClassGenerator
)

add_custom_target(GenerateReflection
    DEPENDS ${GENERATED_EDITOR} ${GENERATED_SANDBOX}
)

# ---------------------------
# 4) Add Editor + Sandbox
# ---------------------------
add_subdirectory(Editor)
add_subdirectory(Sandbox)
