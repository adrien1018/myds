#ifndef GRAPH_H_
#define GRAPH_H_

#include <cstring>

struct Edge {
  int to, nxt;
};
template <class T> struct WeightedEdge {
  int to; T w; int nxt;
};

template <int N, int M, class Ed = Edge> class Graph {
  Ed ed[M];
  int head[N];
  int top;
public:
  Graph(int V = N) : top(0) { memset(head, 0xff, V * 4); }
  template <class T> void ForEach(int x, T func) {
    for (int i = head[x]; i != -1; i = ed[i].nxt) {
      func(ed[i].to);
    }
  }
  void AddEdgeBi(int u, int v) {
    ed[top] = {v, head[u]}; head[u] = top++;
    ed[top] = {u, head[v]}; head[v] = top++;
  }
  void AddEdge(int u, int v) {
    ed[top] = {v, head[u]}; head[u] = top++;
  }
  void clear(int V = N) { top = 0; memset(head, 0xff, V * 4); }
};

template <int N, int M, class T = int, class Ed = WeightedEdge<T>>
class WeightedGraph {
  Ed ed[M];
  int head[N];
  int top;
public:
  WeightedGraph(int V = N) : top(0) { memset(head, 0xff, V * 4); }
  template <class U> void ForEach(int x, U func) {
    for (int i = head[x]; i != -1; i = ed[i].nxt) {
      func(ed[i].to, ed[i].w);
    }
  }
  void AddEdgeBi(int u, int v, T w) {
    ed[top] = {v, w, head[u]}; head[u] = top++;
    ed[top] = {u, w, head[v]}; head[v] = top++;
  }
  void AddEdge(int u, int v, T w) {
    ed[top] = {v, w, head[u]}; head[u] = top++;
  }
  void clear(int V = N) { top = 0; memset(head, 0xff, V * 4); }
};

#endif
