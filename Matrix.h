#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include <algorithm>
#include <stdexcept>

#ifdef DEBUG
#include <cstdio>
#define dbgln(x) puts(x)
#define dbg(...) printf(__VA_ARGS__)
#else
#define dbgln(x)
#define dbg(x)
#endif

// helper templates to decide if types are compound-assignable
template<typename T, typename U, typename = void>
struct __is_mul_assignable : std::false_type {};
template<typename T, typename U>
struct __is_mul_assignable<T, U,
    decltype(std::declval<T>() *= std::declval<U>(), void())>
        : std::true_type {};
template<typename T, typename U, typename = void>
struct __is_div_assignable : std::false_type {};
template<typename T, typename U>
struct __is_div_assignable<T, U,
    decltype(std::declval<T>() /= std::declval<U>(), void())>
        : std::true_type {};

template <class T> class Matrix {
  size_t row_, col_;
  T* mat_;

  class MatrixRow {
    T* ptr_;
  public:
    MatrixRow(T* ptr) : ptr_(ptr) {}
    T& operator[](size_t sz) const { return ptr_[sz]; }
  };

  class MatrixRowConst {
    const T* ptr_;
  public:
    MatrixRowConst(const T* ptr) : ptr_(ptr) {}
    const T& operator[](size_t sz) const { return ptr_[sz]; }
  };

  template <class U> using Mul_ = typename std::enable_if<
      __is_mul_assignable<T&, const U&>::value>::type;
  template <class U> using Div_ = typename std::enable_if<
      __is_div_assignable<T&, const U&>::value>::type;
public:
  typedef T value_type;

  enum MatrixCreationType {
    Empty, Zeros, Ones, Eye
  };

  // constructors
  explicit Matrix(size_t row = 1, size_t col = 1) : row_(row), col_(col) {
    mat_ = new T[row_ * col_]();
    dbgln("onstruct");
  }
  Matrix(size_t row, size_t col, MatrixCreationType type)
      : row_(row), col_(col) {
    switch (type) {
      case Empty: {
        mat_ = new T[row_ * col_];
        break;
      }
      case Zeros: {
        mat_ = new T[row_ * col_];
        std::fill_n(mat_, row_ * col_, 0);
        break;
      }
      case Ones: {
        mat_ = new T[row_ * col_];
        std::fill_n(mat_, row_ * col_, 1);
        break;
      }
      case Eye: {
        mat_ = new T[row_ * col_];
        std::fill_n(mat_, row_ * col_, 0);
        size_t d = std::min(row_, col_);
        for (T* it = mat_; d--; it += col_ + 1) *it = 1;
        break;
      }
    }
    dbgln("construct");
  }
  Matrix(const Matrix& rhs) : row_(rhs.row_), col_(rhs.col_) {
    mat_ = new T[row_ * col_];
    std::copy(rhs.mat_, rhs.mat_ + row_ * col_, mat_);
    dbgln("copy construct");
  }
  Matrix(Matrix&& rhs) : row_(rhs.row_), col_(rhs.col_), mat_(rhs.mat_) {
    rhs.mat_ = nullptr;
    dbgln("move construct");
  }

  // destructor
  ~Matrix() { if (mat_) delete[] mat_; dbgln("destruct"); }

  // assignments
  const Matrix& operator=(const Matrix& rhs) {
    if (mat_ != rhs.mat_) {
      if (row_ * col_ != rhs.row_ * rhs.col_) {
        delete[] mat_;
        mat_ = new T[rhs.row_ * rhs.col_];
      }
      row_ = rhs.row_;
      col_ = rhs.col_;
      std::copy(rhs.mat_, rhs.mat_ + row_ * col_, mat_);
    }
    dbgln("copy assign");
    return *this;
  }
  const Matrix& operator=(Matrix&& rhs) {
    row_ = rhs.row_;
    col_ = rhs.col_;
    std::swap(mat_, rhs.mat_);
    dbgln("move assign");
    return *this;
  }

  // element accessing
  T& at(size_t r, size_t c) {
    if (r >= row_ || c >= col_) throw std::out_of_range("Matrix::at");
    return mat_[r * col_ + c];
  }
  const T& at(size_t r, size_t c) const {
    if (r >= row_ || c >= col_) throw std::out_of_range("Matrix::at");
    return mat_[r * col_ + c];
  }
  MatrixRow operator[](size_t sz) { return mat_ + sz * col_; }
  MatrixRowConst operator[](size_t sz) const { return mat_ + sz * col_; }

  // size
  size_t row() const { return row_; }
  size_t col() const { return col_; }

  // matrix add/subtract
  const Matrix& operator+=(const Matrix& rhs) {
    if (row_ != rhs.row_ || col_ != rhs.col_)
      throw std::length_error("Matrix::operator+=");
    size_t size = row_ * col_;
    for (size_t i = 0; i < size; i++) mat_[i] += rhs.mat_[i];
    return *this;
  }
  const Matrix& operator-=(const Matrix& rhs) {
    if (row_ != rhs.row_ || col_ != rhs.col_)
      throw std::length_error("Matrix::operator-=");
    size_t size = row_ * col_;
    for (size_t i = 0; i < size; i++) mat_[i] -= rhs.mat_[i];
    return *this;
  }

  const Matrix& operator*=(const Matrix& rhs) {
    *this = *this * rhs;
    return *this;
  }

  template <class U, class = Mul_<U>>
  const Matrix& operator*=(const U& rhs) {
    size_t size = row_ * col_;
    for (size_t i = 0; i < size; i++) mat_[i] *= rhs;
    return *this;
  }
  template <class U, class = Div_<U>>
  const Matrix& operator/=(const U& rhs) {
    size_t size = row_ * col_;
    for (size_t i = 0; i < size; i++) mat_[i] /= rhs;
    return *this;
  }

  template <class U>
  friend Matrix<U> operator+(const Matrix<U>& a, const Matrix<U>& b);
  template <class U>
  friend Matrix<U> operator-(const Matrix<U>& a, const Matrix<U>& b);
  template <class U>
  friend Matrix<U> operator-(const Matrix<U>& a, Matrix<U>&& b);
  template <class U>
  friend Matrix<U> operator*(const Matrix<U>& a, const Matrix<U>& b);
};

