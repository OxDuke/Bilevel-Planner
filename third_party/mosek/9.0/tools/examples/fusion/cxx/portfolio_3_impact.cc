/*
  File : portfolio_3_impact.cc

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Implements a basic portfolio optimization model
                 with transaction costs of order x^(3/2).
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
        Extends the basic Markowitz model with a market cost term.

    Input:
        n: Number of assets
        mu: An n dimmensional vector of expected returns
        GT: A matrix with n columns so (GT')*GT  = covariance matrix'
        x0: Initial holdings
        w: Initial cash holding
        gamma: Maximum risk (=std. dev) accepted
        m: It is assumed that  market impact cost for the j'th asset is
           m_j|x_j-x0_j|^3/2

    Output:
       Optimal expected return and the optimal portfolio

*/
void MarkowitzWithMarketImpact
( int n,
  std::shared_ptr<ndarray<double, 1>> mu,
  std::shared_ptr<ndarray<double, 2>> GT,
  std::shared_ptr<ndarray<double, 1>> x0,
  double      w,
  double      gamma,
  std::shared_ptr<ndarray<double, 1>> m,
  std::vector<double> & xsol,
  std::vector<double> & tsol)
{
  Model::t M = new Model("Markowitz portfolio with market impact");  auto M_ = finally([&]() { M->dispose(); });

  // Defines the variables. No shortselling is allowed.
  Variable::t x = M->variable("x", n, Domain::greaterThan(0.0));

  // Variables computing the impact cost
  Variable::t t = M->variable("t", n, Domain::unbounded());

  //  Maximize expected return
  M->objective("obj", ObjectiveSense::Maximize, Expr::dot(mu, x));

  // Invested amount + slippage cost = initial wealth
  M->constraint("budget", Expr::add(Expr::sum(x), Expr::dot(m, t)), Domain::equalsTo(w + sum(x0)));

  // Imposes a bound on the risk
  M->constraint("risk", Expr::vstack( gamma, Expr::mul(GT, x)),
                Domain::inQCone());

  // t >= |x-x0|^1.5, using a power cone
  M->constraint("tz", Expr::hstack(t, Expr::constTerm(n, 1.0), Expr::sub(x, x0)), Domain::inPPowerCone(2.0/3.0));

  M->solve();

  xsol.resize(n);
  tsol.resize(n);
  auto xlvl = x->level();
  auto tlvl = t->level();

  std::copy(xlvl->flat_begin(), xlvl->flat_end(), xsol.begin());
  std::copy(tlvl->flat_begin(), tlvl->flat_end(), tsol.begin());
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

  // Somewhat arbirtrary choice of m
  std::shared_ptr<ndarray<double, 1>> m(new ndarray<double, 1>(shape_t<1>(n), 1.0e-2));
  double gamma = (*gammas)[0];
  std::vector<double> x;
  std::vector<double> t;

  MarkowitzWithMarketImpact(n, mu, GT, x0, w, gamma, m, x, t);

  std::cout << std::resetiosflags(std::ios::left);
  std::cout << std::endl
            << "-----------------------------------------------------------------------------------" << std::endl
            << "Markowitz portfolio optimization with market impact cost" << std::endl
            << "-----------------------------------------------------------------------------------" << std::endl
            << std::endl
            << "Expected return: " << dot(mu, x) << " St deviation: " << gamma << " Market impact cost: " << dot(m, t) << std::endl;

  return 0;
}