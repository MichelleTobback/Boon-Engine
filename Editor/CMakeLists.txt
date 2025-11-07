# ============================
# === Editor/CMakeLists.txt
# ============================

file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS src/*.cpp)

# ============================
# === Editor Executable
# ============================
add_executable(Editor
    ${EDITOR_SOURCES}
)

target_link_libraries(Editor PRIVATE BoonEngine)

target_compile_definitions(BoonEngine PRIVATE BOON_WITH_EDITOR)

target_include_directories(Editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# =========================================
# === Output directories
# =========================================
set_target_properties(Editor PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Editor/$<CONFIG>
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Editor/$<CONFIG>
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Editor/$<CONFIG>
)

# =========================================
# === Copy assets after build
# =========================================
add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:Editor>/Assets
    COMMENT "Copying Editor Assets..."
)

add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Resources
        $<TARGET_FILE_DIR:Editor>/Assets
    COMMENT "Copying Editor Resources..."
)

# =========================================
# === ImGui (docking)
# =========================================
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

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_SOURCE_DIR}/BoonEngine/external/glfw/include
)

target_link_libraries(imgui PUBLIC glfw)

target_link_libraries(Editor PRIVATE imgui)

target_include_directories(Editor PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

add_dependencies(Editor GenerateReflection)
target_sources(Editor PRIVATE ${CMAKE_SOURCE_DIR}/Editor/generated/Generated_Components.cpp)
