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

int main( ) try {
   int a, b;
   std::cin >> a >> b;

   std::vector<point> points(a + b);
   for (int i = 0; i < a + b; ++i) {
      std::cin >> points[i].x >> points[i].y;
   }

   std::vector<int> nearest(a + b);
   for (int i = 0; i < a + b; ++i) {
      auto [ini, fin] = (i < a ? std::pair(points.begin( ) + a, points.end( )) : std::pair(points.begin( ), points.begin( ) + a));
      nearest[i] = std::min_element(ini, fin, [&](const point& a, const point& b) {
         return distance(points[i], a) < distance(points[i], b);
      }) - points.begin( );
   }

   auto algorithm = [&](int begin, int end, int step) {
      std::vector<edge> used; double total = 0;
      std::vector<bool> covered(a + b, false);
      for (int i = begin; i != end; i += step) {
         if (!covered[i]) {
            used.push_back(edge{i, nearest[i]});
            covered[i] = true;
            covered[nearest[i]] = true;
            total += distance(points[i], points[nearest[i]]);
         }
      }
      return std::pair(total, used);
   };

   auto [total, used] = std::min(algorithm(0, a + b, +1), algorithm(a + b - 1, -1, -1), [](const auto& p1, const auto& p2) {
      return p1.first < p2.first;
   });
   std::cout << used.size( ) << "\n";
   for (auto current : used) {
      std::cout << current.p1 << " " << current.p2 << "\n";
   }
   std::cout << std::setprecision(9) << std::fixed << total << "\n";
} catch (...) {
   return -1;
}
