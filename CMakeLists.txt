cmake_minimum_required(VERSION 3.22)
project(BoonEngineProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add projects
add_subdirectory(Boon)
add_subdirectory(Editor)
add_subdirectory(Sandbox)

#add_custom_target(copy_resources ALL)
#add_custom_command(TARGET copy_resources POST_BUILD 
#						COMMAND ${CMAKE_COMMAND} -E copy_directory
#						${CMAKE_SOURCE_DIR}/Resources
#						${CMAKE_CURRENT_BINARY_DIR}/Resources)