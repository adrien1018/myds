#ifndef UNORDERED_H_
#define UNORDERED_H_

#include <cstring>
#include <iterator>
#include <algorithm>

struct DefaultClassifier {
  size_t operator()(size_t a, size_t N) const { return a & (N - 1); }
};
template <class T> struct Self {
  T operator()(const T& a) const { return a; }
};
template<typename T> struct Identity { typedef T type; };

template <class T, class Hash, class Pred, class Cls = DefaultClassifier, class GetKey = Self<T>>
class UnorderedBase {
public:
  struct Node {
    Node() {}
    Node(const T& val, Node* nxt = nullptr) : val(val), nxt(nxt) {}
    Node(T&& val, Node* nxt = nullptr) : val(val), nxt(nxt) {}
    Node(const Node& a) : val(a.val), nxt(a.nxt) {}
    Node(Node&& a) : val(std::move(a.val)), nxt(a.nxt) {}
    T val;
    Node* nxt;
  };
  class Iter {
    Node **bucket, **end;
    Node* node;
    Iter(Node** a, Node** b, Node* c) : bucket(a), end(b), node(c) {}

    void Next() {
      if (bucket == end) return;
      node = node->nxt;
      if (node) return;
      do {
        bucket++;
        if (*bucket) {
          node = *bucket;
          return;
        }
      } while (bucket != end);
    }
  public:
    Iter() : bucket(nullptr), end(nullptr), node(nullptr) {}

    const Iter& operator++() { Next(); return *this; }
    Iter operator++(int) { Iter prv(*this); Next(); return prv; }
    bool operator==(const Iter& a) const { return node == a.node; }
    bool operator!=(const Iter& a) const { return node != a.node; }
    T& operator*() { return node->val; }
    T* operator->() { return &(node->val); }

    friend class ConstIter;
    friend class UnorderedBase;
  };
  struct ConstIter {
    Node* const* bucket;
    Node* const* end;
    const Node* node;
    ConstIter(const Node** a, const Node** b, const Node* c) : bucket(a), end(b), node(c) {}

    void Next() {
      if (bucket == end) return;
      node = node->nxt;
      if (node) return;
      do {
        bucket++;
        if (*bucket) {
          node = *bucket;
          return;
        }
      } while (bucket != end);
    }
  public:
    ConstIter() : bucket(nullptr), end(nullptr), node(nullptr) {}
    ConstIter(const Iter& a) : bucket(a.bucket), end(a.end), node(a.node) {}

    const ConstIter& operator++() { Next(); return *this; }
    ConstIter operator++(int) { ConstIter prv(*this); Next(); return prv; }
    bool operator==(const ConstIter& a) const { return node == a.node; }
    bool operator!=(const ConstIter& a) const { return node != a.node; }
    const T& operator*() const { return node->val; }
    const T* operator->() const { return &(node->val); }

    friend class UnorderedBase;
  };
private:
  Node** buckets_;
  Node** buckets_end_;
  size_t size_;
  Hash hasher_;
  Pred pred_;
  Cls classifier_;
  GetKey keyget_;

  template<class U> Node** GetBucket(const U& val, Identity<U>) {
    return buckets_ + classifier_(hasher_(val), buckets_end_ - buckets_);
  }
  template<class U> const Node** GetBucket(const U& val, Identity<U>) const {
    return buckets_ + classifier_(hasher_(val), buckets_end_ - buckets_);
  }
  template<class U> Node* FindBucket(Node* nd, const U& val, Identity<U>) const {
    for (; nd; nd = nd->nxt) {
      if (pred_(keyget_(nd->val), val)) return nd;
    }
    return nullptr;
  }
  Node** GetBucket(const T& val, Identity<T>) {
    return buckets_ + classifier_(hasher_(keyget_(val)), buckets_end_ - buckets_);
  }
  const Node** GetBucket(const T& val, Identity<T>) const {
    return buckets_ + classifier_(hasher_(keyget_(val)), buckets_end_ - buckets_);
  }
  Node* FindBucket(Node* nd, const T& val, Identity<T>) const {
    for (; nd; nd = nd->nxt) {
      if (pred_(keyget_(nd->val), keyget_(val))) return nd;
    }
    return nullptr;
  }

