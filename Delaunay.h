#ifndef DELAUNAY_H_
#define DELAUNAY_H_

#include <cstdlib>
#include <Array.h>
#include <Point.h>

template <class U, class T>
bool InsideCircum(const Point2D<T>& a, const Point2D<T>& b,
                  const Point2D<T>& c, const Point2D<T>& d) {
  T x1 = a.x - d.x, y1 = a.y - d.y; U z1 = (U)x1 * x1 + (U)y1 * y1;
  T x2 = b.x - d.x, y2 = b.y - d.y; U z2 = (U)x2 * x2 + (U)y2 * y2;
  T x3 = c.x - d.x, y3 = c.y - d.y; U z3 = (U)x3 * x3 + (U)y3 * y3;
  return x1 * (z3 * y2 - z2 * y3) + x2 * (z1 * y3 - z3 * y1) +
         x3 * (z2 * y1 - z1 * y2) > 0;
}

template <class T, class U>
class IncrementalDelaunay {
  typedef Point2D<T> Point;
  typedef int Index;
  Array<Point> pts_;
 public:
  struct Triangle {
    Index v[3];
  };
 private:
  const Point dir_[3] = {Point(1, 0), Point(0, 1), Point(-1, -1)};
  int A1_(int x) { return 1 << x & 3; } // (x+1)%3
  int A2_(int x) { return 3 >> x ^ 1; } // (x+2)%3
  bool InsideCircum_(const Triangle& x, Index d) {
    auto Pt = [&](int y)->const Point&{ return pts_[x.v[y]]; };
    if (x.v[0] < 0 || x.v[1] < 0 || x.v[2] < 0) {
      int ed = -1;
      for (int i : {0, 1, 2}) {
        if (x.v[i] < 0) ed = i;
      }
      if (d < 0) {
        return Cross(Pt(A1_(ed)), Pt(A2_(ed)), Pt(A1_(ed)) + dir_[~d]) > 0;
      }
      return Cross(Pt(A1_(ed)), Pt(A2_(ed)), pts_[d]) > 0;
    } else if (d < 0) {
      return false;
    }
    return InsideCircum<U>(pts_[x.v[0]], pts_[x.v[1]], pts_[x.v[2]], pts_[d]);
  }
  int InsideTriangle_(const Triangle& x, const Point& d) {
    int ret = 0;
    auto Pt = [&](int y)->const Point&{ return pts_[x.v[y]]; };
    if (x.v[0] >= 0 && x.v[1] >= 0 && x.v[2] >= 0) {
      T t1 = Cross(Pt(2), d, Pt(1));
      T t2 = Cross(Pt(0), d, Pt(2));
      T t3 = Cross(Pt(1), d, Pt(0));
      if (t1 < 0 || t2 < 0 || t3 < 0) return 0;
      if (t1 > 0) ret |= 1;
      if (t2 > 0) ret |= 2;
      if (t3 > 0) ret |= 4;
    } else {
      int q = 0, ed1 = 0, ed2 = 0;
      for (int i : {0, 1, 2}) {
        if (x.v[i] < 0) q++;
        (x.v[i] < 0 ? ed1 : ed2) = i;
      }
      if (q == 3) return 7;
      if (q == 2) {
        T t1 = Cross(Pt(ed2), d, Pt(ed2) + dir_[~x.v[A2_(ed2)]]);
        T t2 = Cross(Pt(ed2), Pt(ed2) + dir_[~x.v[A1_(ed2)]], d);
        if (t1 < 0 || t2 < 0) return 0;
        ret |= 1 << ed2;
        if (t1 > 0) ret |= 1 << (A1_(ed2));
        if (t2 > 0) ret |= 1 << (A2_(ed2));
      } else {
        T t1 = Cross(Pt(A2_(ed1)), d, Pt(A1_(ed1)));
        T t2 = Cross(Pt(A2_(ed1)), Pt(A2_(ed1)) + dir_[~x.v[ed1]], d);
        T t3 = Cross(Pt(A1_(ed1)), d, Pt(A1_(ed1)) + dir_[~x.v[ed1]]);
        if (t1 < 0 || t2 < 0 || t3 < 0) return 0;
        if (t1 > 0) ret |= 1 << ed1;
        if (t2 > 0) ret |= 1 << (A1_(ed1));
        if (t3 > 0) ret |= 1 << (A2_(ed1));
      }
    }
    return ret;
  }
  struct Tree_ {
    Triangle tri;
    struct Adj {
      Tree_ *p; int ed;
      Adj() : p(nullptr), ed(0) {}
      Adj(Tree_* p, int ed) : p(p), ed(ed) {}
    } adj[3];
    Tree_ *ch[3], *prev, *next;
    bool flag;
    Tree_(Index a, Index b, Index c, bool f) : tri{a, b, c}, adj(),
        ch{nullptr, nullptr, nullptr}, prev(nullptr), next(nullptr), flag(f) {}
  } *root_, *head_;
  Tree_* NewNode_(Index a, Index b, Index c, bool f = false) {
    Tree_* ret = (Tree_*)malloc(sizeof(Tree_));
    new(ret) Tree_(a, b, c, f);
    return ret;
  }
  void Push_(Tree_* nd) {
    nd->next = head_;
    head_->prev = nd;
    head_ = nd;
  }
  void Erase_(Tree_* nd) {
    if (nd->prev) nd->prev->next = nd->next;
    if (nd->next) nd->next->prev = nd->prev;
    nd->next = nd->prev = nullptr;
  }
  void ConnectAdj_(Tree_* a, int ae, Tree_* b, int be) {
    a->adj[ae] = {b, be};
    b->adj[be] = {a, ae};
  }
  void ReplaceAdj_(Tree_* a, int ae, Tree_* b, int be) {
    auto& aj = a->adj[ae];
    b->adj[be] = aj;
    if (aj.p) aj.p->adj[aj.ed] = {b, be};
  }
  void CheckFlip_(Tree_* nd, int ed) {
    if (!nd->adj[ed].p) return;
    Tree_* adj = nd->adj[ed].p;
    int aed = nd->adj[ed].ed;
    if (InsideCircum_(nd->tri, adj->tri.v[aed])) {
      Index v1 = nd->tri.v[A1_(ed)];
      Index v2 = nd->tri.v[A2_(ed)];
      adj->ch[0] = nd->ch[0] = NewNode_(nd->tri.v[ed], adj->tri.v[aed], v2, true);
      adj->ch[1] = nd->ch[1] = NewNode_(nd->tri.v[ed], v1, adj->tri.v[aed], true);
      Push_(nd->ch[0]); Push_(nd->ch[1]);
      ReplaceAdj_(adj, A2_(aed), nd->ch[0], 0);
      ReplaceAdj_(nd,  A1_(ed),  nd->ch[0], 1);
      ReplaceAdj_(adj, A1_(aed), nd->ch[1], 0);
      ReplaceAdj_(nd,  A2_(ed),  nd->ch[1], 2);
      nd->ch[0]->adj[2] = {nd->ch[1], 1};
      nd->ch[1]->adj[1] = {nd->ch[0], 2};
      Erase_(nd); Erase_(adj);
      CheckFlip_(nd->ch[0], 0);
      CheckFlip_(nd->ch[1], 0);
    }
  }
  bool FindInsert_(Tree_* nd, Index x, int in) {
    const auto& pt = pts_[x];
    if (!nd->ch[0]) {
      if (in == 7) {
        nd->ch[0] = NewNode_(x, nd->tri.v[1], nd->tri.v[2]);
        nd->ch[1] = NewNode_(nd->tri.v[0], x, nd->tri.v[2]);
        nd->ch[2] = NewNode_(nd->tri.v[0], nd->tri.v[1], x);
        for (int i : {0, 1, 2}) {
          ReplaceAdj_(nd, i, nd->ch[i], i);
          ConnectAdj_(nd->ch[i], A1_(i), nd->ch[A1_(i)], i);
          Push_(nd->ch[i]);
        }
        Erase_(nd);
        for (int i : {0, 1, 2}) CheckFlip_(nd->ch[i], i);
      } else {
        int ed = in >> 1 ^ 3;
        Tree_* adj = nd->adj[ed].p;
        int aed = nd->adj[ed].ed;
        Index v1 = nd->tri.v[A1_(ed)];
        Index v2 = nd->tri.v[A2_(ed)];
        nd->ch[0] = NewNode_(nd->tri.v[ed], x, v2);
        nd->ch[1] = NewNode_(nd->tri.v[ed], v1, x);
        adj->ch[0] = NewNode_(adj->tri.v[aed], x, v1);
        adj->ch[1] = NewNode_(adj->tri.v[aed], v2, x);
        ConnectAdj_(nd->ch[0], 0, adj->ch[1], 0);
        ConnectAdj_(nd->ch[1], 0, adj->ch[0], 0);
        ReplaceAdj_(nd, A1_(ed), nd->ch[0], 1);
        ReplaceAdj_(nd, A2_(ed), nd->ch[1], 2);
        ReplaceAdj_(adj, A1_(aed), adj->ch[0], 1);
        ReplaceAdj_(adj, A2_(aed), adj->ch[1], 2);
        for (Tree_* i : {nd, adj}) {
          ConnectAdj_(i->ch[0], 2, i->ch[1], 1);
          Push_(i->ch[0]); Push_(i->ch[1]);
          Erase_(i);
        }
        CheckFlip_(nd->ch[0], 1);
        CheckFlip_(nd->ch[1], 2);
        CheckFlip_(adj->ch[0], 1);
        CheckFlip_(adj->ch[1], 2);
      }
      return true;
    }
    for (int i = 0; i < 3 && nd->ch[i]; i++) {
      int nin = InsideTriangle_(nd->ch[i]->tri, pt);
      if (!nin) continue;
      if ((nin & -nin) == nin) return false;
      return FindInsert_(nd->ch[i], x, nin);
    }
    __builtin_unreachable();
  }
  bool Insert_(const Point& pt) {
    pts_.push_back(pt);
    if (!FindInsert_(root_, pts_.size() - 1, 7)) {
      pts_.pop_back();
      return false;
    }
    return true;
  }
  void DestructTree_(Tree_* nd) {
    if (nd->flag) {
      nd->flag = false; return;
    }
    for (int i : {0, 1, 2}) {
      if (nd->ch[i]) DestructTree_(nd->ch[i]);
    }
    free(nd);
  }
  void DestructTree_() {
    if (root_) DestructTree_(root_), head_ = root_ = nullptr;
  }
 public:
  class TriangleIter : std::iterator<std::forward_iterator_tag, Triangle,
      ptrdiff_t, const Triangle*, const Triangle&> {
    const Tree_* ptr_;
    TriangleIter(const Tree_* ptr) : ptr_(ptr) {}
   public:
    TriangleIter() {}
    bool operator==(const TriangleIter& x) const { return ptr_ == x.ptr_; }
    bool operator!=(const TriangleIter& x) const { return ptr_ != x.ptr_; }
    TriangleIter& operator++() { ptr_ = ptr_->next; return *this; }
    TriangleIter operator++(int) {
      Tree_* tmp = ptr_;
      ptr_ = ptr_->next;
      return tmp;
    }
    const Triangle& operator*() const { return ptr_->tri; }
    const Triangle* operator->() const { return &ptr_->tri; }
    TriangleIter Neighbor(int x) const { return TriangleIter(ptr_->adj[x].p); }
    TriangleIter Neighbor(int x, int& p) const {
      p = ptr_->adj[x].ed;
      return TriangleIter(ptr_->adj[x].p);
    }
    friend class IncrementalDelaunay;
  };
  IncrementalDelaunay() : pts_(), root_(NewNode_(-1, -2, -3)), head_(root_) {}
  ~IncrementalDelaunay() { DestructTree_(); }
  void Clear() {
    DestructTree_();
    head_ = root_ = NewNode_(-1, -2, -3);
  }
  void Reserve(size_t x) { pts_.reserve(x); }
  bool Insert(const Point& x) { return Insert_(x); }
  const Array<Point>& GetPoints() const { return pts_; }
  TriangleIter TriBegin() const { return head_; }
  TriangleIter TriEnd() const { return nullptr; }
};

#endif
