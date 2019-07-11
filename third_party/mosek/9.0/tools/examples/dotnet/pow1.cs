/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      pow1.cs

  Purpose: Demonstrates how to solve the problem

    maximize x^0.2*y^0.8 + z^0.4 - x
          st x + y + 0.5z = 2
             x,y,z >= 0
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

  public class ceo1
  {
    public static void Main ()
    {
      const int numcon = 1;
      const int numvar = 6;

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

          /* Set up the linear part of the problem */
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
          task.appendcone(mosek.conetype.ppow, 0.2, new int[3] {0, 1, 3});
          task.appendcone(mosek.conetype.ppow, 0.4, new int[3] {2, 5, 4});
          
          task.putobjsense(mosek.objsense.maximize);
          task.optimize();

          // Print a summary containing information
          // about the solution for debugging purposes
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
              for (int j = 0; j < 3; ++j)
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