  template <class U> Node** GetBucket(const U& val) { return GetBucket(val, Identity<U>()); }
  template <class U> const Node** GetBucket(const U& val) const { return GetBucket(val, Identity<U>()); }
  template <class U> Node* FindBucket(Node* nd, const U& val) const { return FindBucket(nd, val, Identity<U>()); }

  void ClearAndRemove() {
    if (buckets_) {
      for (Node** it = buckets_; it != buckets_end_; ++it) {
        for (Node *a = *it, *nxt; a; a = nxt) {
          nxt = a->nxt;
          delete a;
        }
      }
      delete[] buckets_;
      buckets_ = buckets_end_ = nullptr;
    }
  }

  Iter MakeIter(Node** a, Node* b) { return (Iter){a, buckets_end_, b}; }
  ConstIter MakeConstIter(Node** a, Node* b) { return (ConstIter){a, buckets_end_, b}; }
public:
  UnorderedBase(size_t bucket = 1, const Hash& hf = Hash(), const Pred& eq = Pred(),
      const Cls& cs = Cls()) : size_(0), hasher_(hf), pred_(eq), classifier_(cs), keyget_() {
    if (bucket) {
      buckets_ = new Node*[bucket]();
      buckets_end_ = buckets_ + bucket;
    } else {
      buckets_ = buckets_end_ = nullptr;
    }
  }
  ~UnorderedBase() {
    if (!buckets_) return;
    for (Node** it = buckets_; it != buckets_end_; ++it) {
      for (Node *a = *it, *nxt; a; a = nxt) {
        nxt = a->nxt;
        delete a;
      }
    }
    delete[] buckets_;
  }
  UnorderedBase(const UnorderedBase& mp) : size_(mp.size_), hasher_(mp.hasher_),
      pred_(mp.pred_), classifier_(mp.classifier_), keyget_() {
    buckets_ = new Node*[mp.buckets_end_ - mp.buckets_]();
    buckets_end_ = buckets_ + (mp.buckets_end_ - mp.buckets_);
    for (Node **it = buckets_, **org = mp.buckets_; it != buckets_end_; ++it, ++org) {
      Node** prv = it;
      for (Node *a = *org; a; a = a->nxt) {
        *prv = new Node(*a);
        prv = &a->nxt;
      }
    }
  }
  UnorderedBase(UnorderedBase&& mp) : buckets_(mp.buckets_), buckets_end_(mp.buckets_end_),
      size_(mp.size_), hasher_(mp.hasher_), pred_(mp.pred_), classifier_(mp.classifier_), keyget_() {
    mp.buckets_ = nullptr;
  }

  const UnorderedBase& operator=(const UnorderedBase& mp) {
    if (buckets_ == mp.buckets_) return *this;
    if (buckets_) {
      for (Node** it = buckets_; it != buckets_end_; ++it) {
        for (Node *a = *it, *nxt; a; a = nxt) {
          nxt = a->nxt;
          delete a;
        }
      }
      if (buckets_end_ - buckets_ != mp.buckets_end_ - mp.buckets_) {
        delete[] buckets_;
        buckets_ = new Node*[mp.buckets_end_ - mp.buckets_]();
        buckets_end_ = buckets_ + (mp.buckets_end_ - mp.buckets_);
      } else {
        std::fill(buckets_, buckets_end_, nullptr);
      }
    } else {
      buckets_ = new Node*[mp.buckets_end_ - mp.buckets_]();
      buckets_end_ = buckets_ + (mp.buckets_end_ - mp.buckets_);
    }
    for (Node **it = buckets_, **org = mp.buckets_; it != buckets_end_; ++it, ++org) {
      Node** prv = it;
      for (Node *a = *org; a; a = a->nxt) {
        *prv = new Node(*a);
        prv = &a->nxt;
      }
    }
    size_ = mp.size_;
    hasher_ = mp.hasher_;
    pred_ = mp.pred_;
    classifier_ = mp.classifier_;
    return *this;
  }
  const UnorderedBase& operator=(UnorderedBase&& mp) {
    if (buckets_ == mp.buckets_) return *this;
    buckets_ = mp.buckets_;
    buckets_end_ = mp.buckets_end_;
    size_ = mp.size_;
    hasher_ = mp.hasher_;
    pred_ = mp.pred_;
    classifier_ = mp.classifier_;
    mp.buckets_ = nullptr;
    return *this;
  }

