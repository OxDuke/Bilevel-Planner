// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      logistic.cc
//
// Purpose: Implements logistic regression with regulatization.
//
//          Demonstrates using the exponential cone and log-sum-exp in Fusion.

#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <utility>
#include "fusion.h"
using namespace mosek::fusion;
using namespace monty;

// t >= log( 1 + exp(u) ) coordinatewise
void softplus(Model::t      M,
              Expression::t t,
              Expression::t u)
{
    int n = (*t->getShape())[0];
    auto z1 = M->variable(n);
    auto z2 = M->variable(n);
    M->constraint(Expr::add(z1, z2), Domain::equalsTo(1));
    M->constraint(Expr::hstack(z1, Expr::constTerm(n, 1.0), Expr::sub(u,t)), Domain::inPExpCone());
    M->constraint(Expr::hstack(z2, Expr::constTerm(n, 1.0), Expr::neg(t)), Domain::inPExpCone());
}

// Model logistic regression (regularized with full 2-norm of theta)
// X - n x d matrix of data points
// y - length n vector classifying training points
// lamb - regularization parameter
std::pair<Model::t, Variable::t>
logisticRegression(std::vector<std::vector<double>> & X, 
                   std::vector<bool>                & y,
                   double                           lamb)
{
    int n = X.size();
    int d = X[0].size();       // num samples, dimension
    
    Model::t M = new Model();   

    auto theta = M->variable(d);
    auto t     = M->variable(n);
    auto reg   = M->variable();

    M->objective(ObjectiveSense::Minimize, Expr::add(Expr::sum(t), Expr::mul(lamb,reg)));
    M->constraint(Var::vstack(reg, theta), Domain::inQCone());

    auto signs = std::make_shared<ndarray<double,1>>(shape(n), [y](ptrdiff_t i) { return y[i] ? -1 : 1; });   

    softplus(M, t, Expr::mulElm(Expr::mul(new_array_ptr<double>(X), theta), signs));

    return std::make_pair(M, theta);
}


int main(int arc, char** argv)
{

  // Test: detect and approximate a circle using degree 2 polynomials
  std::vector<std::vector<double>> X;
  std::vector<bool> Y;

  for(double x = -1; x <= 1; x += 1.0/15)
    for(double y = -1; y <= 1; y += 1.0/15) 
  {
    std::vector<double> p;
    p.push_back(1.0); p.push_back(x); p.push_back(y); p.push_back(x*y);
    p.push_back(x*x); p.push_back(y*y);
    X.push_back(p);
    Y.push_back(x*x+y*y>=0.69);
  }

  auto res = logisticRegression(X, Y, 0.1);
  auto M = res.first;
  auto theta = res.second;

  M->setLogHandler([](const std::string & msg) { std::cout << msg << std::flush; });
  M->solve();
  std::cout << *(theta->level());
  M->dispose();

  return 0;
}