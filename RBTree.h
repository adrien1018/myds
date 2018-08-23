#ifndef RBTREE_H_
#define RBTREE_H_

#include <cstdint>
#include <iterator>
#include <algorithm>

template <class T, class PullFunc, class PushFunc> class RBTree;

namespace RBTreeBase_ {

struct Node_ {
  Node_ *left, *right, *parent;
  size_t size;
  uint8_t black_height; // starts from 1; 0 indicates phantom root
  bool black;
  Node_() : left(nullptr), right(nullptr), parent(nullptr),
    size(1), black_height(1), black(false) {}
  Node_(const Node_& x, Node_* p) : left(nullptr), right(nullptr), parent(p),
    size(x.size), black_height(x.black_height), black(x.black) {}
};

template <class T> struct NodeVal_ : Node_ {
  T value;
  NodeVal_(const T& val) : Node_(), value(val) {}
  NodeVal_(T&& val) : Node_(), value(std::move(val)) {}
  NodeVal_(NodeVal_* x, Node_* p) : Node_(*x, p), value(x->value) {}
  template <class... Args> NodeVal_(Args&&... args) : Node_(), value(args...) {}
};

inline Node_* First_(Node_* nd) {
  for (; nd->left; nd = nd->left);
  return nd;
}
inline Node_* Last_(Node_* nd) {
  for (; nd->right; nd = nd->right);
  return nd;
}
inline Node_* PostorderFirst_(Node_* nd) {
  while (true) {
    if (nd->left) nd = nd->left;
    else if (nd->right) nd = nd->right;
    else return nd;
  }
}
inline Node_* PreorderLast_(Node_* nd) {
  while (true) {
    if (nd->right) nd = nd->right;
    else if (nd->left) nd = nd->left;
    else return nd;
  }
}

inline void ConnectLeft_(Node_* p, Node_* ch) {
  p->left = ch;
  if (ch) ch->parent = p;
}
inline void ConnectLeftNoCheck_(Node_* p, Node_* ch) {
  p->left = ch; ch->parent = p;
}
inline void ConnectRight_(Node_* p, Node_* ch) {
  p->right = ch;
  if (ch) ch->parent = p;
}
inline void ConnectRightNoCheck_(Node_* p, Node_* ch) {
  p->right = ch; ch->parent = p;
}
inline void ConnectParent_(Node_* orig, Node_* n) {
  Node_* p = n->parent = orig->parent;
  if (p) (p->left == orig ? p->left : p->right) = n;
}
inline void ConnectParentNoCheck_(Node_* orig, Node_* n) {
  Node_* p = n->parent = orig->parent;
  (p->left == orig ? p->left : p->right) = n;
}
inline size_t Size_(Node_* nd) {
  return nd ? nd->size : 0;
}

inline Node_* Next_(Node_* nd) {
  if (nd->right) return First_(nd->right);
  for (; nd->black_height && nd->parent->right == nd; nd = nd->parent);
  return nd->parent;
}
inline Node_* Prev_(Node_* nd) {
  if (nd->left) return Last_(nd->left);
  for (; nd->black_height && nd->parent->left == nd; nd = nd->parent);
  return nd->parent;
}
inline Node_* PreorderNext_(Node_* nd) {
  if (nd->left) return nd->left;
  if (nd->right) return nd->right;
  for (; nd->black_height && (!nd->parent->right || nd->parent->right == nd);
       nd = nd->parent);
  return nd->black_height ? nd->parent->right : nd;
}
inline Node_* PreorderPrev_(Node_* nd) {
  if (!nd->black_height) return PreorderLast_(nd->left);
  if (!nd->parent->left || nd->parent->left == nd) return nd->parent;
  return PreorderLast_(nd->parent->left);
}
inline Node_* PostorderNext_(Node_* nd) {
  if (!nd->parent->right || nd->parent->right == nd) return nd->parent;
  return PostorderFirst_(nd->parent->right);
}
inline Node_* PostorderPrev_(Node_* nd) {
  if (nd->right) return nd->right;
  if (nd->left) return nd->left;
  for (; !nd->parent->left || nd->parent->left == nd; nd = nd->parent);
  return nd->parent->left; // since begin-- is UB
}
inline Node_* Select_(Node_* nd, size_t x) {
  while (true) {
    if (Size_(nd->left) == x) return nd;
    if (Size_(nd->left) > x) {
      nd = nd->left;
    } else {
      x -= Size_(nd->left) + 1;
      nd = nd->right;
    }
  }
}
inline Node_* Advance_(Node_* nd, ptrdiff_t x) {
  if (!x) return nd;
  if (x < 0) {
    size_t g = -x;
    while (Size_(nd->left) < g) {
      g -= Size_(nd->left) + 1;
      for (; nd->black_height && nd->parent->left == nd; nd = nd->parent);
      nd = nd->parent;
      if (!g) return nd;
    }
    return Select_(nd->left, Size_(nd->left) - g);
  } else {
    size_t g = x;
    while (Size_(nd->right) < g) {
      g -= Size_(nd->right) + 1;
      for (; nd->black_height && nd->parent->right == nd; nd = nd->parent);
      nd = nd->parent;
      if (!g) return nd;
    }
    return Select_(nd->right, g - 1);
  }
}

inline size_t Order_(Node_* nd) {
  size_t ans = Size_(nd->left);
  for (; nd->black_height; nd = nd->parent) {
    if (nd->parent->right == nd) ans += Size_(nd->parent->left) + 1;
  }
  return ans;
}
inline ptrdiff_t Difference_(Node_* a, Node_* b) {
  return (ptrdiff_t)Order_(a) - (ptrdiff_t)Order_(b);
}

template <class T> class ConstIterator_;
template <class T> class PreorderIterator_;
template <class T> class PostorderIterator_;
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
  Iterator_(const PreorderIterator_<T>& it) : ptr_(it.ptr_) {}
  Iterator_(const PostorderIterator_<T>& it) : ptr_(it.ptr_) {}

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
  reference operator[](difference_type x) const {
    return static_cast<NodeVal_<T>*>(Advance_(ptr_, x))->value;
  }
  bool operator==(const Self_& it) const { return ptr_ == it.ptr_; }
  bool operator!=(const Self_& it) const { return ptr_ != it.ptr_; }
  bool operator<(const Self_& it) const {
    return Difference_(ptr_, it.ptr_) < 0;
  }
  bool operator<=(const Self_& it) const { return !(it < *this); }
  bool operator>(const Self_& it) const { return it < *this; }
  bool operator>=(const Self_& it) const { return !(*this < it); }

