/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      portfolio_5_card.java

  Description :  Implements a basic portfolio optimization model
                 with cardinality constraints on number of assets traded.
*/
package com.mosek.example;

public class portfolio_5_card {

  // Since the value infinity is never used, we define
  // 'infinity' symbolic purposes only
  static final double infinity = 0;

  public static double[] markowitz_with_card(int        n,
                                             double[]   x0,
                                             double     w,
                                             double     gamma,
                                             double[]   mu,
                                             double[][] GT,
                                             int        k) 
  {
    // Total initial wealth
    double U = w;
    for(int i=0; i<n; i++) U += x0[i];

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
    int offset_con_card = offset_con_ind + n;

    int numcon = 4 * n  + 2;

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

      // sum(y) <= k
      task.putconbound(offset_con_card, mosek.boundkey.up, -infinity, k);
      task.putconname(offset_con_card, "cardinality");

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

        // sum(y)
        task.putaij(offset_con_card, offsety + i, 1.0);
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
      //Turn all log output off.
      task.putintparam(mosek.iparam.log,0);

      //task.writedata("dump.opf");

      task.optimize();
      task.solutionsummary(mosek.streamtype.log);

      double[] xx = new double[n];
      task.getxxslice(mosek.soltype.itg, offsetx, offsetx + n, xx);
      return xx;
    }
  }

  public static void main (String[] args) 
  {
    int n = 3;
    double gamma = 0.05;
    double[]   mu = {0.1073,  0.0737,  0.0627};
    double[][] GT = {
      {0.1667,  0.0232,  0.0013},
      {0.0000,  0.1033, -0.0022},
      {0.0000,  0.0000,  0.0338}
    };
    double[] x0 = {0.0, 0.0, 0.0};
    double   w = 1.0;

    for(int k=1; k<=3; k++)
    {
      double[] xx = markowitz_with_card(n, x0, w, gamma, mu, GT, k);
      double expret = 0;
      System.out.printf("Bound %d:  x = ", k);
      for(int i=0; i<n; i++)
      { 
        System.out.printf("%.5f ", xx[i]);
        expret += xx[i]*mu[i];
      }
      System.out.printf("  Return:  %.5f\n", expret);
    }
  }
}