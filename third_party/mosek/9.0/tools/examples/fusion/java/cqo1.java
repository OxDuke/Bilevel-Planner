//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      cqo1.java
//
//   Purpose: Demonstrates how to solve the problem
//
//   minimize y1 + y2 + y3
//   such that
//            x1 + x2 + 2.0 x3  = 1.0
//                    x1,x2,x3 >= 0.0
//   and
//            (y1,x1,x2) in C_3,
//            (y2,y3,x3) in K_3
//
//   where C_3 and K_3 are respectively the quadratic and
//   rotated quadratic cone of size 3 defined as
//       C_3 = { z1,z2,z3 :      z1 >= sqrt(z2^2 + z3^2) }
//       K_3 = { z1,z2,z3 : 2 z1 z2 >= z3^2              }
//

package com.mosek.fusion.examples;
import mosek.fusion.*;

public class cqo1 {
  public static void main(String[] args)
  throws SolutionError {
    Model M = new Model("cqo1");
    try {
      Variable x = M.variable("x", 3, Domain.greaterThan(0.0));
      Variable y = M.variable("y", 3, Domain.unbounded());

      // Create the aliases
      //      z1 = [ y[0],x[0],x[1] ]
      //  and z2 = [ y[1],y[2],x[2] ]
      Variable z1 = Var.vstack(y.index(0),  x.slice(0, 2));
      Variable z2 = Var.vstack(y.slice(1, 3), x.index(2));

      // Create the constraint
      //      x[0] + x[1] + 2.0 x[2] = 1.0
      double[] aval = new double[] {1.0, 1.0, 2.0};
      M.constraint("lc", Expr.dot(aval, x), Domain.equalsTo(1.0));

      // Create the constraints
      //      z1 belongs to C_3
      //      z2 belongs to K_3
      // where C_3 and K_3 are respectively the quadratic and
      // rotated quadratic cone of size 3, i.e.
      //                 z1[0] >= sqrt(z1[1]^2 + z1[2]^2)
      //  and  2.0 z2[0] z2[1] >= z2[2]^2
      Constraint qc1 = M.constraint("qc1", z1, Domain.inQCone());
      Constraint qc2 = M.constraint("qc2", z2, Domain.inRotatedQCone());

      // Set the objective function to (y[0] + y[1] + y[2])
      M.objective("obj", ObjectiveSense.Minimize, Expr.sum(y));

      // Solve the problem
      M.writeTask("cqo1.task");
      M.solve();


      // Get the linear solution values
      double[] solx = x.level();
      double[] soly = y.level();
      System.out.printf("x1,x2,x3 = %e, %e, %e\n", solx[0], solx[1], solx[2]);
      System.out.printf("y1,y2,y3 = %e, %e, %e\n", soly[0], soly[1], soly[2]);

      // Get conic solution of qc1
      double[] qc1lvl = qc1.level();
      double[] qc1sn  = qc1.dual();
      
      System.out.printf("qc1 levels = %e", qc1lvl[0]);
      for (int i = 1; i < qc1lvl.length; ++i)
        System.out.printf(", %e", qc1lvl[i]);
      System.out.print("\n");

      System.out.printf("qc1 dual conic var levels = %e", qc1sn[0]);
      for (int i = 1; i < qc1sn.length; ++i)
        System.out.printf(", %e", qc1sn[i]);
      System.out.print("\n");

    } finally {
      M.dispose();
    }
  }
}