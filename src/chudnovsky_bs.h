#include <gmp.h>
#include <math.h>

typedef struct bs {
  mpz_t Pab, Qab, Tab;
} bs;

bs* chudnovsky(unsigned long long, int, int);