  bool is_null() const { return !ptr_; }
  bool is_root() const { return !ptr_->parent || !ptr_->parent->black_height; }
  size_t tree_size() const { return ptr_ ? ptr_->size : 0; }
  Self_ parent() const { return ptr_->parent; }
  Self_ left_child() const { return ptr_->left; }
  Self_ right_child() const { return ptr_->right; }
  Self_ first() const { return First_(ptr_); }
  Self_ last() const { return Last_(ptr_); }
  bool is_black() const { return ptr_->black; }
  int black_height() const { return ptr_->black_height; }

  friend class ConstIterator_<T>;
  friend class PreorderIterator_<T>;
  friend class PostorderIterator_<T>;
  template <class, class, class> friend class ::RBTree;
};

template <class T>
inline Iterator_<T> operator+(ptrdiff_t x, Iterator_<T> it) {
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
  reference operator[](difference_type x) const {
    return static_cast<NodeVal_<T>*>(Advance_(ptr_, x))->value;
  }
  bool operator==(const Self_& it) const { return ptr_ == it.ptr_; }
  bool operator!=(const Self_& it) const { return ptr_ != it.ptr_; }
  bool operator<(const Self_& it) const {
    return Difference_(ptr_, it.ptr_) < 0;
  }
  bool operator<=(const Self_& it) const { return !(it < *this); }
  bool operator>(const Self_& it) const { return it < *this; }
  bool operator>=(const Self_& it) const { return !(*this < it); }

  bool is_null() const { return !ptr_; }
  bool is_root() const { return !ptr_->parent || !ptr_->parent->black_height; }
  size_t tree_size() const { return ptr_->size; }
  Self_ parent() const { return ptr_->parent; }
  Self_ left_child() const { return ptr_->left; }
  Self_ right_child() const { return ptr_->right; }
  Self_ first() const { return First_(ptr_); }
  Self_ last() const { return Last_(ptr_); }
  bool is_black() const { return ptr_->black; }
  int black_height() const { return ptr_->black_height; }

  template <class, class, class> friend class ::RBTree;
};

