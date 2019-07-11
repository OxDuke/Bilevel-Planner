/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      response.cs

  Purpose:   This example demonstrates proper response handling
             for problems solved with the interior-point optimizers.
*/
using System;
using mosek;
using mosek.fusion;
using System.Text;

namespace mosek.fusion.example
{
  public class response
  {
    // Set up a small artificial conic problem to test with
    public static void SetupExample(Model M) {
      Variable x = M.Variable("x", 3, Domain.GreaterThan(0.0));
      Variable y = M.Variable("y", 3, Domain.Unbounded());
      Variable z1 = Var.Vstack(y.Index(0), x.Slice(0, 2));
      Variable z2 = Var.Vstack(y.Slice(1, 3), x.Index(2));
      M.Constraint("lc", Expr.Dot(new double[] {1.0, 1.0, 2.0}, x), Domain.EqualsTo(1.0));
      M.Constraint("qc1", z1, Domain.InQCone());
      M.Constraint("qc2", z2, Domain.InRotatedQCone());
      M.Objective("obj", ObjectiveSense.Minimize, Expr.Sum(y));
    }

    public static void Main(String[] argv) {
      Model M = new Model();

      // (Optional) set a log stream
      // M.SetLogHandler(Console.Out);
      
      // (Optional) uncomment to see what happens when solution status is unknown
      // M.SetSolverParam("intpntMaxIterations", 1);

      // In this example we set up a small conic problem
      SetupExample(M);

      // Optimize
      try 
      {
        M.Solve();

        // We expect solution status OPTIMAL (this is also default)
        M.AcceptedSolutionStatus(AccSolutionStatus.Optimal);

        Variable x = M.GetVariable("x");
        long xsize = x.GetSize();
        double[] xVal = x.Level();
        Console.Write("Optimal value of x = ");
        for(int i = 0; i < xsize; ++i)
          Console.Write(xVal[i] + " ");
        Console.WriteLine("\nOptimal primal objective: " + M.PrimalObjValue());
        // .. continue analyzing the solution

      }
      catch (OptimizeError e)
      {
        Console.WriteLine("Optimization failed. Error: " + e.ToString());
      }
      catch (SolutionError)
      {
        // The solution with at least the expected status was not available.
        // We try to diagnoze why.
        Console.WriteLine("Requested solution was not available.");
        ProblemStatus prosta = M.GetProblemStatus();
        switch(prosta)
        {
          case ProblemStatus.DualInfeasible:
            Console.WriteLine("Dual infeasibility certificate found.");
            break;

          case ProblemStatus.PrimalInfeasible:
            Console.WriteLine("Primal infeasibility certificate found.");
            break;

          case ProblemStatus.Unknown:
            // The solutions status is unknown. The termination code
            // indicates why the optimizer terminated prematurely.
            Console.WriteLine("The solution status is unknown.");
            StringBuilder symname = new StringBuilder();
            StringBuilder desc = new StringBuilder();
            Env.getcodedesc((rescode)M.GetSolverIntInfo("optimizeResponse"), symname, desc);
            Console.WriteLine("  Termination code: {0} {1}", symname, desc);
            break;

          default:
            Console.WriteLine("Another unexpected problem status: " + prosta);
            break;
        }
      }
      catch (Exception e)
      {
        Console.WriteLine("Unexpected error: " + e.ToString());
      }

      M.Dispose();
    }
  }
}