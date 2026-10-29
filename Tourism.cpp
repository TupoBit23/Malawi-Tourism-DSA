// Malawi Tourism Circuit - Dijkstra's shortest path and Kruskal's MST
// Usage: ./tourism graph.txt dijkstra <source> <destination> [-v]
//        ./tourism graph.txt mst

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <algorithm>
#include <limits>
#include <iomanip>
using namespace std;

const int INF = numeric_limits<int>::max();

struct Edge { int to, w; };
struct FullEdge { int u, v, w; };

struct Graph {
    int n = 0;
    vector<string> names;
    map<string,int> id;
    vector<vector<Edge>> adj;
    vector<FullEdge> edges;
};

bool loadGraph(const string& file, Graph& g, string& err) {
    ifstream in(file);
    if (!in) { err = "Cannot open file: " + file; return false; }
    int m;
    if (!(in >> g.n) || g.n <= 0) { err = "Invalid node count"; return false; }
    g.names.resize(g.n);
    g.adj.assign(g.n, {});
    for (int i = 0; i < g.n; i++) {
        if (!(in >> g.names[i])) { err = "Missing node name"; return false; }
        g.id[g.names[i]] = i;
    }
    if (!(in >> m) || m < 0) { err = "Invalid edge count"; return false; }
    for (int i = 0; i < m; i++) {
        string a, b; int w;
        if (!(in >> a >> b >> w)) { err = "Malformed edge line " + to_string(i+1); return false; }
        if (!g.id.count(a) || !g.id.count(b)) { err = "Unknown node in edge: " + a + " " + b; return false; }
        if (w < 0) { err = "Negative weight not allowed (Dijkstra): " + a + " " + b; return false; }
        int u = g.id[a], v = g.id[b];
        g.adj[u].push_back({v, w});
        g.adj[v].push_back({u, w});
        g.edges.push_back({u, v, w});
    }
    return true;
}

void dijkstra(const Graph& g, int s, int t, bool verbose) {
    vector<int> dist(g.n, INF), prev(g.n, -1);
    vector<bool> done(g.n, false);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    dist[s] = 0; pq.push({0, s});
    int step = 0;
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (done[u]) continue;
        done[u] = true;
        if (verbose) cout << "Step " << ++step << ": settle " << g.names[u] << " (dist=" << d << ")\n";
        if (u == t) break;
        for (auto e : g.adj[u]) {
            if (!done[e.to] && d + e.w < dist[e.to]) {
                if (verbose) cout << "    relax " << g.names[u] << "->" << g.names[e.to]
                                  << ": " << (dist[e.to]==INF ? string("INF") : to_string(dist[e.to]))
                                  << " -> " << d + e.w << "\n";
                dist[e.to] = d + e.w; prev[e.to] = u;
                pq.push({dist[e.to], e.to});
            }
        }
    }
    if (dist[t] == INF) { cout << "No path from " << g.names[s] << " to " << g.names[t] << "\n"; return; }
    vector<int> path;
    for (int v = t; v != -1; v = prev[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    cout << "Shortest distance: " << dist[t] << " km\nPath: ";
    for (size_t i = 0; i < path.size(); i++) cout << g.names[path[i]] << (i+1<path.size() ? " -> " : "\n");
}

struct DSU {
    vector<int> p;
    DSU(int n) : p(n) { for (int i = 0; i < n; i++) p[i] = i; }
    int find(int x) { return p[x]==x ? x : p[x]=find(p[x]); }
    bool unite(int a, int b) { a=find(a); b=find(b); if (a==b) return false; p[a]=b; return true; }
};

void kruskal(const Graph& g) {
    vector<FullEdge> es = g.edges;
    sort(es.begin(), es.end(), [](auto&a, auto&b){ return a.w < b.w; });
    DSU d(g.n); int total = 0, cnt = 0;
    for (auto& e : es) {
        if (d.unite(e.u, e.v)) {
            cout << "ADD    " << g.names[e.u] << " - " << g.names[e.v] << " (" << e.w << ")\n";
            total += e.w; cnt++;
        } else {
            cout << "SKIP   " << g.names[e.u] << " - " << g.names[e.v] << " (" << e.w << ") forms a cycle\n";
        }
    }
    if (cnt != g.n - 1) cout << "Graph is disconnected: minimum spanning FOREST, " << cnt << " edges\n";
    cout << "MST edges: " << cnt << ", total weight: " << total << " km\n";
}

int main(int argc, char** argv) {
    if (argc < 3) { cerr << "Usage: tourism <graph.txt> dijkstra <src> <dst> [-v] | mst\n"; return 1; }
    Graph g; string err;
    if (!loadGraph(argv[1], g, err)) { cerr << "Error: " << err << "\n"; return 1; }
    string cmd = argv[2];
    if (cmd == "mst") { kruskal(g); return 0; }
    if (cmd == "dijkstra") {
        if (argc < 5) { cerr << "Error: need source and destination\n"; return 1; }
        string a = argv[3], b = argv[4];
        if (!g.id.count(a)) { cerr << "Error: unknown node '" << a << "'\n"; return 1; }
        if (!g.id.count(b)) { cerr << "Error: unknown node '" << b << "'\n"; return 1; }
        bool v = (argc > 5 && string(argv[5]) == "-v");
        dijkstra(g, g.id[a], g.id[b], v);
        return 0;
    }
    cerr << "Error: unknown command '" << cmd << "'\n"; return 1;
}