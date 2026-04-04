cmake_minimum_required(VERSION 3.20)
project(BClassGenerator LANGUAGES CXX)

add_executable(BClassGenerator
    src/main.cpp
)

target_compile_features(BClassGenerator PRIVATE cxx_std_20)

boon_set_output_dirs(BClassGenerator Tools)