/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      portfolio_4_transcost.java

   Purpose :   Implements a basic portfolio optimization model
               with fixed setup costs and transaction costs
               as a mixed-integer problem.
*/
package com.mosek.example;

public class portfolio_4_transcost {
  static final int n = 3;

  public static void main (String[] args) {
    // Since the value infinity is never used, we define
    // 'infinity' symbolic purposes only
    double infinity = 0;
    double gamma = 0.05;
    double[]   mu = {0.1073,  0.0737,  0.0627};
    double[][] GT = {
      {0.1667,  0.0232,  0.0013},
      {0.0000,  0.1033, -0.0022},
      {0.0000,  0.0000,  0.0338}
    };
    double[] x0 = {0.0, 0.0, 0.0};
    double   w = 1.0;
    double[] f = {0.01, 0.01, 0.01};
    double[] g = {0.001, 0.001, 0.001};

    // Total initial wealth
    double U = w;
    for(int i=0; i< n; i++) U += x0[i];

    int offsetx = 0;
    int offsets = offsetx + n;
    int offsett = offsets + 1;
    int offsetz = offsett + n;
    int offsety = offsetz + n;

    int numvar = 4 * n + 1;

    int offset_con_budget = 0;
    int offset_con_gx_t = offset_con_budget + 1;
    int offset_con_abs1 = offset_con_gx_t + n;
    int offset_con_abs2 = offset_con_abs1 + n;
    int offset_con_ind  = offset_con_abs2 + n;

    int numcon = 4 * n  +1;

    try ( mosek.Env env  = new mosek.Env ();
          mosek.Task task = new mosek.Task (env, 0, 0) ) {

      // Directs the log task stream to the user specified
      // method task_msg_obj.stream
      task.set_Stream(
        mosek.streamtype.log,
        new mosek.Stream()
      { public void stream(String msg) { System.out.print(msg); }});

      //Set up constraint bounds, names and variable coefficients
      task.appendcons(numcon);
      for (int i = 0; i < n; ++i) {
        task.putconbound(offset_con_gx_t + i, mosek.boundkey.fx, 0., 0.);
        task.putconname(offset_con_gx_t + i, "GT[" + (i + 1) + "]");

        task.putconbound(offset_con_abs1 + i, mosek.boundkey.lo, -x0[i], infinity);
        task.putconname(offset_con_abs1 + i, "zabs1[" + (i + 1) + "]");

        task.putconbound(offset_con_abs2 + i, mosek.boundkey.lo, x0[i], infinity);
        task.putconname(offset_con_abs2 + i, "zabs2[" + (i + 1) + "]");

        task.putconbound(offset_con_ind + i, mosek.boundkey.up, -infinity, 0.0);
        task.putconname(offset_con_ind + i, "ind[" + (i + 1) + "]");
      }
      // e x = w + e x0
      task.putconbound(offset_con_budget, mosek.boundkey.fx, U, U);
      task.putconname(offset_con_budget, "budget");

      //Variables.
      task.appendvars(numvar);

      //the objective function coefficients
      for (int i = 0; i < n; i++) 
      {
        task.putcj(offsetx + i, mu[i]);
      }

      double[] one_m_one = {1.0, -1.0};
      double[] one_one = {1.0, 1.0};

      //set up variable bounds and names
      for (int i = 0; i < n; ++i) {
        task.putvarbound(offsetx + i, mosek.boundkey.lo, 0., infinity);
        task.putvarbound(offsett + i, mosek.boundkey.fr, -infinity, infinity);
        task.putvarbound(offsetz + i, mosek.boundkey.fr, -infinity, infinity);
        task.putvarbound(offsety + i, mosek.boundkey.ra, 0.0, 1.0);
        task.putvartype(offsety + i, mosek.variabletype.type_int);

        task.putvarname(offsetx + i, "x[" + (i + 1) + "]");
        task.putvarname(offsett + i, "t[" + (i + 1) + "]");
        task.putvarname(offsetz + i, "z[" + (i + 1) + "]");
        task.putvarname(offsety + i, "y[" + (i + 1) + "]");

        for (int j = i; j < n; ++j)
          task.putaij(offset_con_gx_t + i, j, GT[i][j]);

        task.putaij(offset_con_gx_t + i, offsett + i, -1.0);

        task.putaij(offset_con_budget, offsetx + i, 1.0);
        task.putaij(offset_con_budget, offsetz + i, g[i]);
        task.putaij(offset_con_budget, offsety + i, f[i]);

        // z_j - x_j >= -x0_j
        int[] indx1 = {offsetz + i, offsetx + i};
        task.putarow(offset_con_abs1 + i, indx1, one_m_one);
        // z_j + x_j >= +x0_j
        int[] indx2 = {offsetz + i, offsetx + i};
        task.putarow(offset_con_abs2 + i, indx2, one_one);

        // z_j - U*y_j <= 0
        int[] indx3 = {offsetz + i, offsety + i};
        double[] va = {1.0, -U};
        task.putarow(offset_con_ind + i, indx3, va);        
      }
      task.putvarbound(offsets, mosek.boundkey.fx, gamma, gamma);
      task.putvarname(offsets, "s");

      //Cones.
      int[] csub = new int[n+1];
      csub[0] = offsets;
      for(int i = 0; i < n; i++) csub[i + 1] = offsett + i;
      task.appendcone(mosek.conetype.quad, 0.0, csub);
      task.putconename(0, "stddev");

      /* A maximization problem */
      task.putobjsense(mosek.objsense.maximize);

      /* Solve the problem */
      try {
        //Turn all log output off.
        //task.putintparam(mosek.iparam.log,0);

        //task.writedata("dump.opf");

        task.optimize();

        task.solutionsummary(mosek.streamtype.log);

        double expret = 0.0, stddev = 0.0;
        double[] xx = new double[numvar];

        task.getxx(mosek.soltype.itg, xx);

        for (int j = 0; j < n; ++j)
          expret += mu[j] * xx[j + offsetx];

        System.out.printf("Expected return %e for gamma %e\n\n", expret, xx[offsets]);

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