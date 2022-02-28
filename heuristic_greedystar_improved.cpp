#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

struct point {
   double x, y;
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

bool connected(int p1, int p2, const std::vector<std::set<int>>& adjacency) {
   return (!adjacency[p1].empty( ) && !adjacency[p2].empty( ) && (p1 == *adjacency[p2].begin( ) || p2 == *adjacency[p1].begin( ) || *adjacency[p1].begin( ) == *adjacency[p2].begin( )));
}

bool is_isolated(int p, const std::vector<std::set<int>>& adjacency) {
   return adjacency[p].size( ) == 0;
}

bool is_edge(int p, const std::vector<std::set<int>>& adjacency) {
   return adjacency[p].size( ) == 1 && adjacency[*adjacency[p].begin( )].size( ) == 1;
}

bool is_central(int p, const std::vector<std::set<int>>& adjacency) {
   return adjacency[p].size( ) >= 2;
}

bool is_leaf(int p, const std::vector<std::set<int>>& adjacency) {
   return adjacency[p].size( ) == 1 && adjacency[*adjacency[p].begin( )].size( ) >= 2;
}

int main( ) try {
   int a, b;
   std::cin >> a >> b;

   std::vector<point> points(a + b);
   for (int i = 0; i < a + b; ++i) {
      std::cin >> points[i].x >> points[i].y;
   }

   std::vector<edge> edges;
   for (int i = 0; i < a; ++i) {
      for (int j = a; j < a + b; ++j) {
         edges.push_back(edge{i, j});
      }
   }
   std::sort(edges.begin( ), edges.end( ), [&](const edge& a1, const edge& a2) {
      return distance_magnitude(points[a1.p1], points[a1.p2]) < distance_magnitude(points[a2.p1], points[a2.p2]);
   });

   std::vector<int> nearest(a + b, -1);
   std::vector<std::set<int>> adjacency(a + b);
   for (auto [p1, p2] : edges) {
      if (nearest[p1] == -1) {
         nearest[p1] = p2;
      }
      if (nearest[p2] == -1) {
         nearest[p2] = p1;
      }
      auto local_decision = [&]( )->std::pair<std::vector<edge>, std::vector<edge>> {
         auto detect_case = [&](auto c1, auto c2) {
            return (c1(p1, adjacency) && c2(p2, adjacency) || (std::swap(p1, p2), c1(p1, adjacency) && c2(p2, adjacency)));
         };
         if (connected(p1, p2, adjacency)) {
            return { };
         } else if (detect_case(is_isolated, is_isolated)) {
            return { { edge(p1, p2) }, { } };
         } else if (detect_case(is_isolated, is_edge)) {
            return { { edge(p1, p2) }, { } };
         } else if (detect_case(is_isolated, is_central)) {
            return { { edge(p1, p2) }, { } };
         } else if (detect_case(is_isolated, is_leaf)) {
            return { { edge(p1, p2) }, { edge(p2, *adjacency[p2].begin( )) } };
         } else if (detect_case(is_edge, is_edge)) {
            int p1x = *adjacency[p1].begin( ), p2x = *adjacency[p2].begin( );
            if (distance(points[p1], points[p2]) + distance(points[p1x], points[p2x]) < distance(points[p1], points[p1x]) + distance(points[p2], points[p2x])) {
               return { { edge(p1, p2), edge(p1x, p2x) }, { edge(p1, p1x), edge(p2, p2x) } };
            } else {
               return { };
            }
         } else if (detect_case(is_edge, is_central)) {
            return { };
         } else if (detect_case(is_edge, is_leaf)) {
            int p1x = *adjacency[p1].begin( ), p2x = *adjacency[p2].begin( );
            if (distance(points[p1], points[p2]) + distance(points[p1x], points[p2x]) < distance(points[p1], points[p1x]) + distance(points[p2], points[p2x])) {
               return { { edge(p1, p2), edge(p1x, p2x) }, { edge(p1, p1x), edge(p2, p2x) } };
            } else {
               return { };
            }
         } else if (detect_case(is_central, is_central)) {
            return { };
         } else if (detect_case(is_central, is_leaf)) {
            return { };
         } else if (detect_case(is_leaf, is_leaf)) {
            int p1x = *adjacency[p1].begin( ), p2x = *adjacency[p2].begin( );
            if (distance(points[p1], points[p2]) < distance(points[p1], points[p1x]) + distance(points[p2], points[p2x])) {
               return { { edge(p1, p2) }, { edge(p1, p1x), edge(p2, p2x) } };
            } else {
               return { };
            }
         }
      };

      auto [append, remove] = local_decision( );
      for (edge current : remove) {
         adjacency[current.p1].erase(current.p2);
         adjacency[current.p2].erase(current.p1);
      }
      for (edge current : append) {
         adjacency[current.p1].insert(current.p2);
         adjacency[current.p2].insert(current.p1);
      }
   }

   bool changes = true;
   for (int i = 0; i < a + b && changes; ++i) {
      changes = false;
      for (int i = 0; i < a + b; ++i) {
         if (is_leaf(i, adjacency) && nearest[i] != *adjacency[i].begin( )) {
            changes = true;
            adjacency[*adjacency[i].begin( )].erase(i);
            adjacency[i].erase(*adjacency[i].begin( ));
            adjacency[i].insert(nearest[i]);
            adjacency[nearest[i]].insert(i);
         }
      }
   }

   int used = 0;
   for (int i = 0; i < a; ++i) {
      used += adjacency[i].size( );
   }
   std::cout << used << "\n";

   double total = 0;
   for (int i = 0; i < a; ++i) {
      for (int neighbor : adjacency[i]) {
         std::cout << i << " " << neighbor << "\n";
         total += distance(points[i], points[neighbor]);
      }
   }
   std::cout << std::setprecision(9) << std::fixed << total << "\n";
} catch (...) {
   return -1;
}
