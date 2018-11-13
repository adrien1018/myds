#ifndef SA_IS_H_
#define SA_IS_H_

#include <cstdint>
#include <cstring>

using Sub_ = uint32_t;
using Bset_ = uint64_t;

inline bool Getbit(const Bset_ bs[], int x) {
  return bs[x >> 6] >> (x & 63) & 1;
}
inline void Setbit(Bset_ bs[], int x, bool y) {
  if (y) {
    bs[x >> 6] |= 1ll << (x & 63);
  } else {
    bs[x >> 6] &= ~(1ll << (x & 63));
  }
}

template <class T>
inline void GetBuckets(const T str[], int bucket[], int N, int K, bool is_end) {
  memset(bucket, 0, sizeof(*bucket) * (K + 1));
  for (int i = 0; i < N; i++) bucket[(Sub_)str[i]]++;
  for (int i = 0, sum = 0; i <= K; i++) {
    sum += bucket[i];
    bucket[i] = is_end ? sum : sum - bucket[i];
  }
}

template <class T>
inline void InduceSAl(const Bset_ bs[], int SA[], const T str[], int bucket[],
    int N, int K, bool is_end) {
  GetBuckets(str, bucket, N, K, is_end);
  for (int i = 0; i < N; i++) {
    int j = SA[i] - 1;
    if (j >= 0 && !Getbit(bs, j)) SA[bucket[(Sub_)str[j]]++] = j;
  }
}
template <class T>
inline void InduceSAs(const Bset_ bs[], int SA[], const T str[], int bucket[],
    int N, int K, bool is_end) {
  GetBuckets(str, bucket, N, K, is_end);
  for (int i = N - 1; i >= 0; i--) {
    int j = SA[i] - 1;
    if (j >= 0 && Getbit(bs, j)) SA[--bucket[(Sub_)str[j]]] = j;
  }
}

template <class T>
void SA_IS(const T str[], int SA[], int N, int K, uint8_t buffer[]) {
  Bset_* bs = (Bset_*)buffer;
  buffer += (N + 63) >> 6 << 3;
  auto IsLMS = [&](int i){ return i > 0 && Getbit(bs, i) && !Getbit(bs, i - 1); };
  int* bucket = (int*)buffer;
  Setbit(bs, N - 2, false);
  Setbit(bs, N - 1, true);
  for (int i = N - 3; i >= 0; i--) {
    Setbit(bs, i, str[i] < str[i + 1] ||
        (str[i] == str[i + 1] && Getbit(bs, i + 1)));
  }
  GetBuckets(str, bucket, N, K, true);
  memset(SA, 0xff, sizeof(*SA) * N);
  for (int i = 1; i < N; i++) {
    if (IsLMS(i)) SA[--bucket[(Sub_)str[i]]] = i;
  }
  InduceSAl(bs, SA, str, bucket, N, K, false);
  InduceSAs(bs, SA, str, bucket, N, K, true);

  int n1 = 0;
  for (int i = 0; i < N; i++) {
    if (IsLMS(SA[i])) SA[n1++] = SA[i];
  }
  memset(SA + n1, 0xff, sizeof(*SA) * (N - n1));
  int name = 0, prv = -1;
  for (int i = 0; i < n1; i++) {
    int pos = SA[i];
    bool flag = false;
    for (int j = 0; j < N; j++) {
      int nj = pos + j, pj = prv + j;
      if (prv == -1 || str[nj] != str[pj] || Getbit(bs, nj) != Getbit(bs, pj)) {
        flag = true; break;
      } else if (j > 0 && (IsLMS(nj) || IsLMS(pj))) {
        break;
      }
    }
    if (flag) name++, prv = pos;
    pos = (pos - (pos & 1)) >> 1;
    SA[n1 + pos] = name - 1;
  }
  for (int i = N - 1, j = N - 1; i >= n1; i--) {
    if (SA[i] >= 0) SA[j--] = SA[i];
  }

  int* s1 = SA + N - n1;
  if (name < n1) {
    SA_IS(s1, SA, n1, name - 1, buffer);
  } else {
    for (int i = 0; i < n1; i++) SA[s1[i]] = i;
  }
  GetBuckets(str, bucket, N, K, true);
  for (int i = 1, j = 0; i < N; i++) {
    if (IsLMS(i)) s1[j++] = i;
  }
  for (int i = 0; i < n1; i++) SA[i] = s1[SA[i]];
  memset(SA + n1, 0xff, sizeof(*SA) * (N - n1));
  for (int i = n1 - 1; i >= 0; i--) {
    int j = SA[i];
    SA[i] = -1;
    SA[--bucket[(Sub_)str[j]]] = j;
  }
  InduceSAl(bs, SA, str, bucket, N, K, false);
  InduceSAs(bs, SA, str, bucket, N, K, true);
}

void GetRank(const int SA[], int rank[], int N) {
  for (int i = 0; i < N; i++) rank[SA[i]] = i;
}

template <class T>
void GetLCPRank(const T str[], const int SA[], int lcp[], int rank[], int N) {
  GetRank(SA, rank, N);
  for (int i = 0, len = 0; i < N; i++) {
    if (rank[i]) {
      int j = SA[rank[i] - 1];
      if (len) len--;
      while (str[i + len] == str[j + len]) len++;
      lcp[i] = len;
    } else {
      lcp[i] = 0;
    }
  }
}

#endif
