//
//    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//    File:    mioinitsol.java
//
//    Purpose:  Demonstrates how to solve a small mixed
//              integer linear optimization problem.
//
package com.mosek.fusion.examples;
import mosek.fusion.*;

public class mioinitsol {
  public static void main(String[] args)
  throws SolutionError {
    int n = 4;
    double[] c = { 7.0, 10.0, 1.0, 5.0 };

    Model M = new Model("mioinitsol");
    try {
      Variable x = M.variable("x", n, Domain.greaterThan(0.0));
      x.slice(0,3).makeInteger();

      M.constraint(Expr.sum(x), Domain.lessThan(2.5));

      M.objective("obj", ObjectiveSense.Maximize, Expr.dot(c, x));

      // Assign values to integer variables.
      // We only set a slice of x     
      double[] init_sol = { 1, 1, 0 };
      x.slice(0,3).setLevel( init_sol );

      M.solve();

      // Get the solution values
      double[] sol = x.level();
      System.out.printf("x = [");
      for (int i = 0; i < n; i++) {
        System.out.printf("%e, ", sol[i]);
      }
      System.out.printf("]\n");
      
      // Was the initial solution used?
      int constr = M.getSolverIntInfo("mioConstructSolution");
      double constrVal = M.getSolverDoubleInfo("mioConstructSolutionObj");
      System.out.println("Initial solution utilization: " + constr);
      System.out.println("Initial solution objective: " +  constrVal);

    } finally {
      M.dispose();
    }
  }
}