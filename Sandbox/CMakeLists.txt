# Sandbox/CMakeLists.txt

file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_link_libraries(Sandbox
    PRIVATE BoonEngine
)

target_include_directories(Sandbox
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set output directory for Sandbox executable
set_target_properties(Sandbox PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Sandbox/$<CONFIG>
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Sandbox/$<CONFIG>
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Sandbox/$<CONFIG>
)

# =========================================
# === Copy assets after build
# =========================================
add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:Sandbox>/Assets
    COMMENT "Copying Editor Assets folder to output directory..."
)