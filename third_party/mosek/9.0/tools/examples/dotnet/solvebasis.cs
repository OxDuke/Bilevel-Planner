/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File     : solvebasis.cs

  Purpose  :  To demonstrate the usage of
              MSK_solvewithbasis on the problem:

              maximize  x0 + x1
              st.
                      x0 + 2.0 x1 <= 2
                      x0  +    x1 <= 6
                      x0 >= 0, x1>= 0

               The problem has the slack variables
               xc0, xc1 on the constraints
               and the variabels x0 and x1.

               maximize  x0 + x1
               st.
                  x0 + 2.0 x1 -xc1       = 2
                  x0  +    x1       -xc2 = 6
                  x0 >= 0, x1>= 0,
                  xc1 <=  0 , xc2 <= 0
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

  public class solvebasis
  {
    public static void Main ()
    {
      const int numcon = 2;
      const int numvar = 2;

      // Since the value infinity is never used, we define
      // 'infinity' symbolic purposes only
      double
      infinity = 0;

      double[] c    = {1.0, 1.0};
      int[]    ptrb = {0, 2};
      int[]    ptre = {2, 3};
      int[]    asub = {0, 1,
                       0, 1
                      };
      double[] aval = {1.0, 1.0,
                       2.0, 1.0
                      };
      mosek.boundkey[] bkc  = {mosek.boundkey.up,
                               mosek.boundkey.up
                              };

      double[] blc  = { -infinity,
                        -infinity
                      };
      double[] buc  = {2.0,
                       6.0
                      };

      mosek.boundkey[]  bkx  = {mosek.boundkey.lo,
                                mosek.boundkey.lo
                               };
      double[]  blx  = {0.0,
                        0.0
                       };

      double[]  bux  = { +infinity,
                         +infinity
                       };
      double[] w1 = {2.0, 6.0};
      double[] w2 = {1.0, 0.0};
      try
      {
        using (mosek.Env env = new mosek.Env())
        {
          using (mosek.Task task = new mosek.Task(env))
          {
            task.set_Stream (mosek.streamtype.log, new msgclass ("[task]"));
            task.inputdata(numcon, numvar,
                           c,
                           0.0,
                           ptrb,
                           ptre,
                           asub,
                           aval,
                           bkc,
                           blc,
                           buc,
                           bkx,
                           blx,
                           bux);
            task.putobjsense(mosek.objsense.maximize);
            try
            {
              task.optimize();
            }
            catch (mosek.Warning w)
            {
              Console.WriteLine("Mosek warning:");
              Console.WriteLine (w.Code);
              Console.WriteLine (w);
            }

            int[] basis = new int[numcon];
            task.initbasissolve(basis);

            //List basis variables corresponding to columns of B
            int[] varsub = {0, 1};
            for (int i = 0; i < numcon; i++) {
              if (basis[varsub[i]] < numcon)
                Console.WriteLine ("Basis variable no {0} is xc{1}",
                                   i,
                                   basis[i]);
              else
                Console.WriteLine ("Basis variable no {0} is x{1}",
                                   i,
                                   basis[i] - numcon);
            }

            // solve Bx = w1
            // varsub contains index of non-zeros in b.
            //  On return b contains the solution x and
            // varsub the index of the non-zeros in x.
            int nz = 2;

            task.solvewithbasis(0, ref nz, varsub, w1);
            Console.WriteLine ("nz = {0}", nz);
            Console.WriteLine ("Solution to Bx = w1:\n");

            for (int i = 0; i < nz; i++) {
              if (basis[varsub[i]] < numcon)
                Console.WriteLine ("xc {0} = {1}",
                                   basis[varsub[i]],
                                   w1[varsub[i]] );
              else
                Console.WriteLine ("x{0} = {1}",
                                   basis[varsub[i]] - numcon,
                                   w1[varsub[i]]);
            }

            // Solve B^Tx = w2
            nz = 1;
            varsub[0] = 0; // Only w2[0] is nonzero.

            task.solvewithbasis(1, ref nz, varsub, w2);

            Console.WriteLine ("\nSolution to B^Ty = w2:\n");

            for (int i = 0; i < nz; i++)
            {
              Console.WriteLine ("y {0} = {1}",
                                 varsub[i],
                                 w2[varsub[i]]);
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