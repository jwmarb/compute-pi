cmake_minimum_required(VERSION 3.29.0)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(FMT_INC_PATH /usr/local/vcpkg/packages/fmt_x64-linux/include)
set(FMT_LIB_PATH /usr/local/vcpkg/packages/fmt_x64-linux/lib/libfmt.a)

set(GMP_INC_PATH /usr/local/gmp-6.3.0/)
set(GMP_LIB_PATH /usr/local/gmp-6.3.0/.libs/libgmp.a)

project(pi_calculator)

# find_package(fmt CONFIG REQUIRED)
include_directories(${PROJECT_NAME} ${GMP_INC_PATH})

include_directories(${PROJECT_NAME} ${FMT_INC_PATH})

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} ${GMP_LIB_PATH} ${FMT_LIB_PATH})