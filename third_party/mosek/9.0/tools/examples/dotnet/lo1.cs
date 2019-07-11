/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      lo1.cs

   Purpose:   Demonstrates how to solve small linear
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

  public class lo1
  {
    public static void Main ()
    {
      const int numcon = 3;
      const int numvar = 4;

      // Since the value of infinity is ignored, we define it solely
      // for symbolic purposes
      double infinity = 0;

      double[] c    = {3.0, 1.0, 5.0, 1.0};
      int[][]  asub = { 
            new int[] {0, 1},
            new int[] {0, 1, 2},
            new int[] {0, 1},
            new int[] {1, 2}
      };
      double[][] aval = { 
               new double[] {3.0, 2.0},
               new double[] {1.0, 1.0, 2.0},
               new double[] {2.0, 3.0},
               new double[] {1.0, 3.0}
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

      try
      {
        // Make mosek environment.
        using (mosek.Env env = new mosek.Env())
        {
          // Create a task object.
          using (mosek.Task task = new mosek.Task(env, 0, 0))
          {
            // Directs the log task stream to the user specified
            // method msgclass.streamCB
            task.set_Stream (mosek.streamtype.log, new msgclass (""));

            // Append 'numcon' empty constraints.
            // The constraints will initially have no bounds.
            task.appendcons(numcon);

            // Append 'numvar' variables.
            // The variables will initially be fixed at zero (x=0).
            task.appendvars(numvar);

            for (int j = 0; j < numvar; ++j)
            {
              // Set the linear term c_j in the objective.
              task.putcj(j, c[j]);

              // Set the bounds on variable j.
              // blx[j] <= x_j <= bux[j]
              task.putvarbound(j, bkx[j], blx[j], bux[j]);

              // Input column j of A
              task.putacol(j,                     /* Variable (column) index.*/
                           asub[j],               /* Row index of non-zeros in column j.*/
                           aval[j]);              /* Non-zero Values of column j. */
            }

            // Set the bounds on constraints.
            // blc[i] <= constraint_i <= buc[i]
            for (int i = 0; i < numcon; ++i)
              task.putconbound(i, bkc[i], blc[i], buc[i]);

            // Input the objective sense (minimize/maximize)
            task.putobjsense(mosek.objsense.maximize);

            // Solve the problem
            task.optimize();

            // Print a summary containing information
            // about the solution for debugging purposes
            task.solutionsummary(mosek.streamtype.msg);

            // Get status information about the solution
            mosek.solsta solsta;

            task.getsolsta(mosek.soltype.bas, out solsta);

            switch (solsta)
            {
              case mosek.solsta.optimal:
                double[] xx  = new double[numvar];
                task.getxx(mosek.soltype.bas, // Request the basic solution.
                           xx);

                Console.WriteLine ("Optimal primal solution\n");
                for (int j = 0; j < numvar; ++j)
                  Console.WriteLine ("x[{0}]: {1}", j, xx[j]);
                break;
              case mosek.solsta.dual_infeas_cer:
              case mosek.solsta.prim_infeas_cer:
                Console.WriteLine("Primal or dual infeasibility certificate found.\n");
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
      catch (mosek.Exception e) {
        mosek.rescode res = e.Code;
        Console.WriteLine("Response code {0}\nMessage       {1}", res, e.Message);
      }
    }
  }
}