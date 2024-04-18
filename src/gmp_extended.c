// https://gmplib.org/list-archives/gmp-discuss/2007-November/002981.html
#include <gmp.h>
#include <stdio.h>

int mpf_out_raw (FILE *f, mpf_t X) {
   int expt; mpz_t Z; size_t nz;
   expt = X->_mp_exp;
   fwrite(&expt, sizeof(int), 1, f);
   nz = X->_mp_size;
   Z->_mp_alloc = nz; 
   Z->_mp_size  = nz; 
   Z->_mp_d     = X->_mp_d;
   return (mpz_out_raw(f, Z) + sizeof(int));
}

void mpf_inp_raw(FILE *f, mpf_t X) { 
   int expt; mpz_t Z; size_t nz;
   mpz_init (Z);
   fread(&expt, sizeof(int), 1, f);
   mpz_inp_raw  (Z, f);
   mpf_set_z    (X, Z); 
   X->_mp_exp   = expt;
   mpz_clear (Z);
}