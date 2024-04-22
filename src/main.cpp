// Compute n digits of pi
// This is NOT optimized to be fast running

#include <omp.h>
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
  mpf_t sum;
  mp_exp_t exp;
  unsigned long digits = atoi(argv[1]);
  chudnovsky(sum, digits);

  FILE* file = fopen("pi.bin", "w");
  mpf_out_raw(file, sum);
  fclose(file);
  gmp_printf("π = %.10Ff\n", sum);
  mpf_clear(sum);
  auto e = std::chrono::steady_clock::now() - s;
  auto hours = std::chrono::duration_cast<std::chrono::hours>(e).count();
  long mins = std::chrono::duration_cast<std::chrono::minutes>(e).count();
  long ms = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();
  int n_threads = 0;
  #pragma omp parallel reduction(+:n_threads)
  n_threads += 1;
  fmt::print("With {} thread{}, it took {}h {}m {}s to calculate {} digits of pi\n", n_threads, n_threads != 1 ? "s" : "",hours, mins, (ms%6000)/1000.0, digits);
  return 0;
}
