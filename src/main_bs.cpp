// Compute n digits of pi
// This is NOT optimized to be fast running

#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <gmp.h>
#include <iostream>
#include <chrono>
#include <fmt/core.h>
#include <cstdlib>
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
  
  mpz_t sum;
  mp_exp_t exp;
  unsigned long digits = atoi(argv[1]);
  bs* r = chudnovsky(digits, rank+1, n_processes);
  
  // gmp_printf("π = %.100Ff\n", pi);
  // MPI_Finalize();
  // return 0;

  FILE* file;
  
  if (rank != 0) {
    file = fopen(fmt::format("{}-pi.bin", rank).c_str(), "w");
    mpz_out_raw(file, r->Qab);
    fclose(file);
    mpz_clears(r->Pab, r->Qab, r->Tab, NULL);
    free(r);
  }
  MPI_Finalize();
  if (rank == 0) {

    for (int i = 1; i < n_processes; ++i) {
      const char* file_name = fmt::format("{}-pi.bin", i).c_str();
      mpz_t pi_file;
      mpz_init(pi_file);
      file = fopen(file_name, "r");
      mpz_inp_raw(pi_file, file);
      mpz_add(r->Qab, r->Qab, pi_file);
      fclose(file);
      remove(file_name);
      mpz_clear(pi_file); 
    }
    mpz_t sqrtC, one;
    mpf_t pi, one_f;
    mpz_inits(sqrtC, one, NULL);
    mpf_inits(pi, one_f, NULL);

    mpz_ui_pow_ui(one, 10, digits);
    mpf_set_ui(one_f, 10);
    mpf_pow_ui(one_f, one_f, digits/2);
    mpz_mul_ui(sqrtC, one, 10005);
    mpz_sqrt(sqrtC, sqrtC);

    mpz_mul(r->Qab, r->Qab, sqrtC);
    mpz_mul_ui(r->Qab, r->Qab, 426880);
    mpz_div(r->Qab, r->Qab, r->Tab);

    mpf_set_z(pi, r->Qab);
    mpf_div(pi, pi, one_f);

  //   mpf_ui_div(sum, 1, sum); // invert fraction
    // mpf_mul(sum, sum, constant); // multiply by constant sqrt part
    gmp_printf("π = %Zd\n", r->Qab);
  //   file = fopen("pi.bin", "w");
  //   mpf_out_raw(file, sum);
  //   auto e = std::chrono::steady_clock::now() - s;
  //   auto hours = std::chrono::duration_cast<std::chrono::hours>(e).count();
  //   long mins = std::chrono::duration_cast<std::chrono::minutes>(e).count();
  //   long ms = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();
  //   int n_threads = 0;
  //   #pragma omp parallel reduction(+:n_threads)
  //   n_threads += 1;
  //   fmt::print("With {} processor{} with {} thread{} per processor ({} in total), it took {}h {}m {}s to calculate {} digits of pi\n", n_processes, n_processes != 1 ? "s" : "", n_threads, n_threads != 1 ? "s" : "", n_threads * n_processes, hours, mins, (ms%6000)/1000.0, digits);
  }

  return 0;
}
