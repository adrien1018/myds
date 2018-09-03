#ifndef IO_H_
#define IO_H_

#include <cstdio>
#include <cstring>
#ifndef NOFLOAT
#include <cmath>
#endif

template <size_t kBufSize = 131072> class FileReader {
  FILE* file_;
  char buf_[kBufSize];
  char *now_, *end_;
  char GetChar_() {
    if (now_ == end_) {
      end_ = fread(now_ = buf_, 1, kBufSize, file_) + buf_;
      if (now_ == end_) return 0;
    }
    return *now_++;
  }
  template <class T> void PosLoop_(T& a, char& p, char start) {
    a = start;
    while ((p = GetChar_()) >= '0' && p <= '9') a = a * 10 + (p ^ '0');
  }
  template <class T> void NegLoop_(T& a, char& p, char start) {
    a = start;
    while ((p = GetChar_()) >= '0' && p <= '9') a = a * 10 - (p ^ '0');
  }
  template <class T> void DecPointLoop_(T& a, char& p) {
    T tmp = T(1);
    while ((p = GetChar_()) >= '0' && p <= '9') {
      tmp /= T(10);
      a += tmp * (p ^ '0');
    }
  }
  bool ToNext_(char& p) {
    while ((p = GetChar_()) <= ' ') {
      if (!p) return false;
    }
    return true;
  }
 public:
  FileReader(FILE* f) : file_(f), now_(nullptr), end_(nullptr) {}
  template <class T> bool GetUInt(T& a) {
    char p;
    if (!ToNext_(p)) return false;
    PosLoop_(a, p, p ^ '0');
    return true;
  }
  template <class T> bool GetInt(T& a) {
    char p;
    if (!ToNext_(p)) return false;
    if (p == '-') {
      NegLoop_(a, p, 0);
    } else {
      PosLoop_(a, p, p == '+' ? 0 : (p ^ '0'));
    }
    return true;
  }
  template <class T> bool GetFloatExp(T& a) {
    char p; bool neg = false;
    if (!ToNext_(p)) return false;
    neg = p == '-';
    if (p != '.') PosLoop_(a, p, p == '+' || p == '-' ? 0 : (p ^ '0'));
    if (p == '.') DecPointLoop_(a, p);
    if (p == 'e' || p == 'E') {
      int pw;
      p = GetChar_();
      if (p == '-') {
        NegLoop_(pw, p, 0);
      } else {
        PosLoop_(pw, p, p == '+' ? 0 : (p ^ '0'));
      }
      a *= pow(T(10), pw);
    }
    if (neg) a = -a;
    return true;
  }
  template <class T> bool GetFloat(T& a) {
    char p; bool neg = false;
    if (!ToNext_(p)) return false;
    neg = p == '-';
    if (p != '.') PosLoop_(a, p, p == '+' || p == '-' ? 0 : (p ^ '0'));
    if (p == '.') DecPointLoop_(a, p);
    if (neg) a = -a;
    return true;
  }
  bool GetNthChar(int N, char& a) {
    if (!ToNext_(a)) return false;
    while (N--) a = GetChar_();
    for (; GetChar_() > ' ';);
    return true;
  }
  bool GetStr(char* a) {
    if (!ToNext_(*a)) return false;
    while ((*++a = GetChar_()) > ' ');
    *a = 0;
    return true;
  }
};

template <size_t kBufSize = 131072> class FileWriter {
  FILE* file_;
  char buf_[kBufSize];
  size_t size_;
  void Flush_() { fwrite(buf_, 1, size_, file_); size_ = 0; }
  void CheckFlush_(size_t sz) { if (sz + size_ > kBufSize) Flush_(); }
 public:
  FileWriter(FILE* f) : file_(f), size_(0) {}
  size_t size() const { return size_; }
  void Flush() { Flush_(); }
  template <class T> void PutUInt(const T& a) {
    static char tmp[22] = "01234567890123456789\n";
    CheckFlush_(21);
    int tail = 20;
    if (!a) {
      tmp[--tail] = '0';
    } else {
      for (; a; a /= 10) tmp[--tail] = (a % 10) ^ '0';
    }
    memcpy(buf_ + size_, tmp + tail, 21 - tail);
    size_ += 21 - tail;
  }
  template <class T> void PutInt(const T& a) {
    static char tmp[22] = "01234567890123456789\n";
    CheckFlush_(21);
    int tail = 20;
    bool neg = a < 0;
    if (neg) a = -a;
    if (!a) {
      tmp[--tail] = '0';
    } else {
      for (; a; a /= 10) tmp[--tail] = (a % 10) ^ '0';
    }
    if (neg) tmp[--tail] = '-';
    memcpy(buf_ + size_, tmp + tail, 21 - tail);
    size_ += 21 - tail;
  }
  void PutStr(const char* str, size_t sz) {
    CheckFlush_(sz);
    memcpy(buf_ + size_, str, sz);
    size_ += sz;
  }
  void PutStr(const char* str) {
    PutStr(str, strlen(str));
  }
  void PutChar(char a) {
    CheckFlush_(1);
    buf_[size_++] = a;
  }
  void PutStrNoCheck(const char* str, size_t sz) {
    memcpy(buf_ + size_, str, sz);
    size_ += sz;
  }
  void PutStrNoCheck(const char* str) {
    while (*str) buf_[size_++] = *str++;
  }
  void PutCharNoCheck(char a) {
    buf_[size_++] = a;
  }
};

#endif
