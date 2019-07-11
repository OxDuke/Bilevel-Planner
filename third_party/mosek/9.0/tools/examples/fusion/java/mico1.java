//
//    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//    File:    mico1.java
//
//    Purpose:  Demonstrates how to solve a small mixed
//              integer conic optimization problem.
//
//              minimize    x^2 + y^2
//              subject to  x >= e^y + 3.8
//                          x, y - integer
//
package com.mosek.fusion.examples;
import mosek.fusion.*;

public class mico1 {
  public static void main(String[] args)
  throws SolutionError {

    Model M = new Model("mico1");

    try {
      Variable x = M.variable(Domain.integral(Domain.unbounded()));
      Variable y = M.variable(Domain.integral(Domain.unbounded()));
      Variable t = M.variable();

      M.constraint(Expr.vstack(t, x, y), Domain.inQCone());
      M.constraint(Expr.vstack(Expr.sub(x, 3.8), 1, y), Domain.inPExpCone());

      M.objective(ObjectiveSense.Minimize, t);

      M.solve();

      System.out.printf("x, y = %f, %f\n", x.level()[0], y.level()[0]);

    } finally {
      M.dispose();
    }
  }
}