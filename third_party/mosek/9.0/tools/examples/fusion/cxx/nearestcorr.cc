/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      nearestcorr.cc

   Purpose:
   Solves the nearest correlation matrix problem

     minimize   || A - X ||_F   s.t.  diag(X) = e, X is PSD

   as the equivalent conic program

     minimize     t

     subject to   (t, vec(A-X)) in Q
                  diag(X) = e
                  X >= 0
*/
#include <iostream>
#include <iomanip>
#include <cmath>

#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

//-------------------------------------

void nearestcorr ( std::shared_ptr<ndarray<double, 2>>  A);

void nearestcorr_nn (
  std::shared_ptr<ndarray<double, 2>>  A,
  const std::vector<double>         & gammas,
  std::vector<double>         & res,
  std::vector<double>         & rank);

Expression::t vec(Expression::t e);

void print_mat(std::ostream & os, const std::shared_ptr<ndarray<double, 1>> & a);

int main(int argc, char ** argv)
{
  int N = 5;
  auto A = new_array_ptr<double, 2>(
  { { 0.0,  0.5,  -0.1,  -0.2,   0.5},
    { 0.5,  1.25, -0.05, -0.1,   0.25},
    { -0.1, -0.05,  0.51,  0.02, -0.05},
    { -0.2, -0.1,   0.02,  0.54, -0.1},
    { 0.5,  0.25, -0.05, -0.1,   1.25}
  });

  nearestcorr(A);

  std::vector<double> gammas(11); for (int i = 0; i < gammas.size(); ++i) gammas[i] = 0.1 * i;
  std::vector<double> res(gammas.size());
  std::vector<double> rank(gammas.size());

  nearestcorr_nn(A,
                 gammas,
                 res,
                 rank);

  std::cout << std::setprecision(2) << std::fixed;
  for (int i = 0; i < gammas.size(); ++i)
    std::cout << "gamma = " << gammas[i] << ", rank = " << rank[i] << ", res = " << res[i] << std::endl;
  std::cout.unsetf(std::ios::floatfield);

  return 0;
}


void nearestcorr( std::shared_ptr<ndarray<double, 2>> A)
{
  int N = A->size(0);

  // Create a model
  Model::t M = new Model("NearestCorrelation"); auto _M = finally([&]() { M->dispose(); });

  // Setting up the variables
  Variable::t X = M->variable("X", Domain::inPSDCone(N));
  Variable::t t = M->variable("t", 1, Domain::unbounded());

  // (t, vec (A-X)) \in Q
  M->constraint( Expr::vstack(t, vec(Expr::sub(A, X))), Domain::inQCone() );

  // diag(X) = e
  M->constraint(X->diag(), Domain::equalsTo(1.0));

  // Objective: Minimize t
  M->objective(ObjectiveSense::Minimize, t);

  // Solve the problem
  M->solve();

  // Get the solution values
  std::cout << "X = \n"; print_mat(std::cout, X->level());
  std::cout << "t = " << *(t->level()->begin()) << std::endl;
}

void nearestcorr_nn(
  std::shared_ptr<ndarray<double, 2>>  A,
  const std::vector<double>           & gammas,
  std::vector<double>                 & res,
  std::vector<double>                 & rank)
{
  int N = A->size(0);

  Model::t M = new Model("NucNorm"); auto M_ = monty::finally([&]() { M->dispose(); });

  // Setup variables
  Variable::t t = M->variable("t", 1, Domain::unbounded());
  Variable::t X = M->variable("X", Domain::inPSDCone(N));
  Variable::t w = M->variable("w", N, Domain::greaterThan(0.0));

  // (t, vec (X + diag(w) - A)) in Q
  Expression::t D = Expr::mulElm( Matrix::eye(N), Var::repeat(w, 1, N) );
  M->constraint( Expr::vstack( t, vec(Expr::sub(Expr::add(X, D), A)) ), Domain::inQCone() );

  // Trace(X)
  auto TrX = Expr::sum(X->diag());

  for (int k = 0; k < gammas.size(); ++k)
  {
    // Objective: Minimize t + gamma*Tr(X)
    M->objective(ObjectiveSense::Minimize, Expr::add(t, Expr::mul(gammas[k], TrX )));
    M->solve();

    // Find the eigenvalues of X and approximate its rank
    auto d = new_array_ptr<double, 1>(N);
    mosek::LinAlg::syeig(MSK_UPLO_LO, N, X->level(), d);
    int rnk = 0; for (int i = 0; i < N; ++i) if ((*d)[i] > 1e-6) ++rnk;

    res[k]  = (*(t->level()))[0];
    rank[k] = rnk;
  }
}


/** Assuming that e is an NxN expression, return the lower triangular part as a vector.
*/
Expression::t vec(Expression::t e)
{

  int N = (*e->getShape())[0];
  int dim = N * (N + 1) / 2;

  auto msubi = new_array_ptr<int, 1>(dim);
  auto msubj = new_array_ptr<int, 1>(dim);
  auto mcof  = new_array_ptr<double, 1>(dim);

  for (int i = 0, k = 0; i < N; ++i)
    for (int j = 0; j < i + 1; ++j, ++k)
    {
      (*msubi)[k] = k;
      (*msubj)[k] = i * N + j;
      (*mcof) [k] = (i == j) ? 1.0 : std::sqrt(2.0);
    }

  Matrix::t S = Matrix::sparse(N * (N + 1) / 2, N * N, msubi, msubj, mcof);
  return Expr::mul(S, Expr::reshape(e, N * N));
}

/* Utility methods */
void print_mat(std::ostream & os, const std::shared_ptr<ndarray<double, 1>> & a)
{
  if ( a->size() == 1)
    os << *(a->begin());
  else
  {
    int n = std::sqrt(a->size());
    int i = 0;
    for (auto aa = a->begin(); aa != a->end(); ++aa, ++i)
    {
      if (i % n == 0)
        os << "[";
      os << " " << *aa;
      if ( (i + 1) % n == 0)
        os << "]\n";
    }
  }
}