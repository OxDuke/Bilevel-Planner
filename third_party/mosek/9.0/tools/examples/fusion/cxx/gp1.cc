//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      gp1.cc
//
//   Purpose:   Demonstrates how to solve a simple Geometric Program (GP)
//              cast into conic form with exponential cones and log-sum-exp.
//
//              Example from
//                https://gpkit.readthedocs.io/en/latest/examples.html//maximizing-the-volume-of-a-box
//
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

// Models log(sum(exp(Ax+b))) <= 0.
// Each row of [A b] describes one of the exp-terms
void logsumexp(Model::t                             M, 
               std::shared_ptr<ndarray<double, 2>>  A, 
               Variable::t                          x,
               std::shared_ptr<ndarray<double, 1>>  b)
{
  int k = A->size(0);
  auto u = M->variable(k);
  M->constraint(Expr::sum(u), Domain::equalsTo(1.0));
  M->constraint(Expr::hstack(u,
                             Expr::constTerm(k, 1.0),
                             Expr::add(Expr::mul(A, x), b)), Domain::inPExpCone());
}

// maximize     h*w*d
// subjecto to  2*(h*w + h*d) <= Awall
//              w*d <= Afloor
//              alpha <= h/w <= beta
//              gamma <= d/w <= delta
//
// Variable substitutions:  h = exp(x), w = exp(y), d = exp(z).
//
// maximize     x+y+z
// subject      log( exp(x+y+log(2/Awall)) + exp(x+z+log(2/Awall)) ) <= 0
//                              y+z <= log(Afloor)
//              log( alpha ) <= x-y <= log( beta )
//              log( gamma ) <= z-y <= log( delta )
std::shared_ptr<ndarray<double, 1>> max_volume_box(double Aw, double Af, 
                                                   double alpha, double beta, double gamma, double delta)
{
  Model::t M = new Model("max_vol_box"); auto _M = finally([&]() { M->dispose(); });

  auto xyz = M->variable(3);
  M->objective("Objective", ObjectiveSense::Maximize, Expr::sum(xyz));
    
  logsumexp(M, 
            new_array_ptr<double,2>({{1,1,0}, {1,0,1}}), 
            xyz, 
            new_array_ptr<double,1>({log(2.0/Aw), log(2.0/Aw)}));
    
  M->constraint(Expr::dot(new_array_ptr<double,1>({0,1,1}), xyz), Domain::lessThan(log(Af)));
  M->constraint(Expr::dot(new_array_ptr<double,1>({1,-1,0}), xyz), Domain::inRange(log(alpha),log(beta)));
  M->constraint(Expr::dot(new_array_ptr<double,1>({0,-1,1}), xyz), Domain::inRange(log(gamma),log(delta)));
    
  M->setLogHandler([](const std::string & msg) { std::cout << msg << std::flush; } );
  M->solve();
  
  return std::make_shared<ndarray<double, 1>>(shape(3), [xyz](ptrdiff_t i) { return exp((*(xyz->level()))[i]); });
}

int main()
{
  double Aw    = 200.0;
  double Af    = 50.0;
  double alpha = 2.0;
  double beta  = 10.0;
  double gamma = 2.0;
  double delta = 10.0;
  
  auto hwd = max_volume_box(Aw, Af, alpha, beta, gamma, delta);

  std::cout << std::setprecision(4);
  std::cout << "h=" << (*hwd)[0] << " w=" << (*hwd)[1] << " d=" << (*hwd)[2] << std::endl;
}