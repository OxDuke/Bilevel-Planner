/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:    lo2.cs

   Purpose: Demonstrates how to solve small linear
            optimization problem using the MOSEK C# API.
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

  public class lo2
  {
    public static void Main ()
    {
      const int numcon = 3;
      const int numvar = 4;

      // Since the value infinity is never used, we define
      // 'infinity' symbolic purposes only
      double
      infinity = 0;

      double[] c    = {3.0, 1.0, 5.0, 1.0};
      int[][]  asub = {
            new int[] {0, 1, 2},
            new int[] {0, 1, 2, 3},
            new int[] {1, 3}
      };
      double[][] aval = {
            new double[] {3.0, 1.0, 2.0},
            new double[] {2.0, 1.0, 3.0, 1.0},
            new double[] {2.0, 3.0}
      };


      mosek.boundkey[] bkc  = {mosek.boundkey.fx,
                               mosek.boundkey.lo,
                               mosek.boundkey.up
                              };

      double[] blc  = {30.0,
                       15.0,
                       -infinity
                      };
      double[] buc  = {30.0,
                       +infinity,
                       25.0
                      };
      mosek.boundkey[]  bkx  = {mosek.boundkey.lo,
                                mosek.boundkey.ra,
                                mosek.boundkey.lo,
                                mosek.boundkey.lo
                               };
      double[]  blx  = {0.0,
                        0.0,
                        0.0,
                        0.0
                       };
      double[]  bux  = { +infinity,
                         10.0,
                         +infinity,
                         +infinity
                       };

      mosek.Task task = null;
      mosek.Env  env  = null;

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

        /* Optionally add a constant term to the objective. */
        task.putcfix(0.0);

        for (int j = 0; j < numvar; ++j)
        {
          /* Set the linear term c_j in the objective.*/
          task.putcj(j, c[j]);
          /* Set the bounds on variable j.
                    blx[j] <= x_j <= bux[j] */
          task.putvarbound(j, bkx[j], blx[j], bux[j]);
        }
        /* Set the bounds on constraints.
                for i=1, ...,numcon : blc[i] <= constraint i <= buc[i] */
        for (int i = 0; i < numcon; ++i)
        {
          task.putconbound(i, bkc[i], blc[i], buc[i]);

          /* Input row i of A */
          task.putarow(i,                     /* Row index.*/
                       asub[i],               /* Column indexes of non-zeros in row i.*/
                       aval[i]);              /* Non-zero Values of row i. */
        }

        task.putobjsense(mosek.objsense.maximize);
        task.optimize();

        // Print a summary containing information
        //   about the solution for debugging purposes
        task.solutionsummary(mosek.streamtype.msg);

        mosek.solsta solsta;
        /* Get status information about the solution */
        task.getsolsta(mosek.soltype.bas, out solsta);
        task.getxx(mosek.soltype.bas, // Basic solution.
                   xx);

        switch (solsta)
        {
          case mosek.solsta.optimal:
            Console.WriteLine ("Optimal primal solution\n");
            for (int j = 0; j < numvar; ++j)
              Console.WriteLine ("x[{0}]: {1}", j, xx[j]);
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
        Console.WriteLine (e.Code);
        Console.WriteLine (e);
        throw;
      }
      finally
      {
        if (task != null) task.Dispose ();
        if (env  != null)  env.Dispose ();
      }
    }
  }
}