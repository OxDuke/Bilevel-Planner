/*
* Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
*
* File:      qcqp_sdo_relaxation.cc
*
* Purpose:   Demonstrate how to use SDP to solve
*            convex relaxation of a mixed-integer QCQP
**/
#include <random>
#include <iostream>
#include <ctime>
#include "fusion.h"

using namespace mosek::fusion;
using namespace mosek::LinAlg;
using namespace monty;


Model::t miqcqp_sdo_relaxation(int n, Matrix::t P, const std::shared_ptr<ndarray<double, 1>> & q) {
  Model::t M = new Model();

  Variable::t Z = M->variable("Z", Domain::inPSDCone(n + 1));

  Variable::t X = Z->slice(new_array_ptr<int, 1>({0, 0}), new_array_ptr<int, 1>({n, n}));
  Variable::t x = Z->slice(new_array_ptr<int, 1>({0, n}), new_array_ptr<int, 1>({n, n + 1}));

  M->constraint( Expr::sub(X->diag(), x), Domain::greaterThan(0.) );
  M->constraint( Z->index(n, n), Domain::equalsTo(1.) );

  M->objective( ObjectiveSense::Minimize, Expr::add(
                  Expr::sum( Expr::mulElm( P, X ) ),
                  Expr::mul( 2.0, Expr::dot(x, q) )
                ) );

  return M;
}

// A direct integer model for minimizing |Ax-b|
Model::t int_least_squares(int n, Matrix::t A, const std::shared_ptr<ndarray<double, 1>> & b) {
  Model::t M = new Model();

  Variable::t x = M->variable("x", n, Domain::integral(Domain::unbounded()));
  Variable::t t = M->variable("t", 1, Domain::unbounded());

  M->constraint( Expr::vstack(t, Expr::sub(Expr::mul(A, x), b)), Domain::inQCone() );
  M->objective( ObjectiveSense::Minimize, t );

  return M;
}


int main(int argc, char ** argv)
{
  std::default_random_engine generator;
  generator.seed(time(0));
  std::uniform_real_distribution<double> unif_distr(0., 1.);
  std::normal_distribution<double> normal_distr(0., 1.);

  // problem dimensions
  int n = 20;
  int m = 2 * n;

  auto c = new_array_ptr<double, 1>(n);
  auto A = new_array_ptr<double, 1>(n * m);
  auto P = new_array_ptr<double, 1>(n * n);
  auto b = new_array_ptr<double, 1>(m);
  auto q = new_array_ptr<double, 1>(n);


  std::generate(A->begin(), A->end(), std::bind(normal_distr, generator));
  std::generate(c->begin(), c->end(), std::bind(unif_distr, generator));
  std::fill(b->begin(), b->end(), 0.0);
  std::fill(q->begin(), q->end(), 0.0);

  // P = A^T A
  syrk(MSK_UPLO_LO, MSK_TRANSPOSE_YES,
       n, m, 1.0, A, 0., P);
  for (int j = 0; j < n; j++) for (int i = j + 1; i < n; i++) (*P)[i * n + j] = (*P)[j * n + i];

  // q = -P c, b = A c
  gemv(MSK_TRANSPOSE_NO, n, n, -1.0, P, c, 0., q);
  gemv(MSK_TRANSPOSE_NO, m, n, 1.0, A, c, 0., b);

  // Solve the problems
  {
    Model::t M = miqcqp_sdo_relaxation(n, Matrix::dense(n, n, P), q);
    Model::t Mint = int_least_squares(n, Matrix::dense(n, m, A)->transpose(), b);
    M->solve();
    Mint->solve();

    auto xRound = M->getVariable("Z")->
                  slice(new_array_ptr<int, 1>({0, n}), new_array_ptr<int, 1>({n, n + 1}))->level();
    for (int i = 0; i < n; i++) (*xRound)[i] = round((*xRound)[i]);
    auto yRound = new_array_ptr<double, 1>(m);
    auto xOpt   = Mint->getVariable("x")->level();
    auto yOpt   = new_array_ptr<double, 1>(m);
    std::copy(b->begin(), b->end(), yRound->begin());
    std::copy(b->begin(), b->end(), yOpt->begin());
    gemv(MSK_TRANSPOSE_NO, m, n, 1.0, A, xRound, -1.0, yRound);        // Ax_round-b
    gemv(MSK_TRANSPOSE_NO, m, n, 1.0, A, xOpt, -1.0, yOpt);            // Ax_opt-b

    std::cout << M->getSolverDoubleInfo("optimizerTime") << " " << Mint->getSolverDoubleInfo("optimizerTime") << "\n";
    double valRound, valOpt;
    dot(m, yRound, yRound, valRound); dot(m, yOpt, yOpt, valOpt);
    std::cout << sqrt(valRound) << " " << sqrt(valOpt) << "\n";
  }
  return 0;
};