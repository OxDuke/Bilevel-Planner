/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      callback.cc

   Purpose:   To demonstrate how to use the progress
              callback.

              Compile and link the file with MOSEK, then
              use as follows:

              callback psim
              callback dsim
              callback intpnt

              The argument tells which optimizer to use
              i.e. psim is primal simplex, dsim is dual simplex
              and intpnt is interior-point.
 */


#include <memory>
#include <random>
#include <iostream>
#include <string>

#include "mosek.h"
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;


/* Note: This function is declared using MSKAPI,
         so the correct calling convention is
         employed. */

// For demonstration. Actually defined in mosektask.h as datacbhandler_t and cbhandler_t
typedef std::function<bool(MSKcallbackcodee, const double *, const int32_t *, const int64_t *)> callbackHandler_t;
typedef std::function<int(MSKcallbackcodee)> progressHandler_t;

static int MSKAPI usercallback( MSKcallbackcodee caller,
                                const double   * douinf,
                                const int32_t  * intinf,
                                const int64_t  * lintinf,
                                Model::t mod,
                                const double maxtime)
{
  switch ( caller )
  {
    case MSK_CALLBACK_BEGIN_INTPNT:
      std::cerr << "Starting interior-point optimizer\n";
      break;
    case MSK_CALLBACK_INTPNT:
      std::cerr << "Iterations: " << intinf[MSK_IINF_INTPNT_ITER];
      std::cerr << " (" << douinf[MSK_DINF_OPTIMIZER_TIME] << "/";
      std::cerr << douinf[MSK_DINF_INTPNT_TIME] << ")s. \n";
      std::cerr << "Primal obj.: " << douinf[MSK_DINF_INTPNT_PRIMAL_OBJ];
      std::cerr << " Dual obj.: " <<  douinf[MSK_DINF_INTPNT_DUAL_OBJ] << std::endl;
      break;
    case MSK_CALLBACK_END_INTPNT:
      std::cerr << "Interior-point optimizer finished.\n";
      break;
    case MSK_CALLBACK_BEGIN_PRIMAL_SIMPLEX:
      std::cerr << "Primal simplex optimizer started.\n";
      break;
    case MSK_CALLBACK_UPDATE_PRIMAL_SIMPLEX:
      std::cerr << "Iterations: " << intinf[MSK_IINF_SIM_PRIMAL_ITER];
      std::cerr << "  Elapsed time: " << douinf[MSK_DINF_OPTIMIZER_TIME];
      std::cerr << "(" << douinf[MSK_DINF_SIM_TIME] << ")\n";
      std::cerr << "Obj.: " << douinf[MSK_DINF_SIM_OBJ] << std::endl;
      break;
    case MSK_CALLBACK_END_PRIMAL_SIMPLEX:
      std::cerr << "Primal simplex optimizer finished.\n";
      break;
    case MSK_CALLBACK_BEGIN_DUAL_SIMPLEX:
      std::cerr << "Dual simplex optimizer started.\n";
      break;
    case MSK_CALLBACK_UPDATE_DUAL_SIMPLEX:
      std::cerr << "Iterations: " << intinf[MSK_IINF_SIM_DUAL_ITER];
      std::cerr << "  Elapsed time: " << douinf[MSK_DINF_OPTIMIZER_TIME];
      std::cerr << "(" << douinf[MSK_DINF_SIM_TIME] << ")\n";
      std::cerr << "Obj.: " << douinf[MSK_DINF_SIM_OBJ] << std::endl;
      break;
    case MSK_CALLBACK_END_DUAL_SIMPLEX:
      std::cerr << "Dual simplex optimizer finished.\n";
      break;
    case MSK_CALLBACK_BEGIN_BI:
      std::cerr << "Basis identification started.\n";
      break;
    case MSK_CALLBACK_END_BI:
      std::cerr << "Basis identification finished.\n";
      break;
    default:
      break;
  }
  if ( douinf[MSK_DINF_OPTIMIZER_TIME] >= maxtime )
  {
    std::cerr << "MOSEK is spending too much time.Terminate it.\n";
    return 1;
  }
  return 0;
} /* usercallback */

int main(int argc, char ** argv)
{
  std::string slvr("intpnt");

  if (argc <= 1)
  {
    std::cerr << "Usage: ( psim | dsim | intpnt ) \n";
  }

  if (argc >= 2) slvr = argv[1];

  /* Solve a big random linear optimization problem */
  int n = 150, m = 700;
  std::default_random_engine generator;
  std::uniform_real_distribution<double> unif_distr(0., 10.);
  auto c = new_array_ptr<double, 1>(n);
  auto b = new_array_ptr<double, 1>(m);
  auto A = new_array_ptr<double, 1>(m * n);
  std::generate(c->begin(), c->end(), std::bind(unif_distr, generator));
  std::generate(b->begin(), b->end(), std::bind(unif_distr, generator));
  std::generate(A->begin(), A->end(), std::bind(unif_distr, generator));

  Model::t M = new Model();
  auto x = M->variable(n, Domain::unbounded());
  M->constraint(Expr::mul(Matrix::dense(m, n, A), x), Domain::lessThan(b));
  M->objective(ObjectiveSense::Maximize, Expr::dot(c, x));

  if ( slvr == "psim")
    M->setSolverParam("optimizer", "primalSimplex");
  else if ( slvr == "dsim")
    M->setSolverParam("optimizer", "dualSimplex");
  else if ( slvr == "intpnt")
    M->setSolverParam("optimizer", "intpnt");

  double maxtime = 0.07;

  callbackHandler_t cllbck = [&](MSKcallbackcodee caller,
                                 const double * douinf, const int32_t* intinf, const int64_t* lintinf)
  {
    return usercallback(caller, douinf, intinf, lintinf, M, maxtime);
  };

  M->setDataCallbackHandler(cllbck);

  M->setSolverParam("log", 0);
  M->solve();

  return 0;
}



