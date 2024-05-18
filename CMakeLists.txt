cmake_minimum_required(VERSION 3.28.3)

set(C_FLAGS "-O3 -fopenmp")
set(CMAKE_CXX_FLAGS ${C_FLAGS})
set(CMAKE_C_FLAGS ${C_FLAGS})

set(GMP_INC_PATH /opt/ext/gmp)
set(GMP_LIB_PATH /opt/ext/gmp/.libs/libgmp.so)

set(MPI_INC_PATH /openmpi-3.1.4/ompi/include/)
set(MPI_LIB_PATH /usr/local/lib/libmpi.so)

# For running on local machine
# set(MPI_INC_PATH ${CMAKE_SOURCE_DIR}/ext/ompi/include/)
# set(MPI_LIB_PATH ${CMAKE_SOURCE_DIR}/ext/ompi/lib/libmpi.so)
# set(GMP_INC_PATH ${CMAKE_SOURCE_DIR}/ext/gmp)
# set(GMP_LIB_PATH ${CMAKE_SOURCE_DIR}/ext/gmp/.libs/libgmp.so)

project(pi_calculator)

add_library(
  chudnovsky SHARED
  src/chudnovsky.c 
  src/chudnovsky.h
)

add_library(
  gmp_extended SHARED
  src/gmp_extended.c
  src/gmp_extended.h
)

include_directories(${GMP_INC_PATH} ${MPI_INC_PATH})

add_executable(${PROJECT_NAME} 
  src/main.cpp
)

add_executable(pi_reducer src/reducer.cpp)

target_link_libraries(${PROJECT_NAME} ${GMP_LIB_PATH} ${MPI_LIB_PATH} chudnovsky gmp_extended)
target_link_libraries(pi_reducer ${GMP_LIB_PATH} chudnovsky gmp_extended)