/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      lownerjohn_ellipsoid.cc

  Purpose:
  Computes the Lowner-John inner and outer ellipsoidal
  approximations of a polytope.

  References:
    [1] "Lectures on Modern Optimization", Ben-Tal and Nemirovski, 2000.
    [2] "MOSEK modeling manual", 2013
*/

#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "fusion.h"
#include <cassert>

using namespace mosek::fusion;
using namespace monty;

/**
Models the convex set

  S = { (x, t) \in R^n x R | x >= 0, t <= (x1 * x2 * ... * xn)^(1/n) }

using three-dimensional power cones
*/
void geometric_mean(Model::t M, Variable::t x, Variable::t t)
{
  int n = (int) x->getSize();
  if (n==1)
      M->constraint(Expr::sub(t, x), Domain::lessThan(0.0));
  else
  {
    Variable::t t2 = M->variable();
    M->constraint(Var::hstack(t2, x->index(n-1), t), Domain::inPPowerCone(1-1.0/n));
    geometric_mean(M, x->slice(0,n-1), t2);
  }
}

/**
 Purpose: Models the hypograph of the n-th power of the
 determinant of a positive definite matrix. See [1,2] for more details.

   The convex set (a hypograph)

   C = { (X, t) \in S^n_+ x R |  t <= det(X)^{1/n} },

   can be modeled as the intersection of a semidefinite cone

   [ X, Z; Z^T Diag(Z) ] >= 0

   and a number of rotated quadratic cones and affine hyperplanes,

   t <= (Z11*Z22*...*Znn)^{1/n}  (see geometric_mean).
*/
Variable::t det_rootn(Model::t M, Variable::t t, int n)
{
  // Setup variables
  Variable::t Y = M->variable(Domain::inPSDCone(2 * n));

  Variable::t X   = Y->slice(new_array_ptr<int,1>({0, 0}), new_array_ptr<int,1>({n, n}));
  Variable::t Z   = Y->slice(new_array_ptr<int,1>({0, n}), new_array_ptr<int,1>({n, 2 * n}));
  Variable::t DZ  = Y->slice(new_array_ptr<int,1>({n, n}), new_array_ptr<int,1>({2 * n, 2 * n}));

  // Z is lower-triangular
  std::shared_ptr<ndarray<int,2>> low_tri( new ndarray<int,2>( shape_t<2>(n*(n-1)/2, 2) )); 
  int k = 0;
  for(int i = 0; i < n; i++)
    for(int j = i+1; j < n; j++)
       (*low_tri)(k,0) = i, (*low_tri)(k,1) = j, ++k;
  M->constraint(Z->pick(low_tri), Domain::equalsTo(0.0));
  // DZ = Diag(Z)
  M->constraint(Expr::sub(DZ, Expr::mulElm(Z, Matrix::eye(n))), Domain::equalsTo(0.0));

  // t^n <= (Z11*Z22*...*Znn)
  geometric_mean(M, DZ->diag(), t);

  // Return an n x n PSD variable which satisfies t <= det(X)^(1/n)
  return X;
}

/**
  The inner ellipsoidal approximation to a polytope

     S = { x \in R^n | Ax < b }.

  maximizes the volume of the inscribed ellipsoid,

     { x | x = C*u + d, || u ||_2 <= 1 }.

  The volume is proportional to det(C)^(1/n), so the
  problem can be solved as

    maximize         t
    subject to       t       <= det(C)^(1/n)
                || C*ai ||_2 <= bi - ai^T * d,  i=1,...,m
                C is PSD

  which is equivalent to a mixed conic quadratic and semidefinite
  programming problem.
*/
std::pair<std::shared_ptr<ndarray<double, 1>>, std::shared_ptr<ndarray<double, 1>>>
    lownerjohn_inner
    ( std::shared_ptr<ndarray<double, 2>> A,
      std::shared_ptr<ndarray<double, 1>> b)
{
  Model::t M = new Model("lownerjohn_inner"); auto _M = finally([&]() { M->dispose(); });
  int m = A->size(0);
  int n = A->size(1);

  // Setup variables
  Variable::t t = M->variable("t", 1, Domain::greaterThan(0.0));
  Variable::t C = det_rootn(M, t, n);
  Variable::t d = M->variable("d", n, Domain::unbounded());

  // quadratic cones
  M->constraint(Expr::hstack(Expr::sub(b, Expr::mul(A, d)), Expr::mul(A, C)),
                Domain::inQCone());

  // Objective: Maximize t
  M->objective(ObjectiveSense::Maximize, t);
  M->solve();

  return std::make_pair(C->level(), d->level());
}

