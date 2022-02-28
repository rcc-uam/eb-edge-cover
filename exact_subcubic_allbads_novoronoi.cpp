#include <boost/heap/fibonacci_heap.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

struct point {
   double x, y;

   bool operator<(const point& p) const {
      return x < p.x || (x == p.x && y < p.y);
   }
};

struct edge {
   int p1, p2;
};

double distance_magnitude(const point& p1, const point& p2) {
   double dx = p1.x - p2.x, dy = p1.y - p2.y;
   return dx * dx + dy * dy;
}

double distance(const point& p1, const point& p2) {
   return std::sqrt(distance_magnitude(p1, p2));
}

double reduced_cost(int p1, int p2, const std::vector<point>& points, const std::vector<double>& nearest) {
   return nearest[p1] + nearest[p2] - distance(points[p1], points[p2]);
}

std::set<int> find_bads(int a, int b, const std::vector<int>& matching, const std::vector<double>& beta) {
   std::set<int> res;
   for (int j = a; j < a + b; ++j) {
      if (matching[j] == -1 && beta[j] > 1e-15) {
         res.insert(j);
      }
   }
   return res;
}

void update_matching(std::vector<int>& matching, std::vector<int>& tree, int start, int a, int b) {
   std::vector<std::pair<int, int>> path;
   for (int p = start; tree[p] != -1; p = tree[p]) {
      path.emplace_back(p, tree[p]);
   }
   for (int c = 1 - path.size( ) % 2; c < path.size( ); c += 2) {
      for (int p : { path[c].first, path[c].second }) {
         if (matching[p] != -1) {
            matching[matching[p]] = -1;
         }
      }
      matching[path[c].first] = path[c].second;
      matching[path[c].second] = path[c].first;
   }
}

struct mapping {
   const std::vector<point>& points;
   const std::vector<double>& weight;

   mapping(const std::vector<point>& p, const std::vector<double>& w)
   : points(p), weight(w) {
   }
};

struct voronoi_diagram {
   std::vector<std::pair<int, double>> sites;

   voronoi_diagram( ) = default;
   voronoi_diagram(const std::set<int>& set, const mapping& m) {
      insert(set, m);
   }
   void insert(const std::set<int>& set, const mapping& m) {
      for (int v : set) {
         sites.emplace_back(v, m.weight[v]);
      }
   }
   int find(int v, const mapping& m) {
      return std::min_element(sites.begin( ), sites.end( ), [&](auto elem1, auto elem2) {
         return (distance(m.points[elem1.first], m.points[v]) - elem1.second < distance(m.points[elem2.first], m.points[v]) - elem2.second);
      })->first;
   }
   bool empty( ) const {
      return sites.empty( );
   }
};

struct min_heap {
   boost::heap::fibonacci_heap<std::tuple<double, int, int>, boost::heap::compare<std::greater<>>> heap;
   std::vector<std::vector<decltype(heap)::handle_type>> handles;

   min_heap(int a, int b)
   : handles(a) {
   }
   void push(int i, int j, const mapping& m) {
      handles[i].push_back(heap.emplace(distance(m.points[i], m.points[j]) - m.weight[i] - m.weight[j], i, j));
   }
   void erase(int i) {
      for (; !handles[i].empty( ); handles[i].pop_back( )) {
         heap.erase(handles[i].back( ));
      }
   }
   void clear( ) {
      heap.clear( );
      handles = std::vector<std::vector<decltype(heap)::handle_type>>(handles.size( ));
   }
   std::tuple<double, int, int> top( ) const {
      return heap.top( );
   }
   bool empty( ) const {
      return heap.empty( );
   }
};