  void clear() {
    if (buckets_) {
      for (Node** it = buckets_; it != buckets_end_; ++it) {
        for (Node *a = *it, *nxt; a; a = nxt) {
          nxt = a->nxt;
          delete a;
        }
      }
      std::fill(buckets_, buckets_end_, nullptr);
    }
  }

  size_t BucketCount() const { return buckets_end_ - buckets_; }
  size_t size() const { return size_; }
  void swap(UnorderedBase& mp) {
    std::swap(buckets_, mp.buckets_);
    std::swap(buckets_end_, mp.buckets_end_);
    std::swap(size_, mp.size_);
    std::swap(hasher_, mp.hasher_);
    std::swap(pred_, mp.pred_);
    std::swap(classifier_, mp.classifier_);
  }

  Iter Insert(const T& val) {
    size_++;
    Node** nd = GetBucket(val);
    return MakeIter(nd, *nd = new Node(val, *nd));
  }
  Iter Insert(T&& val) {
    size_++;
    Node** nd = GetBucket(val);
    return MakeIter(nd, *nd = new Node(std::move(val), *nd));
  }
  std::pair<Iter, bool> InsertIf(const T& val) {
    Iter it;
    it.bucket = GetBucket(val);
    it.end = buckets_end_;
    if ((it.node = FindBucket(*(it.bucket), val))) {
      return {it, false};
    } else {
      size_++;
      it.node = *(it.bucket) = new Node(val, *(it.bucket));
      return {it, true};
    }
  }
  std::pair<Iter, bool> InsertIf(T&& val) {
    Iter it;
    it.bucket = GetBucket(val);
    it.end = buckets_end_;
    if ((it.node = FindBucket(*(it.bucket), val))) {
      return {it, false};
    } else {
      size_++;
      it.node = *(it.bucket) = new Node(std::move(val), *(it.bucket));
      return {it, true};
    }
  }

  template <class U> Iter Find(const U& val) {
    Iter it;
    it.bucket = GetBucket(val);
    it.node = FindBucket(*(it.bucket), val);
    it.end = buckets_end_;
    if (it.node) return it;
    else return IterEnd();
  }
  template <class U> ConstIter Find(const U& val) const {
    ConstIter it;
    it.bucket = GetBucket(val);
    it.node = FindBucket(*(it.bucket), val);
    it.end = buckets_end_;
    if (it.node) return it;
    else return ConstIterEnd();
  }

  void Rehash(size_t sz) {
    if (buckets_ && sz) {
      Node **oldbucket = buckets_, **oldend = buckets_end_;
      buckets_ = new Node*[sz]();
      buckets_end_ = buckets_ + sz;
      for (Node** it = oldbucket; it != oldend; ++it) {
        for (Node *a = *it, *nxt; a; a = nxt) {
          Node** nd = GetBucket(a->val);
          nxt = a->nxt;
          a->nxt = *nd;
          *nd = a;
        }
      }
      delete[] oldbucket;
    } else if (sz) {
      buckets_ = new Node*[sz]();
      buckets_end_ = buckets_ + sz;
    } else if (buckets_) {
      ClearAndRemove();
    }
  }


  Iter IterBegin() const {
    if (!size_) return IterEnd();
    Iter it;
    it.bucket = buckets_;
    it.end = buckets_end_;
    while (!*(it.bucket)) it.bucket++;
    it.node = *it.bucket;
    return it;
  }
  ConstIter ConstIterBegin() const {
    return IterBegin();
  }
  Iter IterEnd() const {
    return {buckets_end_, buckets_end_, nullptr};
  }
  ConstIter ConstIterEnd() const {
    return IterEnd();
  }
};

