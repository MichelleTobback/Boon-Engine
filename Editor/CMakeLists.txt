cmake_minimum_required(VERSION 3.20)
project(Editor LANGUAGES CXX)

# --------------------------------------------------
# Reflection for editor module
# --------------------------------------------------
set(EDITOR_GENERATED_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/Generated_Components.cpp
)

boon_add_reflected_module(
    NAME Editor
    OUTPUT ${EDITOR_GENERATED_CPP}
    SCAN_DIRS
        ${CMAKE_SOURCE_DIR}/Boon/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    VERBOSE
)

set_source_files_properties(${EDITOR_GENERATED_CPP} PROPERTIES GENERATED TRUE)

# --------------------------------------------------
# ImGui
# --------------------------------------------------
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui
    PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw)

# --------------------------------------------------
# Sources
# --------------------------------------------------
file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

file(GLOB_RECURSE EDITOR_HEADERS CONFIGURE_DEPENDS
    include/*.h
    include/*.hpp
)

# --------------------------------------------------
# Editor executable
# --------------------------------------------------
add_executable(Editor WIN32
    ${EDITOR_SOURCES}
    ${EDITOR_HEADERS}
    ${EDITOR_GENERATED_CPP}
)
target_compile_definitions(Editor PRIVATE BOON_EDITOR_NO_CONSOLE)

add_dependencies(Editor GenerateReflection)

target_compile_features(Editor PRIVATE cxx_std_20)

target_compile_definitions(Editor PRIVATE BOON_MODULE_NAME=Editor)

target_link_libraries(Editor
    PRIVATE
        BoonEngine
        imgui
)

target_compile_definitions(Editor
    PRIVATE
        BOON_WITH_EDITOR
)

target_include_directories(Editor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

boon_set_output_dirs(Editor Editor)

# --------------------------------------------------
# Copy assets/resources
# --------------------------------------------------

add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Resources
        $<TARGET_FILE_DIR:Editor>/Assets
    COMMENT "Copying Editor Resources..."
)

set(STEAMNET_DIR     "${CMAKE_SOURCE_DIR}/Boon/external/GameNetworkingSockets")
set(STEAMNET_BIN_DIR "${STEAMNET_DIR}/bin")

if (WIN32)
    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${STEAMNET_BIN_DIR}/$<CONFIG>/GameNetworkingSockets.dll"
            "${STEAMNET_BIN_DIR}/$<CONFIG>/libcrypto-3-x64.dll"
            "$<$<CONFIG:Debug>:${STEAMNET_BIN_DIR}/Debug/libprotobufd.dll>"
            "$<$<CONFIG:Release>:${STEAMNET_BIN_DIR}/Release/libprotobuf.dll>"
            "${STEAMNET_BIN_DIR}/$<CONFIG>/abseil_dll.dll"
            "$<TARGET_FILE_DIR:Editor>"
        VERBATIM
    )
endif()