#ifndef FIXSTRING_H_
#define FIXSTRING_H_

#include <cstdlib>
#include <cstring>
#include <iterator>
#include <algorithm>
#include <functional>

class FixString {
  char* str_;
  size_t size_;
public:
  typedef char* iterator;
  typedef const char* const_iterator;
  typedef std::reverse_iterator<char*> reverse_iterator;
  typedef std::reverse_iterator<const char*> const_reverse_iterator;

  FixString() : str_(nullptr), size_(0) {}
  ~FixString() { if (str_) free(str_); }
  FixString(const char* s) : size_(strlen(s)) {
    str_ = (char*)malloc(size_ + 1);
    memcpy(str_, s, size_ + 1);
  }
  FixString(const FixString& s) : size_(s.size_) {
    str_ = (char*)malloc(size_ + 1);
    memcpy(str_, s.str_, size_ + 1);
  }
  FixString(size_t sz) : size_(sz) { str_ = (char*)calloc(1, size_ + 1); }
  FixString(FixString&& s) : str_(s.str_), size_(s.size_) { s.str_ = nullptr; }

  const FixString& operator=(const FixString& s) {
    if (s.str_ == str_) return *this;
    if (s.size_ != size_) {
      if (str_) free(str_);
      if (s.str_) str_ = (char*)malloc(s.size_ + 1);
      size_ = s.size_;
    }
    memcpy(str_, s.str_, s.size_ + 1);
    return *this;
  }
  const FixString& operator=(FixString&& s) {
    if (s.str_ == str_) return *this;
    if (str_) free(str_);
    str_ = s.str_; size_ = s.size_;
    s.str_ = nullptr;
  }

  size_t size() const { return size_; }
  bool operator<(const FixString& str) const {
    return std::lexicographical_compare(str_, str_ + size_, str.str_, str.str_ + str.size_);
  }
  bool operator>(const FixString& str) const {
    return std::lexicographical_compare(str.str_, str.str_ + str.size_, str_, str_ + size_);
  }
  bool operator>=(const FixString& str) const {
    return !std::lexicographical_compare(str_, str_ + size_, str.str_, str.str_ + str.size_);
  }
  bool operator<=(const FixString& str) const {
    return !std::lexicographical_compare(str.str_, str.str_ + str.size_, str_, str_ + size_);
  }
  bool operator==(const FixString& str) const {
    if (str.size_ != size_) return false;
    for (size_t i = 0; i < size_; i++)
      if (str_[i] != str.str_[i]) return false;
    return true;
  }
  bool operator!=(const FixString& str) const {
    if (str.size_ != size_) return true;
    for (size_t i = 0; i < size_; i++)
      if (str_[i] != str.str_[i]) return true;
    return false;
  }

  char& operator[](size_t pos) { return str_[pos]; }
  const char& operator[](size_t pos) const { return str_[pos]; }

  char* c_str() { return str_; }
  const char* c_str() const { return str_; }

  iterator begin() { return str_; }
  iterator end() { return str_ + size_; }
  const_iterator cbegin() const { return str_; }
  const_iterator cend() const { return str_ + size_; }
  reverse_iterator rbegin() { return reverse_iterator(str_ + size_); }
  reverse_iterator rend() { return reverse_iterator(str_); }
  const_reverse_iterator crbegin() { return const_reverse_iterator(str_ + size_); }
  const_reverse_iterator crend() { return const_reverse_iterator(str_); }
};

namespace std {

template <> hash<FixString> {
  size_t operator()(const FixString& str) {
    size_t num = 819638524362401573ll;
    for (size_t i = 0; i < size_; i++) {
      num += (num << 11 >> 3) + (num >> 53) + str[i];
      num = (num & ((1ll << 61) - 1)) + (num >> 61);
    }
    return num;
  }
};

} // std

#endif // MATRIX_H_INCLUDED
