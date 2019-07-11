////
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      sospoly.cc
//
//  Purpose:
//  Models the cone of nonnegative polynomials and nonnegative trigonometric
//  polynomials using Nesterov's framework  [1].
//
//  Given a set of coefficients (x0, x1, ..., xn), the functions model the
//  cone of nonnegative polynomials
//
//  P_m = { x \in R^{n+1} | x0 + x1*t + ... xn*t^n >= 0, forall t \in I }
//
//  where I can be the entire real axis, the semi-infinite interval [0,inf), or
//  a finite interval I = [a, b], respectively.
//
//  References:
//
//  [1] "Squared Functional Systems and Optimization Problems",
//      Y. Nesterov, in High Performance Optimization,
//      Kluwer Academic Publishers, 2000.
#include <string>
#include <iostream>
#include <cmath>
#include "fusion.h"
#include <cassert>

using namespace mosek::fusion;
using namespace monty;

std::shared_ptr<ndarray<int, 1>> range (int start, int stop, int step);
std::shared_ptr<ndarray<int, 1>> range (int start, int stop);
std::shared_ptr<ndarray<int, 1>> range (int stop);
std::shared_ptr<ndarray<double, 1>> range (double start, double stop, double step);
std::shared_ptr<ndarray<double, 1>> range (double start, double stop);
std::shared_ptr<ndarray<double, 1>> range (double stop);
template<typename T> std::ostream & operator<<(std::ostream & os, ndarray<T, 1> & a);

// Creates a Hankel matrix of dimension n+1, where
// H_lk = a if l+k=i, and 0 otherwise.
Matrix::t Hankel(int n, int i, double a)
{
  if (i < n + 1)
    return Matrix::sparse(n + 1, n + 1, range(i, -1, -1),      range(i + 1),      a);
  else
    return Matrix::sparse(n + 1, n + 1, range(n, i - n - 1, -1), range(i - n, n + 1), a);
}

// Models the cone of nonnegative polynomials on the real axis
void nn_inf(Model::t M, Variable::t x)
{
  int m = (int)x->getSize() - 1;
  int n = (m / 2); // degree of polynomial is 2*n

  // Setup variables
  Variable::t X = M->variable(Domain::inPSDCone(n + 1));

  // x_i = Tr H(n, i) * X  i=0,...,m
  for (int i = 0; i < m + 1; ++i)
    M->constraint( Expr::sub(x->index(i), Expr::dot(Hankel(n, i, 1.0), X)), Domain::equalsTo(0.0));
}

// Models the cone of nonnegative polynomials on the semi-infinite interval [0,inf)
void nn_semiinf(Model::t M, Variable::t x)
{
  int n = (int)x->getSize() - 1;
  int n1 = n / 2,
      n2 = (n - 1) / 2;

  // Setup variables
  Variable::t X1 = M->variable(Domain::inPSDCone(n1 + 1));
  Variable::t X2 = M->variable(Domain::inPSDCone(n2 + 1));

  // x_i = Tr H(n1, i) * X1 + Tr H(n2,i-1) * X2, i=0,...,n
  for (int i = 0; i < n + 1; ++i)
    M->constraint( Expr::sub(x->index(i),
                             Expr::add(Expr::dot(Hankel(n1, i, 1.0),  X1),
                                       Expr::dot(Hankel(n2, i - 1, 1.0), X2))), Domain::equalsTo(0.0) );
}

// Models the cone of nonnegative polynomials on the finite interval [a,b]
void nn_finite(Model::t M, Variable::t x, double a, double b)
{
  int m = (int)x->getSize() - 1;
  int n = m / 2;

  if (m == 2 * n)
  {
    Variable::t X1 = M->variable(Domain::inPSDCone(n + 1));
    Variable::t X2 = M->variable(Domain::inPSDCone(n));

    // x_i = Tr H(n,i)*X1 + (a+b)*Tr H(n-1,i-1) * X2 - a*b*Tr H(n-1,i)*X2 - Tr H(n-1,i-2)*X2, i=0,...,m
    for (int i = 0; i < m + 1; ++i)
      M->constraint( Expr::sub(x->index(i),
                               Expr::add(Expr::sub(Expr::dot(Hankel(n, i, 1.0),  X1), Expr::dot(Hankel(n - 1, i, a * b), X2)),
                                         Expr::sub(Expr::dot(Hankel(n - 1, i - 1, a + b), X2), Expr::dot(Hankel(n - 1, i - 2, 1.0),  X2)))),
                     Domain::equalsTo(0.0) );
  }
  else
  {
    Variable::t X1 = M->variable(Domain::inPSDCone(n + 1));
    Variable::t X2 = M->variable(Domain::inPSDCone(n + 1));

    // x_i = Tr H(n,i-1)*X1 - a*Tr H(n,i)*X1 + b*Tr H(n,i)*X2 - Tr H(n,i-1)*X2, i=0,...,m
    for (int i = 0; i < m + 1; ++i)
      M->constraint(Expr::sub(x->index(i),
                              Expr::add(Expr::sub(Expr::dot(Hankel(n, i - 1, 1.0),  X1), Expr::dot(Hankel(n, i, a), X1)),
                                        Expr::sub(Expr::dot(Hankel(n, i, b), X2), Expr::dot(Hankel(n, i - 1, 1.0),  X2)))),
                    Domain::equalsTo(0.0) );
  }
}

