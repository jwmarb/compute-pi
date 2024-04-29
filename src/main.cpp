// Compute n digits of pi
// This is NOT optimized to be fast running

#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <gmp.h>
#include <iostream>
#include <chrono>
#include <fmt/core.h>
extern "C" {
  #include "gmp_extended.h"
  #include "chudnovsky.h"
}

int main(int argc, char** argv) {
  auto s = std::chrono::steady_clock::now();
  int rank, n_processes, rc;
  rc = MPI_Init(&argc, &argv);
  if (rc != MPI_SUCCESS) {
    fmt::print("MPI_Init() failed\n");
    return EXIT_FAILURE;
  }

  rc = MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

  if (rc != MPI_SUCCESS) {
    fmt::print("MPI_Comm_size() failed\n");
    return EXIT_FAILURE;
  }

  rc = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   
  if (rc != MPI_SUCCESS) {
    fmt::print("MPI_Comm_rank() failed\n");
    return EXIT_FAILURE;
  }
  
  mpf_t sum;
  mp_exp_t exp;
  unsigned long digits = atoi(argv[1]);
  chudnovsky(sum, digits, rank+1, n_processes);

  FILE* file;
  
  if (rank != 0) {
    file = fopen(fmt::format("{}-pi.bin", rank).c_str(), "w");
    mpf_out_raw(file, sum);
    fclose(file);
    mpf_clear(sum);
  }
  MPI_Finalize();
  if (rank == 0) {
    mpf_t constant;
    mpf_init(constant);
    mpf_sqrt_ui(constant, 10005UL);
    mpf_mul_ui(constant, constant, 426880UL);
    for (int i = 1; i < n_processes; ++i) {
      const char* file_name = fmt::format("{}-pi.bin", i).c_str();
      mpf_t pi_file;
      mpf_init(pi_file);
      file = fopen(file_name, "r");
      mpf_inp_raw(file, pi_file);
      mpf_add(sum, sum, pi_file);
      fclose(file);
      remove(file_name);
      mpf_clear(pi_file); 
    }

    mpf_ui_div(sum, 1, sum); // invert fraction
    mpf_mul(sum, sum, constant); // multiply by constant sqrt part
    gmp_printf("π = %.32Ff\n", sum);
    auto e = std::chrono::steady_clock::now() - s;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(e).count();
    long mins = std::chrono::duration_cast<std::chrono::minutes>(e).count();
    long ms = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();
    int n_threads = 0;
    #pragma omp parallel reduction(+:n_threads)
    n_threads += 1;
    fmt::print("With {} processor{} with {} thread{} per processor ({} in total), it took {}h {}m {}s to calculate {} digits of pi\n", n_processes, n_processes != 1 ? "s" : "", n_threads, n_threads != 1 ? "s" : "", n_threads * n_processes, hours, mins, (ms%6000)/1000.0, digits);
  }

  return 0;
}
