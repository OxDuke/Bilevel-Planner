//
//    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//    File:    mico1.cc
//
//    Purpose:  Demonstrates how to solve a small mixed
//              integer conic optimization problem.
//
//              minimize    x^2 + y^2
//              subject to  x >= e^y + 3.8
//                          x, y - integer
//
#include <iostream>
#include <iomanip>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  Model::t M = new Model("mico1"); auto _M = finally([&]() { M->dispose(); });

  Variable::t x = M->variable(Domain::integral(Domain::unbounded()));
  Variable::t y = M->variable(Domain::integral(Domain::unbounded()));
  Variable::t t = M->variable();
  
  M->constraint(Expr::vstack(t, x, y), Domain::inQCone());
  M->constraint(Expr::vstack(Expr::sub(x, 3.8), 1, y), Domain::inPExpCone());

  M->objective(ObjectiveSense::Minimize, t);

  M->solve();

  std::cout << std::setprecision(2)
            << "x = " << (*(x->level()))[0] << std::endl
            << "y = " << (*(y->level()))[0] << std::endl ;

  return 0;
}