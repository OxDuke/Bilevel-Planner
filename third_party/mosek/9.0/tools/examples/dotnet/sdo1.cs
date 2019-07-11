/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      sdo1.cs

   Purpose:   Solves the following small semidefinite optimization problem
              using the MOSEK API.

     minimize    Tr [2, 1, 0; 1, 2, 1; 0, 1, 2]*X + x0

     subject to  Tr [1, 0, 0; 0, 1, 0; 0, 0, 1]*X + x0           = 1
                 Tr [1, 1, 1; 1, 1, 1; 1, 1, 1]*X      + x1 + x2 = 0.5
                 (x0,x1,x2) \in Q,  X \in PSD
*/
using System;

namespace mosek.example
{
  public class sdo1
  {
    public static void Main(string[] args)
    {
      int    numcon    = 2;  /* Number of constraints.              */
      int    numvar    = 3;  /* Number of conic quadratic variables */
      int[]  dimbarvar = { 3 };         /* Dimensions of semidefinite cones */
      int[]  lenbarvar = { 3 * (3 + 1) / 2 }; /* Number of scalar SD variables  */

      mosek.boundkey[] bkc = { mosek.boundkey.fx, mosek.boundkey.fx };
      double[]     blc     = { 1.0, 0.5 };
      double[]     buc     = { 1.0, 0.5 };

      int[]        barc_i  = { 0,   1,   1,   2,   2 },
                   barc_j  = { 0,   0,   1,   1,   2 };
      double[]     barc_v  = { 2.0, 1.0, 2.0, 1.0, 2.0 };

      int[][]      asub    = { new int[] {0},   new int[] {1, 2}}; /* column subscripts of A */
      double[][]   aval    = { new double[] {1.0}, new double[] {1.0, 1.0}};

      int[][]      bara_i  = { new int[] {0,   1,   2},   new int[] {0,   1 ,  2,   1,   2,   2 } },
                   bara_j  = { new int[] {0,   1,   2},   new int[] {0,   0 ,  0,   1,   1,   2 } };
      double[][]   bara_v  = { new double[] {1.0, 1.0, 1.0}, new double[] {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
      int[]        conesub = { 0, 1, 2 };



      using (mosek.Env env = new mosek.Env())
      {
        // Create a task object.
        using (mosek.Task task = new mosek.Task(env, 0, 0))
        {
          // Directs the log task stream to the user specified
          // method msgclass.streamCB
          task.set_Stream (mosek.streamtype.log, new msgclass (""));
          /* Append 'NUMCON' empty constraints.
             The constraints will initially have no bounds. */
          task.appendcons(numcon);

          /* Append 'NUMVAR' variables.
             The variables will initially be fixed at zero (x=0). */
          task.appendvars(numvar);

          /* Append 'NUMBARVAR' semidefinite variables. */
          task.appendbarvars(dimbarvar);

          /* Optionally add a constant term to the objective. */
          task.putcfix(0.0);

          /* Set the linear term c_j in the objective.*/
          task.putcj(0, 1.0);

          for (int j = 0; j < numvar; ++j)
            task.putvarbound(j, mosek.boundkey.fr, -0.0, 0.0);

          /* Set the linear term barc_j in the objective.*/
          {
            long[] idx = new long[1];
            double[] falpha = { 1.0 };
            idx[0] = task.appendsparsesymmat(dimbarvar[0],
                                             barc_i,
                                             barc_j,
                                             barc_v);
            task.putbarcj(0, idx, falpha);
          }

          /* Set the bounds on constraints.
            for i=1, ...,numcon : blc[i] <= constraint i <= buc[i] */

          for (int i = 0; i < numcon; ++i)
            task.putconbound(i,           /* Index of constraint.*/
                             bkc[i],      /* Bound key.*/
                             blc[i],      /* Numerical value of lower bound.*/
                             buc[i]);     /* Numerical value of upper bound.*/

          /* Input A row by row */
          for (int i = 0; i < numcon; ++i)
            task.putarow(i,
                         asub[i],
                         aval[i]);

          /* Append the conic quadratic cone */
          task.appendcone(mosek.conetype.quad,
                          0.0,
                          conesub);

          /* Add the first row of barA */
          {
            long[] idx = new long[1];
            double[] falpha = {1.0};
            task.appendsparsesymmat(dimbarvar[0],
                                    bara_i[0],
                                    bara_j[0],
                                    bara_v[0],
                                    out idx[0]);

            task.putbaraij(0, 0, idx, falpha);
          }

          {
            long[] idx = new long[1];
            double[] falpha = {1.0};
            /* Add the second row of barA */
            task.appendsparsesymmat(dimbarvar[0],
                                    bara_i[1],
                                    bara_j[1],
                                    bara_v[1],
                                    out idx[0]);

            task.putbaraij(1, 0, idx, falpha);
          }

          /* Run optimizer */
          task.optimize();

          /* Print a summary containing information
             about the solution for debugging purposes*/
          task.solutionsummary (mosek.streamtype.msg);

          mosek.solsta solsta;
          task.getsolsta (mosek.soltype.itr, out solsta);

          switch (solsta)
          {
            case mosek.solsta.optimal:
              double[] xx = new double[numvar];
              double[] barx = new double[lenbarvar[0]];

              task.getxx(mosek.soltype.itr, xx);
              task.getbarxj(mosek.soltype.itr,    /* Request the interior solution. */
                            0,
                            barx);
              Console.WriteLine("Optimal primal solution");
              for (int i = 0; i < numvar; ++i)
                Console.WriteLine("x[{0}]   : {1}", i, xx[i]);

              for (int i = 0; i < lenbarvar[0]; ++i)
                Console.WriteLine("barx[{0}]: {1}", i, barx[i]);
              break;
            case mosek.solsta.dual_infeas_cer:
            case mosek.solsta.prim_infeas_cer:
              Console.WriteLine("Primal or dual infeasibility certificate found.");
              break;
            case mosek.solsta.unknown:
              Console.WriteLine("The status of the solution could not be determined.");
              break;
            default:
              Console.WriteLine("Other solution status.");
              break;
          }
        }
      }
    }
  }



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
}