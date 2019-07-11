/*
  File : portfolio_5_card.cc

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Implements a basic portfolio optimization model
                 with cardinality constraints on number of assets traded.
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
    Description:
        Extends the basic Markowitz model with cardinality constraints.

    Input:
        n: Number of assets
        mu: An n dimmensional vector of expected returns
        GT: A matrix with n columns so (GT')*GT  = covariance matrix
        x0: Initial holdings
        w: Initial cash holding
        gamma: Maximum risk (=std. dev) accepted
        k: Maximal number of assets in which we allow to change position.

    Output:
       Optimal expected return and the optimal portfolio

*/
std::shared_ptr<ndarray<double, 1>> MarkowitzWithCardinality
                                 ( int n,
                                   std::shared_ptr<ndarray<double, 1>> mu,
                                   std::shared_ptr<ndarray<double, 2>> GT,
                                   std::shared_ptr<ndarray<double, 1>> x0,
                                   double                              w,
                                   double                              gamma,
                                   int                                 k)
{
  // Upper bound on the traded amount
  std::shared_ptr<ndarray<double, 1>> u(new ndarray<double, 1>(shape_t<1>(n), w + sum(x0)));

  Model::t M = new Model("Markowitz portfolio with cardinality constraints");  auto M_ = finally([&]() { M->dispose(); });

  // Defines the variables. No shortselling is allowed.
  Variable::t x = M->variable("x", n, Domain::greaterThan(0.0));

  // Addtional "helper" variables
  Variable::t z = M->variable("z", n, Domain::unbounded());
  // Binary varables
  Variable::t y = M->variable("y", n, Domain::binary());

  //  Maximize expected return
  M->objective("obj", ObjectiveSense::Maximize, Expr::dot(mu, x));

  // The amount invested  must be identical to initial wealth
  M->constraint("budget", Expr::sum(x), Domain::equalsTo(w + sum(x0)));

  // Imposes a bound on the risk
  M->constraint("risk", Expr::vstack( gamma, Expr::mul(GT, x)),
                Domain::inQCone());

  // z >= |x-x0|
  M->constraint("buy", Expr::sub(z, Expr::sub(x, x0)), Domain::greaterThan(0.0));
  M->constraint("sell", Expr::sub(z, Expr::sub(x0, x)), Domain::greaterThan(0.0));

  // Consraints for turning y off and on. z-diag(u)*y<=0 i.e. z_j <= u_j*y_j
  M->constraint("y_on_off", Expr::sub(z, Expr::mul(Matrix::diag(u), y)), Domain::lessThan(0.0));

  // At most k assets change position
  M->constraint("cardinality", Expr::sum(y), Domain::lessThan(k));

  // Integer optimization problems can be very hard to solve so limiting the
  // maximum amount of time is a valuable safe guard
  M->setSolverParam("mioMaxTime", 180.0);
  M->solve();

  return x->level();
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

  std::cout << std::setprecision(4)
            << std::setiosflags(std::ios::scientific);

  std::cout << std::endl
            << "-----------------------------------------------------------------------------------" << std::endl
            << "Markowitz portfolio optimization with cardinality bounds" << std::endl
            << "-----------------------------------------------------------------------------------" << std::endl
            << std::endl;

  double gamma = (*gammas)[0];

  for(int k=1; k<=n; k++)
  {
    auto x = MarkowitzWithCardinality(n, mu, GT, x0, w, (*gammas)[0], k);
    std::cout << "Bound " << k << "  Portfolio: ";
    for (int i = 0; i < n; ++i)
      std::cout << (*x)[i] << " ";
    std::cout << std::endl;
  }

  return 0;
}