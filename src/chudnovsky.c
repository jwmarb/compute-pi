#include <gmp.h>
#include <math.h>
#include <omp.h>
// #include <stdio.h>

// how many decimal digits the algorithm generates per iteration:
#define DIGITS_PER_ITERATION log10(151931373056000) // https://mathoverflow.net/questions/261162/chudnovsky-algorithm-and-pi-precision

void chudnovsky(mpf_t sum, unsigned long long digits, int rank, int n_processes) {
  double bits_per_digit = log2(10);

  unsigned long precision_bits, // bits required to meet digit precision
                iterations;     // number of iterations

  precision_bits = (bits_per_digit * digits) + 1;
  iterations = ((digits/DIGITS_PER_ITERATION) + 1);

  mpf_set_default_prec(precision_bits);

  // mpf_t constant;

  mpf_inits(
    // constant, 
    sum, 
    NULL);

  // mpf_sqrt_ui(constant, 10005UL);
  // mpf_mul_ui(constant, constant, 426880UL);

  mpf_set_ui(sum, 0UL); // initialize sum to 0

  #pragma omp parallel shared(sum,iterations)
  {
    double tasks = iterations / (double) (n_processes);
    double subtasks = tasks / (double) (omp_get_num_threads());
    unsigned long k = ceil((tasks * (rank - 1)) - (subtasks * (omp_get_thread_num() - omp_get_num_threads() + 1))),
                  iter = ceil((tasks * (rank - 1)) - (subtasks * (omp_get_thread_num() - omp_get_num_threads()))),
                  three_k;        // 3k <- for reuse

    // debugging purposes
    // printf("%d\tp%d-t%d: %d - %d\n", iterations, rank-1, omp_get_thread_num() , k, iter);

    // numerator portion
    mpz_t a, // (6k)! 
          b, // 13591409 + 545140134k

    // denominator portion
          c, // (3k)!
          d, // (k!)^3
          e; // (-640320)^(3k)

    mpf_t numerator, denominator, division_result, local_sum;

    mpz_inits(a, b, c, d, e, NULL);
    mpf_inits(numerator, denominator, division_result, local_sum, NULL);

    mpf_set_ui(local_sum, 0UL);

    for (; k < iter; ++k) {
      three_k = 3 * k;
      
      mpz_fac_ui(a, 6 * k); // a = (6k)!

      mpz_set_ui(b, 545140134UL);
      mpz_mul_ui(b, b, k); // b = 545140134k
      mpz_add_ui(b, b, 13591409UL); // b = 13591409 + 545140134k

      mpz_fac_ui(c, three_k); // c = (3k)!

      mpz_fac_ui(d, k); // d = k!
      mpz_pow_ui(d, d, 3); // d = (k!)^3

      mpz_ui_pow_ui(e, 640320UL, three_k); // 640320^(3k)
      if (three_k & 1 == 1) { // odd power means negative value
        mpz_neg(e, e);
      }

      mpz_mul(a, a, b);
      mpf_set_z(numerator, a); // numerator = (6k!) * (13591409 + 545140134k)

      mpz_mul(c, c, d); // c = (3k)! * (k!)^3
      mpz_mul(c, c, e); // c = (3k)! * (k!)^3 * (-640320)^(3k)
      mpf_set_z(denominator, c); // denominator = (3k)! * (k!)^3 * (-640320)^(3k)

      mpf_div(division_result, numerator, denominator); // numerator/denominator
      mpf_add(local_sum, local_sum, division_result); // adds to sum
    }
    // clean up
    mpf_add(sum, sum, local_sum);
    mpf_clears(numerator, denominator, division_result, local_sum, NULL);
    mpz_clears(a, b, c, d, e, NULL);
  }

  // mpf_ui_div(sum, 1, sum); // invert fraction
  // mpf_mul(sum, sum, constant); // multiply by constant sqrt part
}