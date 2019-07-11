//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      ceo1.java
//
//   Purpose: Demonstrates how to solve the problem
//
//   minimize x1 + x2 
//   such that
//            x1 + x2 + x3  = 1.0
//                x1,x2    >= 0.0
//   and      x1 >= x2 * exp(x3/x2)
//

package com.mosek.fusion.examples;
import mosek.fusion.*;

public class ceo1 {
  public static void main(String[] args)
  throws SolutionError {
    Model M = new Model("ceo1");
    try {
      Variable x = M.variable("x", 3, Domain.unbounded());

      // Create the constraint
      //      x[0] + x[1] + x[2] = 1.0
      M.constraint("lc", Expr.sum(x), Domain.equalsTo(1.0));

      // Create the conic exponential constraint
      Constraint expc = M.constraint("expc", x, Domain.inPExpCone());

      // Set the objective function to (x[0] + x[1])
      M.objective("obj", ObjectiveSense.Minimize, Expr.sum(x.slice(0,2)));

      // Solve the problem
      M.solve();

      // Get the linear solution values
      double[] solx = x.level();
      System.out.printf("x1,x2,x3 = %e, %e, %e\n", solx[0], solx[1], solx[2]);

      // Get conic solution of expc
      double[] expclvl = expc.level();
      double[] expcsn  = expc.dual();
      
      System.out.printf("expc levels = %e", expclvl[0]);
      for (int i = 1; i < expclvl.length; ++i)
        System.out.printf(", %e", expclvl[i]);
      System.out.print("\n");

      System.out.printf("expc dual conic var levels = %e", expcsn[0]);
      for (int i = 1; i < expcsn.length; ++i)
        System.out.printf(", %e", expcsn[i]);
      System.out.print("\n");

    } finally {
      M.dispose();
    }
  }
}