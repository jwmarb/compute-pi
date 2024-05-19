// Compute n digits of pi
// Optimized for high performance computing

#include <omp.h>
#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <gmp.h>
#include <iostream>
#include <chrono>
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

int main(int argc, char** argv) {
  // mp_set_memory_functions(&_malloc, &_realloc, &_free);
  auto s = std::chrono::steady_clock::now();
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
  bs* r = chudnovsky(digits, rank+1, n_processes);
  
  // gmp_printf("π = %.100Ff\n", pi);
  // MPI_Finalize();
  // return 0;

  char* file_name;
  if (asprintf(&file_name, "PAB%d.bin", rank) < 0) {
    printf("Error formatting string \"file_name\"");
    return 1;
  };
  FILE* pab_file = fopen(file_name, "w");
  mpz_out_raw(pab_file, r->Pab);
  fclose(pab_file);
  free(file_name);

  if (asprintf(&file_name, "QAB%d.bin", rank) < 0) {
    printf("Error formatting string \"file_name\"");
    return 1;
  };
  FILE* qab_file = fopen(file_name, "w");
  mpz_out_raw(qab_file, r->Qab);
  fclose(qab_file);
  free(file_name);
  
  if (asprintf(&file_name, "TAB%d.bin", rank) < 0) {
    printf("Error formatting string \"file_name\"");
    return 1;
  };
  FILE* tab_file = fopen(file_name, "w");
  mpz_out_raw(tab_file, r->Tab);
  fclose(tab_file);
  free(file_name);
  
  mpz_clears(r->Pab, r->Qab, r->Tab, NULL);
  free(r);
  MPI_Finalize();
  if (rank == 0) {
    auto e = std::chrono::steady_clock::now() - s;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(e).count();
    long mins = std::chrono::duration_cast<std::chrono::minutes>(e).count();
    long ms = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();
    int n_threads = 0;
    #pragma omp parallel reduction(+:n_threads)
    n_threads += 1;
    printf("With %d processor%s with %d thread%s per processor (%d in total), it took %luh %lum %.2fs to calculate %lu digits of pi\n", n_processes, n_processes != 1 ? "s" : "", n_threads, n_threads != 1 ? "s" : "", n_threads * n_processes, hours, mins, (ms%6000)/1000.0, digits/2);

    detect_mem_leak();
  }
  return 0;
}
