/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      reoptimization.java

   Purpose :   Demonstrates how to solve a  linear
               optimization problem using the MOSEK API
               and modify and re-optimize the problem.
*/
package com.mosek.example;
import mosek.*;

public class reoptimization {
  public static void main (String[] args) {

    // Since the value infinity is never used, we define
    // 'infinity' symbolic purposes only
    double        infinity = 0;

    int             numcon = 3;
    int             numvar = 3;
    double             c[] = {1.5, 2.5, 3.0 };
    mosek.boundkey   bkc[] = { mosek.boundkey.up,
                               mosek.boundkey.up,
                               mosek.boundkey.up
                             };
    double           blc[] = { -infinity,
                               -infinity,
                               -infinity
                             };
    double           buc[] = { 100000,
                               50000,
                               60000
                             };
    mosek.boundkey   bkx[] = { mosek.boundkey.lo,
                               mosek.boundkey.lo,
                               mosek.boundkey.lo
                             };
    double           blx[] = { 0.0, 0.0, 0.0 };
    double           bux[] = { +infinity,
                               +infinity,
                               +infinity
                             };

    int asub[][] = {
      {0, 1, 2},
      {0, 1, 2},
      {0, 1, 2}
    };

    double aval[][]   = { 
      { 2.0, 3.0, 2.0 },
      { 4.0, 2.0, 3.0 },
      { 3.0, 3.0, 2.0 }
    };

    double[] xx  = new double[numvar];

    try (Env  env  = new Env();
         Task task = new Task(env, 0, 0)) {
      /* Append the constraints. */
      task.appendcons(numcon);

      /* Append the variables. */
      task.appendvars(numvar);

      /* Put C. */
      for (int j = 0; j < numvar; ++j)
        task.putcj(j, c[j]);

      /* Put constraint bounds. */
      for (int i = 0; i < numcon; ++i)
        task.putconbound(i, bkc[i], blc[i], buc[i]);

      /* Put variable bounds. */
      for (int j = 0; j < numvar; ++j)
        task.putvarbound(j, bkx[j], blx[j], bux[j]);

      /* Put A. */
      if ( numcon > 0 ) {
        for (int j = 0; j < numvar; ++j)
          task.putacol(j,
                       asub[j],
                       aval[j]);
      }

      /* A maximization problem */
      task.putobjsense(mosek.objsense.maximize);
      /* Solve the problem */
      mosek.rescode termcode = task.optimize();

      task.getxx(mosek.soltype.bas, // Request the basic solution.
                 xx);

      for (int j = 0; j < numvar; ++j)
        System.out.println ("x[" + j + "]:" + xx[j]);

      /****************** Make a change to the A matrix ******************/
      task.putaij(0, 0, 3.0);

      termcode = task.optimize();
      task.getxx(mosek.soltype.bas, // Request the basic solution.
                 xx);

      for (int j = 0; j < numvar; ++j)
        System.out.println ("x[" + j + "]:" + xx[j]);

      /***************** Add a new variable ******************************/
      /* Get index of new variable. */
      int[] varidx = new int[1];
      task.getnumvar(varidx);

      /* Append a new variable x_3 to the problem */
      task.appendvars(1);
      numvar++;

      /* Set bounds on new varaible */
      task.putvarbound(varidx[0],
                       mosek.boundkey.lo,
                       0,
                       +infinity);

      /* Change objective */
      task.putcj(varidx[0], 1.0);

      /* Put new values in the A matrix */
      int[] acolsub    =  new int[] {0,   2};
      double[] acolval =  new double[] {4.0, 1.0};

      task.putacol(varidx[0], /* column index */
                   acolsub,
                   acolval);

      /* Change optimizer to simplex free and reoptimize */
      task.putintparam(mosek.iparam.optimizer, mosek.optimizertype.free_simplex.value);
      termcode = task.optimize();

      xx = new double[numvar];
      task.getxx(mosek.soltype.bas, // Request the basic solution.
                 xx);

      for (int j = 0; j < numvar; ++j)
        System.out.println ("x[" + j + "]:" + xx[j]);

      /********************** Add a new constraint ***************************/
      /* Get index of new constraint. */
      int[] conidx = new int[1];
      task.getnumcon(conidx);

      /* Append a new constraint */
      task.appendcons(1);
      numcon++;

      /* Set bounds on new constraint */
      task.putconbound(conidx[0],
                       mosek.boundkey.up,
                       -infinity,
                       30000);

      /* Put new values in the A matrix */
      int[] arowsub = new int[] {0,   1,   2,   3  };
      double[] arowval = new double[]  {1.0, 2.0, 1.0, 1.0};

      task.putarow(conidx[0], /* row index */
                   arowsub,
                   arowval);

      termcode = task.optimize();

      task.getxx(mosek.soltype.bas, // Request the basic solution.
                 xx);

      for (int j = 0; j < numvar; ++j)
        System.out.println ("x[" + j + "]:" + xx[j]);


      /********************** Change constraint bounds ********************/
      mosek.boundkey[] newbkc  = {mosek.boundkey.up,
                                  mosek.boundkey.up,
                                  mosek.boundkey.up,
                                  mosek.boundkey.up
                                 };
      double[] newblc          = { -infinity,
                                   -infinity,
                                   -infinity,
                                   -infinity
                                 };
      double[] newbuc          = { 80000, 40000, 50000, 22000 };

      task.putconboundslice(0, numcon, newbkc, newblc, newbuc);

      task.optimize();

      task.getxx(mosek.soltype.bas, // Request the basic solution.
                 xx);

      for (int j = 0; j < numvar; ++j)
        System.out.println ("x[" + j + "]:" + xx[j]);
      
    } catch (mosek.Exception e)
      /* Catch both Error and Warning */
    {
      System.out.println ("An error was encountered");
      System.out.println (e.getMessage ());
      throw e;
    }
  }
}