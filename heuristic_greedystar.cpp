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

   std::vector<std::set<int>> adjacency(a + b);
   for (auto [p1, p2] : edges) {
      auto accept_edge = [&]( )->std::pair<bool, std::vector<edge>> {
         auto detect_case = [&](auto c1, auto c2) {
            return (c1(p1, adjacency) && c2(p2, adjacency) || (std::swap(p1, p2), c1(p1, adjacency) && c2(p2, adjacency)));
         };
         if (connected(p1, p2, adjacency)) {
            return { false, { } };
         } else if (detect_case(is_isolated, is_isolated)) {
            return { true, { } };
         } else if (detect_case(is_isolated, is_edge)) {
            return { true, { } };
         } else if (detect_case(is_isolated, is_central)) {
            return { true, { } };
         } else if (detect_case(is_isolated, is_leaf)) {
            return { true, { edge(p2, *adjacency[p2].begin( )) } };
         } else if (detect_case(is_edge, is_edge)) {
            return { false, { } };
         } else if (detect_case(is_edge, is_central)) {
            return { false, { } };
         } else if (detect_case(is_edge, is_leaf)) {
            return { false, { } };
         } else if (detect_case(is_central, is_central)) {
            return { false, { } };
         } else if (detect_case(is_central, is_leaf)) {
            return { false, { } };
         } else if (detect_case(is_leaf, is_leaf)) {
            if (distance(points[p1], points[*adjacency[p1].begin( )]) + distance(points[p2], points[*adjacency[p2].begin( )]) > distance(points[p1], points[p2])) {
               return { true, { edge(p1, *adjacency[p1].begin( )), edge(p2, *adjacency[p2].begin( )) } };
            } else {
               return { false, { } };
            }
         }
      };

      if (auto [ok, remove] = accept_edge( ); ok) {
         for (edge current : remove) {
            adjacency[current.p1].erase(current.p2);
            adjacency[current.p2].erase(current.p1);
         }
         adjacency[p1].insert(p2);
         adjacency[p2].insert(p1);
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
