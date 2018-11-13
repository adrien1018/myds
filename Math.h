#ifndef MATH_H_
#define MATH_H_

#include <algorithm>

template <class T> inline T Gcd(T a, T b) {
  int shift = __builtin_ctzll(a | b);
  a >>= __builtin_ctzll(a);
  do {
    b >>= __builtin_ctzll(b);
    if (a > b) std::swap(a, b);
  } while (b -= a);
  return a << shift;
}

template <class T> inline T ModPow(T a, T b, T m) {
  T ret = 1;
  while (true) {
    if (b & 1) ret = ret * a % m;
    if ((b >>= 1) == 0) break;
    a = a * a % m;
  }
  return ret;
}

#endif
