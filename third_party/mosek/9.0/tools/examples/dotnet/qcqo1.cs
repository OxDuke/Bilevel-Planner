/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      qcqo1.cs

   Purpose:   Demonstrate how to solve a quadratic
              optimization problem using the MOSEK API.

              minimize  x0^2 + 0.1 x1^2 +  x2^2 - x0 x2 - x1
              s.t 1 <=  x0 + x1 + x2 - x0^2 - x1^2 - 0.1 x2^2 + 0.2 x0 x2
              x >= 0
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

  public class qcqo1
  {
    public static void Main ()
    {
      const double inf = 0.0; /* We don't actually need any value for infinity */

      const int numcon = 1;   /* Number of constraints.             */
      const int numvar = 3;   /* Number of variables.               */

      mosek.boundkey[]
      bkc = { mosek.boundkey.lo },
      bkx = { mosek.boundkey.lo, mosek.boundkey.lo, mosek.boundkey.lo };
      int[][]  asub  = { new int[] {0}, new int[] {0}, new int[] {0} };
      double[][] aval  = { new double[]{1.0}, new double[]{1.0}, new double[]{1.0} };

      double[]
      blc  = { 1.0 },
      buc  = { inf },
      c    = { 0.0, -1.0, 0.0 },
      blx  = { 0.0, 0.0, 0.0 },
      bux  = { inf, inf, inf },
      xx   = new double[numvar];
      try
      {
        using (mosek.Env env = new mosek.Env())
        {
          using (mosek.Task task = new mosek.Task(env))
          {
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

            {
              int[]
              qsubi = { 0, 1, 2, 2 },
              qsubj = { 0, 1, 0, 2 };
              double[]
              qval = { 2.0, 0.2, -1.0, 2.0 };

              /* Input the Q for the objective. */

              task.putqobj(qsubi, qsubj, qval);
            }
            /*
             * The lower triangular part of the Q^0
             * matrix in the first constraint is specified.
             * This corresponds to adding the term
             *  - x0^2 - x1^2 - 0.1 x2^2 + 0.2 x0 x2
             */
            {
              int[]
              qsubi = { 0, 1, 2, 2 },
              qsubj = { 0, 1, 2, 0 };
              double[]
              qval = { -2.0, -2.0, -0.2, 0.2 };

              /* put Q^0 in constraint with index 0. */

              task.putqconk (0,
                             qsubi,
                             qsubj,
                             qval);
            }

            task.putobjsense(mosek.objsense.minimize);

            task.optimize();

            // Print a summary containing information
            //   about the solution for debugging purposes
            task.solutionsummary(mosek.streamtype.msg);

            mosek.solsta solsta;
            /* Get status information about the solution */
            task.getsolsta(mosek.soltype.itr, out solsta);

            task.getxx(mosek.soltype.itr, // Basic solution.
                       xx);

            switch (solsta)
            {
              case mosek.solsta.optimal:
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
        }
      }
      catch (mosek.Exception e)
      {
        Console.WriteLine (e);
        throw;
      }

    } /* Main */
  }
}