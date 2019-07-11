/*
  File : portfolio_1_basic.cs

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Implements a basic portfolio optimization model.
*/
using System;

namespace mosek.example
{
  /* Log handler class */
  class msgclass : mosek.Stream
  {
    string prefix;
    public msgclass (string prfx) { prefix = prfx; }

    public override void streamCB (string msg)
    {
      Console.Write ("{0}{1}", prefix, msg);
    }
  }

  public class portfolio_1_basic
  {
    public static void Main (String[] args)
    {
      const int n = 3;

      // Since the value infinity is never used, we define
      // 'infinity' for symbolic purposes only
      double infinity = 0.0;
      double gamma = 0.05;
      double[] mu = {0.1073,  0.0737,  0.0627};
      double[,] GT = {
        {0.1667,  0.0232,  0.0013},
        {0.0000,  0.1033, -0.0022},
        {0.0000,  0.0000,  0.0338}
      };
      double[] x0 = {0.0, 0.0, 0.0};
      double   w = 1.0;
      double   totalBudget;

      int numvar = 2 * n + 1;
      int numcon = n + 1;

      // Offset of variables into the API variable.
      int offsetx = 0;
      int offsets = n;
      int offsett = n + 1;

      // Make mosek environment.
      using (mosek.Env env = new mosek.Env())
      {
        // Create a task object.
        using (mosek.Task task = new mosek.Task(env, 0, 0))
        {
          // Directs the log task stream
          task.set_Stream(mosek.streamtype.log, new msgclass (""));

          // Constraints.
          task.appendcons(numcon);

          // Constraint bounds. Compute total budget.
          totalBudget = w;
          for (int i = 0; i < n; ++i)
          {
            totalBudget += x0[i];
            /* Constraint bounds c^l = c^u = 0 */
            task.putconbound(i + 1, mosek.boundkey.fx, 0.0, 0.0);
            task.putconname(i + 1, "GT[" + (i + 1) + "]");
          }
          /* The total budget constraint c^l = c^u = totalBudget in first row of A. */
          task.putconbound(0, mosek.boundkey.fx, totalBudget, totalBudget);
          task.putconname(0, "budget");

          // Variables.
          task.appendvars(numvar);

          /* x variables. */
          for (int j = 0; j < n; ++j)
          {
            /* Return of asset j in the objective */
            task.putcj(offsetx + j, mu[j]);
            /* Coefficients in the first row of A */
            task.putaij(0, offsetx + j, 1.0);
            /* No short-selling - x^l = 0, x^u = inf */
            task.putvarbound(offsetx + j, mosek.boundkey.lo, 0.0, infinity);
            task.putvarname(offsetx + j, "x[" + (j + 1) + "]");            
          }

          /* s variable is a constant equal to gamma. */
          task.putvarbound(offsets, mosek.boundkey.fx, gamma, gamma);
          task.putvarname(offsets, "s");

          /* t variables (t = GT*x). */
          for (int j = 0; j < n; ++j)
          {
            /* Copying the GT matrix in the appropriate block of A */
            for (int k = 0; k < n; ++k)
              if ( GT[k,j] != 0.0 )
                task.putaij(1 + k, offsetx + j, GT[k,j]);
            /* Diagonal -1 entries in a block of A */
            task.putaij(1 + j, offsett + j, -1.0);
            /* Free - no bounds */
            task.putvarbound(offsett + j, mosek.boundkey.fr, -infinity, infinity);
            task.putvarname(offsett + j, "t[" + (j + 1) + "]");
          }

          /* Define the cone spanned by (s, t), i.e. of dimension n + 1 */
          int[] csub = new int[n + 1];
          csub[0] = offsets;
          for(int j = 0; j< n; j++) csub[j + 1] = offsett + j;
          task.appendcone( mosek.conetype.quad,
                           0.0, /* For future use only, can be set to 0.0 */
                           csub );
          task.putconename(0, "stddev");

          /* A maximization problem */
          task.putobjsense(mosek.objsense.maximize);

          task.optimize();

          /* Display solution summary for quick inspection of results */
          task.solutionsummary(mosek.streamtype.log);

          task.writedata("dump.opf");

          /* Read the results */
          double expret = 0.0;
          double[] xx = new double[n + 1];

          task.getxxslice(mosek.soltype.itr, 0, offsets + 1, xx);
          for (int j = 0; j < n; ++j)
            expret += mu[j] * xx[j + offsetx];

          Console.WriteLine("\nExpected return {0:E} for gamma {1:E}", expret, xx[offsets]);
        }
      }
    }
  }
}