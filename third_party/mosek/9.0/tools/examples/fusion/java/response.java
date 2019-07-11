/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      response.java

   Purpose :   This example demonstrates proper response handling
               for problems solved with the interior-point optimizers.
*/
package com.mosek.fusion.examples;
import mosek.*;
import mosek.fusion.*;

public class response {
  // Set up a small artificial conic problem to test with
  public static void setupExample(Model M) {
    Variable x = M.variable("x", 3, Domain.greaterThan(0.0));
    Variable y = M.variable("y", 3, Domain.unbounded());
    Variable z1 = Var.vstack(y.index(0), x.slice(0, 2));
    Variable z2 = Var.vstack(y.slice(1, 3), x.index(2));
    M.constraint("lc", Expr.dot(new double[] {1.0, 1.0, 2.0}, x), Domain.equalsTo(1.0));
    M.constraint("qc1", z1, Domain.inQCone());
    M.constraint("qc2", z2, Domain.inRotatedQCone());
    M.objective("obj", ObjectiveSense.Minimize, Expr.sum(y));
  }

  public static void main(String[] argv) {
    Model M = new Model();

    // (Optional) set a log stream
    /*
      task.set_Stream(
        streamtype.log,
        new mosek.Stream()
      { public void stream(String msg) { System.out.print(msg); }});
    */

    // (Optional) uncomment to see what happens when solution status is unknown
    // M.setSolverParam("intpntMaxIterations", 1);

    // In this example we set up a small conic problem
    setupExample(M);

    // Optimize
    try 
    {
      M.solve();

      // We expect solution status OPTIMAL (this is also default)
      M.acceptedSolutionStatus(AccSolutionStatus.Optimal);

      Variable x = M.getVariable("x");
      long xsize = x.getSize();
      double[] xVal = x.level();
      System.out.print("Optimal value of x = ");
      for(int i = 0; i < xsize; ++i)
        System.out.print(xVal[i] + " ");
      System.out.println("\nOptimal primal objective: " + M.primalObjValue());
      // .. continue analyzing the solution

    }
    catch (OptimizeError e)
    {
      System.out.println("Optimization failed. Error: " + e.toString());
    }
    catch (SolutionError e)
    {
      // The solution with at least the expected status was not available.
      // We try to diagnoze why.
      System.out.println("Requested solution was not available.");
      ProblemStatus prosta = M.getProblemStatus();
      switch(prosta)
      {
        case DualInfeasible:
          System.out.println("Dual infeasibility certificate found.");
          break;

        case PrimalInfeasible:
          System.out.println("Primal infeasibility certificate found.");
          break;

        case Unknown:
          // The solutions status is unknown. The termination code
          // indicates why the optimizer terminated prematurely.
          System.out.println("The solution status is unknown.");
          StringBuffer symname = new StringBuffer();
          StringBuffer desc = new StringBuffer();
          Env.getcodedesc(rescode.fromValue(M.getSolverIntInfo("optimizeResponse")), symname, desc);
          System.out.printf("   Termination code: %s %s\n", symname, desc);
          break;

        default:
          System.out.println("Another unexpected problem status: " + prosta);
      }
    }
    catch (java.lang.Exception e)
    {
      System.out.println("Unexpected error: " + e.toString());
    }

    M.dispose();
  }
}
