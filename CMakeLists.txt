cmake_minimum_required(VERSION 3.29.0)

set(C_FLAGS "-O3 -fopenmp")
set(CMAKE_CXX_FLAGS ${C_FLAGS})
set(CMAKE_C_FLAGS ${C_FLAGS})

set(GMP_INC_PATH /usr/local/gmp/)
set(GMP_LIB_PATH /usr/local/gmp/.libs/libgmp.so)

set(MPI_INC_PATH /usr/local/openmpi/include/)
set(MPI_LIB_PATH /usr/local/openmpi/lib/libmpi.so)

set(FMT_INC_PATH /usr/local/fmt/include)
set(FMT_LIB_PATH /usr/local/fmt/out/build/libfmt.so)

project(pi_calculator)

include_directories(${PROJECT_NAME} ${GMP_INC_PATH} ${MPI_INC_PATH} ${FMT_INC_PATH})

add_executable(${PROJECT_NAME} 
  src/main.cpp
  src/gmp_extended.c
  src/chudnovsky.c
  )

target_link_libraries(${PROJECT_NAME} ${GMP_LIB_PATH} ${MPI_LIB_PATH} ${FMT_LIB_PATH})