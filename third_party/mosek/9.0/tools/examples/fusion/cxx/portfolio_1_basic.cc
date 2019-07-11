/*
  File : portfolio_1_basic.cc

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :
    Implements a basic portfolio optimization model.
*/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "monty.h"
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

static double sum(std::shared_ptr<ndarray<double, 1>> x)
{
  double r = 0.0;
  for (auto v : *x) r += v;
  return r;
}

static double dot(std::shared_ptr<ndarray<double, 1>> x,
                  std::shared_ptr<ndarray<double, 1>> y)
{
  double r = 0.0;
  for (int i = 0; i < x->size(); ++i) r += (*x)[i] * (*y)[i];
  return r;
}

static double dot(std::shared_ptr<ndarray<double, 1>> x,
                  std::vector<double> & y)
{
  double r = 0.0;
  for (int i = 0; i < x->size(); ++i) r += (*x)[i] * y[i];
  return r;
}

/*
Purpose:
    Computes the optimal portfolio for a given risk

Input:
    n: Number of assets
    mu: An n dimmensional vector of expected returns
    GT: A matrix with n columns so (GT')*GT  = covariance matrix
    x0: Initial holdings
    w: Initial cash holding
    gamma: Maximum risk (=std. dev) accepted

Output:
    Optimal expected return and the optimal portfolio
*/
double BasicMarkowitz
( int                                n,
  std::shared_ptr<ndarray<double, 1>> mu,
  std::shared_ptr<ndarray<double, 2>> GT,
  std::shared_ptr<ndarray<double, 1>> x0,
  double                             w,
  double                             gamma)
{
  Model::t M = new Model("Basic Markowitz"); auto _M = finally([&]() { M->dispose(); });
  // Redirect log output from the solver to stdout for debugging.
  // M->setLogHandler([](const std::string & msg) { std::cout << msg << std::flush; } );

  // Defines the variables (holdings). Shortselling is not allowed.
  Variable::t x = M->variable("x", n, Domain::greaterThan(0.0));

  //  Maximize expected return
  M->objective("obj", ObjectiveSense::Maximize, Expr::dot(mu, x));

  // The amount invested  must be identical to intial wealth
  M->constraint("budget", Expr::sum(x), Domain::equalsTo(w + sum(x0)));

  // Imposes a bound on the risk
  M->constraint("risk", Expr::vstack(gamma, Expr::mul(GT, x)), Domain::inQCone());

  // Solves the model.
  M->solve();

  return dot(mu, x->level());
}


/*
  The example reads in data and solves the portfolio models.
 */
int main(int argc, char ** argv)
{

  int        n      = 3;
  double     w      = 1.0;
  auto       mu     = new_array_ptr<double, 1>( {0.1073, 0.0737, 0.0627} );
  auto       x0     = new_array_ptr<double, 1>({0.0, 0.0, 0.0});
  auto       gammas = new_array_ptr<double, 1>({0.035, 0.040, 0.050, 0.060, 0.070, 0.080, 0.090});
  auto       GT     = new_array_ptr<double, 2>({
    { 0.166673333200005, 0.0232190712557243 ,  0.0012599496030238 },
    { 0.0              , 0.102863378954911  , -0.00222873156550421},
    { 0.0              , 0.0                ,  0.0338148677744977 }
  });

  std::cout << std::endl << std::endl
            << "================================" << std::endl
            << "Markowitz portfolio optimization" << std::endl
            << "================================" << std::endl;

  std::cout << std::endl
            << "-----------------------------------------------------------------------------------" << std::endl
            << "Basic Markowitz portfolio optimization" << std::endl
            << "-----------------------------------------------------------------------------------" << std::endl;

  std::cout << std::setprecision(4)
            << std::setiosflags(std::ios::scientific);

  for (auto gamma : *gammas)
    std::cout << "Expected return: " << BasicMarkowitz( n, mu, GT, x0, w, gamma) << " St deviation: " << gamma << std::endl;

  return 0;
}