/*
  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File :      modelLib.cc

  Purpose: Library of simple building blocks in Mosek Fusion.
*/
#include <iostream>
#include "monty.h"
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

// Duplicate Variable::ts
// x = y
static void dup(Model::t M, Variable::t x, Variable::t y) {
  M->constraint(Expr::sub(x,y), Domain::equalsTo(0.0));
}

// Absolute value
// t >= |x|, where t, x have the same shape
static void abs(Model::t M, Variable::t t, Variable::t x) {
  M->constraint(Expr::add(t,x), Domain::greaterThan(0.0));
  M->constraint(Expr::sub(t,x), Domain::greaterThan(0.0));
}

// 1-norm
// t >= sum( |x_i| ), x is a vector variable
static void norm1(Model::t M, Variable::t t, Variable::t x) {
  Variable::t u = M->variable(x->getShape(), Domain::unbounded());
  abs(M, u, x);
  M->constraint(Expr::sub(t, Expr::sum(u)), Domain::greaterThan(0.0));
}

// Square
// t >= x^2
static void sq(Model::t M, Variable::t t, Variable::t x) {
  M->constraint(Expr::hstack(0.5, t, x), Domain::inRotatedQCone());
}

// 2-norm
// t >= sqrt(x_1^2 + ->->-> + x_n^2) where x is a vector
static void norm2(Model::t M, Variable::t t, Variable::t x) {
  M->constraint(Expr::vstack(t, x), Domain::inQCone());
}

// Power with exponent > 1
// t >= |x|^p (where p>1)
static void pow(Model::t M, Variable::t t, Variable::t x, double p) {
  M->constraint(Expr::hstack(t, 1, x), Domain::inPPowerCone(1.0/p));
}

// Inverse of power 
// t >= 1/|x|^p, x>0 (where p>0)
static void pow_inv(Model::t M, Variable::t t, Variable::t x, double p) {
  M->constraint(Expr::hstack(t, x, 1), Domain::inPPowerCone(1.0/(1.0+p)));
}

// p-norm, p>1
// t >= \|x\|_p (where p>1), x is a vector variable
static void pnorm(Model::t M, Variable::t t, Variable::t x, double p) {
  int n = (int) x->getSize();
  Variable::t r = M->variable(n);
  M->constraint(Expr::sub(t, Expr::sum(r)), Domain::equalsTo(0.0));
  M->constraint(Expr::hstack(Var::repeat(t,n), r, x), Domain::inPPowerCone(1.0-1.0/p));
}

// Geometric mean
// |t| <= (x_1->->->x_n)^(1/n), x_i>=0, x is a vector variable of length >= 1
static void geo_mean(Model::t M, Variable::t t, Variable::t x) {
  int n = (int) x->getSize();
  if (n==1) {
      abs(M, x, t);
  }
  else {
      Variable::t t2 = M->variable();
      M->constraint(Expr::hstack(t2, x->index(n-1), t), Domain::inPPowerCone(1.0-1.0/n));
      geo_mean(M, t2, x->slice(0,n-1));
  }
}

// Logarithm
// t <= log(x), x>=0
static void log(Model::t M, Variable::t t, Variable::t x) {
  M->constraint(Expr::hstack(x, 1, t), Domain::inPExpCone());
}

// Exponential
// t >= exp(x)
static void exp(Model::t M, Variable::t t, Variable::t x) {
  M->constraint(Expr::hstack(t, 1, x), Domain::inPExpCone());
}

// Entropy
// t >= x * log(x), x>=0
static void ent(Model::t M, Variable::t t, Variable::t x) {
  M->constraint(Expr::hstack(1, x, Expr::neg(t)), Domain::inPExpCone());
}

// Relative entropy
// t >= x * log(x/y), x,y>=0
static void relent(Model::t M, Variable::t t, Variable::t x, Variable::t y) {
  M->constraint(Expr::hstack(y, x, Expr::neg(t)), Domain::inPExpCone());
}

