cmake_minimum_required(VERSION 3.22)
project(BoonEngineProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add projects
add_subdirectory(Boon)
add_subdirectory(Editor)
add_subdirectory(Sandbox)
