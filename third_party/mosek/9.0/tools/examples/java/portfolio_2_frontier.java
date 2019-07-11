/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      portfolio_2_frontier.java

   Purpose :   Implements a basic portfolio optimization model.
               Computes points on the efficient frontier.
*/
package com.mosek.example;

import mosek.*;

public class portfolio_2_frontier {
  static final int n = 3;

  public static void main (String[] args) {
    // Since the value infinity is never used, we define
    // 'infinity' symbolic purposes only
    double infinity = 0;
    double[]   mu = {0.1073,  0.0737,  0.0627};
    double[][] GT = {
      {0.1667,  0.0232,  0.0013},
      {0.0000,  0.1033, -0.0022},
      {0.0000,  0.0000,  0.0338}
    };
    double[] x0 = {0.0, 0.0, 0.0};
    double   w = 1.0;
    double[] alphas = {0.0, 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5};
    int numalphas = 12;

    int numvar = 2 * n + 2;
    int numcon = n + 1;


    //Offset of variables into the API variable.
    int offsetx = 0;
    int offsets = n;
    int offsett = n + 1;
    int offsetu = 2 * n + 1;

    try ( Env env  = new mosek.Env ();
          Task task = new mosek.Task (env, 0, 0) ) {

      // Directs the log task stream to the user specified
      // method task_msg_obj.stream
      task.set_Stream(
        mosek.streamtype.log,
        new mosek.Stream()
      { public void stream(String msg) { System.out.print(msg); }});
    
      task.appendcons(numcon);
      task.appendvars(numvar);

      //Constraints.
      for ( int i = 1; i <= n; ++i) {
        w += x0[i - 1];
        task.putconbound(i, mosek.boundkey.fx, 0., 0.);
        task.putconname(i, "GT[" + i + "]");
      }
      for (int i = 0; i < n; ++i)
        task.putaij(0, offsetx + i, 1.0);
      task.putconbound(0, mosek.boundkey.fx, w, w);
      task.putconname(0, "budget");

      // Objective coefficients
      for(int i = 0; i < n; i++)
        task.putcj(offsetx + i, mu[i]);

      for ( int i = 0; i < n; ++i) {
        // The constraint t = GT * x
        for ( int j = i; j < n; ++j)
          task.putaij(i + 1, offsetx + j, GT[i][j]);

        task.putaij(i + 1, offsett + i, -1.0);

        // x variables
        task.putvarbound(offsetx + i, mosek.boundkey.lo, 0., 0.);

        task.putvarname(offsetx + i, "x[" + (i + 1) + "]");
        task.putvarname(offsett + i, "t[" + (i + 1) + "]");

        // t variable
        task.putvarbound(offsett + i, mosek.boundkey.fr, -infinity, infinity);
      }

      // s variable
      task.putvarbound(offsets, mosek.boundkey.fr, -infinity, infinity);
      task.putvarname(offsets, "s");

      // u variable
      task.putvarbound(offsetu, mosek.boundkey.fx, 0.5, 0.5);
      task.putvarname(offsetu, "u");

      //Cones.
      int[] csub = new int[n+2];
      csub[0] = offsets; 
      csub[1] = offsetu;
      for(int i = 0; i < n; i++)
        csub[i+2] = offsett + i;

      task.appendcone(mosek.conetype.rquad,
                      0.0, /* For future use only, can be set to 0.0 */
                      csub);
      task.putconename(0, "variance");

      /* A maximization problem */
      task.putobjsense(mosek.objsense.maximize);

      //task.writedata("dump.opf");

      try {
        //Turn all log output off.
        task.putintparam(mosek.iparam.log, 0);

        System.out.printf("%-12s  %-12s  %-12s", "alpha", "exp ret", "variance");
        for (int k = 0; k < numalphas; ++k) {
          task.putcj(offsets, -alphas[k]);

          task.optimize();

          task.solutionsummary(mosek.streamtype.log);

          double expret = 0.0, stddev = 0.0;
          double[] xx = new double[numvar];

          task.getxx(mosek.soltype.itr, xx);

          for (int j = 0; j < n; ++j)
            expret += mu[j] * xx[j + offsetx];

          System.out.printf("%-12.3e  %-12.3e  %-12.3e\n", alphas[k], expret, xx[offsets]);

        }
        System.out.println("");
      } catch (mosek.Warning mw) {
        System.out.println (" Mosek warning:");
        System.out.println (mw.toString ());
      }
    } catch ( mosek.Exception e) {
      System.out.println ("An error/warning was encountered");
      System.out.println (e.toString ());
      throw e;
    }
  }
}