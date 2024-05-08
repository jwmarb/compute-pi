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
  #include "gmp_extended.h"
  #include "chudnovsky_bs.h"
}

bs* combine(int a, int b) {
  bs* result = (bs*) malloc(sizeof(bs));
  mpz_inits(result->Pab, result->Qab, result->Tab, NULL);
  if (b - a == 1) {
    char* file_name;
    if (asprintf(&file_name, "PAB%d.bin", a) < 0) {
      printf("Error formatting string \"file_name\"");
    };
    FILE* pab_file = fopen(file_name, "r");
    mpz_inp_raw(result->Pab, pab_file);
    fclose(pab_file);
    remove(file_name);
    free(file_name);
    
    if (asprintf(&file_name, "QAB%d.bin", a) < 0) {
      printf("Error formatting string \"file_name\"");
    };
    FILE* qab_file = fopen(file_name, "r");
    mpz_inp_raw(result->Qab, qab_file);
    fclose(qab_file);
    remove(file_name);
    free(file_name);
    
    if (asprintf(&file_name, "TAB%d.bin", a) < 0) {
      printf("Error formatting string \"file_name\"");
    };
    FILE* tab_file = fopen(file_name, "r");
    mpz_inp_raw(result->Tab, tab_file);
    fclose(tab_file);
    remove(file_name);
    free(file_name);

    return result;
  } else {
    mpz_t c;
    mpz_init(c);
    int m = (a + b)/2;
    bs* l = combine(a, m);
    bs* r = combine(m, b);

    mpz_mul(result->Pab, l->Pab, r->Pab);
    mpz_mul(result->Qab, l->Qab, r->Qab);

    mpz_mul(c, l->Pab, r->Tab);
    mpz_mul(result->Tab, r->Qab, l->Tab);
    mpz_add(result->Tab, result->Tab, c);

    mpz_clears(
      c, 
      l->Pab, 
      l->Qab, 
      l->Tab,
      r->Pab,
      r->Qab,
      r->Tab, 
      NULL
    );
    free(l);
    free(r);
    return result;
  }
}

int main(int argc, char** argv) {
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
  unsigned long digits = atoi(argv[1]) * 2;
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
    bs* r = combine(0, n_processes);
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

    gmp_printf("π = %.100Ff\n", pi);
    FILE* file = fopen("pi.bin", "w");
    mpf_out_raw(file, pi);
    auto e = std::chrono::steady_clock::now() - s;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(e).count();
    long mins = std::chrono::duration_cast<std::chrono::minutes>(e).count();
    long ms = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();
    int n_threads = 0;
    #pragma omp parallel reduction(+:n_threads)
    n_threads += 1;
    printf("With %d processor%s with %d thread%s per processor (%d in total), it took %luh %lum %.2fs to calculate %lu digits of pi\n", n_processes, n_processes != 1 ? "s" : "", n_threads, n_threads != 1 ? "s" : "", n_threads * n_processes, hours, mins, (ms%6000)/1000.0, digits/2);
  }

  return 0;
}
