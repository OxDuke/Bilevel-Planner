/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      lo2.java

   Purpose :   Demonstrates how to solve a small linear
               optimization problem using the MOSEK Java API.
*/
package com.mosek.example;
import mosek.*;

public class lo2 {
  static final int numcon = 3;
  static final int numvar = 4;
  static final int NUMANZ = 9;

  public static void main (String[] args) {
    // Since the value infinity is never used, we define
    // 'infinity' symbolic purposes only
    double
    infinity = 0;

    double c[]    = {3.0, 1.0, 5.0, 1.0};
    int    asub[][] = { 
      {0, 1, 2},
      {0, 1, 2, 3},
      {1, 3}
    };
    double aval[][] = { 
      {3.0, 1.0, 2.0 },
      {2.0, 1.0, 3.0, 1.0},
      {2.0, 3.0}
    };
    mosek.boundkey[]
    bkc    = {mosek.boundkey.fx,
              mosek.boundkey.lo,
              mosek.boundkey.up
             };
    double  blc[]  = {30.0,
                      15.0,
                      -infinity
                     };
    double  buc[]  = {30.0,
                      +infinity,
                      25.0
                     };
    mosek.boundkey
    bkx[]  = {mosek.boundkey.lo,
              mosek.boundkey.ra,
              mosek.boundkey.lo,
              mosek.boundkey.lo
             };
    double  blx[]  = {0.0,
                      0.0,
                      0.0,
                      0.0
                     };
    double  bux[]  = { +infinity,
                       10.0,
                       +infinity,
                       +infinity
                     };
    double[] xx  = new double[numvar];

    try (Env  env  = new Env();
         Task task = new Task(env, 0, 0)) {
      // Directs the log task stream to the user specified
      // method task_msg_obj.stream
      task.set_Stream(
        mosek.streamtype.log,
        new mosek.Stream()
      { public void stream(String msg) { System.out.print(msg); }});

      /* Give MOSEK an estimate of the size of the input data.
      This is done to increase the speed of inputting data.
      However, it is optional. */
      /* Append 'numcon' empty constraints.
      The constraints will initially have no bounds. */
      task.appendcons(numcon);

      /* Append 'numvar' variables.
      The variables will initially be fixed at zero (x=0). */
      task.appendvars(numvar);

      for (int j = 0; j < numvar; ++j) {
        /* Set the linear term c_j in the objective.*/
        task.putcj(j, c[j]);
        /* Set the bounds on variable j.
           blx[j] <= x_j <= bux[j] */
        task.putvarbound(j, bkx[j], blx[j], bux[j]);
      }
      /* Set the bounds on constraints.
       for i=1, ...,numcon : blc[i] <= constraint i <= buc[i] */
      for (int i = 0; i < numcon; ++i) {
        task.putconbound(i, bkc[i], blc[i], buc[i]);

        /* Input row i of A */
        task.putarow(i,                     /* Row index.*/
                     asub[i],               /* Column indexes of non-zeros in row i.*/
                     aval[i]);              /* Non-zero Values of row i. */
      }

      /* A maximization problem */
      task.putobjsense(mosek.objsense.maximize);

      /* Solve the problem */
      mosek.rescode r = task.optimize();

      // Print a summary containing information
      //   about the solution for debugging purposes
      task.solutionsummary(mosek.streamtype.msg);

      mosek.solsta solsta[] = new mosek.solsta[1];
      /* Get status information about the solution */
      task.getsolsta(mosek.soltype.bas, solsta);

      task.getxx(mosek.soltype.bas, // Basic solution.
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
    }
    catch (mosek.Exception e) {
      System.out.println ("An error/warning was encountered");
      System.out.println (e.toString());
      throw e;
    }
  }
}