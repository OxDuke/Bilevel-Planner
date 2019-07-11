// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      duality.java
//
// Purpose: Show how to read the dual value of a constraint.
package com.mosek.fusion.examples;
import mosek.fusion.*;

public class duality {
  public static void main(String[] args)
  throws SolutionError {
    double[][] A =  { { -0.5, 1.0 }  };
    double[]   b =  { 1.0 };
    double[]   c =  { 1.0, 1.0 };

    Model M = new Model("duality");
    try {
      Variable x = M.variable("x", 2, Domain.greaterThan(0.0));

      Constraint con = M.constraint(Expr.sub(Expr.mul(Matrix.dense(A), x), b), Domain.equalsTo(0.0));

      M.objective("obj", ObjectiveSense.Minimize, Expr.dot(c, x));

      M.solve();
      double[] xsol = x.level();
      double[] ssol = x.dual();
      double[] ysol = con.dual();

      System.out.printf("x1,x2,s1,s2,y = %e, %e, %e, %e, %e\n", xsol[0], xsol[1], ssol[0], ssol[1], ysol[0]);
    } finally {
      M.dispose();
    }
  }
}