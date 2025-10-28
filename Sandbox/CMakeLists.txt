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
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Sandbox
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Sandbox
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Sandbox
)