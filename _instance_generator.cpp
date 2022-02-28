#include <algorithm>
#include <functional>
#include <iostream>
#include <numbers>
#include <random>
#include <vector>

struct point {
   double x, y;
   bool operator==(const point& p) const {
      return x == p.x && y == p.y;
   }
};

double distance_magnitude(const point& p1, const point& p2) {
   double dx = p1.x - p2.x, dy = p1.y - p2.y;
   return dx * dx + dy * dy;
}

double distance(const point& p1, const point& p2) {
   return std::sqrt(distance_magnitude(p1, p2));
}

bool collinear(point p1, point p2, point p3) {
   auto impl = [](point p1, point p2, point p3) {
      return std::abs(distance(p1, p3) - (distance(p1, p2) + distance(p2, p3))) <= 1e-15;
   };
   return impl(p1, p2, p3) + impl(p2, p3, p1) + impl(p3, p1, p2) == 1;
}

std::function<point( )> make_generator1(double side, char c, std::mt19937_64& gen) {
   if (c == 'R') {
      std::uniform_real_distribution<double> dist(0, side);
      return [=, &gen] mutable {
         return point{dist(gen), dist(gen)};
      };
   } else if (c == 'P') {
      std::uniform_real_distribution<double> distL(0, side / 2);
      std::uniform_real_distribution<double> distG(0, 2 * std::numbers::pi_v<double> - std::numeric_limits<double>::epsilon( ));
      return [=, &gen] mutable {
         double r = distL(gen), g = distG(gen);
         double x = r * std::cos(g), y = r * std::sin(g);
         return point{x + side / 2, y + side / 2};
      };
   } else if (c == 'E') {
      std::uniform_real_distribution<double> distL(0, std::log2(side) / 2);
      std::uniform_real_distribution<double> distG(0, 2 * std::numbers::pi_v<double> - std::numeric_limits<double>::epsilon( ));
      return [=, &gen] mutable {
         double r = distL(gen), g = distG(gen);
         double x = r * std::cos(g), y = r * std::sin(g);
         return point{x + side / 2, y + side / 2};
      };
   }
}

std::function<point( )> make_generator2(double side, char c, std::mt19937_64& gen) {
   if (c == 'R' || c == 'P') {
      return make_generator1(side, c, gen);
   } else if (c == 'E') {
      std::uniform_real_distribution<double> distL(side / 2 - std::log2(side) / 2, side / 2);
      std::uniform_real_distribution<double> distG(0, 2 * std::numbers::pi_v<double> - std::numeric_limits<double>::epsilon( ));
      return [=, &gen] mutable {
         double r = distL(gen), g = distG(gen);
         double x = r * std::cos(g), y = r * std::sin(g);
         return point{x + side / 2, y + side / 2};
      };
   }
}

int main( ) try {
   int a, b; char c; int s;
   std::cin >> a >> b >> c >> s;

   std::vector<point> points;
   std::mt19937_64 gen(s);
   double side = 3 * std::sqrt(a + b);
   auto gen1 = make_generator1(side, c, gen);
   auto gen2 = make_generator2(side, c, gen);
   do {
      while (points.size( ) < a) {
         points.emplace_back(gen1( ));
      }
      while (points.size( ) < a + b) {
         points.emplace_back(gen2( ));
      }

      for (int i = 0; i < a + b; ++i) {
         bool remove = [&]{
            if (c == 'E' || a + b >= 1000) {
               return false;
            }
            for (int j = i + 1; j < a + b; ++j) {
               for (int k = j + 1; k < a + b; ++k) {
                  if (collinear(points[i], points[j], points[k])) {
                     return true;
                  }
               }
            }
            return false;
         }( );
         if (remove) {
            points[i] = { -1, -1 };
         }
      }
      std::erase(points, point{-1, -1});
   } while (points.size( ) != a + b);

   std::cout << a << " " << b << "\n";
   for (point p : points) {
      std::cout << p.x << " " << p.y << "\n";
   }
} catch (...) {
   return -1;
}
