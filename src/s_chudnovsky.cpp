#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <cmath>
#include <cstdio>
#include <gmp.h>
#include <fmt/core.h>

// // how many decimal digits the algorithm generates per iteration:
#define DIGITS_PER_ITERATION log10(151931373056000) // https://mathoverflow.net/questions/261162/chudnovsky-algorithm-and-pi-precision

void chudnovsky(mpz_t sum, unsigned long long digits, int process_id, int num_processes) {
  mpz_t k,
        six_k,
        a_k,
          a_k1,
          a_k2,
          a_k3,
        b,
        a_sum,
        b_sum,
          d,
        total,
        one;

  unsigned long C = 640320;
  unsigned long C3_OVER_24 = (C * C * C)/24;

  mpz_inits(sum, six_k, a_k, a_k1, a_k2, a_k3, b, a_sum, b_sum, d, total, one, NULL);
  mpz_ui_pow_ui(one, 10, digits);
  mpz_set(a_k, one);
  mpz_set(a_sum, one);
  mpz_set_ui(b_sum, 0);
  mpz_set_ui(k, 1);
  
  for (;;) {
    mpz_mul_ui(six_k, k, 6); // Let 6k be a constant

    mpz_set(a_k1, six_k); // a_k1 = 6k
    mpz_sub_ui(a_k1, a_k1, 5); // a_k1 = 6k - 5
    mpz_neg(a_k1, a_k1); // a_k1 = -(6k - 5)


    mpz_mul_ui(a_k2, k, 2); // a_k2 = 2k
    mpz_sub_ui(a_k2, a_k2, 1); // a_k2 = 2k - 1

    mpz_set(a_k3, six_k); // a_k3 = 6k
    mpz_sub_ui(a_k3, a_k3, 1); // a_k3 = 6k - 1

    mpz_mul(a_k, a_k, a_k1); // a_k *= a_k1
    mpz_mul(a_k, a_k, a_k2); // a_k *= a_k2
    mpz_mul(a_k, a_k, a_k3); // a_k *= a_k3
    
    mpz_pow_ui(b, k, 3); // b = k^3
    mpz_mul_ui(b, b, C3_OVER_24); // b = k^3 * (C^3)/24

    mpz_div(a_k, a_k, b); // a_k = a_k/b

    mpz_add(a_sum, a_sum, a_k); // a_sum += a_k
    
    mpz_mul(d, k, a_k); // d = k * a_k

    mpz_add(b_sum, b_sum, d); // b_sum += d

    if (mpz_cmp_ui(a_k, 0) == 0) break;
    mpz_add_ui(k, k, 1);
  }

  gmp_printf("Took %Zd iterations\n", k);

  mpz_mul_ui(a_sum, a_sum, 13591409);
  mpz_mul_ui(b_sum, b_sum, 545140134);

  mpz_add(total, a_sum, b_sum);

  mpz_pow_ui(a_k, one, 2); // a_k = one^2
  mpz_mul_ui(a_k, a_k, 10005); // a_k = one^2 * 10005
  mpz_sqrt(a_k, a_k); // a_k = sqrt(one^2 * 10005)
  mpz_mul_ui(sum, a_k, 426880); // sum = 426880 * sqrt(one^2 * 10005) 
  mpz_mul(sum, sum, one); // sum = 426880 * sqrt(one^2 * 10005) * one
  mpz_div(sum, sum, total);

  mpz_div_ui(sum, sum, 1000); // the last 3 digits of this algorithm are not exact

  mpz_clears(k, six_k, a_k, a_k1, a_k2, a_k3, b, a_sum, b_sum, d, total, one, NULL);
}

int main(int argc, char** argv) {
  auto s = std::chrono::steady_clock::now();
  // Determine digits to calculate
  // add 2 to offset the last 3 digits, of which is composed three inaccurate digits
  // Without getting rid of garbage values (and including the addition of 2 digits), calculating the first 4 digits of pi yields 3141721
  // 3141 is accurate, 721 is not. The result is divided by 1000 to rid of those inaccurate digits to get 3141 only.
  unsigned long digits = atoi(argv[1]) + 2;
  mpz_t sum;
  chudnovsky(sum, digits, rank, n_processes);
  gmp_printf("%Zd\n", sum);

  return 0;
}