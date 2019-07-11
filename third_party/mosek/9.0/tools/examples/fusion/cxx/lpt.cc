//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      lpt.cc
//
// Purpose:  Demonstrates how to solve the multi-processor
//           scheduling problem using the Fusion API.


#include <iostream>
#include <random>
#include <sstream>

#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;


int main(int arc, char** argv)
{
  double lb = 1.0;          //Bounds for the length of a short task
  double ub = 5.;

  int     n = 30;           //Number of tasks
  int     m = 6;            //Number of processors

  double  sh = 0.8;         //The proportion of short tasks
  int     n_short = (int)(sh * n);
  int     n_long = n - n_short;

  auto gen = std::bind(std::uniform_real_distribution<double>(lb, ub), std::mt19937(0));

  auto T = std::shared_ptr<ndarray<double, 1> >(new ndarray<double, 1>(n));
  for (int i = 0; i < n_short; i++) (*T)[i] = gen();
  for (int i = n_short; i < n; i++) (*T)[i] = 20 * gen();
  std::sort(T->begin(), T->end(), std::greater<double>());

  Model::t M = new Model("Multi-processor scheduling"); auto _M = finally([&]() { M->dispose(); });

  Variable::t x = M->variable("x", new_array_ptr<int, 1>({m, n}), Domain::binary());
  Variable::t t = M->variable("t", 1, Domain::unbounded());

  M->constraint( Expr::sum(x, 0), Domain::equalsTo(1.) );
  M->constraint( Expr::sub( Var::repeat(t, m), Expr::mul(x, T) ) , Domain::greaterThan(0.) );

  M->objective( ObjectiveSense::Minimize, t );

  //LPT heuristic
  auto schedule = std::shared_ptr<ndarray<double, 1> >(new ndarray<double, 1>(m, 0.));
  auto init = std::shared_ptr<ndarray<double, 1> >(new ndarray<double, 1>(n * m, 0.));

  for (int i = 0; i < n; i++)
  {
    auto pos = std::distance(schedule->begin(), std::min_element(schedule->begin(), schedule->end()));
    (*schedule)[pos] += (*T)[i];
    (*init)[pos * n + i] = 1;
  }

  //Comment this line to switch off feeding in the initial LPT solution
  x->setLevel(init);

  M->setLogHandler([ = ](const std::string & msg) { std::cout << msg << std::flush; } );

  M->setSolverParam("mioTolRelGap", .01);
  M->solve();

  std::cout << "initial solution: \n";
  for (int i = 0; i < m; i++)
  {
    std::cout << "M " << i << " [";
    for (int y = 0; y < n; y++)
      std::cout << int( (*init)[i * n + y] ) << ", ";
    std::cout << "]\n";
  }

  std::cout << "MOSEK solution:\n";
  for (int i = 0; i < m; i++)
  {
    std::cout << "M " << i << " [";
    for (int y = 0; y < n; y++)
      std::cout << int((*(x->index(i, y)->level()))[0]) << ", ";
    std::cout << "]\n";
  }

  return 0;
}