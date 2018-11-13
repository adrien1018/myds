#ifndef POINT_H_
#define POINT_H_

template <class T> struct Point2D {
  T x, y;
  Point2D() {}
  Point2D(const T& x, const T& y) : x(x), y(y) {}
  Point2D& operator+=(const Point2D& a) {
    x += a.x; y += a.y; return *this;
  }
  Point2D& operator-=(const Point2D& a) {
    x -= a.x; y -= a.y; return *this;
  }
  template <class U> Point2D& operator*=(const U& a) {
    x *= a; y *= a; return *this;
  }
  template <class U> Point2D& operator/=(const U& a) {
    x /= a; y /= a; return *this;
  }
};

template <class T> Point2D<T> operator+(Point2D<T> a, const Point2D<T>& b) {
  a += b; return a;
}
template <class T> Point2D<T> operator-(Point2D<T> a, const Point2D<T>& b) {
  a -= b; return a;
}
template <class T, class U> Point2D<T> operator*(Point2D<T> a, const U& b) {
  a *= b; return a;
}
template <class T, class U> Point2D<T> operator/(Point2D<T> a, const U& b) {
  a /= b; return a;
}
template <class T> Point2D<T> operator-(const Point2D<T>& a) {
  return Point2D<T>(-a.x, -a.y);
}

#endif
