//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      pow1.java
//
//   Purpose: Demonstrates how to solve the problem
//
//     maximize x^0.2*y^0.8 + z^0.4 - x
//           st x + y + 0.5z = 2
//              x,y,z >= 0
//

package com.mosek.fusion.examples;
import mosek.fusion.*;

public class pow1 {
  public static void main(String[] args)
  throws SolutionError {
    Model M = new Model("pow1");
    try {
      Variable x  = M.variable("x", 3, Domain.unbounded());
      Variable x3 = M.variable();
      Variable x4 = M.variable();

      // Create the linear constraint
      double[] aval = new double[] {1.0, 1.0, 0.5};
      M.constraint(Expr.dot(x, aval), Domain.equalsTo(2.0));

      // Create the conic constraints
      M.constraint(Var.vstack(x.slice(0,2), x3), Domain.inPPowerCone(0.2));
      M.constraint(Expr.vstack(x.index(2), 1.0, x4), Domain.inPPowerCone(0.4));     

      // Set the objective function
      double[] cval = new double[] {1.0, 1.0, -1.0};
      M.objective(ObjectiveSense.Maximize, Expr.dot(cval, Var.vstack(x3, x4, x.index(0))));

      // Solve the problem
      M.solve();

      // Get the linear solution values
      double[] solx = x.level();
      System.out.printf("x, y, z = %e, %e, %e\n", solx[0], solx[1], solx[2]);
    } finally {
      M.dispose();
    }
  }
}