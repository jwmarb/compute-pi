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

bs* chudnovsky(unsigned long long digits, int rank, int n_processes) {
  double bits_per_digit = log2(10);

  unsigned long precision_bits, // bits required to meet digit precision
                iterations;     // number of iterations

  precision_bits = (bits_per_digit * digits) + 1;
  iterations = ((digits/DIGITS_PER_ITERATION) + 1);

  mpf_set_default_prec(precision_bits);

  // mpf_t constant;

  mpz_t C3_OVER_24;

  mpz_inits(C3_OVER_24, NULL);

  mpz_set_ui(C3_OVER_24, 640320ul);
  mpz_pow_ui(C3_OVER_24, C3_OVER_24, 3);
  mpz_div_ui(C3_OVER_24, C3_OVER_24, 24);

  // mpf_inits(
  //   // constant, 
  //   sum, 
  //   NULL);

  // mpf_sqrt_ui(constant, 10005UL);
  // mpf_mul_ui(constant, constant, 426880UL);

  // mpz_set_ui(sum, 0ul); // initialize sum to 0
  bs* r = (bs*) malloc(sizeof(bs));
  mpz_inits(r->Pab, r->Qab, r->Tab, NULL);
  mpz_set_ui(r->Pab, 0);
  mpz_set_ui(r->Qab, 0);
  mpz_set_ui(r->Tab, 0);
  #pragma omp parallel shared(iterations, n_processes,r)
  {
    double tasks = iterations / (double) (n_processes);
    double subtasks = tasks / (double) (omp_get_num_threads());
    unsigned long k = ceil((tasks * (rank - 1)) - (subtasks * (omp_get_thread_num() - omp_get_num_threads() + 1))),
                  iter = ceil((tasks * (rank - 1)) - (subtasks * (omp_get_thread_num() - omp_get_num_threads())));
  printf("total=%lu\tk=%lu\titer=%lu\tn_processes=%d\n", iterations, k, iter, n_processes);

    bs* subr = bs_util(k, iter, C3_OVER_24);
    mpz_add(r->Pab, r->Pab, subr->Pab);
    mpz_add(r->Qab, r->Qab, subr->Qab);
    mpz_add(r->Tab, r->Tab, subr->Tab);
    mpz_clears(subr->Pab, subr->Qab, subr->Tab, NULL);
    free(subr);
  }

  return r;

  // mpf_ui_div(sum, 1, sum); // invert fraction
  // mpf_mul(sum, sum, constant); // multiply by constant sqrt part
}