int main( ) try {
   int a, b;
   std::cin >> a >> b;

   std::vector<point> points(a + b);
   for (int v = 0; v < a + b; ++v) {
      std::cin >> points[v].x >> points[v].y;
   }

   std::vector<int> closest_v(a + b);
   std::vector<double> nearest(a + b, std::numeric_limits<double>::max( ));
   for (int i = 0; i < a; ++i) {
      for (int j = a; j < a + b; ++j) {
         double d = distance(points[i], points[j]);
         if (d < nearest[i]) {
            nearest[i] = d;
            closest_v[i] = j;
         }
         if (d < nearest[j]) {
            nearest[j] = d;
            closest_v[j] = i;
         }
      }
   }

   std::vector<int> matching(a + b, -1);
   std::vector<double> alpha(a, 0);
   std::vector<double> beta(a + b, std::numeric_limits<double>::lowest( ));
   for (int j = a; j < a + b; ++j) {
      for (int i = 0; i < a; ++i) {
         beta[j] = std::max(beta[j], reduced_cost(i, j, points, nearest));
      }
   }

   int h = std::ceil(std::sqrt(a));
   for (std::set<int> s1, s2; !(s1 = find_bads(a, b, matching, beta)).empty( ); s2.clear( )) {
      int ej = *std::min_element(s1.begin( ), s1.end( ), [&](int j1, int j2) {
         return beta[j1] < beta[j2];
      }); double epsilon = beta[ej];
      std::set<int> t; std::vector<std::set<int>> fi(h); int f_tam = a;
      for (int i = 0; i < a; ++i) {
         fi[i / h].insert(i);
      }
      std::vector<int> tree(a + b, -1);

      std::vector<double> weight(a + b);
      for (int i = 0; i < a; ++i) {
         weight[i] = nearest[i] - alpha[i];
      }
      for (int j = a; j < a + b; ++j) {
         weight[j] = nearest[j] - beta[j];
      }
      double change = 0;

      mapping m(points, weight);
      voronoi_diagram voronoi_s1(s1, m);
      std::vector<voronoi_diagram> voronoi_fi(h);
      for (int hi = 0; hi < h; ++hi) {
         voronoi_fi[hi] = voronoi_diagram(fi[hi], m);
      }

      min_heap heap_f_s1(a, b), heap_s2_f(a, b);
      for (int hi = 0; hi < h; ++hi) {
         for (int i : fi[hi]) {
            heap_f_s1.push(i, voronoi_s1.find(i, m), m);
         }
      }
      for (int j : s2) {
         for (int hi = 0; hi < h; ++hi) {
            heap_s2_f.push(voronoi_fi[hi].find(j, m), j, m);
         }
      }

      for (;;) {
         double delta = std::numeric_limits<double>::max( ); int di, dj;
         for (auto heap : { &heap_f_s1, &heap_s2_f }) {
            if (!heap->empty( )) {
               if (auto [d, i, j] = heap->top( ); delta > d) {
                  delta = d, di = i, dj = j;
               }
            }
         }
         delta -= change;

         // case 1
         if (std::abs(delta) <= 1e-15 && matching[di] == -1) {
            tree[di] = dj;
            fi[di / h].erase(di), t.insert(di), f_tam -= 1;
            weight[di] += change;
            update_matching(matching, tree, di, a, b);
            break;
         }

         // case 2
         if (std::abs(delta) <= 1e-15 && matching[di] != -1) {
            int kj = matching[di];
            tree[kj] = di, tree[di] = dj;
            fi[di / h].erase(di), t.insert(di), s2.insert(kj), f_tam -= 1;
            weight[di] += change, weight[kj] -= change;
            if (beta[kj] < epsilon) {
               epsilon = beta[kj], ej = kj;
            }

            voronoi_fi[di / h] = voronoi_diagram(fi[di / h], m);
            if (s2.size( ) <= h) {
               heap_f_s1.erase(di), heap_s2_f.erase(di);
               for (int j : s2) {
                  if (!voronoi_fi[di / h].empty( )) {
                     heap_s2_f.push(voronoi_fi[di / h].find(j, m), j, m);
                  }
               }
               for (int hi = 0; hi < h; ++hi) {
                  if (!voronoi_fi[hi].empty( ) && hi != di / h) {
                     heap_s2_f.push(voronoi_fi[hi].find(kj, m), kj, m);
                  }
               }
            } else {
               voronoi_s1.insert(s2, m);
               s1.merge(std::move(s2));
               heap_f_s1.clear( ), heap_s2_f.clear( );
               for (int hi = 0; hi < h; ++hi) {
                  for (int i : fi[hi]) {
                     heap_f_s1.push(i, voronoi_s1.find(i, m), m);
                  }
               }
            }
            continue;
         }

         // case 3
         if (epsilon > delta) {
            change += delta;
            epsilon -= delta;
            continue;
         }

         // case 4
         if (delta >= epsilon) {
            change += epsilon;
            epsilon -= epsilon;
            update_matching(matching, tree, ej, a, b);
            break;
         }
      }

      for (int i : t) {
         alpha[i] = nearest[i] - weight[i] + change;
      }
      for (const auto& s : { s1, s2 }) {
         for (int j : s) {
            beta[j] = nearest[j] - weight[j] - change;
         }
      }
   }

   std::vector<edge> used;
   std::vector<bool> covered(a + b, false);
   double total = 0;
   for (int i = 0; i < a + b; ++i) {
      if (!covered[i]) {
         int matched = (i < a && matching[i] != -1 ? matching[i] : closest_v[i]);
         used.emplace_back(i, matched);
         covered[i] = true;
         covered[matched] = true;
         total += distance(points[i], points[matched]);
      }
   }

   std::cout << used.size( ) << "\n";
   for (auto [p1, p2] : used) {
      std::cout << p1 << " " << p2 << "\n";
   }
   std::cout << std::setprecision(9) << std::fixed << total << "\n";
} catch (...) {
   return -1;
}
