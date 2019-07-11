/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      sensitivity.java

   Purpose :   To demonstrate how to perform sensitivity
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
package com.mosek.example;
import mosek.*;

public class sensitivity {
  public static void main (String[] args) {
    // Since the value infinity is never used, we define
    // 'infinity' symbolic purposes only
    double
    infinity = 0;

    try (Env  env  = new Env();
         Task task = new Task(env, 0, 0)) {
      mosek.boundkey[] bkc = {
        mosek.boundkey.up, mosek.boundkey.up,
        mosek.boundkey.up, mosek.boundkey.fx,
        mosek.boundkey.fx, mosek.boundkey.fx,
        mosek.boundkey.fx
      };
      mosek.boundkey[] bkx = {
        mosek.boundkey.lo, mosek.boundkey.lo,
        mosek.boundkey.lo, mosek.boundkey.lo,
        mosek.boundkey.lo, mosek.boundkey.lo,
        mosek.boundkey.lo
      };
      int[] ptrb = {0, 2, 4, 6, 8, 10, 12};
      int[] ptre = {2, 4, 6, 8, 10, 12, 14};
      int[] sub = {0, 3, 0, 4, 1, 5, 1, 6, 2, 3, 2, 5, 2, 6};
      double[] blc = { -infinity, -infinity,
                       -infinity, 800, 100, 500, 500
                     };
      double[] buc = {400, 1200, 1000, 800, 100, 500, 500};
      double[] c   = {1.0, 2.0, 5.0, 2.0, 1.0, 2.0, 1.0};
      double[] blx = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
      double[] bux = {infinity, infinity,
                      infinity, infinity,
                      infinity, infinity,
                      infinity
                     };
      double[] val = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                      1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
                     };

      int numcon = 7;  /* Number of constraints.             */
      int numvar = 7;  /* Number of variables.               */
      int NUMANZ = 14; /* Number of non-zeros in A.           */

      // Directs the log task stream to the user specified
      // method task_msg_obj.print
      task.set_Stream(
        mosek.streamtype.log,
        new mosek.Stream()
      { public void stream(String msg) { System.out.print(msg); }});

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

      task.optimize();

      /* Analyze upper bound on c1 and the equality constraint on c4 */
      int subi[]  = {0, 3};
      mosek.mark marki[] = {mosek.mark.up, mosek.mark.up};

      /* Analyze lower bound on the variables x12 and x31 */
      int subj[]  = {1, 4};
      mosek.mark markj[] = {mosek.mark.lo, mosek.mark.lo};

      double[] leftpricei  = new  double[2];
      double[] rightpricei  = new  double[2];
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

      System.out.println("Results from sensitivity analysis on bounds:\n");

      System.out.println("For constraints:\n");
      for (int i = 0; i < 2; ++i)
        System.out.print("leftprice = " + leftpricei[i] +
                         " rightprice = " + rightpricei[i] +
                         " leftrange = " + leftrangei[i] +
                         " rightrange = " + rightrangei[i] + "\n");

      System.out.print("For variables:\n");
      for (int i = 0; i < 2; ++i)
        System.out.print("leftprice = " + leftpricej[i] +
                         " rightprice = " + rightpricej[i] +
                         " leftrange = " + leftrangej[i] +
                         " rightrange = " + rightrangej[i] + "\n");


      double[] leftprice  = new  double[2];
      double[] rightprice = new  double[2];
      double[] leftrange  = new  double[2];
      double[] rightrange = new  double[2];
      int subc[]  = {2, 5};

      task.dualsensitivity(  subc,
                             leftprice,
                             rightprice,
                             leftrange,
                             rightrange
                          );

      System.out.println(
        "Results from sensitivity analysis on objective coefficients:"
      );

      for (int i = 0; i < 2; ++i)
        System.out.print("leftprice = " + leftprice[i] +
                         " rightprice = " + rightprice[i] +
                         " leftrange = " + leftrange[i] +
                         " rightrange = " +  rightrange[i] + "\n");


    } catch (mosek.Exception e)
      /* Catch both mosek.Error and mosek.Warning */
    {
      System.out.println ("An error or warning was encountered");
      System.out.println (e.getMessage ());
      throw e;
    }
  }
}