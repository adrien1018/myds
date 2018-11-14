#ifndef ARRAY_H_
#define ARRAY_H_

#include <cstdlib>
#include <algorithm>

template <class T> class Array {
 public:
  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
 private:
  size_type sz_, cap_;
  value_type* arr_;
  void CheckRealloc_(size_type x) {
    if (x > cap_) {
      arr_ = (value_type*)realloc(arr_, cap_ = std::max(x, cap_ * 2 + 1));
    }
  }
  template <class... Args> void Construct(size_type x, Args&&... args) {
    new(arr_ + x) value_type(args...);
  }
  void Destruct(size_type x) { (arr_ + x)->~value_type(); }
  void DestructAll() { for (size_type i = 0; i < sz_; i++) Destruct(i); }
 public:
  Array() : sz_(0), cap_(0), arr_(nullptr) {}
  explicit Array(size_type N) : sz_(N), cap_(N),
      arr_((value_type*)malloc(sizeof(value_type) * N)) {
    for (size_type i = 0; i < N; i++) Construct(i);
  }
  Array(size_type N, const value_type& val) : sz_(N), cap_(N),
      arr_((value_type*)malloc(sizeof(value_type) * N)) {
    for (size_type i = 0; i < N; i++) Construct(i, val);
  }
  template <class Iter, class = typename std::iterator_traits<Iter>::iterator_category>
  Array(Iter first, Iter last) {
    cap_ = sz_ = std::distance(first, last);
    arr_ = (value_type*)malloc(sizeof(T) * cap_);
    for (size_type i = 0; i < sz_; ++i, ++first) Construct(i, *first);
  }
  Array(const Array& x) : sz_(x.sz_), cap_(x.sz_),
      arr_((value_type*)malloc(sizeof(value_type) * x.sz_)) {
    for (size_type i = 0; i < sz_; i++) Construct(i, x.arr_[i]);
  }
  Array(Array&& x) : sz_(x.sz_), cap_(x.cap_), arr_(x.arr_) {
    x.arr_ = nullptr;
  }
  Array(std::initializer_list<value_type> x) : sz_(x.size()), cap_(x.size()),
      arr_((value_type*)malloc(sizeof(value_type) * x.size())) {
    auto it = x.begin();
    for (size_type i = 0; i < sz_; ++i, ++it) Construct(i, *it);
  }
  ~Array() {
    DestructAll();
    if (arr_) free(arr_);
  }

  Array& operator=(const Array& x) {
    DestructAll();
    CheckRealloc_(sz_ = x.sz_);
    for (size_type i = 0; i < sz_; i++) Construct(i, x.arr_[i]);
    return *this;
  }
  Array& operator=(Array&& x) {
    sz_ = x.sz_; cap_ = x.cap_;
    std::swap(arr_, x.arr_);
    return *this;
  }
  Array& operator=(std::initializer_list<value_type> x) {
    DestructAll();
    CheckRealloc_(sz_ = x.size());
    auto it = x.begin();
    for (size_type i = 0; i < sz_; ++i, ++it) Construct(i, *it);
    return *this;
  }

  typedef T* iterator;
  typedef const T* const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  iterator begin() { return iterator(arr_); }
  const_iterator begin() const { return const_iterator(arr_); }
  const_iterator cbegin() const { return const_iterator(arr_); }
  iterator end() { return iterator(arr_ + sz_); }
  const_iterator end() const { return const_iterator(arr_ + sz_); }
  const_iterator cend() const { return const_iterator(arr_ + sz_); }
  reverse_iterator rbegin() { return reverse_iterator(arr_ + sz_); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(arr_ + sz_);
  }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(arr_ + sz_);
  }
  reverse_iterator rend() { return reverse_iterator(arr_); }
  const_reverse_iterator rend() const { return const_reverse_iterator(arr_); }
  const_reverse_iterator crend() const { return const_reverse_iterator(arr_); }

  size_type size() const { return sz_; }
  void resize(size_type N) {
    if (N > sz_) {
      CheckRealloc_(N);
      for (; sz_ < N; sz_++) Construct(sz_);
    } else if (N < sz_) {
      for (size_type i = N; i < sz_; i++) Destruct(i);
      sz_ = N;
    }
  }
  void resize(size_type N, const value_type& val) {
    if (N > sz_) {
      CheckRealloc_(N);
      for (; sz_ < N; sz_++) Construct(sz_, val);
    } else if (N < sz_) {
      for (size_type i = N; i < sz_; i++) Destruct(i);
      sz_ = N;
    }
  }
  size_type capacity() const { return cap_; }
  bool empty() const { return sz_ == 0; }
  void reserve(size_type N) { CheckRealloc_(N); }
  void shrink_to_fit() {
    if (sz_ == cap_) return;
    arr_ = (value_type*)realloc(arr_, cap_ = sz_);
  }

  reference operator[](size_type i) { return arr_[i]; }
  const_reference operator[](size_type i) const { return arr_[i]; }
  reference front() { return *arr_; }
  const_reference front() const { return *arr_; }
  reference back() { return arr_[sz_ - 1]; }
  const_reference back() const { return arr_[sz_ - 1]; }
  value_type* data() { return arr_; }
  const value_type* data() const { return arr_; }

  template <class Iter> void assign(Iter first, Iter last) {
    DestructAll();
    CheckRealloc_(sz_ = std::distance(first, last));
    for (size_type i = 0; i < sz_; ++i, ++first) Construct(i, *first);
  }
  void assign(size_type N, const value_type& val) {
    DestructAll();
    CheckRealloc_(sz_ = N);
    for (size_type i = 0; i < sz_; i++) Construct(i, val);
  }
  void assign(std::initializer_list<value_type> x) { operator=(x); }
  void push_back(const value_type& val) {
    CheckRealloc_(++sz_);
    Construct(sz_ - 1, val);
  }
  void push_back(value_type&& val) {
    CheckRealloc_(++sz_);
    Construct(sz_ - 1, std::move(val));
  }
  void pop_back() { Destruct(--sz_); }
  void swap(Array& x) {
    std::swap(sz_, x.sz_);
    std::swap(cap_, x.cap_);
    std::swap(arr_, x.arr_);
  }
  void clear() { DestructAll(); sz_ = 0; }
  template <class... Args> void emplace_back(Args&&... args) {
    CheckRealloc_(++sz_);
    Construct(sz_ - 1, args...);
  }

  bool operator==(const Array& x) const {
    if (sz_ != x.sz_) return false;
    for (size_type i = 0; i < sz_; i++) {
      if (arr_[i] != x.arr_[i]) return false;
    }
    return true;
  }
  bool operator<(const Array& x) const {
    return std::lexicographical_compare(arr_, arr_ + sz_,
        x.arr_, x.arr_ + x.sz_);
  }
  bool operator!=(const Array& x) const { return !operator==(x); }
  bool operator>=(const Array& x) const { return !operator<(x); }
  bool operator>(const Array& x) const { return x < *this; }
  bool operator<=(const Array& x) const { return !(x < *this); }
};

template <class T> void swap(Array<T>& a, Array<T>& b) {
  a.swap(b);
}

#endif
