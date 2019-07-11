//
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      sdo1.cc
//
//  Purpose:
//  Solves the mixed semidefinite and conic quadratic optimization problem
//
//                   [2, 1, 0]
//    minimize    Tr [1, 2, 1] * X + x0
//                   [0, 1, 2]
//
//                   [1, 0, 0]
//    subject to  Tr [0, 1, 0] * X + x0           = 1.0
//                   [0, 0, 1]
//
//                   [1, 1, 1]
//                Tr [1, 1, 1] * X      + x1 + x2 = 0.5
//                   [1, 1, 1]
//
//                   X >> 0,  x0 >= (x1^2 + x2^2) ^ (1/2)
//
#include <iostream>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  Model::t M = new Model("sdo1"); auto _M = finally([&]() { M->dispose(); });

  // Setting up the variables
  Variable::t X  = M->variable("X", Domain::inPSDCone(3));
  Variable::t x  = M->variable("x", Domain::inQCone(3));

  // Setting up the constant coefficient matrices
  Matrix::t C  = Matrix::dense ( new_array_ptr<double, 2>({{2., 1., 0.}, {1., 2., 1.}, {0., 1., 2.}}));
  Matrix::t A1 = Matrix::eye(3);
  Matrix::t A2 = Matrix::ones(3, 3);

  // Objective
  M->objective(ObjectiveSense::Minimize, Expr::add(Expr::dot(C, X), x->index(0)));

  // Constraints
  M->constraint("c1", Expr::add(Expr::dot(A1, X), x->index(0)), Domain::equalsTo(1.0));
  M->constraint("c2", Expr::add(Expr::dot(A2, X), Expr::sum(x->slice(1, 3))), Domain::equalsTo(0.5));

  M->solve();

  std::cout << "Solution : " << std::endl;
  std::cout << "  X = " << *(X->level()) << std::endl;
  std::cout << "  x = " << *(x->level()) << std::endl;

  return 0;
}