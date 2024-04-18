#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <cstdio>
#include <mpi.h>
#include <gmp.h>
extern "C" { 
  #include "gmp_extended.h"
  #include "chudnovsky.h"
}

int main(int argc, char** argv) {
  auto s = std::chrono::steady_clock::now();

  int n_processes, rank;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  unsigned long digits = atoi(argv[1]);
  mpf_t sum;
  // std::cout << "hello from process " << rank << "/" << n_processes << std::endl;
  chudnovsky(sum, digits, rank, n_processes);
  if (rank != 0) {
    std::string file_name = ".bin";
    FILE* file;
    file_name = std::to_string(rank) + file_name;
    file = fopen(file_name.c_str(), "w");
    mpf_out_raw(file, sum);
    mpf_clear(sum);
  }
  // std::cout << "DONE from process " << rank << "/" << n_processes << std::endl;
  MPI_Finalize();

  if (rank == 0) {
    FILE* file;
    mpf_t pi_sum_slice, constant;
    std::string file_name;
    const char* file_name_c;
    mpf_inits(pi_sum_slice, constant, NULL);
    
    mpf_sqrt_ui(constant, 10005UL);
    mpf_mul_ui(constant, constant, 426880UL);
    
    for (int i = 1; i < n_processes; ++i) {
      file_name = std::to_string(i) + ".bin";
      file_name_c = file_name.c_str();
      file = fopen(file_name_c, "r");
      mpf_inp_raw(file, pi_sum_slice);
      fclose(file);
      file = NULL;
      remove(file_name_c);
      mpf_add(sum, sum, pi_sum_slice);
    }

    mpf_ui_div(sum, 1, sum); // invert fraction
    mpf_mul(sum, sum, constant); // multiply by constant sqrt part

    file = fopen("pi.bin", "w");
    mpf_out_raw(file, sum);
    // std::string output = "%" + std::to_string(digits) + ".Ff\n\n";
    // gmp_printf(output.c_str(), sum);
    fclose(file);

    auto e = std::chrono::steady_clock::now();
    std::cout << "Wrote " << mpf_size(sum)*8 << " bytes into \"pi.bin\"" << std::endl;
    std::cout << "With " << n_processes << " processes, " << "calculating pi to " << digits << " digits took " << std::chrono::duration_cast<std::chrono::duration<double>>(e - s).count() << " seconds" << std::endl;

    mpf_clears(pi_sum_slice, constant, sum, NULL);
  }

  return 0;
}