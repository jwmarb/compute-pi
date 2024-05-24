#include <gmp.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include "allocate.h"

// how many decimal digits the algorithm generates per iteration:
#define DIGITS_PER_ITERATION log10(151931373056000) // https://mathoverflow.net/questions/261162/chudnovsky-algorithm-and-pi-precision

mpz_t C3_OVER_24;

typedef struct bs {
  mpz_t Pab, Qab, Tab;
} bs;

void bs_util(bs* result, unsigned long a, unsigned long b) {
  if (b - a == 1) {
    if (a == 0) {
      mpz_set_ui(result->Pab, 1);
      mpz_set_ui(result->Qab, 1);
    } 
    else {
      mpz_t k1, k2, k3;
      mpz_inits(k1, k2, k3, NULL);
      
      // k1 = (6*a-5)
      mpz_set_ui(k1, a);
      mpz_mul_ui(k1, k1, 6);
      mpz_sub_ui(k1, k1, 5);

      // k2 = (2*a-1)
      mpz_set_ui(k2, a);
      mpz_mul_ui(k2, k2, 2);
      mpz_sub_ui(k2, k2, 1);

      // k3 = (6*a-1)
      mpz_set_ui(k3, a);
      mpz_mul_ui(k3, k3, 6);
      mpz_sub_ui(k3, k3, 1);

      mpz_mul(result->Pab, k1, k2);
      mpz_mul(result->Pab, result->Pab, k3);

      mpz_set_ui(result->Qab, a);
      mpz_pow_ui(result->Qab, result->Qab, 3);
      mpz_mul(result->Qab, result->Qab, C3_OVER_24);

      mpz_clears(k1, k2, k3, NULL);
    }

    mpz_set_ui(result->Tab, a);
    mpz_mul_ui(result->Tab, result->Tab, 545140134);
    mpz_add_ui(result->Tab, result->Tab, 13591409);
    mpz_mul(result->Tab, result->Tab, result->Pab);
    if (a & 1) {
      mpz_neg(result->Tab, result->Tab);
    }
  } else {
    unsigned long m = (a + b)/2;
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

    bs_util(l, a, m);
    bs_util(r, m, b);

    mpz_mul(result->Pab, l->Pab, r->Pab);
    mpz_mul(result->Qab, l->Qab, r->Qab);
    mpz_mul(result->Tab, r->Qab, l->Tab);

    // c = l.Pab * r.Tab
    mpz_mul(c, l->Pab, r->Tab);
    mpz_add(result->Tab, result->Tab, c);

    mpz_clears(c, l->Pab, l->Qab, l->Tab, r->Pab, r->Qab, r->Tab, NULL);
    free(l);
    free(r);
  }
}

void combine(bs* result, unsigned long a, unsigned long b, bs** arr) {
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

      #pragma omp task shared(r,a,m,arr) depend(inout:r,r->Pab,r->Qab,r->Tab)
      combine(r, a, m, arr);

      #pragma omp task shared(l,m,b,arr) depend(inout:l,l->Pab,l->Qab,l->Tab)
      combine(l, m, b, arr);
      
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

char* get_size_requirement(unsigned long bits) {
  double bytes = bits/8.0;
  char* size;
  if (gmp_asprintf(&size, "%.2f %s",
    bytes <= 1024ul ?
    bytes :
    bytes <= (1024ul * 1024ul) ?
    bytes/1024ul :
    bytes <= (1024ul * 1024ul * 1024ul) ?
    bytes/(1024ul * 1024ul) :
    bytes <= (1024ul * 1024ul * 1024ul * 1024ul) ?
    bytes/(1024ul * 1024ul * 1024ul) : bits/(1024ul * 1024ul * 1024ul * 1024ul),
    bytes <= 1024ul ?
    "B" :
    bytes <= (1024ul * 1024ul) ?
    "KB" :
    bytes <= (1024ul * 1024ul * 1024ul) ?
    "MB" :
    bytes <= (1024ul * 1024ul * 1024ul * 1024ul) ?
    "GB" : "TB"
  ) < 0) {
    fprintf(stderr, "Could not allocate for size\n");
    exit(1);
  }
  return size;
}

unsigned long init_precision_bits(unsigned long long digits) {
  double bits_per_digit = log2(10);

  unsigned long precision_bits, // bits required to meet digit precision
                iterations;     // number of iterations

  precision_bits = (bits_per_digit * digits) + 1;
  iterations = ((digits/DIGITS_PER_ITERATION) + 1);

  mpf_set_default_prec(precision_bits);

  return iterations;
}

bs* chudnovsky(unsigned long long digits, int rank, int n_processes) {
  unsigned long iterations = init_precision_bits(digits);
  double tasks = iterations / (double) (n_processes);

  mpz_inits(C3_OVER_24, NULL);
  mpz_set_ui(C3_OVER_24, 640320ul);
  mpz_pow_ui(C3_OVER_24, C3_OVER_24, 3);
  mpz_div_ui(C3_OVER_24, C3_OVER_24, 24);

  int n_threads = 0;
  #pragma omp parallel reduction(+:n_threads)
  n_threads += 1;

  bs** arr = (bs**) calloc(sizeof(bs*), n_threads);
  
  #pragma omp parallel
  #pragma omp single
  for (int t = 0; t < n_threads; ++t) {
    double subtasks = tasks / (double) n_threads;
    unsigned long k = ceil((tasks * (rank - 1)) - (subtasks * (t - n_threads + 1))),
                  iter = ceil((tasks * (rank - 1)) - (subtasks * (t - n_threads)));
    // printf("t#=%d, total=%lu\tk=%lu\titer=%lu\tn_processes=%d\n", t, iterations, k, iter, n_processes);

    // if (rank == 1 && omp_get_thread_num() == 0) {
    //   unsigned long bits = (unsigned long) (log2(10) * digits + 1);
    //   char* size_per_process = get_size_requirement(bits * log2(subtasks)); // 3 
    //   printf("\n------------------------\n[CHUDNOVSKY.C]\nTotal # iterations: %lu\n# Iterations per process: %lu\n# Iterations per thread: %lu\napproximate memory requirement(bits): %lu\napproximate memory requirement: %s\n------------------------\n\n", iterations, (unsigned long) tasks, (unsigned long) subtasks, bits, size_per_process);
    //   free(size_per_process);
    // }

    arr[t] = (bs*) malloc(sizeof(bs));
    mpz_inits(arr[t]->Pab, arr[t]->Qab, arr[t]->Tab, NULL);

    #pragma omp task
    bs_util(arr[t], k, iter);
  }

  bs* result = (bs*) malloc(sizeof(bs));
  mpz_inits(result->Qab, result->Pab, result->Tab, NULL);

  #pragma omp parallel
  #pragma omp single
  combine(result, 0, n_threads, arr);

  // manually free the pointer
  // we do not need to free each individual element since it has been freed inside of combine method
  free(arr);

  return result;
}