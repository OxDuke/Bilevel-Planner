/*
  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File :      parameters.cs

  Purpose :   Demonstrates a very simple example about how to set
              parameters and read information items
              with MOSEK Fusion
*/
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class parameters
  {
    public static void Main(string[] args)
    {
      // Create the Model        
      using (Model M = new Model()) {
        Console.WriteLine("Test MOSEK parameter get/set functions");

        // Set log level (integer parameter)
        M.SetSolverParam("log", 1);
        // Select interior-point optimizer... (parameter with symbolic string values)
        M.SetSolverParam("optimizer", "intpnt");
        // ... without basis identification (parameter with symbolic string values)
        M.SetSolverParam("intpntBasis", "never");
        // Set relative gap tolerance (double parameter)
        M.SetSolverParam("intpntCoTolRelGap", 1.0e-7);

        // The same in a different way
        // M.SetSolverParam("intpntCoTolRelGap", "1.0e-7");

        // Incorrect value
        try {
          M.SetSolverParam("intpntCoTolRelGap", -1);
        }
        catch (mosek.fusion.ParameterError e) {
          Console.WriteLine("Wrong parameter value");
        }


        // Define and solve an optimization problem here
        // M.Solve()
        // After optimization: 

        Console.WriteLine("Get MOSEK information items");

        double tm = M.GetSolverDoubleInfo("optimizerTime");
        int it = M.GetSolverIntInfo("intpntIter");

        Console.WriteLine("Time: " + tm);
        Console.WriteLine("Iterations: " + it);
      }
    }
  }
}