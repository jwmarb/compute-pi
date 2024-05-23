#include <gmp.h>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <omp.h>
#include <mpi.h>
extern "C" {
  #include "gmp_extended.h"
  #include "chudnovsky.h"
  #include "allocate.h"
}
#include "benchmark.hpp"

#define Pab 0
#define Qab 1
#define Tab 2

#define RECV_PAB_SIZE 0
#define RECV_QAB_SIZE 1
#define RECV_TAB_SIZE 2
#define RECV_PAB 3
#define RECV_QAB 4
#define RECV_TAB 5

int rank, 
    n_processes, 
    rc;

unsigned long digits, 
              n_bins, 
              n_bins_per_process,
              dim;

char* tmp_dir;

void inp_bin(mpz_t out, const char*__restrict__ __fmt, unsigned long i) {
  // printf(__fmt, tmp_dir, i);
  // printf(" :\tthread %d\n", omp_get_thread_num());
  char *file_name;
  if (asprintf(&file_name, __fmt, tmp_dir, i) < 0) {
    printf("Error formatting string \"file_name\"\n");
    exit(EXIT_FAILURE);
  };
  FILE *file = fopen(file_name, "r");
  mpz_inp_raw(out, file);
  remove(file_name);
  fclose(file);
  free(file_name);
}

void combine(mpz_t** result, unsigned int a, unsigned int b, mpz_t** bins) {
  if (b - a == 1) {
    *result = bins[a];
  } else {
    mpz_t c;
    int m = (a + b)/2;
    mpz_t* l = (mpz_t*) malloc(sizeof(mpz_t) * 3);
    mpz_t* r = (mpz_t*) malloc(sizeof(mpz_t) * 3);
    
    // #pragma omp taskgroup
    // {
      mpz_inits(
        c, 
        l[Pab], 
        l[Qab], 
        l[Tab],
        r[Pab],
        r[Qab],
        r[Tab], 
        NULL
      );

      // #pragma omp task shared(l,a,m,bins) depend(inout:l,l[Pab],l[Qab],l[Tab])
      combine(&l, a, m, bins);

      // #pragma omp task shared(r,m,b,bins) depend(inout:r,r[Pab],r[Qab],r[Tab])
      combine(&r, m, b, bins);

      // #pragma omp task depend(in:r[Pab],l[Pab]) depend(inout:result[0][Pab])
      mpz_mul((*result)[Pab], l[Pab], r[Pab]);
      
      // #pragma omp task depend(in:r[Qab],l[Qab]) depend(inout:result[0][Qab])
      mpz_mul((*result)[Qab], l[Qab], r[Qab]);

      // #pragma omp task depend(inout:result[0][Tab]) depend(in:r[Qab],l[Tab])
      mpz_mul((*result)[Tab], r[Qab], l[Tab]);

      // #pragma omp task depend(in:result[0][Tab])
      // {
        mpz_mul(c, l[Pab], r[Tab]);
        mpz_add((*result)[Tab], (*result)[Tab], c);
      // }
    // }

    mpz_clears(
      c, 
      l[Pab], 
      l[Qab], 
      l[Tab],
      r[Pab],
      r[Qab],
      r[Tab], 
      NULL
    );
    free(l);
    free(r);
  }
}

