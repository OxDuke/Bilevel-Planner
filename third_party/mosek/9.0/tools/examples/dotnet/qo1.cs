/*
  File : qo1.cs

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Purpose: Demonstrate how to solve a quadratic
           optimization problem using the MOSEK .NET API.
*/
using System;

namespace mosek.example
{
  class msgclass : mosek.Stream
  {
    string prefix;
    public msgclass (string prfx)
    {
      prefix = prfx;
    }

    public override void streamCB (string msg)
    {
      Console.Write ("{0}{1}", prefix, msg);
    }
  }

  public class qo1
  {
    public static void Main ()
    {
      // Since the value infinity is never used, we define
      // 'infinity' symbolic purposes only
      const double infinity = 0;
      const int numcon = 1;   /* Number of constraints.             */
      const int numvar = 3;   /* Number of variables.               */

      double[] c = {0.0, -1.0, 0.0};

      mosek.boundkey[]  bkc   = {mosek.boundkey.lo};
      double[] blc = {1.0};
      double[] buc = {infinity};

      mosek.boundkey[]  bkx   = {mosek.boundkey.lo,
                                 mosek.boundkey.lo,
                                 mosek.boundkey.lo
                                };
      double[] blx  = {0.0,
                       0.0,
                       0.0
                      };
      double[] bux  = { +infinity,
                        +infinity,
                        +infinity
                      };

      int[][]    asub  = { new int[] {0},   new int[] {0},   new int[] {0}};
      double[][] aval  = { new double[] {1.0}, new double[] {1.0}, new double[] {1.0}};

      mosek.Task task = null;
      mosek.Env  env = null;
      double[] xx  = new double[numvar];
      try
      {
        // Make mosek environment.
        env  = new mosek.Env ();
        // Create a task object linked with the environment env.
        task = new mosek.Task (env, 0, 0);
        // Directs the log task stream to the user specified
        // method task_msg_obj.streamCB
        task.set_Stream (mosek.streamtype.log, new msgclass (""));

        /* Give MOSEK an estimate of the size of the input data.
             This is done to increase the speed of inputting data.
             However, it is optional. */
        /* Append 'numcon' empty constraints.
             The constraints will initially have no bounds. */
        task.appendcons(numcon);

        /* Append 'numvar' variables.
             The variables will initially be fixed at zero (x=0). */
        task.appendvars(numvar);

        for (int j = 0; j < numvar; ++j)
        {
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
         * The lower triangular part of the Q
         * matrix in the objective is specified.
         */

        int[]    qsubi = {0,   1,    2,   2  };
        int[]    qsubj = {0,   1,    0,   2  };
        double[] qval =  {2.0, 0.2, -1.0, 2.0};

        /* Input the Q for the objective. */

        task.putobjsense(mosek.objsense.minimize);

        task.putqobj(qsubi, qsubj, qval);

        task.optimize();

        // Print a summary containing information
        //   about the solution for debugging purposes
        task.solutionsummary(mosek.streamtype.msg);

        mosek.solsta solsta;
        /* Get status information about the solution */
        task.getsolsta(mosek.soltype.itr, out solsta);
        switch (solsta)
        {
          case mosek.solsta.optimal:
            task.getxx(mosek.soltype.itr, // Interior point solution.
                       xx);

            Console.WriteLine ("Optimal primal solution\n");
            for (int j = 0; j < numvar; ++j)
              Console.WriteLine ("x[{0}]:", xx[j]);
            break;
          case mosek.solsta.dual_infeas_cer:
          case mosek.solsta.prim_infeas_cer:
            Console.WriteLine("Primal or dual infeasibility.\n");
            break;
          case mosek.solsta.unknown:
            Console.WriteLine("Unknown solution status.\n");
            break;
          default:
            Console.WriteLine("Other solution status");
            break;
        }
      }
      catch (mosek.Exception e)
      {
        Console.WriteLine (e);
        throw;
      }
      finally
      {
        if (task != null) task.Dispose ();
        if (env  != null)  env.Dispose ();
      }
    } /* Main */
  }
}