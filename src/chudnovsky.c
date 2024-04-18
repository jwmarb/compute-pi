#include <gmp.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>

// how many decimal digits the algorithm generates per iteration:
#define DIGITS_PER_ITERATION log10(151931373056000) // https://mathoverflow.net/questions/261162/chudnovsky-algorithm-and-pi-precision

void chudnovsky(mpf_t sum, unsigned long long digits, int process_id, int num_processes) {

  double bits_per_digit = log2(10);

  unsigned long k,              // for # of iterations
                three_k,        // 3k <- for reuse
                precision_bits, // bits required to meet digit precision
                iterations,     // number of iterations
                subroutine;

  precision_bits = (bits_per_digit * digits) + 1;
  iterations = (digits/DIGITS_PER_ITERATION) + 1;
  iterations /= num_processes;
  subroutine = iterations;
  iterations *= process_id + 1;
  k = process_id * subroutine;

  if (process_id == num_processes - 1) {
    ++iterations;
  }

  mpf_set_default_prec(precision_bits);
  
  // numerator portion
  mpz_t a, // (6k)! 
        b, // 13591409 + 545140134k

  // denominator portion
        c, // (3k)!
        d, // (k!)^3
        e; // (-640320)^(3k)

  mpf_t numerator, denominator, division_result;
  mp_exp_t exp; // holds the exponent for the result string

  mpz_inits(a, b, c, d, e, NULL);
  mpf_inits(numerator, denominator, division_result, sum, NULL);

  mpf_set_ui(sum, 0UL); // initialize sum to 0

  printf("process %d: %lu/%lu\n", process_id, k, iterations);

  for (; k < iterations; ++k) {
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
    mpf_add(sum, sum, division_result); // adds to sum
  }

  // output = mpf_get_str(NULL, &exp, 10, digits, sum);

  // clean up
  mpf_clears(numerator, denominator, division_result,  NULL);
  mpz_clears(a, b, c, d, e, NULL);
}