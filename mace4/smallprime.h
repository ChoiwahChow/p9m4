
#ifndef MACE4_SMALLPRIME_H
#define MACE4_SMALLPRIME_H

/*
 * A simple singleton class to check for small prime numbers (less than MAX_PRIME)
 *
 * Thread-safe?:  Yes - using Meyer's singleton.
 */


class SmallPrime {
public:
  static SmallPrime& getInstance() {
    static SmallPrime instance;
    return instance;
  }

  static constexpr int MAX_PRIME = 1000;
  bool isPrime(int);

private:
  bool  a[MAX_PRIME];

  SmallPrime();
  ~SmallPrime()= default;
  SmallPrime(const SmallPrime&)= delete;
  SmallPrime& operator=(const SmallPrime&)= delete;
};

#endif
