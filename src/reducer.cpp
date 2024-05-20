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

void inp_bin(mpz_t out, const char*__restrict__ __fmt, char* tmp_dir, unsigned int i) {
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

bs* combine(unsigned int a, unsigned int b, bs** arr) {
  if (b - a == 1) {
    return arr[a];
  } else {
    bs* result = (bs*) malloc(sizeof(bs));
    mpz_inits(result->Pab, result->Qab, result->Tab, NULL);
    #pragma omp parallel
    {
      #pragma omp single
      {
        mpz_t c;
        mpz_init(c);
        int m = (a + b)/2;
        bs *l, *r;
        #pragma omp task shared(l)
        l = combine(a, m, arr);

        #pragma omp task shared(r)
        r = combine(m, b, arr);

        #pragma omp taskwait
        
        #pragma omp task
        mpz_mul(result->Pab, l->Pab, r->Pab);

        #pragma omp task
        mpz_mul(result->Qab, l->Qab, r->Qab);

        #pragma omp task depend(out:c)
        mpz_mul(c, l->Pab, r->Tab);

        #pragma omp task depend(out:result->Tab)
        mpz_mul(result->Tab, r->Qab, l->Tab);

        #pragma omp task depend(in:c) depend(in:result->Tab)
        mpz_add(result->Tab, result->Tab, c);

        #pragma omp taskwait

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
    return result;
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

bs** input_bins(char* tmp_dir, unsigned long n_processes) {
  bs** arr = (bs**) malloc(sizeof(bs*) * n_processes);

  #pragma omp parallel for
  for (unsigned int i = 0; i < n_processes; ++i) {
    // printf("hello from thread %d\n", omp_get_thread_num());
    arr[i] = (bs*) malloc(sizeof(bs));
    mpz_inits(arr[i]->Pab, arr[i]->Tab, arr[i]->Qab, NULL);

    #pragma omp task
    inp_bin(arr[i]->Pab, "%s/PAB%i.bin", tmp_dir, i);

    #pragma omp task
    inp_bin(arr[i]->Tab, "%s/TAB%i.bin", tmp_dir, i);

    #pragma omp task
    inp_bin(arr[i]->Qab, "%s/QAB%i.bin", tmp_dir, i);
  } 

  return arr;
}

int main(int argc, char** argv) {
  // mp_set_memory_functions(&_malloc, &_realloc, &_free);
  Benchmark timer{clock(), omp_get_wtime()};

  unsigned long digits = strtoull(argv[1], NULL, 10) * 2;
  unsigned long n_processes = strtoull(argv[2], NULL, 10);

  char* tmp_dir = argv[3];
  const char* output_path = argv[4];
  bs** bins = input_bins(tmp_dir, n_processes);
  init_precision_bits(digits);

  bs* r = combine(0, n_processes, bins);

  mpz_t sqrtC, one;
  mpf_t pi, one_f;
  mpz_inits(sqrtC, one, NULL);
  mpf_inits(pi, one_f, NULL);

  #pragma omp parallel
  {
    #pragma omp single
    {
      #pragma omp task depend(out:one)
      {
        // printf("%d\tmpz_ui_pow_ui(one, 10, digits);\n", omp_get_thread_num());
        mpz_ui_pow_ui(one, 10, digits);
      }

      #pragma omp task depend(out:one_f)
      {
        // printf("%d\tmpf_set_ui(one_f, 10);mpf_pow_ui(one_f, one_f, digits/2);\n", omp_get_thread_num());
        mpf_set_ui(one_f, 10);
        mpf_pow_ui(one_f, one_f, digits/2);
      }

      #pragma omp task depend(in:one) depend(out:sqrtC)
      {
        // printf("%d\tmpz_mul_ui(sqrtC, one, 10005);mpz_sqrt(sqrtC, sqrtC);\n", omp_get_thread_num());
        mpz_mul_ui(sqrtC, one, 10005);
        mpz_sqrt(sqrtC, sqrtC);
      }

      #pragma omp task depend(out:r->Qab) depend(in:sqrtC) depend(in:one_f) depend(out:pi)
      {
        // printf("%d\tlong task\n", omp_get_thread_num());

        // Qab = (Qab * sqrtC * 426880)/Tab
        mpz_mul(r->Qab, r->Qab, sqrtC);
        mpz_mul_ui(r->Qab, r->Qab, 426880);
        mpz_div(r->Qab, r->Qab, r->Tab);

        mpf_set_z(pi, r->Qab);
        mpf_div(pi, pi, one_f);
      }

      #pragma omp task depend(in:sqrtC) depend(out:one)
      {
        // printf("%d\tmpz_ui_pow_ui(one, 10, 100);\n", omp_get_thread_num());
        mpz_ui_pow_ui(one, 10, 100);
      }

      #pragma omp task depend(in:one) depend(out:r->Qab)
      {
        // printf("%d\tmpz_mod(r->Qab, r->Qab, one);\n", omp_get_thread_num());
        mpz_mod(r->Qab, r->Qab, one);
      }

      #pragma omp taskwait
      // printf("%d\tfree mpz\n", omp_get_thread_num());
      mpz_clears(r->Pab, r->Tab, sqrtC, NULL);
      mpf_clears(one_f, NULL);
      FILE* file = fopen(output_path, "w");
      mpf_out_raw(file, pi);
      fclose(file);

      Benchmark::Result result = timer.capture(clock(), omp_get_wtime());

      gmp_printf("\n------------------------\n[REDUCTION RESULT]\ncpu time: %luh %lum %.2fs\nwall time: %luh %lum %.2fs\nfirst 100 digits of π: %.100Ff\nlast 100 digits of π: ...%Zd\n------------------------\n\n", result.cpu_hours(), result.cpu_mins(), result.cpu_seconds(), result.wall_hours(), result.wall_mins(), result.wall_seconds(), pi, r->Qab);
      mpz_clear(r->Qab);
      mpf_clear(pi);
      free(r);
      free(bins);
      // detect_mem_leak();
    }
  }
  return 0;
}