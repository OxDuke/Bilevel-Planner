////
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      lo1.java
//
//  Purpose: Demonstrates how to solve the problem
//
//  maximize 3*x0 + 1*x1 + 5*x2 + x3
//  such that
//           3*x0 + 1*x1 + 2*x2        = 30,
//           2*x0 + 1*x1 + 3*x2 + 1*x3 > 15,
//                  2*x1 +      + 3*x3 < 25
//  and
//           x0,x1,x2,x3 > 0,
//           0 < x1 < 10
////
package com.mosek.fusion.examples;
import mosek.fusion.*;

public class lo1 {
  public static void main(String[] args)
  throws SolutionError {
    double[][] A =
    { new double[] { 3.0, 1.0, 2.0, 0.0 },
      new double[] { 2.0, 1.0, 3.0, 1.0 },
      new double[] { 0.0, 2.0, 0.0, 3.0 }
    };
    double[] c = { 3.0, 1.0, 5.0, 1.0  };

    // Create a model with the name 'lo1'
    try(Model M = new Model("lo1"))
    {
      // Create variable 'x' of length 4
      Variable x = M.variable("x", 4, Domain.greaterThan(0.0));

      // Create constraints
      M.constraint(x.index(1), Domain.lessThan(10.0));
      M.constraint("c1", Expr.dot(A[0], x), Domain.equalsTo(30.0));
      M.constraint("c2", Expr.dot(A[1], x), Domain.greaterThan(15.0));
      M.constraint("c3", Expr.dot(A[2], x), Domain.lessThan(25.0));

      // Set the objective function to (c^t * x)
      M.objective("obj", ObjectiveSense.Maximize, Expr.dot(c, x));

      // Solve the problem
      M.solve();

      // Get the solution values
      double[] sol = x.level();
      System.out.printf("[x0,x1,x2,x3] = [%e, %e, %e, %e]\n", sol[0], sol[1], sol[2], sol[3]);
    }
  }
}