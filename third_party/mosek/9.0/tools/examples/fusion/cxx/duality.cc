//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      duality.cc
//
//   Purpose: A small example of how to access dual values.

#include <memory>
#include <iostream>
#include <iomanip>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  auto A = new_array_ptr<double, 2>({ { -0.5, 1.0 }  });
  auto b = new_array_ptr<double, 1>({ 1.0 });
  auto c = new_array_ptr<double, 1>({ 1.0, 1.0 });

  Model::t M = new Model("duality"); auto _M = finally([&]() { M->dispose(); });

  Variable::t x = M->variable("x", 2, Domain::greaterThan(0.0));

  Constraint::t con = M->constraint(Expr::sub(Expr::mul(A, x), b), Domain::equalsTo(0.0));

  M->objective("obj", ObjectiveSense::Minimize, Expr::dot(c, x));

  M->solve();
  auto xsol = x->level();
  auto ssol = x->dual();
  auto ysol = con->dual();

  std::cout << std::setiosflags(std::ios::scientific)
            << "x1 = " << (*xsol)[0] << std::endl
            << "x2 = " << (*xsol)[1] << std::endl
            << "s1 = " << (*ssol)[0] << std::endl
            << "s1 = " << (*ssol)[1] << std::endl
            << "y =  " << (*ysol)[0] << std::endl;
}