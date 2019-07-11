/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      qo1.java

   Purpose :   Demonstrate how to solve a quadratic
               optimization problem using the MOSEK Java API.
 */

package com.mosek.example;
import mosek.*;

public class qo1 {
  static final int numcon = 1;   /* Number of constraints.             */
  static final int numvar = 3;   /* Number of variables.               */
  static final int NUMANZ = 3;   /* Number of numzeros in A.           */
  static final int NUMQNZ = 4;   /* Number of nonzeros in Q.           */

  public static void main (String[] args) {
    // Since the value infinity is never used, we define
    // 'infinity' symbolic purposes only
    double infinity = 0;
    double[] c = {0.0, -1.0, 0.0};

    mosek.boundkey[] bkc  = { mosek.boundkey.lo };
    double[] blc = {1.0};
    double[] buc = {infinity};

    mosek.boundkey[] bkx  = { mosek.boundkey.lo,
                              mosek.boundkey.lo,
                              mosek.boundkey.lo
                            };
    double[] blx = {0.0,
                    0.0,
                    0.0
                   };
    double[] bux = {infinity,
                    infinity,
                    infinity
                   };

    int[][]    asub  = { {0},   {0}, {0} };
    double[][] aval  = { {1.0}, {1.0}, {1.0} };
    double[] xx   = new double[numvar];

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
        /* Input column j of A */
        task.putacol(j,                     /* Variable (column) index.*/
                     asub[j],               /* Row index of non-zeros in column j.*/
                     aval[j]);              /* Non-zero Values of column j. */
      }
      /* Set the bounds on constraints.
       for i=1, ...,numcon : blc[i] <= constraint i <= buc[i] */
      for (int i = 0; i < numcon; ++i)
        task.putconbound(i, bkc[i], blc[i], buc[i]);

      /*
       The lower triangular part of the Q
       matrix in the objective is specified.
      */

      int[]    qsubi = {0,   1,    2,   2  };
      int[]    qsubj = {0,   1,    0,   2  };
      double[] qval =  {2.0, 0.2, -1.0, 2.0};

      /* Input the Q for the objective. */

      task.putqobj(qsubi, qsubj, qval);

      /* Solve the problem */
      mosek.rescode r = task.optimize();
      System.out.println (" Mosek warning:" + r.toString());
      // Print a summary containing information
      //   about the solution for debugging purposes
      task.solutionsummary(mosek.streamtype.msg);

      mosek.solsta solsta[] = new mosek.solsta[1];
      /* Get status information about the solution */
      task.getsolsta(mosek.soltype.itr, solsta);

      /* Get the solution */
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
          System.out.println("Primal or dual infeasibility\n");
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
  } /* Main */
}