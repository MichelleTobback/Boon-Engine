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
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Editor
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Editor
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Editor
)