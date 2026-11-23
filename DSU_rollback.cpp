#include <iostream>
#include <numeric>
#include <vector>
#include <stack>
#include <algorithm>
using namespace std;

class DSU {
private:
    size_t nodes;
    int components;
    vector<int> parents;
    vector<int> sizes;
    stack<pair<int,int>> history;
public:
    DSU(const size_t tam = 1): nodes(tam+1), components(tam) {
        parents.resize(nodes);
        sizes.resize(nodes, 1);
        iota(begin(parents), end(parents), 0);
    }

    int find(int i) {
        if (parents[i] == i) return i;
        return find(parents[i]);
    }

    bool Same_Set(int u, int v) { return find(u) == find(v); }

    bool unite(int i, int j) {
        int x = find(i);
        int y = find(j);
        if (x == y) return false;
        if (sizes[x] < sizes[y]) swap(x, y);
        sizes[x] += sizes[y];
        parents[y] = x;
        history.emplace(x, y);
        components--;
        return true;
    }

    int count_components() { return components; }

    size_t time() { return history.size(); }

    void rollback() {
        if (history.empty()) return;
        auto [u, v] = history.top();
        history.pop();
        parents[v] = v;
        sizes[u] -= sizes[v];
        components++;
    }

    // ---- Solo lectura: las usa la animación para "fotografiar" el estado real. No cambian el comportamiento de la estructura.
    vector<int> path(int i) const {                
        vector<int> p{i};
        while (parents[i] != i) { i = parents[i]; p.push_back(i); }
        return p;
    }
    const vector<int>& get_parents() const { return parents; }
    const vector<int>& get_sizes() const { return sizes; }
    vector<pair<int,int>> get_history() const { 
        vector<pair<int,int>> h;
        auto copia = history;
        while (!copia.empty()) { h.push_back(copia.top()); copia.pop(); }
        reverse(begin(h), end(h));
        return h;
    }
    pair<int,int> top() const { return history.top(); }
};

#ifndef DSU_ANIMACION   // la animación incluye este archivo y define su propio main
struct Edge {
    int u, v;
    long long w;
};

bool by_weight(const Edge& a, const Edge& b) { return a.w < b.w; }

long long kruskal(int n, vector<Edge> edges) {
    sort(begin(edges), end(edges), by_weight);
    DSU dsu(n);
    long long total = 0;

    for (const auto& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            total += e.w;
        }
    }
    return dsu.count_components() == 1 ? total : -1;
}

int main() {
    int n, m;
    cin >> n >> m;
    vector<Edge> edges(m);
    for (auto& e : edges) cin >> e.u >> e.v >> e.w;
    sort(begin(edges), end(edges), by_weight);
    DSU dsu(n);
    long long total = 0;
    for (const auto& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            total += e.w;
        }
    }
    if (dsu.count_components() == 1) cout << total << "\n";
    else cout << "IMPOSSIBLE\n";
    return 0;
}
#endif