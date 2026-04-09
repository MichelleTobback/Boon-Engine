cmake_minimum_required(VERSION 3.20)
project(Runtime LANGUAGES CXX)

# --------------------------------------------------
# Reflection for runtime module
# --------------------------------------------------
set(RUNTIME_GENERATED_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/Generated_Components.cpp
)

boon_add_reflected_module(
    NAME Runtime
    OUTPUT ${RUNTIME_GENERATED_CPP}
    SCAN_DIRS
        ${CMAKE_SOURCE_DIR}/Boon/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    VERBOSE
)

set_source_files_properties(${RUNTIME_GENERATED_CPP} PROPERTIES GENERATED TRUE)

# --------------------------------------------------
# Sources
# --------------------------------------------------
file(GLOB_RECURSE RUNTIME_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

file(GLOB_RECURSE RUNTIME_HEADERS CONFIGURE_DEPENDS
    include/*.h
    include/*.hpp
)

# --------------------------------------------------
# Runtime executable
# --------------------------------------------------
add_executable(Runtime
    ${RUNTIME_SOURCES}
    ${RUNTIME_HEADERS}
    ${RUNTIME_GENERATED_CPP}
)

add_dependencies(Runtime GenerateReflection)

target_compile_features(Runtime PRIVATE cxx_std_20)

target_compile_definitions(Runtime PRIVATE BOON_MODULE_NAME=Runtime)

target_link_libraries(Runtime
    PRIVATE
        BoonEngine
)

target_include_directories(Runtime
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

boon_set_output_dirs(Runtime Runtime)

# --------------------------------------------------
# Copy dependencies
# --------------------------------------------------

set(STEAMNET_DIR     "${CMAKE_SOURCE_DIR}/Boon/external/GameNetworkingSockets")
set(STEAMNET_BIN_DIR "${STEAMNET_DIR}/bin")

if (WIN32)
    add_custom_command(TARGET Runtime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${STEAMNET_BIN_DIR}/$<CONFIG>/GameNetworkingSockets.dll"
            "${STEAMNET_BIN_DIR}/$<CONFIG>/libcrypto-3-x64.dll"
            "$<$<CONFIG:Debug>:${STEAMNET_BIN_DIR}/Debug/libprotobufd.dll>"
            "$<$<CONFIG:Release>:${STEAMNET_BIN_DIR}/Release/libprotobuf.dll>"
            "${STEAMNET_BIN_DIR}/$<CONFIG>/abseil_dll.dll"
            "$<TARGET_FILE_DIR:Runtime>"
        VERBATIM
    )
endif()

