#ifndef MYALLOC_H_
#define MYALLOC_H_

#include <cstdlib>

template <class T> struct allocator {
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  allocator() = default;
  template <class U> constexpr allocator(const allocator<U>&) noexcept {}
  template <class U> struct rebind { typedef allocator<U> other; };
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef void* void_pointer;
  typedef const void* const_void_pointer;
  typedef T& reference;
  typedef const T& const_reference;

  T* allocate(size_t sz) const { return (T*)malloc(sizeof(T) * sz); }
  void deallocate(T* a, size_t) const { free(a); }
  template <class U, class... V> void construct(U* a, V... b) const { new(a) U(b...); }
  template <class U> void destroy(U* a) const { a->~U(); }
  size_t max_size() const { return -1; }

  template <class U> bool operator==(const allocator<U>&) const { return true; }
  template <class U> bool operator!=(const allocator<U>&) const { return false; }
};

#endif // MATRIX_H_INCLUDED
