/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      pow1.java

  Purpose: Demonstrates how to solve the problem

    maximize x^0.2*y^0.8 + z^0.4 - x
          st x + y + 0.5z = 2
             x,y,z >= 0
*/
package com.mosek.example;

import mosek.*;

public class pow1 {
  static final int numcon = 1;
  static final int numvar = 6;

  public static void main (String[] args) throws java.lang.Exception {
    // Since the value infinity is never used, we define
    // 'infinity' symbolic purposes only
    double infinity = 0;

    mosek.boundkey[] bkx = new mosek.boundkey[numvar];
    double[] blx         = new double[numvar];
    double[] bux         = new double[numvar];    

    double[] val   = { 1.0, 1.0, -1.0 };
    int[]    sub   = { 3, 4, 0 };

    double[] aval  = { 1.0, 1.0, 0.5 };
    int[]    asub  = { 0, 1, 2 };

    int i;
    double[] xx  = new double[numvar];

    // create a new environment object
    try (Env  env  = new Env();
         Task task = new Task(env, 0, 0)) {
      // Directs the log task stream to the user specified
      // method task_msg_obj.stream
      task.set_Stream(
        mosek.streamtype.log,
        new mosek.Stream()
      { public void stream(String msg) { System.out.print(msg); }});

      /* Append 'numcon' empty constraints.
         The constraints will initially have no bounds. */
      task.appendcons(numcon);

      /* Append 'numvar' variables.
         The variables will initially be fixed at zero (x=0). */
      task.appendvars(numvar);

      /* Define the linear part of the problem */
      task.putclist(sub, val);
      task.putarow(0, asub, aval);
      task.putconbound(0, mosek.boundkey.fx, 2.0, 2.0);
      for(i=0;i<5;i++) {
        bkx[i] = mosek.boundkey.fr;
        blx[i] = -infinity;
        bux[i] = infinity;
      }
      bkx[5] = mosek.boundkey.fx;
      blx[5] = bux[5] = 1.0;
      task.putvarboundslice(0, numvar, bkx, blx, bux);

      /* Add a conic constraint */
      int[][] csub = { {0, 1, 3}, {2, 5, 4}};
      task.appendcone(mosek.conetype.ppow, 0.2, csub[0]);
      task.appendcone(mosek.conetype.ppow, 0.4, csub[1]);

      task.putobjsense(mosek.objsense.maximize);

      System.out.println ("optimize");
      /* Solve the problem */
      mosek.rescode r = task.optimize();
      System.out.println (" Mosek warning:" + r.toString());
      // Print a summary containing information
      // about the solution for debugging purposes
      task.solutionsummary(mosek.streamtype.msg);

      mosek.solsta solsta[] = new mosek.solsta[1];

      /* Get status information about the solution */
      task.getsolsta(mosek.soltype.itr, solsta);

      task.getxx(mosek.soltype.itr, // Interior solution.
                 xx);

      switch (solsta[0]) {
        case optimal:
          System.out.println("Optimal primal solution\n");
          for (int j = 0; j < 3; ++j)
            System.out.println ("x[" + j + "]:" + xx[j]);
          break;
        case dual_infeas_cer:
        case prim_infeas_cer:
          System.out.println("Primal or dual infeasibility.\n");
          break;
        case unknown:
          System.out.println("Unknown solution status.\n");
          break;
        default:
          System.out.println("Other solution status");
          break;
      }
    } catch (mosek.Exception e) {
      System.out.println ("An error/warning was encountered");
      System.out.println (e.toString());
      throw e;
    }
  }
}