// returns variables u representing the derivative of
//  x(0) + x(1)*t + ... + x(n)*t^n,
// with u(0) = x(1), u(1) = 2*x(2), ..., u(n-1) = n*x(n).
Variable::t diff(Model::t M, Variable::t x)
{
  int n = (int)x->getSize() - 1;
  Variable::t u = M->variable(n, Domain::unbounded());
  Matrix::t mx = Matrix::dense(n, 1, range(1.0, n + 1.0));
  M->constraint(Expr::sub(u, Expr::mulElm(mx, x->slice(1, n + 1))), Domain::equalsTo(0.0));
  return u;
}

std::shared_ptr<ndarray<double, 1>> fitpoly(ndarray<double, 2> & data, int n)
{
  Model::t M = new Model("smooth poly"); auto _M = finally([&]() { M->dispose(); });

  int m = data.size(0);
  // create an (m x n+1) array, initialize each (i,j) entry with pow(data[i,0],j)
  std::shared_ptr<ndarray<double, 2>> A(new ndarray<double, 2>(shape_t<2>(m, n + 1), std::function<double(const shape_t<2>&)>([&](const shape_t<2>& p) { return std::pow(data(p[0], 0), p[1]); })));
  std::shared_ptr<ndarray<double, 1>> b(new ndarray<double, 1>(m, std::function<double(ptrdiff_t)>([&](ptrdiff_t i) { return data(i, 1); })));

  Variable::t x  = M->variable("x", n + 1, Domain::unbounded());
  Variable::t z  = M->variable("z", 1,   Domain::unbounded());
  Variable::t dx = diff(M, x);

  M->constraint(Expr::mul(A, x), Domain::equalsTo(b));

  // z - f'(t) >= 0, for all t \in [a, b]
  Variable::t ub = M->variable(n, Domain::unbounded());
  M->constraint(Expr::sub(ub,
                          Expr::vstack(Expr::sub(z, dx->index(0)), Expr::neg(dx->slice(1, n)))),
                Domain::equalsTo(0.0));
  nn_finite(M, ub, data(0, 0), data(data.size(0) - 1, 0));

  // f'(t) + z >= 0, for all t \in [a, b]
  Variable::t lb = M->variable(n, Domain::unbounded());
  M->constraint(Expr::sub(lb, Expr::vstack(Expr::add(z, dx->index(0)), dx->slice(1, n))),
                Domain::equalsTo(0.0));
  nn_finite(M, lb, data(0, 0), data(data.size(0) - 1, 0));

  M->objective(ObjectiveSense::Minimize, z);
  M->solve();
  return x->level();
}


int main(int argc, char ** argv)
{
  ndarray<double, 2> data
  { { -1.0, 1.0 },
    {  0.0, 0.0 },
    {  1.0, 1.0 } };

  auto x2 = fitpoly(data, 2);
  auto x4 = fitpoly(data, 4);
  auto x8 = fitpoly(data, 8);

  std::cout << "fitpoly(data,2) -> " << x2 << std::endl;
  std::cout << "fitpoly(data,4) -> " << x4 << std::endl;
  std::cout << "fitpoly(data,8) -> " << x8 << std::endl;
}


// Some utility functions to make things look nicer

template<typename T>
std::ostream & operator<<(std::ostream & os, ndarray<T, 1> & a)
{
  os << "[";
  for (auto v : a) os << " " << v;
  os << std::endl;
  return os;
}


std::shared_ptr<ndarray<int, 1>> range (int start, int stop, int step)
{
  range_t<int> r(start, stop, step);
  return std::shared_ptr<ndarray<int, 1>>(new ndarray<int, 1>(shape_t<1>(r.size()), iterable(r)));
}
std::shared_ptr<ndarray<int, 1>> range (int start, int stop) { return range(start, stop, 1); }
std::shared_ptr<ndarray<int, 1>> range (int stop) { return range(0, stop, 1); }

std::shared_ptr<ndarray<double, 1>> range (double start, double stop, double step)
{
  ptrdiff_t num = ( start < stop && step > 0 ? floor((stop - start) / step) :
                    start > step && step < 0 ? floor((stop - start) / step) :
                    0 );
  auto res = new_array_ptr<double, 1>(num);
  double v  = start;
  auto   it = res->begin();
  for (ptrdiff_t i = 0; i < num; ++i, v += step, ++it)
    * it = v;
  return res;
}
std::shared_ptr<ndarray<double, 1>> range (double start, double stop) { return range(start, stop, 1.0); }
std::shared_ptr<ndarray<double, 1>> range (double stop) { return range(0.0, stop, 1.0); }