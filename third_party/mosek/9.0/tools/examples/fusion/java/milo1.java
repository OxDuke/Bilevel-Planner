//
//    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//    File:    milo1.java
//
//    Purpose:  Demonstrates how to solve a small mixed
//              integer linear optimization problem.
//
package com.mosek.fusion.examples;
import mosek.fusion.*;

public class milo1 {
  public static void main(String[] args)
  throws SolutionError {
    double[][] A = {
      { 50.0, 31.0 },
      { 3.0,  -2.0 }
    };
    double[] c = { 1.0, 0.64 };

    Model M = new Model("milo1");
    try {
      Variable x = M.variable("x", 2, Domain.integral(Domain.greaterThan(0.0)));

      // Create the constraints
      //      50.0 x[0] + 31.0 x[1] <= 250.0
      //       3.0 x[0] -  2.0 x[1] >= -4.0
      M.constraint("c1", Expr.dot(A[0], x), Domain.lessThan(250.0));
      M.constraint("c2", Expr.dot(A[1], x), Domain.greaterThan(-4.0));

      // Set max solution time
      M.setSolverParam("mioMaxTime", 60.0);
      // Set max relative gap (to its default value)
      M.setSolverParam("mioTolRelGap", 1e-4);
      // Set max absolute gap (to its default value)
      M.setSolverParam("mioTolAbsGap", 0.0);

      // Set the objective function to (c^T * x)
      M.objective("obj", ObjectiveSense.Maximize, Expr.dot(c, x));

      // Solve the problem
      M.solve();

      // Get the solution values
      double[] sol = x.level();
      System.out.printf("x1,x2 = %e, %e\n", sol[0], sol[1]);
      System.out.printf("MIP rel gap = %.2f (%f)\n",
                        M.getSolverDoubleInfo("mioObjRelGap"),
                        M.getSolverDoubleInfo("mioObjAbsGap"));
    } finally {
      M.dispose();
    }
  }
}