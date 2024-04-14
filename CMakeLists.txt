cmake_minimum_required(VERSION 3.29.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQURIED ON)

project(main)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_compile_options(-O2)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})