// Log-sum-exp
// log( sum_i(exp(x_i)) ) <= t, where x is a vector
static void logsumexp(Model::t M, Variable::t t, Variable::t x) {
  int n = (int) x->getSize();
  Variable::t u = M->variable(n);
  M->constraint(Expr::hstack(u, Expr::constTerm(n, 1.0), Expr::sub(x, Var::repeat(t, n))), Domain::inPExpCone());
  M->constraint(Expr::sum(u), Domain::lessThan(1.0));
}

// Semicontinuous Variable::t
// x = 0 or a <= x <= b
static void semicontinuous(Model::t M, Variable::t x, double a, double b) {
  Variable::t u = M->variable(x->getShape(), Domain::binary());
  M->constraint(Expr::sub(x, Expr::mul(a, u)), Domain::greaterThan(0.0));
  M->constraint(Expr::sub(x, Expr::mul(b, u)), Domain::lessThan(0.0));
}

// Indicator Variable::t
// x!=0 implies t=1. Assumes that |x|<=1 in advance.
static void indicator(Model::t M, Variable::t t, Variable::t x) {
  M->constraint(t, Domain::inRange(0,1));
  t->makeInteger();
  abs(M, t, x);
}

// Logical OR
// x OR y, where x, y are binary
static void logic_or(Model::t M, Variable::t x, Variable::t y) {
  M->constraint(Expr::add(x, y), Domain::greaterThan(1.0));
}
// x_1 OR ->->-> OR x_n, where x is a binary vector
static void logic_or_vect(Model::t M, Variable::t x) {
  M->constraint(Expr::sum(x), Domain::greaterThan(1.0));
}

// SOS1 (NAND)
// at most one of x_1,->->->,x_n, where x is a binary vector (SOS1 constraint)
static void logic_sos1(Model::t M, Variable::t x) {
  M->constraint(Expr::sum(x), Domain::lessThan(1.0));
}
// NOT(x AND y), where x, y are binary
static void logic_nand(Model::t M, Variable::t x, Variable::t y) {
  M->constraint(Expr::add(x, y), Domain::lessThan(1.0));
}

// Cardinality bound
// At most k of entries in x are nonzero, assuming in advance |x_i|<=1.
static void card(Model::t M, Variable::t x, int k) {
  Variable::t t = M->variable(x->getShape(), Domain::binary());
  abs(M, t, x);
  M->constraint(Expr::sum(t), Domain::lessThan(k));
}

// This is just a syntactic test without much sense
static void testModel() {
  Model::t M = new Model();
  Variable::t x = M->variable();
  Variable::t y = M->variable();
  Variable::t t = M->variable();
  Variable::t p = M->variable(5);
  Variable::t a = M->variable(new_array_ptr<int,1>({10,2}));
  Variable::t b = M->variable(new_array_ptr<int,1>({10,2}));
  Variable::t e = M->variable(Domain::binary());
  Variable::t f = M->variable(Domain::binary());

  log(M, t, x);
  exp(M, t, y);
  ent(M, t, x);
  relent(M, t, x, y);
  logsumexp(M, t, p);
  abs(M, a, b);
  norm1(M, t, a);
  sq(M, t, y);
  norm2(M, t, p);
  pow(M, t, x, 1.5);
  pow_inv(M, t, x, 3.3);
  geo_mean(M, t, p);
  semicontinuous(M, y, 1.1, 2.2);
  indicator(M, e, y);
  logic_or(M, e, f);
  logic_nand(M, e, f);
  card(M, b, 5);
}

// A sample problem using functions from the library
//
// max -sqrt(x^2 + y^2) + log(y) - x^1->5
//  st x >= y + 3
//
static void testExample() {
  Model::t M = new Model();
  Variable::t x = M->variable();
  Variable::t y = M->variable();
  Variable::t t = M->variable(3);

  M->constraint(Expr::sub(x, y), Domain::greaterThan(3.0));
  norm2(M, t->index(0), Var::vstack(x,y));
  log  (M, t->index(1), y);
  pow  (M, t->index(2), x, 1.5);

  M->objective(ObjectiveSense::Maximize, Expr::dot(t, new_array_ptr<double,1>({-1,1,-1})));

  M->setLogHandler([](const std::string & msg) { std::cout << msg << std::flush; } );
  M->solve();
}
  
// Full test
int main() {
  testModel();
  testExample();
  return 0;
}