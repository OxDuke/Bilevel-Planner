//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      ceo1.cc
//
//   Purpose: Demonstrates how to solve the problem
//
//   minimize x1 + x2
//   such that
//            x1 + x2 + x3  = 1.0
//                x1,x2    >= 0.0
//   and      x1 >= x2 * exp(x3/x2)
//
#include <iostream>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  Model::t M = new Model("ceo1"); auto _M = finally([&]() { M->dispose(); });

  Variable::t x  = M->variable("x", 3, Domain::unbounded());

  // Create the constraint
  //      x[0] + x[1] + x[2] = 1.0
  M->constraint("lc", Expr::sum(x), Domain::equalsTo(1.0));

  // Create the exponential conic constraint
  Constraint::t expc = M->constraint("expc", x, Domain::inPExpCone());

  // Set the objective function to (x[0] + x[1])
  M->objective("obj", ObjectiveSense::Minimize, Expr::sum(x->slice(0,2)));

  // Solve the problem
  M->solve();

  // Get the linear solution values
  ndarray<double, 1> xlvl   = *(x->level());
  // Get conic solution of expc1
  ndarray<double, 1> expclvl = *(expc->level());
  ndarray<double, 1> expcdl  = *(expc->dual());

  std::cout << "x1,x2,x3 = " << xlvl << std::endl;
  std::cout << "expc levels = " << expclvl << std::endl;
  std::cout << "expc dual conic var levels = " << expcdl << std::endl;
}