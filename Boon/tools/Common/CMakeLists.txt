# Boon/tools/Common/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)

project(BoonToolsCommon LANGUAGES CXX)

add_library(BoonToolsCommon STATIC
    src/TemplateProcessor.cpp
    "include/Tools/TemplateProcessor.h"
)

target_compile_features(BoonToolsCommon PUBLIC cxx_std_20)

target_include_directories(BoonToolsCommon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

boon_set_output_dirs(BoonToolsCommon Tools)