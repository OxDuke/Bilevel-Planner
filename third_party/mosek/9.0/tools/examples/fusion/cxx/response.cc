/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      response.cc

  Purpose:   This example demonstrates proper response handling
             for problems solved with the interior-point optimizers.
*/

#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <utility>
#include "fusion.h"
using namespace mosek::fusion;
using namespace monty;

// Set up a small artificial conic problem to test with
void setupExample(Model::t M)
{
  auto x = M->variable("x", 3, Domain::greaterThan(0.0));
  auto y = M->variable("y", 3, Domain::unbounded());
  auto z1 = Var::vstack(y->index(0), x->slice(0, 2));
  auto z2 = Var::vstack(y->slice(1, 3), x->index(2));
  M->constraint("lc", Expr::dot(new_array_ptr<double, 1>({1.0, 1.0, 2.0}), x), Domain::equalsTo(1.0));
  M->constraint("qc1", z1, Domain::inQCone());
  M->constraint("qc2", z2, Domain::inRotatedQCone());
  M->objective("obj", ObjectiveSense::Minimize, Expr::sum(y));
}

int main(int arc, char** argv)
{
  Model::t M = new Model(); auto _M = finally([&]() { M->dispose(); });

  // (Optional) set a log stream
  // M->setLogHandler([](const std::string & msg) { std::cout << msg << std::flush; });

  // (Optional) uncomment to see what happens when solution status is unknown
  // M->setSolverParam("intpntMaxIterations", 1);

  // In this example we set up a small conic problem
  setupExample(M);

  // Optimize
  try 
  {
    M->solve();

    // We expect solution status OPTIMAL (this is also default)
    M->acceptedSolutionStatus(AccSolutionStatus::Optimal);

    auto x = M->getVariable("x");
    auto xsize = x->getSize();
    auto xVal = x->level();
    std::cout << "Optimal value of x = ";
    for(int i = 0; i < xsize; ++i)
      std::cout << (*xVal)[i] << " ";
    std::cout << "\nOptimal primal objective: " << M->primalObjValue() <<"\n";
    // .. continue analyzing the solution

  }
  catch (const OptimizeError& e)
  {
    std::cout << "Optimization failed. Error: " << e.what() << "\n";
  }
  catch (const SolutionError& e)
  {
    // The solution with at least the expected status was not available.
    // We try to diagnoze why.
    std::cout << "Requested solution was not available.\n";
    auto prosta = M->getProblemStatus();
    switch(prosta)
    {
      case ProblemStatus::DualInfeasible:
        std::cout << "Dual infeasibility certificate found.\n";
        break;

      case ProblemStatus::PrimalInfeasible:
        std::cout << "Primal infeasibility certificate found.\n";
        break;

      case ProblemStatus::Unknown:
        // The solutions status is unknown. The termination code
        // indicates why the optimizer terminated prematurely.
        std::cout << "The solution status is unknown.\n";
        char symname[MSK_MAX_STR_LEN];
        char desc[MSK_MAX_STR_LEN];
        MSK_getcodedesc((MSKrescodee)(M->getSolverIntInfo("optimizeResponse")), symname, desc);
        std::cout << "  Termination code: " << symname << " " << desc << "\n";
        break;

      default:
        std::cout << "Another unexpected problem status: " << prosta << "\n";
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Unexpected error: " << e.what() << "\n";
  }

  M->dispose();
  return 0;
 }