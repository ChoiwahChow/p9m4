
#include <stdexcept>
#include "smallprime.h"


SmallPrime::SmallPrime()
{
  // use the sieve of Eratosthenes to set up an array of bool for prime/non-prime
  a[0] = a[1] = false;
  for (int idx = 2; idx <= SmallPrime::MAX_PRIME; ++idx)
    a[idx] = true;

  int p = 2;
  while (p*p < SmallPrime::MAX_PRIME) {
    int jdx = p*p;
    while (jdx  < SmallPrime::MAX_PRIME) {
      a[jdx] = false;
      jdx = jdx + p;
    }
    do p++; while (!a[p]); // look for next number not ruled out as composite
  }
}

bool
SmallPrime::isPrime(int n)
{
  if (n < 1 || n > SmallPrime::MAX_PRIME)
    throw new std::out_of_range("Prime: n out of range");
  return a[n];
}
