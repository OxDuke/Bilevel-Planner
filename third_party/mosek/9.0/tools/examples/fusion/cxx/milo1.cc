//
//    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//    File:    milo1.cc
//
//    Purpose:  Demonstrates how to solve a small mixed
//              integer linear optimization problem.
//
#include <iostream>
#include <iomanip>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  auto a1 = new_array_ptr<double, 1>({ 50.0, 31.0 });
  auto a2 = new_array_ptr<double, 1>({ 3.0,  -2.0 });
  auto c  = new_array_ptr<double, 1>({  1.0, 0.64 });

  Model::t M = new Model("milo1"); auto _M = finally([&]() { M->dispose(); });
  Variable::t x = M->variable("x", 2, Domain::integral(Domain::greaterThan(0.0)));

  // Create the constraints
  //      50.0 x[0] + 31.0 x[1] <= 250.0
  //       3.0 x[0] -  2.0 x[1] >= -4.0
  M->constraint("c1", Expr::dot(a1, x), Domain::lessThan(250.0));
  M->constraint("c2", Expr::dot(a2, x), Domain::greaterThan(-4.0));

  // Set max solution time
  M->setSolverParam("mioMaxTime", 60.0);
  // Set max relative gap (to its default value)
  M->setSolverParam("mioTolRelGap", 1e-4);
  // Set max absolute gap (to its default value)
  M->setSolverParam("mioTolAbsGap", 0.0);

  // Set the objective function to (c^T * x)
  M->objective("obj", ObjectiveSense::Maximize, Expr::dot(c, x));

  // Solve the problem
  M->solve();

  // Get the solution values
  auto sol = x->level();
  std::cout << std::setiosflags(std::ios::scientific) << std::setprecision(2)
            << "x1 = " << (*sol)[0] << std::endl
            << "x2 = " << (*sol)[1] << std::endl
            << "MIP rel gap = " << M->getSolverDoubleInfo("mioObjRelGap") << " (" << M->getSolverDoubleInfo("mioObjAbsGap") << ")" << std::endl;
}