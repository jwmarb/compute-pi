cmake_minimum_required(VERSION 3.28.3)

set(C_FLAGS "-O3 -fopenmp")
set(CMAKE_CXX_FLAGS ${C_FLAGS})
set(CMAKE_C_FLAGS ${C_FLAGS})

set(GMP_INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ext/gmp/)
set(GMP_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ext/gmp/.libs/libgmp.so)

set(MPI_INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ext/openmpi/include/)
set(MPI_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ext/openmpi/lib/libmpi.so)

set(FMT_INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ext/fmt/include)
set(FMT_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ext/fmt/out/build/libfmt.so)

project(pi_calculator)

include_directories(${PROJECT_NAME} ${GMP_INC_PATH} ${MPI_INC_PATH} ${FMT_INC_PATH})

add_executable(${PROJECT_NAME} 
  src/main_bs.cpp
  src/gmp_extended.c
  src/chudnovsky_bs.c
  )

# add_executable(${PROJECT_NAME} src/mpitest.c)

target_link_libraries(${PROJECT_NAME} ${GMP_LIB_PATH} ${MPI_LIB_PATH} ${FMT_LIB_PATH})