/**
  The outer ellipsoidal approximation to a polytope given
  as the convex hull of a set of points

    S = conv{ x1, x2, ... , xm }

  minimizes the volume of the enclosing ellipsoid,

    { x | || P*x-c ||_2 <= 1 }

  The volume is proportional to det(P)^{-1/n}, so the problem can
  be solved as

    maximize         t
    subject to       t       <= det(P)^(1/n)
                || P*xi - c ||_2 <= 1,  i=1,...,m
                P is PSD.
*/
std::pair<std::shared_ptr<ndarray<double, 1>>, std::shared_ptr<ndarray<double, 1>>>
    lownerjohn_outer(std::shared_ptr<ndarray<double, 2>> x)
{
  Model::t M = new Model("lownerjohn_outer");
  int m = x->size(0);
  int n = x->size(1);

  // Setup variables
  Variable::t t = M->variable("t", 1, Domain::greaterThan(0.0));
  Variable::t P = det_rootn(M, t, n);
  Variable::t c = M->variable("c", n, Domain::unbounded());

  // (1, Px-c) \in Q
  M->constraint(Expr::hstack(
                  Expr::ones(m), Expr::sub(Expr::mul(x, P),
                      Var::reshape(Var::repeat(c, m), new_array_ptr<int, 1>({m, n}))) ),
                Domain::inQCone());

  // Objective: Maximize t
  M->objective(ObjectiveSense::Maximize, t);
  M->solve();

  return std::make_pair(P->level(), c->level());
}

std::ostream & operator<<(std::ostream & os, ndarray<double, 1> & a)
{
  os << "[ ";
  if (a.size() > 0)
  {
    os << a[0];
    for (int i = 1; i < a.size(); ++i)
      os << "," << a[i];
  }
  os << " ]";
  return os;
}

/******************************************************************************************/
int main(int argc, char ** argv)
{
  //Vertices of a pentagon in 2D
  int n = 6;
  std::shared_ptr<ndarray<double, 2>> p =
  new_array_ptr<double, 2>({ {0., 0.}, {1., 3.}, {5.5, 4.5}, {7., 4.}, {7., 1.}, {3., -2.} });

  //The h-representation of that polygon
  std::shared_ptr<ndarray<double, 2>> A(
                                     new ndarray<double, 2>(shape_t<2>(n, 2), std::function<double(const shape_t<2> &)>(
                                         [&](const shape_t<2> & ij)
  { int i = ij[0], j = ij[1];
    if (j == 0)
      return -((*p)(i, 1)) + (*p)((i - 1 + n) % n, 1);
    else
      return  ((*p)(i, 0)) - (*p)((i - 1 + n) % n, 0);
  })));
  std::shared_ptr<ndarray<double, 1>> b(
                                     new ndarray<double, 1>(n, std::function<double(ptrdiff_t)>( [&](ptrdiff_t i)
  { return (*A)(i, 0) * (*p)(i, 0) + (*A)(i, 1) * (*p)(i, 1); } )));

  auto Cd = lownerjohn_inner(A, b);
  auto Pc = lownerjohn_outer(p);

  std::cout << "Inner:" << std::endl;
  std::cout << "  C = " << *Cd.first << std::endl;
  std::cout << "  d = " << *Cd.second << std::endl;
  std::cout << "Outer:" << std::endl;
  std::cout << "  P = " << *Pc.first << std::endl;
  std::cout << "  c = " << *Pc.second << std::endl;
}