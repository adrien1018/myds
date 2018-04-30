#ifndef UNORDERED_H_
#define UNORDERED_H_

#include <cstring>
#include <iterator>
#include <algorithm>

struct DefaultClassifier {
  size_t operator()(size_t a, size_t N) const { return a & (N - 1); }
};

template <class T, class Hash, class Pred, class Cls = DefaultClassifier>
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
private:
  Node** buckets_;
  Node** buckets_end_;
  size_t size_;
  Hash hasher_;
  Pred pred_;
  Cls classifier_;

  Node*& GetBucket(const T& val) {
    return buckets_[classifier_(hasher_(val), buckets_end_ - buckets_)];
  }
  const Node*& GetBucket(const T& val) const {
    return buckets_[classifier_(hasher_(val), buckets_end_ - buckets_)];
  }
  Node* FindBucket(Node* nd, const T& val) const {
    for (; nd; nd = nd->nxt) {
      if (pred_(nd->val, val)) return nd;
    }
    return nullptr;
  }
public:
  UnorderedBase(size_t bucket = 1, const Hash& hf = Hash(), const Pred& eq = Pred(),
      const Cls& cs = Cls()) : size_(0), hasher_(hf), pred_(eq), classifier_(cs) {
    buckets_ = new Node*[bucket]();
    buckets_end_ = buckets_ + bucket;
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
      pred_(mp.pred_), classifier_(mp.classifier_) {
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
      size_(mp.size_), hasher_(mp.hasher_), pred_(mp.pred_), classifier_(mp.classifier_) {
    mp.buckets_ = nullptr;
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

  Node* Insert(const T& val) {
    Node*& nd = GetBucket(val);
    return nd = new Node(val, nd);
  }
  Node* Insert(T&& val) {
    Node*& nd = GetBucket(val);
    return nd = new Node(std::move(val), nd);
  }

  Node* Find(const T& val) {
    return FindBucket(GetBucket(val), val);
  }
  const Node* Find(const T& val) const {
    return FindBucket(GetBucket(val), val);
  }

  void Rehash(size_t sz) {
    Node **oldbucket = buckets_, **oldend = buckets_end_;
    buckets_ = new Node*[sz]();
    buckets_end_ = buckets_ + sz;
    for (Node** it = oldbucket; it != oldend; ++it) {
      for (Node *a = *it, *nxt; a; a = nxt) {
        Node*& nd = GetBucket(a->val);
        nxt = a->nxt;
        a->nxt = nd;
        nd = a;
      }
    }
    delete[] oldbucket;
  }
};

#endif
