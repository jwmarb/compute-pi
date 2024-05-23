#include <gmp.h>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <omp.h>
extern "C" {
  #include "gmp_extended.h"
  #include "chudnovsky.h"
  #include "allocate.h"
}
#include "benchmark.hpp"

char  *tmp_dir, 
      *output_path;

unsigned long digits, 
              n_processes;

void inp_bin(mpz_t out, const char*__restrict__ __fmt, unsigned int i) {
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

void combine(bs* result, unsigned int a, unsigned int b, bs** arr) {
  if (b - a == 1) {
    *result = *(arr[a]);
  } else {
    int m = (a + b)/2;
    mpz_t c;
    bs  *l = (bs*) malloc(sizeof(bs)), 
        *r = (bs*) malloc(sizeof(bs));

    mpz_inits(
      c, 
      l->Pab, 
      l->Qab, 
      l->Tab,
      r->Pab,
      r->Qab,
      r->Tab, 
      NULL
    );

    #pragma omp taskgroup
    {

      #pragma omp task shared(l,a,m,arr) depend(inout:l,l->Pab,l->Qab,l->Tab)
      combine(l, a, m, arr);

      #pragma omp task shared(r,m,b,arr) depend(inout:r,r->Pab,r->Qab,r->Tab)
      combine(r, m, b, arr);
      
      #pragma omp task depend(in:r->Pab,l->Pab) depend(inout:result->Pab)
      mpz_mul(result->Pab, l->Pab, r->Pab);

      #pragma omp task depend(in:r->Qab,l->Qab) depend(inout:result->Qab)
      mpz_mul(result->Qab, l->Qab, r->Qab);

      #pragma omp task depend(inout:result->Tab) depend(in:r->Qab,l->Tab)
      mpz_mul(result->Tab, r->Qab, l->Tab);
      
      #pragma omp task depend(in:result->Tab)
      {
        mpz_mul(c, l->Pab, r->Tab);
        mpz_add(result->Tab, result->Tab, c);
      }
    }
    
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
  }
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

void input_bins(bs** arr) {
  #pragma omp parallel for
  for (unsigned int i = 0; i < n_processes; ++i) {
    // printf("hello from thread %d\n", omp_get_thread_num());
    arr[i] = (bs*) malloc(sizeof(bs));
    mpz_inits(arr[i]->Pab, arr[i]->Tab, arr[i]->Qab, NULL);

    #pragma omp task
    inp_bin(arr[i]->Pab, "%s/PAB%i.bin", i);

    #pragma omp task
    inp_bin(arr[i]->Tab, "%s/TAB%i.bin", i);

    #pragma omp task
    inp_bin(arr[i]->Qab, "%s/QAB%i.bin", i);
  }
}

int main(int argc, char** argv) {
  // mp_set_memory_functions(&_malloc, &_realloc, &_free);
  Benchmark timer{clock(), omp_get_wtime()};

  digits = strtoull(argv[1], NULL, 10) * 2;
  n_processes = strtoull(argv[2], NULL, 10);
  tmp_dir = argv[3];
  output_path = argv[4];

  bs** bins = (bs**) malloc(sizeof(bs*) * n_processes);
  input_bins(bins);
  init_precision_bits(digits);

  bs* r = (bs*) malloc(sizeof(bs));
  mpz_inits(r->Pab, r->Qab, r->Tab, NULL);

  #pragma omp parallel
  #pragma omp single
  combine(r, 0, n_processes, bins);

  free(bins);

  mpz_t sqrtC, one;
  mpf_t pi, one_f;
  mpz_inits(sqrtC, one, NULL);
  mpf_inits(pi, one_f, NULL);

  #pragma omp parallel
  #pragma omp single
  {
    #pragma omp task depend(out:one)
    mpz_ui_pow_ui(one, 10, digits);
    
    #pragma omp task depend(out:one_f)
    {
      mpf_set_ui(one_f, 10);
      mpf_pow_ui(one_f, one_f, digits/2);
    }

    #pragma omp task depend(in:one) depend(out:sqrtC)
    {
      mpz_mul_ui(sqrtC, one, 10005);
      mpz_sqrt(sqrtC, sqrtC);
    }

    #pragma omp task depend(in:sqrtC) depend(out:r->Qab)
    {
      mpz_mul(r->Qab, r->Qab, sqrtC);
      mpz_mul_ui(r->Qab, r->Qab, 426880);
      mpz_div(r->Qab, r->Qab, r->Tab);
    }

    #pragma omp task depend(in:r->Qab) depend(in:one_f)
    {
      mpf_set_z(pi, r->Qab);
      mpf_div(pi, pi, one_f);
    }

    #pragma omp task depend(inout:one) depend(inout:r->Qab)
    {
      mpz_ui_pow_ui(one, 10, 100);
      mpz_mod(r->Qab, r->Qab, one);
    }

    #pragma omp task depend(inout:one_f) depend(inout:r->Pab) depend(inout:r->Tab) depend(inout:sqrtC)
    {
      mpz_clears(r->Pab, r->Tab, sqrtC, NULL);
      mpf_clears(one_f, NULL);
    }

    #pragma omp task depend(in:pi)
    {
      FILE* file = fopen(output_path, "w");
      mpf_out_raw(file, pi);
      fclose(file);
    }

    #pragma omp taskwait

    Benchmark::Result result = timer.capture(clock(), omp_get_wtime());

    gmp_printf("\n------------------------\n[REDUCTION OMP RESULT]\ncpu time: %luh %lum %.2fs\nwall time: %luh %lum %.2fs\nfirst 100 digits of π: %.100Ff\nlast 100 digits of π: ...%Zd\n------------------------\n\n", result.cpu_hours(), result.cpu_mins(), result.cpu_seconds(), result.wall_hours(), result.wall_mins(), result.wall_seconds(), pi, r->Qab);
    mpz_clear(r->Qab);
    mpf_clear(pi);
    free(r);
    // detect_mem_leak();
  }
  return 0;
}