void send(mpz_t* result, int rank, int tag) {
  char** bytes = (char**) malloc(sizeof(char*) * 3);

  size_t  pab_size = (mpz_sizeinbase(result[Pab], 2) + CHAR_BIT - 1) / CHAR_BIT,
          qab_size = (mpz_sizeinbase(result[Qab], 2) + CHAR_BIT - 1) / CHAR_BIT,
          tab_size = (mpz_sizeinbase(result[Tab], 2) + CHAR_BIT - 1) / CHAR_BIT;
        
  bytes[Pab] = (char*) malloc(pab_size);
  bytes[Qab] = (char*) malloc(qab_size);
  bytes[Tab] = (char*) malloc(tab_size);

  mpz_export(bytes[Pab], &pab_size, 1, sizeof(char), 0, 0, result[Pab]);
  mpz_export(bytes[Qab], &qab_size, 1, sizeof(char), 0, 0, result[Qab]);
  mpz_export(bytes[Tab], &tab_size, 1, sizeof(char), 0, 0, result[Tab]);

  MPI_Send(&pab_size, 1, MPI_UNSIGNED_LONG, rank, (RECV_PAB_SIZE + 1) * tag, MPI_COMM_WORLD);
  MPI_Send(&qab_size, 1, MPI_UNSIGNED_LONG, rank, (RECV_QAB_SIZE + 1) * tag, MPI_COMM_WORLD);
  MPI_Send(&tab_size, 1, MPI_UNSIGNED_LONG, rank, (RECV_TAB_SIZE + 1) * tag, MPI_COMM_WORLD);

  MPI_Send(bytes[Pab], pab_size, MPI_BYTE, rank, (RECV_PAB + 1) * tag, MPI_COMM_WORLD);
  MPI_Send(bytes[Qab], qab_size, MPI_BYTE, rank, (RECV_QAB + 1) * tag, MPI_COMM_WORLD);
  MPI_Send(bytes[Tab], tab_size, MPI_BYTE, rank, (RECV_TAB + 1) * tag, MPI_COMM_WORLD);

  free(bytes[Pab]);
  free(bytes[Qab]);
  free(bytes[Tab]);
  free(bytes);
}

