/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      solvebasis.java

   Purpose :   To demonstrate the usage of
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

package com.mosek.example;
import mosek.*;

public class solvebasis {
  public static void main(String[] args) {
    // Since the value infinity is never used, we define
    // 'infinity' symbolic purposes only
    double
    infinity = 0;

    double[] c    = {1.0, 1.0};

    int[]    ptrb = {0, 2};
    int[]    ptre = {2 , 4};

    int[]    asub = {0, 1,
                     0, 1
                    };

    double[] aval = {1.0, 1.0,
                     2.0, 1.0
                    };

    mosek.boundkey[] bkc  = {
      mosek.boundkey.up,
      mosek.boundkey.up
    };
    double[]  blc  = { -infinity,
                       -infinity
                     };

    double[]  buc  = {2.0,
                      6.0
                     };

    mosek.boundkey[] bkx = {
      mosek.boundkey.lo,
      mosek.boundkey.lo
    };
    double[]  blx  = {0.0,
                      0.0
                     };
    double[]  bux  = { +infinity,
                       +infinity
                     };

    int    numvar = 2;
    int    numcon = 2;

    double[] w1 = {2.0, 6.0};
    double[] w2 = {1.0, 0.0};


    try (Env  env  = new Env();
         Task task = new Task(env, 0, 0)) {
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

      System.out.println("optimize");
      try {
        task.optimize();
      } catch (mosek.Warning e) {
        System.out.println("Mosek warning:");
        System.out.println(e.toString());
      }

      int[] basis = new int[numcon];
      task.initbasissolve(basis);

      //List basis variables corresponding to columns of B
      int[] varsub = {0, 1};
      for (int i = 0; i < numcon; i++) {
        System.out.println("Basis i:" + i + " Basis:" + basis[i]);
        if (basis[varsub[i]] < numcon) {
          System.out.println("Basis variable no " + i + " is xc" +
                             basis[i]);
        } else {
          int index = basis[i] - numcon;
          System.out.println("Basis variable no " + i + " is x" +
                             index);
        }
      }

      // solve Bx = w1
      // varsub contains index of non-zeros in b.
      //  On return b contains the solution x and
      // varsub the index of the non-zeros in x.

      int[] nz = new int[1];
      nz[0] = 2;

      task.solvewithbasis(0, nz, varsub, w1);
      System.out.println("nz =" + nz[0]);
      System.out.println("\nSolution to Bx = w1:\n");

      for (int i = 0; i < nz[0]; i++) {
        if (basis[varsub[i]] < numcon) {
          System.out.println("xc" + basis[varsub[i]] + "=" + w1[varsub[i]]);
        } else {
          int index = basis[varsub[i]] - numcon;
          System.out.println("x" + index + " = " + w1[varsub[i]]);
        }
      }

      // Solve B^Tx = w2
      nz[0] = 2;
      varsub[0] = 0;
      varsub[1] = 1;

      task.solvewithbasis(1, nz, varsub, w2);

      System.out.println("\nSolution to B^Tx = w2:\n");

      for (int i = 0; i < nz[0]; i++) {
        if (basis[varsub[i]] < numcon) {
          System.out.println("xc" + basis[varsub[i]] + " = " + w2[varsub[i]]);
        } else {
          int index = basis[varsub[i]] - numcon;
          System.out.println("x" + index + " = " + w2[varsub[i]]);
        }
      }

    } catch (mosek.Exception e)
      /* Catch both Error and Warning */
    {
      System.out.println("An error was encountered");
      System.out.println(e.getMessage());
      throw e;
    }
  }
}