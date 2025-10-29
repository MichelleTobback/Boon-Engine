# Editor/CMakeLists.txt

file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(Editor ${EDITOR_SOURCES})

target_link_libraries(Editor
    PRIVATE BoonEngine
)

target_include_directories(Editor
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set output directory for Editor executable
set_target_properties(Editor PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Editor/$<CONFIG>
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Editor/$<CONFIG>
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Editor/$<CONFIG>
)

# Copy Asset folder to bin/Editor after build
add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:Editor>/Assets
    COMMENT "Copying Editor Assets folder to output directory..."
)