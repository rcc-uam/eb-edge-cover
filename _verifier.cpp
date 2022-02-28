#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

struct point {
   double x, y;
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

   int used;
   std::cin >> used;

   double total = 0;
   std::vector<bool> covered(a + b, false);
   for (int i = 0; i < used; ++i) {
      int p1, p2;
      std::cin >> p1 >> p2;
      covered.at(p1) = true, covered.at(p2) = true;
      total += distance(points[p1], points[p2]);
   }

   double cost;
   std::cin >> cost;
   std::cout << (std::count(covered.begin( ), covered.end( ), true) == covered.size( ) && std::abs(total - cost) < 1e-3) << "\n";
} catch (...) {
   std::cout << 0 << "\n";
}
