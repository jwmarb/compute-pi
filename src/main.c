#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <gmp.h>

// how many decimal digits the algorithm generates per iteration:
#define DIGITS_PER_ITERATION log10(151931373056000) // https://mathoverflow.net/questions/261162/chudnovsky-algorithm-and-pi-precision

void chudnovsky(mpf_t sum, unsigned long long digits) {

  double bits_per_digit = log2(10);

  unsigned long k,              // for # of iterations
                three_k,        // 3k <- for reuse
                precision_bits, // bits required to meet digit precision
                iterations;     // number of iterations

  precision_bits = (bits_per_digit * digits) + 1;
  iterations = (digits/DIGITS_PER_ITERATION) + 1;

  mpf_set_default_prec(precision_bits);
  
  // numerator portion
  mpz_t a, // (6k)! 
        b, // 13591409 + 545140134k

  // denominator portion
        c, // (3k)!
        d, // (k!)^3
        e; // (-640320)^(3k)

  mpf_t numerator, denominator, division_result, constant;
  mp_exp_t exp; // holds the exponent for the result string

  mpz_inits(a, b, c, d, e, NULL);
  mpf_inits(numerator, denominator, division_result, sum, constant, NULL);

  mpf_set_ui(sum, 0UL); // initialize sum to 0

  mpf_sqrt_ui(constant, 10005UL);
  mpf_mul_ui(constant, constant, 426880UL);

  for (k = 0; k <= iterations; ++k) {
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

  mpf_ui_div(sum, 1, sum); // invert fraction
  mpf_mul(sum, sum, constant); // multiply by constant sqrt part

  // output = mpf_get_str(NULL, &exp, 10, digits, sum);

  // clean up
  mpf_clears(numerator, denominator, division_result, constant, NULL);
  mpz_clears(a, b, c, d, e, NULL);
}

int main() {
  
  unsigned long digits;
  printf("num of digits: ");
  scanf("%lu", &digits);
  printf("\n");
  FILE* file;
  mpf_t sum, shift;
  mpz_t c;
  chudnovsky(sum, digits);

  mpz_init(c);
  mpf_init(shift);
  mpf_set_ui(shift, 10);
  mpf_pow_ui(shift, shift, digits);
  
  mpf_mul(sum, sum, shift);
  mpz_set_f(c, sum);

  double bytes_used = 
    mpz_sizeinbase(c, 2)/8.0  // get bits that c uses up and divides by 8 to get bytes
    + 4;                    // 4 bytes are used to represent the size of mpz_t
  printf("%.2f bytes written in \"pi.bin\"\n", bytes_used);

  file = fopen("pi.bin", "w");
  mpz_out_raw(file, c);
  fclose(file);

  return 0;
}