template <class Key, class T, class Hash = std::hash<Key>,
          class Pred = std::equal_to<Key>, class Cls = DefaultClassifier>
class UnorderedMap {
public:
  typedef std::pair<const Key, T> value_type;
  typedef size_t size_type;

private:
  struct GetFirst {
    Key operator()(const value_type& val) const { return val.first; }
  };
  typedef UnorderedBase<value_type, Hash, Pred, Cls, GetFirst> base_type;

  base_type base_;
  float alpha_;

  void CheckRehash() {
    if ((float)base_.size() / base_.BucketCount() > alpha_) {
      base_.Rehash(base_.BucketCount() * 2);
    }
  }
public:
  typedef typename base_type::Iter iterator;
  typedef typename base_type::ConstIter const_iterator;

  explicit UnorderedMap(size_type bucket = 4, const Hash& hf = Hash(), const Pred& eq = Pred(),
      const Cls& cs = Cls()) : base_(2l << std::__lg(bucket - 1), hf, eq, cs), alpha_(1.0) {}
  template <class It> UnorderedMap(It first, It last, size_type bucket = 0,
      const Hash& hf = Hash(), const Pred& eq = Pred(), const Cls& cs = Cls()) :
      base_(2l << std::__lg(std::max(4, std::distance(first, last)) - 1), hf, eq, cs), alpha_(1.0) {
    for (; first != last; first++) base_.InsertIf(*first);
  }
  UnorderedMap(const UnorderedMap& mp) : base_(mp.base_), alpha_(mp.alpha_) {}
  UnorderedMap(UnorderedMap&& mp) : base_(std::move(mp.base_)), alpha_(mp.alpha_) {}

  const UnorderedMap& operator=(const UnorderedMap& mp) {
    base_ = mp.base_;
    alpha_ = mp.alpha_;
  }
  const UnorderedMap& operator=(UnorderedMap&& mp) {
    base_ = std::move(mp.base_);
    alpha_ = mp.alpha_;
  }

  size_type size() const { return base_.size(); }
  bool empty() const { return base_.size() == 0; }
  void clear() { base_.clear(); }
  void swap(UnorderedMap& mp) { base_.swap(mp.base_); std::swap(alpha_, mp.alpha_); }

  iterator begin() const { return base_.IterBegin(); }
  iterator end() const { return base_.IterEnd(); }
  const_iterator cbegin() const { return base_.ConstIterBegin(); }
  const_iterator cend() const { return base_.ConstIterEnd(); }

  T& operator[](const Key& val) {
    CheckRehash();
    auto it = base_.InsertIf(std::make_pair(val, T()));
    return it.first.GetPointer()->second;
  }
  T& operator[](Key&& val) {
    CheckRehash();
    auto it = base_.InsertIf(std::make_pair(std::move(val), T()));
    return it.first->second;
  }

  const_iterator find(const Key& val) const {
    return base_.Find(val);
  }
  iterator find(const Key& val) {
    return base_.Find(val);
  }

  std::pair<iterator, bool> insert(const value_type& val) {
    CheckRehash();
    auto it = base_.InsertIf(val);
    return std::make_pair(iterator(it.first), it.second);
  }
  std::pair<iterator, bool> insert(value_type&& val) {
    CheckRehash();
    auto it = base_.InsertIf(std::move(val));
    return std::make_pair(iterator(it.first), it.second);
  }

  size_type bucket_count() const { return base_.BucketCount(); }
  float load_factor() const { return (float)base_.size() / base_.BucketCount(); }
  float max_load_factor() const { return alpha_; }
  void max_load_factor(float na) {
    alpha_ = na;
    CheckRehash();
  }
  void rehash(size_type b) {
    b = 2l << std::__lg(b - 1);
    if (b > base_.BucketCount() || (float)base_.size() / b <= alpha_) base_.Rehash(b);
  }
  void shrink_to_fit() {
    size_type b = base_.size() / alpha_;
    b = 2l << std::__lg(b - 1);
    if (b != base_.BucketCount()) base_.Rehash(b);
  }
  void reserve(size_type b) {
    rehash(b / alpha_);
  }
};

#endif
