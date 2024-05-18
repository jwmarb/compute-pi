#include <gmp.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

// how many decimal digits the algorithm generates per iteration:
#define DIGITS_PER_ITERATION log10(151931373056000) // https://mathoverflow.net/questions/261162/chudnovsky-algorithm-and-pi-precision

typedef struct bs {
  mpz_t Pab, Qab, Tab;
} bs;

bs* bs_util(unsigned long a, unsigned long b, mpz_t C3_OVER_24) {
  bs* result = (bs*) malloc(sizeof(bs));
  mpz_inits(result->Pab, result->Qab, result->Tab, NULL);
  if (b - a == 1) {
    if (a == 0) {

      mpz_set_ui(result->Pab, 1);
      mpz_set_ui(result->Qab, 1);
    } else {
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
    }
    mpz_set_ui(result->Tab, a);
    mpz_mul_ui(result->Tab, result->Tab, 545140134);
    mpz_add_ui(result->Tab, result->Tab, 13591409);
    mpz_mul(result->Tab, result->Tab, result->Pab);
    if (a & 1) {
      mpz_neg(result->Tab, result->Tab);
    }
  } else {
    mpz_t c;
    mpz_init(c);
    unsigned long m = (a + b)/2;
    bs* l = bs_util(a, m, C3_OVER_24);
    bs* r = bs_util(m, b, C3_OVER_24);
    mpz_mul(result->Pab, l->Pab, r->Pab);
    mpz_mul(result->Qab, l->Qab, r->Qab);

    // c = l.Pab * r.Tab
    mpz_mul(c, l->Pab, r->Tab);
    mpz_mul(result->Tab, r->Qab, l->Tab);
    mpz_add(result->Tab, result->Tab, c);

    mpz_clears(c, l->Pab, l->Qab, l->Tab, r->Pab, r->Qab, r->Tab, NULL);
    free(l);
    free(r);
  }
  return result;
}

bs* combine(unsigned long a, unsigned long b, bs** arr) {
  if (b - a == 1) {
    return arr[a];
  } else {
    bs* result = (bs*) malloc(sizeof(bs));
    mpz_t c;
    mpz_inits(result->Pab, result->Qab, result->Tab, c, NULL);
    unsigned long m = (a + b)/2;
    bs* r = combine(a, m, arr);
    bs* l = combine(m, b, arr);

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
  mpz_t C3_OVER_24;

  mpz_inits(C3_OVER_24, NULL);

  mpz_set_ui(C3_OVER_24, 640320ul);
  mpz_pow_ui(C3_OVER_24, C3_OVER_24, 3);
  mpz_div_ui(C3_OVER_24, C3_OVER_24, 24);

  bs** arr = NULL;
  int n_threads = 0;
  #pragma omp parallel reduction(+:n_threads)
  n_threads += 1;
  arr = (bs**) calloc(n_threads, sizeof(bs*));
  for (int i = 0; i < n_threads; ++i) {
    arr[i] = (bs*) malloc(sizeof(bs));
    mpz_inits(arr[i]->Pab, arr[i]->Qab, arr[i]->Tab, NULL);
    mpz_set_ui(arr[i]->Pab, 0);
    mpz_set_ui(arr[i]->Qab, 0);
    mpz_set_ui(arr[i]->Tab, 0);
  }

  #pragma omp parallel shared(iterations, n_processes)
  {
    double tasks = iterations / (double) (n_processes);
    double subtasks = tasks / (double) (omp_get_num_threads());
    unsigned long k = ceil((tasks * (rank - 1)) - (subtasks * (omp_get_thread_num() - omp_get_num_threads() + 1))),
                  iter = ceil((tasks * (rank - 1)) - (subtasks * (omp_get_thread_num() - omp_get_num_threads())));
    printf("t#=%d, total=%lu\tk=%lu\titer=%lu\tn_processes=%d\n", omp_get_thread_num(), iterations, k, iter, n_processes);

    bs* subr = bs_util(k, iter, C3_OVER_24);
    bs* r = arr[omp_get_thread_num()];
    mpz_set(r->Pab, subr->Pab);
    mpz_set(r->Qab, subr->Qab);
    mpz_set(r->Tab, subr->Tab);
    mpz_clears(subr->Pab, subr->Qab, subr->Tab, NULL);
    free(subr);
  }

  bs* result = combine(0, n_threads, arr);

  // manually free the pointer
  // we do not need to free each individual element since it has been freed inside of combine method
  free(arr);

  return result;
}