template <class T> class PreorderIterator_ {
  Node_* ptr_;
  typedef PreorderIterator_ Self_;

  PreorderIterator_(Node_* ptr) : ptr_(ptr) {}
  PreorderIterator_(NodeVal_<T>* ptr) : ptr_(static_cast<Node_*>(ptr)) {}
 public:
  // iterator tags
  typedef T value_type;
  typedef T& reference;
  typedef T* pointer;
  typedef std::bidirectional_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;

  PreorderIterator_() : ptr_(nullptr) {}
  PreorderIterator_(const Iterator_<T>& it) : ptr_(it.ptr_) {}
  PreorderIterator_(const PostorderIterator_<T>& it) : ptr_(it.ptr_) {}

  reference operator*() const { return static_cast<NodeVal_<T>*>(ptr_)->value; }
  pointer operator->() const { return &static_cast<NodeVal_<T>*>(ptr_)->value; }
  Self_& operator++() { ptr_ = PreorderNext_(ptr_); return *this; }
  Self_& operator--() { ptr_ = PreorderPrev_(ptr_); return *this; }
  Self_ operator++(int) {
    Self_ tmp = *this;
    ptr_ = PreorderNext_(ptr_);
    return tmp;
  }
  Self_ operator--(int) {
    Self_ tmp = *this;
    ptr_ = PreorderPrev_(ptr_);
    return tmp;
  }
  bool operator==(const Self_& it) const { return ptr_ == it.ptr_; }
  bool operator!=(const Self_& it) const { return ptr_ != it.ptr_; }

  friend class Iterator_<T>;
  friend class PostorderIterator_<T>;
  template <class, class, class> friend class ::RBTree;
};

template <class T> class PostorderIterator_ {
  Node_* ptr_;
  typedef PostorderIterator_ Self_;

  PostorderIterator_(Node_* ptr) : ptr_(ptr) {}
  PostorderIterator_(NodeVal_<T>* ptr) : ptr_(static_cast<Node_*>(ptr)) {}
 public:
  // iterator tags
  typedef T value_type;
  typedef T& reference;
  typedef T* pointer;
  typedef std::bidirectional_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;

  PostorderIterator_() : ptr_(nullptr) {}
  PostorderIterator_(const Iterator_<T>& it) : ptr_(it.ptr_) {}
  PostorderIterator_(const PreorderIterator_<T>& it) : ptr_(it.ptr_) {}

  reference operator*() const { return static_cast<NodeVal_<T>*>(ptr_)->value; }
  pointer operator->() const { return &static_cast<NodeVal_<T>*>(ptr_)->value; }
  Self_& operator++() { ptr_ = PostorderNext_(ptr_); return *this; }
  Self_& operator--() { ptr_ = PostorderPrev_(ptr_); return *this; }
  Self_ operator++(int) {
    Self_ tmp = *this;
    ptr_ = PostorderNext_(ptr_);
    return tmp;
  }
  Self_ operator--(int) {
    Self_ tmp = *this;
    ptr_ = PostorderPrev_(ptr_);
    return tmp;
  }
  bool operator==(const Self_& it) const { return ptr_ == it.ptr_; }
  bool operator!=(const Self_& it) const { return ptr_ != it.ptr_; }

  friend class Iterator_<T>;
  friend class PreorderIterator_<T>;
  template <class, class, class> friend class ::RBTree;
};

template <class T>
inline ConstIterator_<T> operator+(ptrdiff_t x, ConstIterator_<T> it) {
  return it + x;
}

} // namespace RBTreeBase_

#ifdef DEBUG
#include <cstdio>
bool MySwitch = false;
extern int desc;
#endif

struct Nop {
  template <class T> void operator()(T&& a) const {}
};

template <class T> using RBTreeIterator = RBTreeBase_::Iterator_<T>;
template <class T> using RBTreeConstIterator = RBTreeBase_::ConstIterator_<T>;
template <class T> using RBTreePreorderIterator = RBTreeBase_::PreorderIterator_<T>;
template <class T> using RBTreePostorderIterator = RBTreeBase_::PostorderIterator_<T>;

