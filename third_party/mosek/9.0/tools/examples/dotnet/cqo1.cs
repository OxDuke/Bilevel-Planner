/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      cqo1.cs

  Purpose:   Demonstrates how to solve a small conic qaudratic
  optimization problem using the MOSEK API.
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

  public class cqo1
  {
    public static void Main ()
    {
      const int numcon = 1;
      const int numvar = 6;

      // Since the value infinity is never used, we define
      // 'infinity' symbolic purposes only
      double infinity = 0;

      mosek.boundkey[] bkc    = { mosek.boundkey.fx };
      double[] blc = { 1.0 };
      double[] buc = { 1.0 };

      mosek.boundkey[] bkx = {mosek.boundkey.lo,
                              mosek.boundkey.lo,
                              mosek.boundkey.lo,
                              mosek.boundkey.fr,
                              mosek.boundkey.fr,
                              mosek.boundkey.fr
                             };
      double[] blx = { 0.0,
                       0.0,
                       0.0,
                       -infinity,
                       -infinity,
                       -infinity
                     };
      double[] bux = { +infinity,
                       +infinity,
                       +infinity,
                       +infinity,
                       +infinity,
                       +infinity
                     };

      double[] c   = { 0.0,
                       0.0,
                       0.0,
                       1.0,
                       1.0,
                       1.0
                     };

      double[][] aval = {
              new double[] {1.0},
              new double[] {1.0},
              new double[] {2.0}
      };

      int[][] asub = {
            new int[] {0},
            new int[] {0},
            new int[] {0}
      };

      int[] csub = new int[3];

      // Make mosek environment.
      using (mosek.Env env = new mosek.Env())
      {
        // Create a task object.
        using (mosek.Task task = new mosek.Task(env, 0, 0))
        {
          // Directs the log task stream to the user specified
          // method msgclass.streamCB
          task.set_Stream (mosek.streamtype.log, new msgclass (""));

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
          }

          for (int j = 0; j < aval.Length; ++j)
            /* Input column j of A */
            task.putacol(j,          /* Variable (column) index.*/
                         asub[j],     /* Row index of non-zeros in column j.*/
                         aval[j]);    /* Non-zero Values of column j. */

          /* Set the bounds on constraints.
               for i=1, ...,numcon : blc[i] <= constraint i <= buc[i] */
          for (int i = 0; i < numcon; ++i)
            task.putconbound(i, bkc[i], blc[i], buc[i]);

          csub[0] = 3;
          csub[1] = 0;
          csub[2] = 1;
          task.appendcone(mosek.conetype.quad,
                          0.0, /* For future use only, can be set to 0.0 */
                          csub);
          
          csub[0] = 4;
          csub[1] = 5;
          csub[2] = 2;
          task.appendcone(mosek.conetype.rquad, 0.0, csub);
          
          task.putobjsense(mosek.objsense.minimize);
          task.optimize();
          // Print a summary containing information
          //   about the solution for debugging purposes
          task.solutionsummary(mosek.streamtype.msg);

          mosek.solsta solsta;
          /* Get status information about the solution */
          task.getsolsta(mosek.soltype.itr, out solsta);

          double[] xx  = new double[numvar];

          task.getxx(mosek.soltype.itr, // Basic solution.
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
      }
    }
  }
}