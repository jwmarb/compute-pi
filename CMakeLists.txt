cmake_minimum_required(VERSION 3.29.0)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(GMP_INC_PATH /usr/local/gmp-6.3.0/)
set(GMP_LIB_PATH /usr/local/gmp-6.3.0/.libs/libgmp.a)

set(MPI_INC_PATH /usr/local/openmpi/include/)
set(MPI_LIB_PATH /usr/local/openmpi/lib/libmpi.so)

project(pi_calculator)

include_directories(${PROJECT_NAME} ${GMP_INC_PATH} ${MPI_INC_PATH})

add_executable(${PROJECT_NAME} 
  src/main.cpp 
  src/gmp_extended.c
  src/chudnovsky.c
  )

target_link_libraries(${PROJECT_NAME} ${GMP_LIB_PATH} ${MPI_LIB_PATH})