template <class T, class PullFunc = Nop, class PushFunc = Nop> class RBTree {
 protected:
  typedef RBTreeBase_::Node_ NodeBase_;
  typedef RBTreeBase_::NodeVal_<T> NodeType_;

#ifdef DEBUG
  void Print_(NodeType_* a) const {
    if (!a) {
      printf("x ");
      return;
    }
    /*printf("%lld,%lld,%lld,sz%d,bh%d,%c ", a->value.a, a->value.sum, a->value.tag,
           (int)a->size, (int)a->black_height, a->black ? 'B' : 'R');*/
    printf("%lld,s%lld,tag%lld ", a->value.a, a->value.sum, a->value.tag);
    if (a->left || a->right) {
      printf("(");
      Print_((NodeType_*)a->left);
      Print_((NodeType_*)a->right);
      printf(") ");
    }
  }

  bool CheckRB_(NodeBase_* a) const {
    if (!a) return true;
    uint8_t z = a->black_height - a->black;
    auto H = [](NodeBase_* a){ return a ? a->black_height : 1; };
    if (H(a->left) != z || H(a->right) != z || (!a->black &&
         ((a->left && !a->left->black) || (a->right && !a->right->black))) ||
        (a->left && a->left->parent != a) ||
        (a->right && a->right->parent != a) ||
        a->size != Size_(a->left) + Size_(a->right) + 1) return false;
    if (Sup_(a)->value.sum !=
        (a->left ? Sup_(a->left)->value.sum + Sup_(a->left)->value.tag * (long)a->left->size : 0) +
        (a->right ? Sup_(a->right)->value.sum + Sup_(a->right)->value.tag * (long)a->right->size : 0) +
        Sup_(a)->value.a) return false;
    return CheckRB_(a->left) && CheckRB_(a->right);
  }
  bool CheckRB_() const {
    return !head_->left || (head_->left->black && CheckRB_(head_->left));
  }
#endif

  void Pull_(NodeBase_* nd) {
    if (!std::is_same<PullFunc, Nop>::value) {
      pull_func_(iterator(nd));
    }
  }
  void Push_(NodeBase_* nd) {
    if (!std::is_same<PushFunc, Nop>::value) {
      push_func_(iterator(nd));
    }
  }

  NodeType_* Sup_(NodeBase_* nd) const {
    return static_cast<NodeType_*>(nd);
  }

  void PaintBlack_(NodeBase_* nd) const {
    if (nd) nd->black_height += !nd->black, nd->black = true;
  }
  void PullSize_(NodeBase_* nd) {
    nd->size = Size_(nd->left) + Size_(nd->right) + 1;
    Pull_(nd);
  }
  void PullSizeNoCheck_(NodeBase_* nd) {
    nd->size = nd->left->size + nd->right->size + 1;
    Pull_(nd);
  }
  NodeBase_* IncreaseSize_(NodeBase_* nd, NodeBase_* head, size_t sz = 1) {
    while (true) {
      nd->size += sz; Pull_(nd);
      if (nd->parent == head) return nd;
      nd = nd->parent;
    }
  }
  void DecreaseSize_(NodeBase_* nd) {
    for (; nd != head_; nd = nd->parent) nd->size--, Pull_(nd);
  }
  void PullFrom_(NodeBase_* nd) {
    if (!std::is_same<PullFunc, Nop>::value) {
      for (nd = nd->parent; nd != head_; nd = nd->parent) Pull_(nd);
    }
  }
  void PushTo_(NodeBase_* nd, NodeBase_* head) {
    if (!std::is_same<PushFunc, Nop>::value) {
      unsigned __int128 dir = 0; // bitmap to locate the node
      int sz = 0;
      for (; nd->parent != head; nd = nd->parent, sz++) {
        dir = nd->parent->right == nd ? dir << 1 | 1 : dir << 1;
      }
      Push_(nd);
      while (sz--) {
        nd = dir & 1 ? nd->right : nd->left;
        Push_(nd);
        dir >>= 1;
      }
    }
  }

  NodeBase_* InsertRepair_(NodeBase_* nd, NodeBase_* head, size_t sz = 1) {
    Pull_(nd);
    while (true) {
      NodeBase_* p = nd->parent;
      if (p == head) { // Case 1
        nd->black = true;
        nd->black_height++;
        return nd;
      }
      if (p->black) { // Case 2
        return IncreaseSize_(p, head, sz);
      }
      NodeBase_* g = p->parent;
      NodeBase_* u = g->left == p ? g->right : g->left;
      if (!u || u->black) { // Case 4
        if (p == g->left) {
          if (nd == p->right) {
            std::swap(nd, p);
            ConnectRight_(nd, p->left);
            ConnectLeftNoCheck_(p, nd);
            PullSize_(nd);
          }
          ConnectParent_(g, p);
          ConnectLeft_(g, p->right);
          ConnectRightNoCheck_(p, g);
        } else {
          if (nd == p->left) {
            std::swap(nd, p);
            ConnectLeft_(nd, p->right);
            ConnectRightNoCheck_(p, nd);
            PullSize_(nd);
          }
          ConnectParent_(g, p);
          ConnectRight_(g, p->left);
          ConnectLeftNoCheck_(p, g);
        }
        PullSize_(g);
        g->black = false; g->black_height--;
        PullSizeNoCheck_(p);
        p->black = true; p->black_height++;
        if (p->parent == head) return p;
        return IncreaseSize_(p->parent, head, sz);
      }
      // Case 3
      p->size += sz; p->black = true; p->black_height++; Pull_(p);
      g->size += sz; g->black = false; Pull_(g);
      u->black = true; u->black_height++;
      nd = g;
    }
  }

  void RemoveRepair_(NodeBase_* p, NodeBase_* s) {
    if (p == head_) return;
    NodeBase_* nd = nullptr;
    while (true) {
      if (!s->black) { // Case 2
        Push_(s);
        p->black = false; p->black_height--;
        s->black = true; s->black_height++;
        ConnectParentNoCheck_(p, s);
        if (p->left == s) {
          ConnectLeft_(p, s->right);
          ConnectRightNoCheck_(s, p);
          PullSize_(p); PullSize_(s);
          p->size++, s->size++;
          s = p->left;
        } else {
          ConnectRight_(p, s->left);
          ConnectLeftNoCheck_(s, p);
          PullSize_(p); PullSize_(s);
          p->size++, s->size++;
          s = p->right;
        }
        break;
      }
      if (p->black && (!s->left || s->left->black) &&
          (!s->right || s->right->black)) { // Case 3
        s->black = false; s->black_height--;
        p->size--; p->black_height--; Pull_(p);
        nd = p; p = nd->parent;
        if (p == head_) return; // Case 1
        s = p->left == nd ? p->right : p->left;
        continue;
      }
      break;
    }
    // s is black here
    NodeBase_* sin = p->left == s ? s->right : s->left;
    NodeBase_* sout = p->left == s ? s->left : s->right;
    if (sout && !sout->black) { // Case 6
      Push_(s);
      sout->black = true; sout->black_height++;
      s->black_height += p->black;
      p->black_height -= p->black;
      s->black = p->black; p->black = true;
      ConnectParentNoCheck_(p, s);
      if (p->left == s) {
        ConnectLeft_(p, s->right);
        ConnectRightNoCheck_(s, p);
      } else {
        ConnectRight_(p, s->left);
        ConnectLeftNoCheck_(s, p);
      }
      PullSize_(p); PullSizeNoCheck_(s);
      DecreaseSize_(s->parent);
    } else if (sin && !sin->black) { // Case 5
      Push_(s); Push_(sin);
      p->black_height -= p->black;
      sin->black_height += 1 + p->black;
      sin->black = p->black;
      p->black = true;
      ConnectParentNoCheck_(p, sin);
      if (p->left == s) {
        ConnectRight_(s, sin->left);
        ConnectLeft_(p, sin->right);
        ConnectRightNoCheck_(sin, p);
        ConnectLeftNoCheck_(sin, s);
      } else {
        ConnectLeft_(s, sin->right);
        ConnectRight_(p, sin->left);
        ConnectLeftNoCheck_(sin, p);
        ConnectRightNoCheck_(sin, s);
      }
      PullSize_(p); PullSize_(s); PullSizeNoCheck_(sin);
      DecreaseSize_(sin->parent);
    } else { // Case 4, p is red here (or it will be Case 3)
      s->black = false; s->black_height--;
      p->black = true;
      DecreaseSize_(p);
    }
  }

  void InsertBefore_(NodeBase_* a, NodeBase_* b) {
    if (a != head_) {
      if (!a->left) {
        if (a == head_->parent) head_->parent = b;
        PushTo_(a, head_);
        ConnectLeftNoCheck_(a, b);
      } else {
        a = Last_(a->left);
        PushTo_(a, head_);
        ConnectRightNoCheck_(a, b);
      }
    } else if (!head_->left) {
      head_->parent = b;
      ConnectLeftNoCheck_(head_, b);
    } else {
      a = Last_(head_->left);
      PushTo_(a, head_);
      ConnectRightNoCheck_(a, b);
    }
    InsertRepair_(b, head_);
  }
  NodeBase_* Remove_(NodeBase_* a) {
    if (a->left && a->right) {
      NodeBase_* tmp = First_(a->right); // begin won't be affected
      PushTo_(tmp, head_);
      using std::swap; swap(Sup_(tmp)->value, Sup_(a)->value);
      a = tmp;
    } else {
      PushTo_(a, head_);
      if (a == head_->parent) {
        head_->parent = a->right ? First_(a->right) : a->parent;
      }
    }
    if (!a->black) { // no child
      (a->parent->left == a ? a->parent->left : a->parent->right) = nullptr;
      DecreaseSize_(a->parent);
    } else {
      NodeBase_* child = a->left ? a->left : a->right;
      if (child) { // child must be red
        child->black = true; child->black_height++;
        ConnectParent_(a, child);
        DecreaseSize_(child->parent);
      } else if (a->parent->left == a) { // no child
        a->parent->left = nullptr;
        RemoveRepair_(a->parent, a->parent->right);
      } else {
        a->parent->right = nullptr;
        RemoveRepair_(a->parent, a->parent->left);
      }
    }
    return a;
  }
  NodeBase_* Merge_(NodeBase_* l, NodeBase_* m, NodeBase_* r) {
    if (!l) {
      m->left = m->right = nullptr; m->size = 1;
      if (!r) {
        m->black = true; m->black_height = 2; Pull_(m);
        return m;
      }
      m->black = false; m->black_height = 1;
      l = First_(r); PushTo_(l, r->parent);
      ConnectLeftNoCheck_(l, m); Pull_(m);
      return InsertRepair_(m, r->parent = nullptr);
    }
    if (!r) {
      m->left = m->right = nullptr; m->size = 1;
      m->black = false; m->black_height = 1;
      r = Last_(l); PushTo_(r, l->parent);
      ConnectRightNoCheck_(r, m); Pull_(m);
      return InsertRepair_(m, l->parent = nullptr);
    }
    if (l->black_height == r->black_height){
      ConnectLeftNoCheck_(m, l);
      ConnectRightNoCheck_(m, r);
      m->black = true; m->black_height = l->black_height + 1;
      PullSizeNoCheck_(m);
      return m;
    }
    if (l->black_height < r->black_height) {
      NodeBase_* ret = r;
      for (; !r->black || l->black_height != r->black_height; r = r->left) {
        Push_(r);
      }
      ConnectParentNoCheck_(r, m);
      ConnectLeftNoCheck_(m, l);
      ConnectRightNoCheck_(m, r);
      m->black = false; m->black_height = l->black_height;
      PullSizeNoCheck_(m);
      ret = InsertRepair_(m, ret->parent = nullptr, l->size + 1);
      return ret;
    } else {
      NodeBase_* ret = l;
      for (; !l->black || l->black_height != r->black_height; l = l->right) {
        Push_(l);
      }
      ConnectParentNoCheck_(l, m);
      ConnectLeftNoCheck_(m, l);
      ConnectRightNoCheck_(m, r);
      m->black = false; m->black_height = l->black_height;
      PullSizeNoCheck_(m);
      ret = InsertRepair_(m, ret->parent = nullptr, r->size + 1);
      return ret;
    }
  }
  void Split_(NodeBase_* nd, NodeBase_*& left, NodeBase_*& right, bool pivot) {
    PushTo_(nd, head_);
    NodeBase_* p = nd->parent;
    left = nd->left; right = nd->right;
    PaintBlack_(left); PaintBlack_(right);
    if (pivot) right = Merge_(nullptr, nd, right);
    while (p != head_) {
      bool is_left = p->left == nd;
      nd = p;
      p = p->parent;
      if (is_left) {
        PaintBlack_(nd->right);
        right = Merge_(right, nd, nd->right);
      } else {
        PaintBlack_(nd->left);
        left = Merge_(nd->left, nd, left);
      }
    }
  }

  void InsertMerge_(NodeBase_* nd, NodeBase_* head2) {
    if (!head_->left) head_->parent = nd;
    ConnectLeft_(head_, Merge_(head_->left, nd, head2->left));
    head2->left = nullptr; head2->parent = head_;
  }

  template <class Pred> NodeBase_* PartitionBound_(Pred&& func) {
    // first element x that func(x) is false, assuming monotonicity
    NodeBase_ *now = head_->left, *last = head_;
    while (now) {
      Push_(now);
      const T& val = Sup_(now)->value; // just add constness
      if (func(val)) {
        now = now->right;
      } else {
        last = now;
        now = now->left;
      }
    }
    return last;
  }
  template <class Pred> NodeBase_* PartitionBoundIter_(Pred&& func) {
    // same as PartitionBound, but const_iterator is passed to func
    NodeBase_ *now = head_->left, *last = head_;
    while (now) {
      Push_(now);
      if (func(const_iterator(now))) {
        now = now->right;
      } else {
        last = now;
        now = now->left;
      }
    }
    return last;
  }

  NodeType_* GenNode_(const T& val) const {
    NodeType_* ptr = static_cast<NodeType_*>(malloc(sizeof(NodeType_)));
    new(ptr) NodeType_(val);
    return ptr;
  }
  NodeType_* GenNode_(T&& val) const {
    NodeType_* ptr = static_cast<NodeType_*>(malloc(sizeof(NodeType_)));
    new(ptr) NodeType_(std::move(val));
    return ptr;
  }
  NodeType_* GenNode_(NodeType_* x, NodeBase_* p) const {
    NodeType_* ptr = static_cast<NodeType_*>(malloc(sizeof(NodeType_)));
    new(ptr) NodeType_(x, p);
    return ptr;
  }
  template <class... Args> NodeType_* GenNodeArgs_(Args&&... args) const {
    NodeType_* ptr = static_cast<NodeType_*>(malloc(sizeof(NodeType_)));
    new(ptr) NodeType_(args...);
    return ptr;
  }
  void FreeNode_(NodeBase_* nd) const {
    Sup_(nd)->value.~T();
    free(nd);
  }

  void ClearTree_(NodeBase_* nd) {
    NodeBase_* now = nd;
    while (true) {
      NodeBase_* tmp = now;
      if (now->left) {
        now = now->left;
        tmp->left = nullptr;
      } else if (now->right) {
        now = now->right;
        tmp->right = nullptr;
      } else if (now == nd) {
        break;
      } else {
        now = now->parent;
        FreeNode_(tmp);
      }
    }
  }
  void ClearTree_() { ClearTree_(head_); head_->parent = head_; }

  void CopyTree_(NodeBase_* dest, NodeBase_* orig) {
    NodeBase_* now = orig;
    while (true) {
      if (!dest->left && now->left) {
        dest->left = GenNode_(Sup_(now->left), dest);
        dest = dest->left;
        now = now->left;
      } else if (!dest->right && now->right) {
        dest->right = GenNode_(Sup_(now->right), dest);
        dest = dest->right;
        now = now->right;
      } else if (now == orig) {
        break;
      } else {
        dest = dest->parent;
        now = now->parent;
      }
    }
  }

  void Init_() {
    head_ = static_cast<NodeBase_*>(malloc(sizeof(NodeBase_)));
    new(head_) NodeBase_();
    head_->parent = head_; head_->black_height = 0;
  }

  NodeBase_* head_;
  PullFunc pull_func_;
  PushFunc push_func_;
 public:
  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef RBTreeIterator<T> iterator;
  typedef RBTreeConstIterator<T> const_iterator;
  typedef RBTreePreorderIterator<T> preorder_iterator;
  typedef RBTreePostorderIterator<T> postorder_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  RBTree() { Init_(); }
  RBTree(const RBTree& tree) {
    Init_();
    CopyTree_(head_, tree.head_);
  }
  RBTree(RBTree&& tree) {
    Init_();
    std::swap(head_, tree.head_);
  }
  ~RBTree() {
    ClearTree_();
    free(head_);
  }

  RBTree& operator=(const RBTree& tree) {
    ClearTree_();
    CopyTree_(head_, tree.head_);
    return *this;
  }
  RBTree& operator=(RBTree&& tree) {
    std::swap(head_, tree.head_);
    return *this;
  }

  iterator begin() { return head_->parent; }
  const_iterator begin() const { return head_->parent; }
  const_iterator cbegin() const { return begin(); }
  iterator end() { return head_; }
  const_iterator end() const { return head_; }
  const_iterator cend() const { return end(); }

  preorder_iterator pre_begin() { return head_->left; }
  preorder_iterator pre_end() { return head_; }
  postorder_iterator post_begin() { return PostorderFirst_(head_->parent); }
  postorder_iterator post_end() { return head_; }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crbegin() const { return rbegin(); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crend() const { return rend(); }

  iterator root() { return head_->left; }
  const_iterator root() const { return head_->left; }

#ifdef DEBUG
  void print_() const { Print_(Sup_(head_->left)); puts(""); }
  bool check_() const { return CheckRB_(); }
#endif

  bool empty() const { return !head_->left; }
  size_type size() const { return Size_(head_->left); }

  reference operator[](size_type x) {
    return Sup_(Select_(head_->left, x))->value;
  }
  const_reference operator[](size_type x) const {
    return Sup_(Select_(head_->left, x))->value;
  }
  reference at(size_type x) {
    NodeBase_* nd = head_->left;
    while (true) {
      Push_(nd);
      if (Size_(nd->left) == x) break;
      if (Size_(nd->left) > x) {
        nd = nd->left;
      } else {
        x -= Size_(nd->left) + 1;
        nd = nd->right;
      }
    }
    return Sup_(nd)->value;
  }
  reference front() { return Sup_(head_->parent)->value; }
  const_reference front() const { return Sup_(head_->parent)->value; }
  reference back() { return Sup_(Last_(head_->left))->value; }
  const_reference back() const { return Sup_(Last_(head_->left))->value; }
  template <class Pred> iterator partition_bound(Pred&& func) {
    return PartitionBound_(func);
  }
  template <class Pred> iterator iter_partition_bound(Pred&& func) {
    return PartitionBoundIter_(func);
  }

  void push_back(const T& val) {
    InsertBefore_(head_, GenNode_(val));
  }
  void push_back(T&& val) {
    InsertBefore_(head_, GenNode_(std::move(val)));
  }
  void push_front(const T& val) {
    InsertBefore_(First_(head_), GenNode_(val));
  }
  void push_front(T&& val) {
    InsertBefore_(First_(head_), GenNode_(std::move(val)));
  }
  void pop_back() { FreeNode_(Remove_(Last_(head_->left))); }
  void pop_front() { FreeNode_(Remove_(First_(head_->left))); }
  template <class... Args> void emplace_back(Args&&... args) {
    InsertBefore_(head_, GenNodeArgs_(args...));
  }
  template <class... Args> void emplace_front(Args&&... args) {
    InsertBefore_(First_(head_), GenNodeArgs_(args...));
  }
  iterator insert(iterator it, const T& val) {
    NodeType_* ptr = GenNode_(val);
    InsertBefore_(it.ptr_, ptr);
    return ptr;
  }
  iterator insert(iterator it, T&& val) {
    NodeType_* ptr = GenNode_(std::move(val));
    InsertBefore_(it.ptr_, ptr);
    return ptr;
  }
  template <class... Args> iterator emplace(iterator it, Args&&... args) {
    NodeType_* ptr = GenNodeArgs_(args...);
    InsertBefore_(it.ptr_, ptr);
    return ptr;
  }
  void erase(iterator it) { FreeNode_(Remove_(it.ptr_)); }
  void clear() { ClearTree_(); }

  void swap(RBTree& x) { std::swap(head_, x.head_); }
  void insert_merge(RBTree& tree, const T& val) {
    InsertMerge_(GenNode_(val), tree.head_);
  }
  void insert_merge(RBTree& tree, T&& val) {
    InsertMerge_(GenNode_(std::move(val)), tree.head_);
  }
  template <class... Args> void emplace_merge(RBTree& tree, Args&&... args) {
    InsertMerge_(GenNodeArgs_(args...), tree.head_);
  }
  void merge(RBTree& tree) {
    if (tree.empty()) return;
    if (empty()) { std::swap(head_, tree.head_); return; }
    NodeBase_* pivot = (tree.head_->left->size < head_->left->size) ?
        tree.Remove_(First_(tree.head_->left)) : Remove_(Last_(head_->left));
    InsertMerge_(pivot, tree.head_);
  }
  void erase_split(iterator it, RBTree& tree) {
    NodeBase_ *l, *r = Next_(it.ptr_);
    tree.ClearTree_();
    if (it.ptr_ == head_->parent) head_->parent = head_;
    if (r != head_) tree.head_->parent = r;
    Split_(it.ptr_, l, r, false);
    FreeNode_(it.ptr_);
    ConnectLeft_(head_, l);
    ConnectLeft_(tree.head_, r);
  }
  void split(iterator it, RBTree& tree) {
    tree.ClearTree_();
    if (it.ptr_ == head_) return;
    NodeBase_ *l, *r;
    if (it.ptr_ == head_->parent) head_->parent = head_;
    tree.head_->parent = it.ptr_;
    Split_(it.ptr_, l, r, true);
    ConnectLeft_(head_, l);
    ConnectLeft_(tree.head_, r);
  }

  void pull_node(iterator it) { Pull_(it.ptr_); }
  void push_node(iterator it) { Push_(it.ptr_); }
  void pull_from(iterator it) { PullFrom_(it.ptr_); }
  void push_to(iterator it) { PushTo_(it.ptr_, head_); }

  PullFunc& get_pull_object() { return pull_func_; }
  PushFunc& get_push_object() { return push_func_; }
  const PullFunc& get_pull_object() const { return pull_func_; }
  const PushFunc& get_push_object() const { return push_func_; }
};

template <class T, class U> void swap(RBTree<T, U>& a, RBTree<T, U>& b) {
  a.swap(b);
}

#endif
