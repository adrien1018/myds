#ifndef RBTREE_H_
#define RBTREE_H_

#include <cstdint>
#include <iterator>
#include <algorithm>

template <class T> class RBTree;

namespace RBTreeBase_ {

struct Node_ {
  Node_ *left, *right, *parent;
  size_t size;
  uint8_t black_height; // 1 byte is very sufficient
  bool black;
  Node_() : left(nullptr), right(nullptr), parent(nullptr),
    size(1), black_height(0), black(false) {}
};

template <class T> struct NodeVal_ : public Node_ {
  T value;
  NodeVal_(const T& val) : Node_(), value(val) {}
  NodeVal_(T&& val) : Node_(), value(std::move(val)) {}
};

Node_* First_(Node_* nd) {
  for (; nd->left; nd = nd->left);
  return nd;
}
Node_* Last_(Node_* nd) {
  for (; nd->right; nd = nd->right);
  return nd;
}

void ConnectLeft_(Node_* p, Node_* ch) {
  p->left = ch;
  if (ch) ch->parent = p;
}
void ConnectLeftNoCheck_(Node_* p, Node_* ch) {
  p->left = ch; ch->parent = p;
}
void ConnectRight_(Node_* p, Node_* ch) {
  p->right = ch;
  if (ch) ch->parent = p;
}
void ConnectRightNoCheck_(Node_* p, Node_* ch) {
  p->right = ch; ch->parent = p;
}
void ConnectParent_(Node_* orig, Node_* n) {
  Node_* p = n->parent = orig->parent;
  if (p) (p->left == orig ? p->left : p->right) = n;
}
size_t Size_(Node_* nd) {
  return nd ? nd->size : 0;
}

Node_* Next_(Node_* nd) {
  if (nd->right) return First_(nd->right);
  for (; nd->parent && nd->parent->right == nd; nd = nd->parent);
  return nd->parent;
}
Node_* Prev_(Node_* nd) {
  if (nd->left) return Last_(nd->left);
  for (; nd->parent && nd->parent->left == nd; nd = nd->parent);
  return nd->parent;
}
Node_* Select_(Node_* nd, size_t x) {
  while (true) {
    if (Size_(nd->left) == x) return nd;
    if (Size_(nd->left) < x) {
      nd = nd->left;
    } else {
      x -= Size_(nd->left) + 1;
      nd = nd->right;
    }
  }
}
Node_* Advance_(Node_* nd, ptrdiff_t x) {
  if (!x) return nd;
  if (x < 0) {
    size_t g = -x;
    while (Size_(nd->left) < g) {
      g -= Size_(nd->left) + 1;
      for (; nd->parent && nd->parent->left == nd; nd = nd->parent);
      nd = nd->parent;
      if (!g) return nd;
    }
    return Select_(nd->left, Size_(nd->left) - g);
  } else {
    size_t g = x;
    while (Size_(nd->right) < g) {
      g -= Size_(nd->right) + 1;
      for (; nd->parent && nd->parent->right == nd; nd = nd->parent);
      nd = nd->parent;
      if (!g) return nd;
    }
    return Select_(nd->right, g - 1);
  }
}

size_t Order_(Node_* nd) {
  size_t ans = Size_(nd->left);
  for (; nd; nd = nd->parent) {
    if (nd->parent->right == nd) ans += Size_(nd->parent->left) + 1;
  }
  return ans;
}
ptrdiff_t Difference_(Node_* a, Node_* b) {
  return (ptrdiff_t)Order_(a) - (ptrdiff_t)Order_(b);
}

template <class T> class ConstIterator_;
template <class T> class Iterator_ {
  Node_* ptr_;
  typedef Iterator_ Self_;
  Iterator_(Node_* ptr) : ptr_(ptr) {}
  Iterator_(NodeVal_<T>* ptr) : ptr_(static_cast<Node_*>(ptr)) {}
 public:
  // iterator tags
  typedef T value_type;
  typedef T& reference;
  typedef T* pointer;
  typedef std::random_access_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;

  Iterator_() : ptr_(nullptr) {}

  reference operator*() const { return static_cast<NodeVal_<T>*>(ptr_)->value; }
  pointer operator->() const { return &static_cast<NodeVal_<T>*>(ptr_)->value; }
  Self_& operator++() { ptr_ = Next_(ptr_); return *this; }
  Self_& operator--() { ptr_ = Prev_(ptr_); return *this; }
  Self_ operator++(int) {
    Self_ tmp = *this;
    ptr_ = Next_(ptr_);
    return tmp;
  }
  Self_ operator--(int) {
    Self_ tmp = *this;
    ptr_ = Prev_(ptr_);
    return tmp;
  }
  Self_& operator+=(difference_type x) {
    ptr_ = Advance_(ptr_, x);
    return *this;
  }
  Self_& operator-=(difference_type x) {
    ptr_ = Advance_(ptr_, -x);
    return *this;
  }
  Self_ operator+(difference_type x) const { return Advance_(ptr_, x); }
  Self_ operator-(difference_type x) const { return Advance_(ptr_, -x); }
  difference_type operator-(const Self_& it) const {
    return Difference_(ptr_, it.ptr_);
  }
  bool operator==(const Self_& it) const { return ptr_ == it.ptr_; }
  bool operator!=(const Self_& it) const { return ptr_ != it.ptr_; }
  bool operator<(const Self_& it) const {
    return Difference_(ptr_, it.ptr_) < 0;
  }
  bool operator<=(const Self_& it) const { return !(it < *this); }
  bool operator>(const Self_& it) const { return it < *this; }
  bool operator>=(const Self_& it) const { return !(*this < it); }

  friend class ConstIterator_<T>;
  friend class RBTree<T>;
};

template <class T>
Iterator_<T> operator+(ptrdiff_t x, Iterator_<T> it) {
  return it + x;
}

template <class T> class ConstIterator_ {
  Node_* ptr_;
  typedef ConstIterator_ Self_;

  ConstIterator_(Node_* ptr) : ptr_(ptr) {}
  ConstIterator_(NodeVal_<T>* ptr) : ptr_(static_cast<Node_*>(ptr)) {}
 public:
  // iterator tags
  typedef T value_type;
  typedef const T& reference;
  typedef const T* pointer;
  typedef std::random_access_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;

  ConstIterator_() : ptr_(nullptr) {}
  ConstIterator_(const Iterator_<T>& it) : ptr_(it.ptr_) {}

  reference operator*() const { return static_cast<NodeVal_<T>*>(ptr_)->value; }
  pointer operator->() const { return &static_cast<NodeVal_<T>*>(ptr_)->value; }
  Self_& operator++() { ptr_ = Next_(ptr_); return *this; }
  Self_& operator--() { ptr_ = Prev_(ptr_); return *this; }
  Self_ operator++(int) {
    Self_ tmp = *this;
    ptr_ = Next_(ptr_);
    return tmp;
  }
  Self_ operator--(int) {
    Self_ tmp = *this;
    ptr_ = Prev_(ptr_);
    return tmp;
  }
  Self_& operator+=(difference_type x) {
    ptr_ = Advance_(ptr_, x);
    return *this;
  }
  Self_& operator-=(difference_type x) {
    ptr_ = Advance_(ptr_, -x);
    return *this;
  }
  Self_ operator+(difference_type x) const { return Advance_(ptr_, x); }
  Self_ operator-(difference_type x) const { return Advance_(ptr_, -x); }
  difference_type operator-(Self_ it) const {
    return Difference_(ptr_, it.ptr_);
  }
  bool operator==(const Self_& it) const { return ptr_ == it.ptr_; }
  bool operator!=(const Self_& it) const { return ptr_ != it.ptr_; }
  bool operator<(const Self_& it) const {
    return Difference_(ptr_, it.ptr_) < 0;
  }
  bool operator<=(const Self_& it) const { return !(it < *this); }
  bool operator>(const Self_& it) const { return it < *this; }
  bool operator>=(const Self_& it) const { return !(*this < it); }

  friend class RBTree<T>;
};

template <class T>
ConstIterator_<T> operator+(ptrdiff_t x, ConstIterator_<T> it) {
  return it + x;
}

} // namespace RBTreeBase_

