/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      ceo1.java

   Purpose :   Demonstrates how to solve a small conic exponential
               optimization problem using the MOSEK API.
*/
package com.mosek.example;

import mosek.*;

public class ceo1 {
  static final int numcon = 1;
  static final int numvar = 3;

  public static void main (String[] args) throws java.lang.Exception {
    // Since the value infinity is never used, we define
    // 'infinity' symbolic purposes only
    double infinity = 0;

    mosek.boundkey bkc    =  mosek.boundkey.fx ;
    double blc =  1.0 ;
    double buc =  1.0 ;

    mosek.boundkey[] bkx = { mosek.boundkey.fr,
                             mosek.boundkey.fr,
                             mosek.boundkey.fr
                           };
    double[] blx = { -infinity,
                     -infinity,
                     -infinity
                   };
    double[] bux = { +infinity,
                     +infinity,
                     +infinity
                   };

    double[] c   = { 1.0,
                     1.0,
                     0.0
                   };

    double[] a   = { 1.0,
                     1.0,
                     1.0
    };
    int[] asub   = {0, 1, 2};
    int[] csub   = new int[numvar];
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
      task.putcslice(0, numvar, c);
      task.putarow(0, asub, a);
      task.putconbound(0, bkc, blc, buc);
      task.putvarboundslice(0, numvar, bkx, blx, bux);

      /* Add a conic constraint */
      csub[0] = 0;
      csub[1] = 1;
      csub[2] = 2;
      task.appendcone(mosek.conetype.pexp,
                      0.0, /* For future use only, can be set to 0.0 */
                      csub);

      task.putobjsense(mosek.objsense.minimize);

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
          for (int j = 0; j < numvar; ++j)
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