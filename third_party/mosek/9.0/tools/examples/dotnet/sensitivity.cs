/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      sensitivity.cs

  Purpose:   To demonstrate how to perform sensitivity
  analysis from the API on a small problem:

  minimize

  obj: +1 x11 + 2 x12 + 5 x23 + 2 x24 + 1 x31 + 2 x33 + 1 x34
  st
  c1:   +  x11 +   x12                                           <= 400
  c2:                  +   x23 +   x24                           <= 1200
  c3:                                  +   x31 +   x33 +   x34   <= 1000
  c4:   +  x11                         +   x31                   = 800
  c5:          +   x12                                           = 100
  c6:                  +   x23                 +   x33           = 500
  c7:                          +   x24                 +   x34   = 500

  The example uses basis type sensitivity analysis.
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

  public class sensitivity
  {
    public static void Main ()
    {
      const double
      infinity = 0;

      mosek.boundkey[] bkc = new mosek.boundkey[] {
        mosek.boundkey.up, mosek.boundkey.up,
        mosek.boundkey.up, mosek.boundkey.fx,
        mosek.boundkey.fx, mosek.boundkey.fx,
        mosek.boundkey.fx
      };

      mosek.boundkey[] bkx = new mosek.boundkey[] {
        mosek.boundkey.lo, mosek.boundkey.lo,
        mosek.boundkey.lo, mosek.boundkey.lo,
        mosek.boundkey.lo, mosek.boundkey.lo,
        mosek.boundkey.lo
      };

      int[] ptrb = new int[] {0, 2, 4, 6, 8, 10, 12};
      int[] ptre = new int[] {2, 4, 6, 8, 10, 12, 14};
      int[] sub = new int[] {0, 3, 0, 4, 1, 5, 1, 6, 2, 3, 2, 5, 2, 6};
      double[] blc = new double[] {
        -infinity, -infinity,
        -infinity, 800, 100, 500, 500
      };

      double[] buc = new double[] {400, 1200, 1000, 800, 100, 500, 500};
      double[] c   = new double[] {1.0, 2.0, 5.0, 2.0, 1.0, 2.0, 1.0};
      double[] blx = new double[] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
      double[] bux = new double[] {infinity,
                                   infinity,
                                   infinity,
                                   infinity,
                                   infinity,
                                   infinity,
                                   infinity
                                  };

      double[] val = new double[] {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                   1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
                                  };

      int numcon = 7;  /* Number of constraints.             */
      int numvar = 7;  /* Number of variables.               */
      try
      {
        using (mosek.Env env = new mosek.Env())
        {
          using (mosek.Task task = new mosek.Task(env))
          {
            // Directs the log task stream to the user specified
            // method task_msg_obj.streamCB
            task.set_Stream(mosek.streamtype.log, new msgclass ("[task]"));

            task.inputdata(numcon, numvar,
                           c,
                           0.0,
                           ptrb,
                           ptre,
                           sub,
                           val,
                           bkc,
                           blc,
                           buc,
                           bkx,
                           blx,
                           bux);

            /* A maximization problem */
            task.putobjsense(mosek.objsense.minimize);

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


            /* Analyze upper bound on c1 and the equality constraint on c4 */
            int[] subi  = new int [] {0, 3};
            mosek.mark[] marki = new mosek.mark[] {mosek.mark.up,
                                                   mosek.mark.up
                                                  };

            /* Analyze lower bound on the variables x12 and x31 */
            int[] subj  = new int [] {1, 4};
            mosek.mark[] markj = new mosek.mark[] {mosek.mark.lo,
                                                   mosek.mark.lo
                                                  };

            double[] leftpricei  = new  double[2];
            double[] rightpricei = new  double[2];
            double[] leftrangei  = new  double[2];
            double[] rightrangei = new  double[2];
            double[] leftpricej  = new  double[2];
            double[] rightpricej = new  double[2];
            double[] leftrangej  = new  double[2];
            double[] rightrangej = new  double[2];


            task.primalsensitivity( subi,
                                    marki,
                                    subj,
                                    markj,
                                    leftpricei,
                                    rightpricei,
                                    leftrangei,
                                    rightrangei,
                                    leftpricej,
                                    rightpricej,
                                    leftrangej,
                                    rightrangej);

            Console.Write("Results from sensitivity analysis on bounds:\n");

            Console.Write("For constraints:\n");
            for (int i = 0; i < 2; ++i)
              Console.Write(
                "leftprice = {0}, rightprice = {1},leftrange = {2}, rightrange ={3}\n",
                leftpricei[i], rightpricei[i], leftrangei[i], rightrangei[i]);

            Console.Write("For variables:\n");
            for (int i = 0; i < 2; ++i)
              Console.Write(
                "leftprice = {0}, rightprice = {1},leftrange = {2}, rightrange ={3}\n",
                leftpricej[i], rightpricej[i], leftrangej[i], rightrangej[i]);


            double[] leftprice  = new  double[2];
            double[] rightprice = new  double[2];
            double[] leftrange  = new  double[2];
            double[] rightrange = new  double[2];
            int[] subc = new int[] {2, 5};

            task.dualsensitivity(  subc,
                                   leftprice,
                                   rightprice,
                                   leftrange,
                                   rightrange
                                );

            Console.Write("Results from sensitivity analysis on objective coefficients:");

            for (int i = 0; i < 2; ++i)
              Console.Write(
                "leftprice = {0}, rightprice = {1},leftrange = {2}, rightrange = {3}\n",
                leftprice[i], rightprice[i], leftrange[i], rightrange[i]);
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