/*
  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File :      parameters.cc

  Purpose :   Demonstrates a very simple example about how to set
              parameters and read information items
              with MOSEK Fusion
*/
#include <iostream>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  Model::t M = new Model(""); auto _M = finally([&]() { M->dispose(); });
  std::cout << "Test MOSEK parameter get/set functions\n";

  // Set log level (integer parameter)
  M->setSolverParam("log", 1);
  // Select interior-point optimizer... (parameter with symbolic string values)
  M->setSolverParam("optimizer", "intpnt");
  // ... without basis identification (parameter with symbolic string values)
  M->setSolverParam("intpntBasis", "never");
  // Set relative gap tolerance (double parameter)
  M->setSolverParam("intpntCoTolRelGap", 1.0e-7);

  // The same in a different way
  M->setSolverParam("intpntCoTolRelGap", "1.0e-7");

  // Incorrect value
  try {
      M->setSolverParam("intpntCoTolRelGap", -1);
  }
  catch (mosek::fusion::ParameterError) {
      std::cout << "Wrong parameter value\n";
  }

  // Define and solve an optimization problem here
  // M->solve()
  // After optimization: 

  std::cout << "Get MOSEK information items\n";

  double tm = M->getSolverDoubleInfo("optimizerTime");
  int it = M->getSolverIntInfo("intpntIter");

  std::cout << "Time: " << tm << "\nIterations: " << it << "\n";

  return 0;
}