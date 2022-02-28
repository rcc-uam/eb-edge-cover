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

   std::vector<edge> used;
   for (int i = 0; i < a + b; ++i) {
      if (nearest[i] != -1) {
         used.push_back(edge{i, nearest[i]});
         if (nearest[nearest[i]] == i) {
            nearest[nearest[i]] = -1;
         }
      }
   }

   double total = 0;
   std::cout << used.size( ) << "\n";
   for (auto current : used) {
      std::cout << current.p1 << " " << current.p2 << "\n";
      total += distance(points[current.p1], points[current.p2]);
   }
   std::cout << std::setprecision(9) << std::fixed << total << "\n";
} catch (...) {
   return -1;
}
