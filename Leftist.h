#ifndef LEFTIST_H_
#define LEFTIST_H_

#include <cstdlib>
#include <algorithm>
#include <functional>

template <class T, class Compare = std::less<T>>
class LeftistTree {
  struct Node_ {
    Node_ *l, *r;
    size_t depth;
    T value;
    Node_(const T& val) : l(nullptr), r(nullptr), depth(0), value(val) {}
    Node_(T&& val) : l(nullptr), r(nullptr), depth(0), value(val) {}
    Node_(const Node_& x) : l(x.l), r(x.r), depth(x.depth), value(x.value) {}
  };

  Compare comp_;
  Node_* root_;
  size_t size_;

  Node_* Merge_(Node_* a, Node_* b) {
    if (!a) return b;
    if (!b) return a;
    if (comp_(a->value, b->value)) std::iter_swap(a, b);
    a->r = Merge_(a->r, b);
    if (!a->l) {
      a->l = a->r;
      a->r = nullptr;
      a->depth = 0;
    } else {
      if (a->l->depth < a->r->depth) std::swap(a->l, a->r);
      a->depth = a->r->depth + 1;
    }
    return a;
  };
  void Erase_(Node_* a) {
    if (a->l) Erase_(a->l);
    if (a->r) Erase_(a->r);
    free(a);
  }
  void Copy_(const Node_* a, Node_*& b) {
    b = (Node_*)malloc(sizeof(Node_));
    new (b) Node_(*a);
    if (a->l) Copy_(a->l, b->l);
    if (a->r) Copy_(a->r, b->r);
  }
  template <class Iter> void Init_(Iter first, Iter last) {
    if (first == last) {
      size_ = 0;
      root_ = nullptr;
      return;
    }
    size_ = std::distance(first, last);
    Node_** vec = (Node_**)malloc(sizeof(Node_*) * size_);
    Node_ **end = vec + size_, **vit, **mid;
    Iter it;
    for (it = first, vit = vec; it != last; ++it, ++vit) {
      *vit = (Node_*)malloc(sizeof(Node_));
      new (*vit) Node_(*it);
    }
    while (end - vec > 1) {
      size_t half = (end - vec + 1) / 2;
      for (vit = vec, mid = vec + half; mid != end; ++vit, ++mid) {
        *vit = Merge_(*vit, *mid);
      }
      end = vec + half;
    }
    root_ = *vec;
    free(vec);
  }
 public:
  LeftistTree(Compare comp = Compare()) :
      comp_(comp), root_(nullptr), size_(0) {}
  template <class Iter>
  LeftistTree(Iter first, Iter last, Compare comp = Compare()) : comp_(comp) {
    Init_(first, last);
  }
  LeftistTree(std::initializer_list<T> il, Compare comp = Compare()) :
      comp_(comp) {
    Init_(il.begin(), il.end());
  }

  LeftistTree(const LeftistTree& x) : comp_(x.comp_), size_(x.size_) {
    if (x.root_) {
      Copy_(x.root_, root_);
    } else {
      root_ = nullptr;
    }
  }
  LeftistTree(LeftistTree&& x) :
      comp_(x.comp_), root_(x.root_), size_(x.size_) {
    x.root_ = nullptr;
  }

  ~LeftistTree() { if (root_) Erase_(root_); }

  LeftistTree& operator=(const LeftistTree& x) {
    if (root_) Erase_(root_);
    comp_ = x.comp_;
    size_ = x.size_;
    if (x.root_) {
      Copy_(x.root_, root_);
    } else {
      root_ = nullptr;
    }
  }
  LeftistTree& operator=(LeftistTree&& x) {
    if (root_) Erase_(root_);
    comp_ = x.comp_;
    root_ = x.root_;
    size_ = x.size_;
    x.root_ = nullptr;
  }
  LeftistTree& operator=(std::initializer_list<T> il) {
    if (root_) Erase_(root_);
    Init_(il.begin(), il.end());
  }

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

  const T& top() const { return root_->value; }

  template <class Iter> void assign(Iter first, Iter last) {
    if (root_) Erase_(root_);
    Init_(first, last);
  }
  void push(const T& val) {
    Node_* nd = (Node_*)malloc(sizeof(Node_));
    new (nd) Node_(val);
    root_ = Merge_(root_, nd);
    size_++;
  }
  void push(T&& val) {
    Node_* nd = (Node_*)malloc(sizeof(Node_));
    new (nd) Node_(std::move(val));
    root_ = Merge_(root_, nd);
    size_++;
  }
  void pop() {
    Node_* tmp = root_;
    root_ = Merge_(root_->l, root_->r);
    free(tmp);
    size_--;
  }
  void swap(LeftistTree& x) {
    std::swap(comp_, x.comp_);
    std::swap(root_, x.root_);
    std::swap(size_, x.size_);
  }
  void clear() {
    if (root_) {
      Erase_(root_);
      size_ = 0;
      root_ = nullptr;
    }
  }
  void join(LeftistTree& x) {
    size_ += x.size_;
    root_ = Merge_(root_, x.root_);
    x.size_ = 0;
    x.root_ = nullptr;
  }
  void join(LeftistTree&& x) {
    size_ += x.size_;
    root_ = Merge_(root_, x.root_);
    x.root_ = nullptr;
  }
};

#endif
