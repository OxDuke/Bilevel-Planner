//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      cqo1.cc
//
//   Purpose: Demonstrates how to solve the problem
//
//   minimize y1 + y2 + y3
//   such that
//            x1 + x2 + 2.0 x3  = 1.0
//                    x1,x2,x3 >= 0.0
//   and
//            (y1,x1,x2) in C_3,
//            (y2,y3,x3) in K_3
//
//   where C_3 and K_3 are respectively the quadratic and
//   rotated quadratic cone of size 3 defined as
//       C_3 = { z1,z2,z3 :      z1 >= sqrt(z2^2 + z3^2) }
//       K_3 = { z1,z2,z3 : 2 z1 z2 >= z3^2              }
//
#include <iostream>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  Model::t M = new Model("cqo1"); auto _M = finally([&]() { M->dispose(); });

  Variable::t x  = M->variable("x", 3, Domain::greaterThan(0.0));
  Variable::t y  = M->variable("y", 3, Domain::unbounded());

  // Create the aliases
  //      z1 = [ y[0],x[0],x[1] ]
  //  and z2 = [ y[1],y[2],x[2] ]
  Variable::t z1 = Var::vstack(y->index(0),  x->slice(0, 2));
  Variable::t z2 = Var::vstack(y->slice(1, 3), x->index(2));

  // Create the constraint
  //      x[0] + x[1] + 2.0 x[2] = 1.0
  auto aval = new_array_ptr<double, 1>({1.0, 1.0, 2.0});
  M->constraint("lc", Expr::dot(aval, x), Domain::equalsTo(1.0));

  // Create the constraints
  //      z1 belongs to C_3
  //      z2 belongs to K_3
  // where C_3 and K_3 are respectively the quadratic and
  // rotated quadratic cone of size 3, i.e.
  //                 z1[0] >= sqrt(z1[1]^2 + z1[2]^2)
  //  and  2.0 z2[0] z2[1] >= z2[2]^2
  Constraint::t qc1 = M->constraint("qc1", z1, Domain::inQCone());
  Constraint::t qc2 = M->constraint("qc2", z2, Domain::inRotatedQCone());

  // Set the objective function to (y[0] + y[1] + y[2])
  M->objective("obj", ObjectiveSense::Minimize, Expr::sum(y));

  // Solve the problem
  M->solve();

  // Get the linear solution values
  ndarray<double, 1> xlvl   = *(x->level());
  ndarray<double, 1> ylvl   = *(y->level());
  // Get conic solution of qc1
  ndarray<double, 1> qc1lvl = *(qc1->level());
  ndarray<double, 1> qc1dl  = *(qc1->dual());

  std::cout << "x1,x2,x2 = " << xlvl << std::endl;
  std::cout << "y1,y2,y3 = " << ylvl << std::endl;
  std::cout << "qc1 levels = " << qc1lvl << std::endl;
  std::cout << "qc1 dual conic var levels = " << qc1dl << std::endl;
}