#include <gmp.h>
#include <limits.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>

#include "prime.h"

char* sieve_of_eratosthenes(const char* lower_str, const char* upper_str)
{
  mpz_t upper, lower;
  mpz_t klimit;
  mpz_t index, counter;
  mpz_t temp, temp_2;
  mpz_t size;

  mpz_init_set_str(upper, upper_str, 10);
  mpz_init_set_str(lower, lower_str, 10);
  mpz_init_set_ui(klimit, 0);
  mpz_init_set_ui(index, 0);
  mpz_init_set_ui(counter, 2);
  mpz_init_set_ui(temp, 0);
  mpz_init_set_ui(temp_2, 0);

  // calculate size of array
  mpz_sub(size, upper, lower);
  mpz_add_ui(size, size, 1);
  char* sieve = (char*)malloc(sizeof(char)*mpz_get_ui(size));

  unsigned int size_ui = mpz_get_ui(size);
  #pragma omp parallel for
  for (unsigned int i = 0; i < size_ui; i++)
    sieve[i] = '0';

  // calculate klimit
  mpz_sqrt(klimit, upper);
  mpz_add_ui(klimit, klimit, 2);

  // first do all the even numbers
  mpz_neg(index, lower);
  mpz_fdiv_q(index, index, counter);
  mpz_neg(index, index);
  mpz_mul(index, index, counter);
  mpz_sub(index, index, lower);

  mpz_set(temp, index);
  #pragma omp parallel for
  for (unsigned int i = mpz_get_ui(index); i < size_ui; i += mpz_get_ui(counter))
    sieve[i] = '1';

  mpz_init_set_ui(counter, 3);
  unsigned int n = mpz_cmp_ui(klimit, UINT_MAX) > 0 ? UINT_MAX : mpz_get_ui(klimit);
  int index_par;
  for (unsigned int i = 3; i < n; i += 2)
  {
    mpz_neg(index, lower);
    mpz_fdiv_q_ui(index, index, i);
    mpz_neg(index, index);
    mpz_mul_ui(index, index, i);
    mpz_sub(index, index, lower);
    index_par = mpz_get_ui(index);

    if (index_par % 2 == 1)
    {
      for (unsigned int j = index_par; j < size_ui; j += i*2)
        sieve[j] = '1';
    }
    else
    {
      if (index_par >= 0 && index_par < size_ui)
        sieve[index_par] = '1';
      index_par += i;
      for (unsigned int j = index_par; j < size_ui; j += i*2)
        sieve[j] = '1';
    }
  }

  // then, the remainders starting from 3
  mpz_init_set_ui(counter, n);
  for ( ; mpz_cmp(counter, klimit) < 0; mpz_add_ui(counter, counter, 2))
  {
    // calculate index -- index = (-(-lower // k) * k) - lower
    mpz_neg(index, lower);
    mpz_fdiv_q(index, index, counter);
    mpz_neg(index, index);
    mpz_mul(index, index, counter);
    mpz_sub(index, index, lower);
    
    if (mpz_odd_p(index))
    {
      mpz_set(temp, index);
      mpz_mul_ui(temp_2, counter, 2);
      for ( ; mpz_cmp(temp, size) < 0; mpz_add(temp, temp, temp_2))
        sieve[mpz_get_ui(temp)] = '1';
    }
    else
    {
      if (mpz_cmp(index, size) < 0 && mpz_cmp_ui(index, 0) >= 0)
        sieve[mpz_get_ui(index)] = '1';

      mpz_set(temp, index);
      mpz_add(temp, index, counter);
      mpz_mul_ui(temp_2, counter, 2);
      for ( ; mpz_cmp(temp, size) < 0; mpz_add(temp, temp, temp_2))
        sieve[mpz_get_ui(temp)] = '1';
    }
  }

  mpz_clears(upper, lower, klimit, index, counter, temp, temp_2, NULL);

  return sieve;
}
