#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <gurobi_c++.h>

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

int main( )
try {
   int a, b;
   std::cin >> a >> b;

   std::vector<point> points(a + b);
   for (int i = 0; i < a + b; ++i) {
      std::cin >> points[i].x >> points[i].y;
   }

   GRBEnv env;
   GRBModel model(env);

   std::vector<std::vector<GRBVar>> adjacency(a + b);
   for (int i = 0; i < a; ++i) {
      for (int j = a; j < a + b; ++j) {
         GRBVar var = model.addVar(0, 1, distance(points[i], points[j]), GRB_BINARY);
         adjacency[i].push_back(var);
         adjacency[j].push_back(var);
      }
   }

   for (int i = 0; i < a + b; ++i) {
      GRBLinExpr ex;
      for (GRBVar var : adjacency[i]) {
         ex += var;
      }
      model.addConstr(ex >= 1);
   }

   model.optimize( );
   if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
      std::vector<edge> res;
      for (int i = 0; i < a; ++i) {
         for (int j = a; j < a + b; ++j) {
            if (adjacency[i][j - a].get(GRB_DoubleAttr_X) > 0.5) {
               res.push_back(edge{i, j});
            }
         }
      }
      std::cout << res.size( ) << "\n";
      for (edge current : res) {
         std::cout << current.p1 << " " << current.p2 << "\n";
      }
      std::cout << std::setprecision(9) << std::fixed << model.get(GRB_DoubleAttr_ObjVal) << "\n";
   } else if (model.get(GRB_IntAttr_Status) == GRB_UNBOUNDED) {
      std::cout << "not bounded\n";
   } else if (model.get(GRB_IntAttr_Status) == GRB_INFEASIBLE) {
      std::cout << "infeasible\n";
   } else {
      std::cout << "not handled\n";
   }
} catch (const GRBException& ex) {
   std::cout << ex.getMessage( ) << "\n";
} catch (...) {
   return -1;
}
