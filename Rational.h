#ifndef RATIONAL_H_
#define RATIONAL_H_

#include <Math.h>

template <class T> struct DefaultLowbit {
  unsigned int operator()(const T& x) {
    return __builtin_ctzll(x);
  }
};

template <class T, class Lowbit = DefaultLowbit<T>> class Rational {
  T num_, denom_;
  Lowbit ctz_;
  void Reduce_() {
    T gcd = Gcd(num_, denom_, ctz_);
    num_ /= gcd; denom_ /= gcd;
  }
  Rational(T x, T y, int) : num_(x), denom_(y) {}
 public:
  Rational(T x = 0) : num_(x), denom_(1) {}
  Rational(T x, T y) : num_(x), denom_(y) { Reduce_(); }
  // use default copy-constructor / copy-assignment
  Rational& operator+=(const Rational& x) {
    T gcd = Gcd(denom_, x.denom_, ctz_);
    T fac = x.denom_ / gcd;
    num_ = fac * num_ + denom_ / gcd * x.num_;
    denom_ *= fac;
    Reduce_();
    return *this;
  }
  Rational& operator-=(const Rational& x) {
    T gcd = Gcd(denom_, x.denom_, ctz_);
    T fac = x.denom_ / gcd;
    num_ = fac * num_ - denom_ / gcd * x.num_;
    denom_ *= fac;
    Reduce_();
    return *this;
  }
  Rational& operator*=(const Rational& x) {
    T gcd1 = Gcd(num_, x.denom_, ctz_);
    T gcd2 = Gcd(denom_, x.num_, ctz_);
    num_ = (num_ / gcd1) * (x.num_ / gcd2);
    denom_ = (denom_ / gcd2) * (x.denom_ / gcd1);
    return *this;
  }
  Rational& operator/=(const Rational& x) {
    T gcd1 = Gcd(num_, x.num_, ctz_);
    T gcd2 = Gcd(denom_, x.denom_, ctz_);
    num_ = (num_ / gcd1) * (x.denom_ / gcd2);
    denom_ = (denom_ / gcd2) * (x.num_ / gcd1);
    return *this;
  }
  Rational operator-() const { return Rational(-num_, denom_, 0); }
  bool operator==(const Rational& x) const {
    return num_ == x.num_ && denom_ == x.denom_;
  }
  bool operator!=(const Rational& x) const { return !operator==(x); }
  bool operator<(const Rational& x) const {
    T gcd = Gcd(denom_, x.denom_, ctz_);
    return x.denom_ / gcd * num_ < denom_ / gcd * x.num_;
  }
  bool operator>(const Rational& x) const { return x.operator<(*this); }
  bool operator<=(const Rational& x) const { return !x.operator<(*this); }
  bool operator>=(const Rational& x) const { return !operator<(x); }
  const T& Num() const { return num_; }
  const T& Den() const { return denom_; }
};

template <class T>
Rational<T> operator+(const Rational<T>& x, const Rational<T>& y) {
  Rational<T> ret(x); ret += y; return ret;
}
template <class T>
Rational<T> operator-(const Rational<T>& x, const Rational<T>& y) {
  Rational<T> ret(x); ret -= y; return ret;
}
template <class T>
Rational<T> operator*(const Rational<T>& x, const Rational<T>& y) {
  Rational<T> ret(x); ret *= y; return ret;
}
template <class T>
Rational<T> operator/(const Rational<T>& x, const Rational<T>& y) {
  Rational<T> ret(x); ret /= y; return ret;
}

template <class T, class Lowbit = DefaultLowbit<T>> class FastRational {
  T num_, denom_;
  Lowbit ctz_;
  void Reduce_() {
    T gcd = Gcd(num_, denom_, ctz_);
    num_ /= gcd; denom_ /= gcd;
  }
  FastRational(T x, T y, int) : num_(x), denom_(y) {}
 public:
  FastRational(T x = 0) : num_(x), denom_(1) {}
  FastRational(T x, T y) : num_(x), denom_(y) { Reduce_(); }
  // use default copy-constructor / copy-assignment
  FastRational& operator+=(const FastRational& x) {
    num_ = num_ * x.denom_ + denom_ * x.num_;
    denom_ *= x.denom_;
    Reduce_();
    return *this;
  }
  FastRational& operator-=(const FastRational& x) {
    num_ = num_ * x.denom_ - denom_ * x.num_;
    denom_ *= x.denom_;
    Reduce_();
    return *this;
  }
  FastRational& operator*=(const FastRational& x) {
    num_ *= x.num_;
    denom_ *= x.denom_;
    Reduce_();
    return *this;
  }
  FastRational& operator/=(const FastRational& x) {
    num_ *= x.denom_;
    denom_ *= x.num_;
    Reduce_();
    return *this;
  }
  FastRational operator-() const { return FastRational(-num_, denom_, 0); }
  bool operator==(const FastRational& x) const {
    return num_ == x.num_ && denom_ == x.denom_;
  }
  bool operator!=(const FastRational& x) const { return !operator==(x); }
  bool operator<(const FastRational& x) const {
    return num_ * x.denom_ < denom_ * x.num_;
  }
  bool operator>(const FastRational& x) const { return x.operator<(*this); }
  bool operator<=(const FastRational& x) const { return !x.operator<(*this); }
  bool operator>=(const FastRational& x) const { return !operator<(x); }
  const T& Num() const { return num_; }
  const T& Den() const { return denom_; }
};

template <class T>
FastRational<T> operator+(const FastRational<T>& x, const FastRational<T>& y) {
  FastRational<T> ret(x); ret += y; return ret;
}
template <class T>
FastRational<T> operator-(const FastRational<T>& x, const FastRational<T>& y) {
  FastRational<T> ret(x); ret -= y; return ret;
}
template <class T>
FastRational<T> operator*(const FastRational<T>& x, const FastRational<T>& y) {
  FastRational<T> ret(x); ret *= y; return ret;
}
template <class T>
FastRational<T> operator/(const FastRational<T>& x, const FastRational<T>& y) {
  FastRational<T> ret(x); ret /= y; return ret;
}

#endif
