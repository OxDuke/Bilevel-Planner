/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File     :  solvelinear.cs

  Purpose  :  To demonstrate the usage of MSK_solvewithbasis
              when solving the linear system:

  1.0  x1             = b1
  -1.0  x0  +  1.0  x1 = b2

  with two different right hand sides

  b = (1.0, -2.0)

  and

  b = (7.0, 0.0)
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

  public class solvelinear
  {
    static public void setup(mosek.Task task,
                             double[][] aval,
                             int[][] asub,
                             int[] ptrb,
                             int[] ptre,
                             int numvar,
                             int[] basis
                            )
    {
      // Since the value infinity is never used, we define
      // 'infinity' symbolic purposes only
      double
      infinity = 0;

      mosek.stakey[] skx = new mosek.stakey [numvar];
      mosek.stakey[] skc = new mosek.stakey [numvar];

      for (int i = 0; i < numvar ; ++i)
      {
        skx[i] = mosek.stakey.bas;
        skc[i] = mosek.stakey.fix;
      }

      task.appendvars(numvar);
      task.appendcons(numvar);

      for (int i = 0; i < numvar ; ++i)
        task.putacol(i,
                     asub[i],
                     aval[i]);

      for (int i = 0 ; i < numvar ; ++i)
        task.putconbound(
          i,
          mosek.boundkey.fx,
          0.0,
          0.0);

      for (int i = 0 ; i < numvar ; ++i)
        task.putvarbound(
          i,
          mosek.boundkey.fr,
          -infinity,
          infinity);

      /* Define a basic solution by specifying
         status keys for variables & constraints. */

      task.deletesolution(mosek.soltype.bas);

      task.putskcslice(mosek.soltype.bas, 0, numvar, skc);
      task.putskxslice(mosek.soltype.bas, 0, numvar, skx);

      task.initbasissolve(basis);
    }

    public static void Main ()
    {
      const int numcon = 2;
      const int numvar = 2;

      double[][]
      aval   = new double[numvar][];

      aval[0] = new double[] { -1.0 };
      aval[1] = new double[] {1.0, 1.0};


      int[][]
      asub = new int[numvar][];

      asub[0] = new int[] {1};
      asub[1] = new int[] {0, 1};

      int []      ptrb  = {0, 1};
      int []      ptre  = {1, 3};

      int[]       bsub  = new int[numvar];
      double[]    b     = new double[numvar];
      int[]       basis = new int[numvar];

      try
      {
        using (mosek.Env env = new mosek.Env())
        {
          using (mosek.Task task = new mosek.Task(env))
          {
            // Directs the log task stream to the user specified
            // method task_msg_obj.streamCB
            task.set_Stream(mosek.streamtype.log, new msgclass ("[task]"));
            /* Put A matrix and factor A.
               Call this function only once for a given task. */

            setup(
              task,
              aval,
              asub,
              ptrb,
              ptre,
              numvar,
              basis
            );

            /* now solve rhs */
            b[0] = 1;
            b[1] = -2;
            bsub[0] = 0;
            bsub[1] = 1;
            int nz = 2;

            task.solvewithbasis(0, ref nz, bsub, b);
            Console.WriteLine ("\nSolution to Bx = b:\n\n");

            /* Print solution and show correspondents
               to original variables in the problem */
            for (int i = 0; i < nz; ++i)
            {
              if (basis[bsub[i]] < numcon)
                Console.WriteLine ("This should never happen\n");
              else
                Console.WriteLine ("x{0} = {1}\n", basis[bsub[i]] - numcon , b[bsub[i]] );
            }

            b[0] = 7;
            bsub[0] = 0;
            nz = 1;

            task.solvewithbasis(0, ref nz, bsub, b);

            Console.WriteLine ("\nSolution to Bx = b:\n\n");
            /* Print solution and show correspondents
               to original variables in the problem */
            for (int i = 0; i < nz; ++i)
            {
              if (basis[bsub[i]] < numcon)
                Console.WriteLine ("This should never happen\n");
              else
                Console.WriteLine ("x{0} = {1}\n", basis[bsub[i]] - numcon , b[bsub[i]] );
            }
          }
        }
      }
      catch (mosek.Exception e)
      {
        Console.WriteLine (e.Code);
        Console.WriteLine (e);
        throw;
      }
    }
  }
}