void receive(mpz_t* result, int rank, int tag) {
  MPI_Status  pab_status,
              qab_status,
              tab_status;
  size_t  pab_size,
          qab_size,
          tab_size;

  MPI_Recv(&pab_size, 1, MPI_UNSIGNED_LONG, rank, (RECV_PAB_SIZE + 1) * tag, MPI_COMM_WORLD, &pab_status);
  MPI_Recv(&qab_size, 1, MPI_UNSIGNED_LONG, rank, (RECV_QAB_SIZE + 1) * tag, MPI_COMM_WORLD, &qab_status);
  MPI_Recv(&tab_size, 1, MPI_UNSIGNED_LONG, rank, (RECV_TAB_SIZE + 1) * tag, MPI_COMM_WORLD, &tab_status);

  char  *pab = (char*) malloc(pab_size),
        *qab = (char*) malloc(qab_size),
        *tab = (char*) malloc(tab_size);

  MPI_Recv(pab, pab_size, MPI_BYTE, rank, (RECV_PAB + 1) * tag, MPI_COMM_WORLD, &pab_status);
  MPI_Recv(qab, qab_size, MPI_BYTE, rank, (RECV_QAB + 1) * tag, MPI_COMM_WORLD, &qab_status);
  MPI_Recv(tab, tab_size, MPI_BYTE, rank, (RECV_TAB + 1) * tag, MPI_COMM_WORLD, &tab_status);


  // computed[i] = (mpz_t*) malloc(sizeof(mpz_t) * 3);
  // mpz_inits(computed[i][Pab], computed[i][Qab], computed[i][Tab], NULL);
  mpz_import(result[Pab], pab_size, 1, sizeof(char), 0, 0, pab);
  mpz_import(result[Qab], qab_size, 1, sizeof(char), 0, 0, qab);
  mpz_import(result[Tab], tab_size, 1, sizeof(char), 0, 0, tab);

  free(pab);
  free(qab);
  free(tab);
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

void input_bins(mpz_t** bins) {
  #pragma omp parallel
  {
    #pragma omp single
    for (unsigned long i = 0; i < n_bins_per_process; ++i) {
      bins[i] = (mpz_t*) malloc(sizeof(mpz_t) * 3);
      mpz_inits(bins[i][Pab], bins[i][Tab], bins[i][Qab], NULL);

      #pragma omp task
      inp_bin(bins[i][Pab], "%s/PAB%lu.bin", i);

      #pragma omp task
      inp_bin(bins[i][Tab], "%s/TAB%lu.bin", i);

      #pragma omp task
      inp_bin(bins[i][Qab], "%s/QAB%lu.bin", i);
    } 
  }
}

int main(int argc, char** argv) {
  // mp_set_memory_functions(&_malloc, &_realloc, &_free);
  Benchmark timer{clock(), omp_get_wtime()};

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

  digits = strtoull(argv[1], NULL, 10) * 2;
  n_bins = strtoull(argv[2], NULL, 10);
  n_bins_per_process = n_bins/n_processes;
  dim = log2(n_processes + __FLT_EPSILON__);
  tmp_dir = argv[3];
  const char* output_path = argv[4];


  mpz_t** bins = (mpz_t**) malloc(sizeof(mpz_t*) * n_bins_per_process);
  input_bins(bins);

  mpz_t* bs_r = (mpz_t*) malloc(sizeof(mpz_t) * 3);
  mpz_inits(bs_r[Pab], bs_r[Tab], bs_r[Qab], NULL);

  // #pragma omp parallel
  // #pragma omp single
  combine(&bs_r, 0, n_bins_per_process, bins);

  if (rank == 0) {
    mpz_t** computed = (mpz_t**) malloc(sizeof(mpz_t*) * n_processes);
    computed[0] = bs_r;
    for (int i = 1; i < n_processes; ++i) {
      computed[i] = (mpz_t*) malloc(sizeof(mpz_t) * 3);
      mpz_inits(computed[i][Pab], computed[i][Qab], computed[i][Tab], NULL);
      receive(computed[i], i, 1);
    }

    mpz_t* r = (mpz_t*) malloc(sizeof(mpz_t) * 3);
    mpz_inits(r[Pab], r[Tab], r[Qab], NULL);

    // #pragma omp parallel
    // #pragma omp single
    combine(&r, 0, n_processes, computed);

    init_precision_bits(digits);

    mpz_t sqrtC, one;
    mpf_t pi, one_f;
    mpz_inits(sqrtC, one, NULL);
    mpf_inits(pi, one_f, NULL);

    mpz_ui_pow_ui(one, 10, digits);
    mpf_set_ui(one_f, 10);
    mpf_pow_ui(one_f, one_f, digits/2);
    mpz_mul_ui(sqrtC, one, 10005);
    mpz_sqrt(sqrtC, sqrtC);
    mpz_mul(r[Qab], r[Qab], sqrtC);
    mpz_mul_ui(r[Qab], r[Qab], 426880);
    mpz_div(r[Qab], r[Qab], r[Tab]);
    mpf_set_z(pi, r[Qab]);
    mpf_div(pi, pi, one_f);
    mpz_ui_pow_ui(one, 10, 100);
    mpz_mod(r[Qab], r[Qab], one);
    mpz_clears(r[Pab], r[Tab], sqrtC, NULL);
    mpf_clears(one_f, NULL);
    FILE* file = fopen(output_path, "w");
    mpf_out_raw(file, pi);
    fclose(file);

    Benchmark::Result result = timer.capture(clock(), omp_get_wtime());

    gmp_printf("\n------------------------\n[REDUCTION OPENMP+MPI RESULT]\ncpu time: %luh %lum %.2fs\nwall time: %luh %lum %.2fs\nfirst 100 digits of π: %.100Ff\nlast 100 digits of π: ...%Zd\n------------------------\n\n", result.cpu_hours(), result.cpu_mins(), result.cpu_seconds(), result.wall_hours(), result.wall_mins(), result.wall_seconds(), pi, r[Qab]);
    mpz_clear(r[Qab]);
    mpf_clear(pi);
    free(r);
    // detect_mem_leak();
  } else {
    send(bs_r, 0, 1);
    free(bs_r);
  }

  MPI_Finalize();
  
  return 0;
}