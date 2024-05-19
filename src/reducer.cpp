#include <gmp.h>
#include <cstdio>
#include <cstdlib>
#include <chrono>
extern "C" {
  #include "gmp_extended.h"
  #include "chudnovsky.h"
  #include "allocate.h"
}

bs* combine(int a, int b, char* tmp_dir) {
  bs* result = (bs*) malloc(sizeof(bs));
  mpz_inits(result->Pab, result->Qab, result->Tab, NULL);
  if (b - a == 1) {
    char* file_name;
    if (asprintf(&file_name, "%s/PAB%d.bin", tmp_dir, a) < 0) {
      printf("Error formatting string \"file_name\"");
    };
    FILE* pab_file = fopen(file_name, "r");
    mpz_inp_raw(result->Pab, pab_file);
    fclose(pab_file);
    remove(file_name);
    free(file_name);
    
    if (asprintf(&file_name, "%s/QAB%d.bin", tmp_dir, a) < 0) {
      printf("Error formatting string \"file_name\"");
    };
    FILE* qab_file = fopen(file_name, "r");
    mpz_inp_raw(result->Qab, qab_file);
    fclose(qab_file);
    remove(file_name);
    free(file_name);
    
    if (asprintf(&file_name, "%s/TAB%d.bin", tmp_dir, a) < 0) {
      printf("Error formatting string \"file_name\"");
    };
    FILE* tab_file = fopen(file_name, "r");
    mpz_inp_raw(result->Tab, tab_file);
    fclose(tab_file);
    remove(file_name);
    free(file_name);

    return result;
  } else {
    mpz_t c;
    mpz_init(c);
    int m = (a + b)/2;
    bs* l = combine(a, m, tmp_dir);
    bs* r = combine(m, b, tmp_dir);

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

int main(int argc, char** argv) {
  // mp_set_memory_functions(&_malloc, &_realloc, &_free);
  unsigned long digits = atoi(argv[1]) * 2;
  unsigned long n_processes = atoi(argv[2]);
  char* tmp_dir = argv[3];
  const char* output_path = argv[4];
  init_precision_bits(digits);
  auto s = std::chrono::steady_clock::now();
  bs* r = combine(0, n_processes, tmp_dir);
  mpz_t sqrtC, one;
  mpf_t pi, one_f;
  mpz_inits(sqrtC, one, NULL);
  mpf_inits(pi, one_f, NULL);

  mpz_ui_pow_ui(one, 10, digits);
  mpf_set_ui(one_f, 10);
  mpf_pow_ui(one_f, one_f, digits/2);
  mpz_mul_ui(sqrtC, one, 10005);
  mpz_sqrt(sqrtC, sqrtC);

  mpz_mul(r->Qab, r->Qab, sqrtC);
  mpz_mul_ui(r->Qab, r->Qab, 426880);
  mpz_div(r->Qab, r->Qab, r->Tab);

  mpf_set_z(pi, r->Qab);
  mpf_div(pi, pi, one_f);

  gmp_printf("π = %.100Ff\n", pi);
  mpz_clears(r->Pab, r->Qab, r->Tab, sqrtC, one, NULL);
  mpf_clears(one_f, NULL);
  free(r);
  FILE* file = fopen(output_path, "w");
  mpf_out_raw(file, pi);
  mpf_clear(pi);
  auto e = std::chrono::steady_clock::now() - s;
  auto hours = std::chrono::duration_cast<std::chrono::hours>(e).count();
  long mins = std::chrono::duration_cast<std::chrono::minutes>(e).count();
  long ms = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();
  printf("Took %luh %lum %.2fs to combine all bin files into pi.bin\n", hours, mins, (ms%6000)/1000.0);
  // detect_mem_leak();
  return 0;
}