template <class T> class RBTree {
  typedef RBTreeBase_::Node_ NodeBase_;
  typedef RBTreeBase_::NodeVal_<T> NodeType_;

  virtual void Pull_(NodeType_* p) {}

  void PullSize_(NodeType_* nd) const {
    nd->size = Size_(nd->left) + Size_(nd->right) + 1;
    Pull_(nd);
  }
  void IncreaseSize_(NodeType_* nd) const {
    for (; nd != head_; nd = nd->parent) nd->size++, Pull_(nd);
  }

  void InsertRepair_(NodeType_* nd) {
    Pull_(nd);
    while (true) {
      NodeType_* p = nd->parent;
      if (p == head_) { // Case 1
        nd->black = true;
        nd->black_height++;
        return;
      }
      if (p->black) { // Case 2
        IncreaseSize_(nd->parent);
        return;
      }
      NodeType_* g = p->parent;
      NodeType_* u = g->left == p ? g->right : g->left;
      if (!u || u->black) { // Case 4
        if (p == g->left) {
          if (nd == p->right) {
            std::swap(nd, p);
            ConnectLeftNoCheck_(p, nd);
            ConnectRight_(nd, p->left);
            PullSize_(nd);
          }
          ConnectParent_(g, p);
          ConnectLeft_(g, p->right);
          ConnectRightNoCheck_(p, g);
        } else {
          if (nd == p->left) {
            std::swap(nd, p);
            ConnectRightNoCheck_(p, nd);
            ConnectLeft_(nd, p->right);
            PullSize_(nd);
          }
          ConnectParent_(g, p);
          ConnectRight_(g, p->left);
          ConnectLeftNoCheck_(p, g);
        }
        PullSize_(g);
        g->black_height--;
        p->size = g->size + nd->size + 1;
        Pull_(p);
        p->black_height++;
        IncreaseSize_(p->parent);
        return;
      }
      // Case 3
      p->size++; p->black = true; p->black_height++; Pull_(p);
      g->size++; g->black = false; Pull_(g);
      u->black = true; u->black_height++;
      nd = g;
    }
  }

  void InsertBefore_(NodeType_* a, NodeType_* b) {
    if (a != head_) {
      if (!a->left) {
        ConnectLeftNoCheck_(a, b);
      } else {
        ConnectRightNoCheck_(Last_(a->left), b);
      }
    } else if (!head_->left) {
      ConnectLeftNoCheck_(head_, b);
    } else {
      ConnectRightNoCheck_(Last_(head_->left), b);
    }
    InsertRepair_(b);
  }

  NodeBase_* head_;
public:
  typedef RBTreeBase_::Iterator_<T> iterator;
  typedef RBTreeBase_::ConstIterator_<T> const_iterator;

  RBTree() : head_((NodeBase_*)malloc(sizeof(NodeBase_))) {
    new(head_) NodeBase_();
  }

  size_t size() const { return Size_(head_->left); }

  iterator begin() { return head_->left ? head_->left : head_; }
  const_iterator begin() const { return head_->left ? head_->left : head_; }
  const_iterator cbegin() const { return begin(); }
  iterator end() { return head_; }
  const_iterator end() const { return head_; }
  const_iterator cend() const { return end(); }
};

#endif
