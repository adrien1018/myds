#ifndef MATH_H_
#define MATH_H_

#include <algorithm>

template <class T> inline T Gcd(T a, T b) {
  if (a == T(0)) return b;
  if (b == T(0)) return a;
  if (a < 0) a = -a;
  if (b < 0) b = -b;
  unsigned int shift = __builtin_ctzll(a | b);
  a >>= __builtin_ctzll(a);
  do {
    b >>= __builtin_ctzll(b);
    if (a > b) std::swap(a, b);
  } while (b -= a);
  return a << shift;
}

template <class T, class Func> inline T Gcd(T a, T b, Func ctz) {
  if (a == T(0)) return b;
  if (b == T(0)) return a;
  if (a < 0) a = -a;
  if (b < 0) b = -b;
  unsigned int shift = ctz(a | b);
  a >>= ctz(a);
  do {
    b >>= ctz(b);
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
