// Compute n digits of pi
// Optimized for high performance computing

#include <omp.h>
#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <gmp.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include "benchmark.hpp"
extern "C" {
  #include "chudnovsky.h"
  #include "gmp_extended.h"
  #include "allocate.h"
}

/**
 * For testing memory leaks, use the functions below
 * Tracking allocations are on a linked list, and if
 * the list has any allocations, it will be printed.
 * 
 * This should not be used in production due to how slow 
*/

// void* _malloc(size_t size) {
//   return d_malloc(size);
// }

// void* _realloc(void* ptr, size_t old_size, size_t new_size) {
//   return d_realloc(ptr, new_size);
// }

// void _free(void* ptr, size_t size) {
//   return d_free(ptr);
// }

std::string get_readable_digits(unsigned long *digits) {
  std::string readable;
  unsigned long actual_digits = (*digits)/2;
  for (int i = 1; actual_digits > 0; ++i) {
    readable = std::to_string(actual_digits % 10) + readable;
    if (i % 3 == 0) {
      readable = "," + readable;
    }
    actual_digits /= 10;
  }
  if (readable.at(0) == ',') {
    readable = readable.substr(1);
  }
  return readable;
}

void out_bin(mpz_t src, const char*__restrict__ __fmt, char* output_dir, int rank) {
  char* file_name;
  if (asprintf(&file_name, __fmt, output_dir, rank) < 0) {
    fprintf(stderr, "Error formatting string \"file_name\"");
    exit(EXIT_FAILURE);
  };
  FILE* file = fopen(file_name, "w");
  mpz_out_raw(file, src);
  fclose(file);
  free(file_name);
}

int main(int argc, char** argv) {
  // mp_set_memory_functions(&_malloc, &_realloc, &_free);
  Benchmark timer{clock(), omp_get_wtime()};
  int rank, n_processes, rc;
  rc = MPI_Init(&argc, &argv);
  if (rc != MPI_SUCCESS) {
    printf("MPI_Init() failed\n");
    return EXIT_FAILURE;
  }

  rc = MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

  if (rc != MPI_SUCCESS) {
    printf("MPI_Comm_size() failed\n");
    return EXIT_FAILURE;
  }

  rc = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   
  if (rc != MPI_SUCCESS) {
    printf("MPI_Comm_rank() failed\n");
    return EXIT_FAILURE;
  }
  
  mpz_t sum;
  mp_exp_t exp;
  unsigned long digits = strtoull(argv[1], NULL, 10) * 2;
  char* output_dir = argv[2];
  bs* r = chudnovsky(digits, rank+1, n_processes);
  
  // gmp_printf("π = %.100Ff\n", pi);
  // MPI_Finalize();
  // return 0;

  #pragma omp parallel
  {
    #pragma omp single
    {
      #pragma omp task
      out_bin(r->Pab, "%s/PAB%d.bin", output_dir, rank);
      
      #pragma omp task
      out_bin(r->Qab, "%s/QAB%d.bin", output_dir, rank);
      
      #pragma omp task
      out_bin(r->Tab, "%s/TAB%d.bin", output_dir, rank);

      #pragma omp taskwait

      mpz_clears(r->Pab, r->Qab, r->Tab, NULL);
      free(r);
      char code;
      if (rank == 0) {
        for (int i = 1; i < n_processes; ++i) {
          MPI_Send(&code, 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        Benchmark::Result result = timer.capture(clock(), omp_get_wtime());

        printf("\n------------------------\n[COMPUTATION RESULT]\nN processes: %d\nN threads per process: %d\nN total CPUs: %d\ncpu time: %luh %lum %.2fs\nwall time: %luh %lum %.2fs\n# of π digits: %s\n------------------------\n\n", n_processes, omp_get_num_threads(), omp_get_num_threads() * n_processes, result.cpu_hours(), result.cpu_mins(), result.cpu_seconds(), result.wall_hours(), result.wall_mins(), result.wall_seconds(), get_readable_digits(&digits).c_str());

        // detect_mem_leak();
      } else {
        MPI_Recv(&code, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Barrier(MPI_COMM_WORLD);
      }
      MPI_Finalize();
    }
  }
  
  return 0;
}