template <class T> Matrix<T> operator+(const Matrix<T>& a, const Matrix<T>& b) {
  if (a.row_ != b.row_ || a.col_ != b.col_)
    throw std::length_error("Matrix::operator+");
  Matrix<T> ret(a.row_, a.col_, Matrix<T>::Empty);
  size_t size = a.row_ * a.col_;
  for (size_t i = 0; i < size; i++) ret.mat_[i] = a.mat_[i] + b.mat_[i];
  return ret;
}
template <class T> Matrix<T> operator+(Matrix<T>&& a, const Matrix<T>& b) {
  a += b;
  return a;
}
template <class T> Matrix<T> operator+(const Matrix<T>& a, Matrix<T>&& b) {
  b += a;
  return b;
}
template <class T> Matrix<T> operator+(Matrix<T>&& a, Matrix<T>&& b) {
  a += b;
  return a;
}

template <class T> Matrix<T> operator-(const Matrix<T>& a, const Matrix<T>& b) {
  if (a.row_ != b.row_ || a.col_ != b.col_)
    throw std::length_error("Matrix::operator-");
  Matrix<T> ret(a.row_, a.col_, Matrix<T>::Empty);
  size_t size = a.row_ * a.col_;
  for (size_t i = 0; i < size; i++) ret.mat_[i] = a.mat_[i] - b.mat_[i];
  return ret;
}
template <class T> Matrix<T> operator-(Matrix<T>&& a, const Matrix<T>& b) {
  a -= b;
  return a;
}

template <class T> Matrix<T> operator-(const Matrix<T>& a, Matrix<T>&& b) {
  if (a.row_ != b.row_ || a.col_ != b.col_)
    throw std::length_error("Matrix::operator-");
  size_t size = a.row_ * a.col_;
  for (size_t i = 0; i < size; i++) b.mat_[i] = a.mat_[i] - b.mat_[i];
  return b;
}
template <class T> Matrix<T> operator-(Matrix<T>&& a, Matrix<T>&& b) {
  a -= b;
  return a;
}

template <class T> Matrix<T> operator*(const Matrix<T>& a, const Matrix<T>& b) {
  if (a.col_ != b.row_) throw std::length_error("Matrix::operator*");
  Matrix<T> ret(a.row_, b.col_, Matrix<T>::Zeros);
  const T* lhs_end = a.mat_ + a.row_ * a.col_;
  const T* rhs_end = b.mat_ + b.row_ * b.col_;
  T* ret_begin = ret.mat_;
  T* ret_end = ret.mat_ + ret.col_;
  for (const T* lhs_it = a.mat_; lhs_it != lhs_end;) {
    for (const T* rhs_it = b.mat_; rhs_it != rhs_end; lhs_it++) {
      for (T* ret_it = ret_begin; ret_it != ret_end;)
        *ret_it++ += *lhs_it * *rhs_it++;
    }
    ret_begin = ret_end;
    ret_end += ret.col_;
  }
  return ret;
}

template <class T, class U>
Matrix<T> operator*(const Matrix<T>& a, const U& b) {
  Matrix<T> z(a);
  z *= b;
  return z;
}
template <class T, class U>
Matrix<T> operator*(const U& b, const Matrix<T>& a) {
  Matrix<T> z(a);
  z *= b;
  return z;
}
template <class T, class U> Matrix<T> operator*(Matrix<T>&& a, const U& b) {
  a *= b;
  return a;
}
template <class T, class U> Matrix<T> operator*(const U& b, Matrix<T>&& a) {
  a *= b;
  return a;
}

template <class T, class U>
Matrix<T> operator/(const Matrix<T>& a, const U& b) {
  Matrix<T> z(a);
  z /= b;
  return z;
}
template <class T, class U> Matrix<T> operator/(Matrix<T>&& a, const U& b) {
  a /= b;
  return a;
}

#endif // MATRIX_H_INCLUDED
