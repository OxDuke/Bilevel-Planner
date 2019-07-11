//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      pow1.cc
//
//   Purpose: Demonstrates how to solve the problem
//
//     maximize x^0.2*y^0.8 + z^0.4 - x
//           st x + y + 0.5z = 2
//              x,y,z >= 0
//
#include <iostream>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  Model::t M = new Model("pow1"); auto _M = finally([&]() { M->dispose(); });

  Variable::t x  = M->variable("x", 3, Domain::unbounded());
  Variable::t x3 = M->variable();
  Variable::t x4 = M->variable();

  // Create the linear constraint
  auto aval = new_array_ptr<double, 1>({1.0, 1.0, 0.5});
  M->constraint(Expr::dot(x, aval), Domain::equalsTo(2.0));

  // Create the conic constraints
  M->constraint(Var::vstack(x->slice(0,2), x3), Domain::inPPowerCone(0.2));
  M->constraint(Expr::vstack(x->index(2), 1.0, x4), Domain::inPPowerCone(0.4));     

  auto cval = new_array_ptr<double, 1>({1.0, 1.0, -1.0});
  M->objective(ObjectiveSense::Maximize, Expr::dot(cval, Var::vstack(x3, x4, x->index(0))));

  // Solve the problem
  M->solve();

  // Get the linear solution values
  ndarray<double, 1> xlvl   = *(x->level());
  std::cout << "x,y,z = " << xlvl << std::endl;
}