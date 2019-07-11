/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      callback.java

   Purpose :   To demonstrate how to use the progress
               callback.

               Use this script as follows:

               callback psim
               callback dsim
               callback intpnt

               The first argument tells which optimizer to use
               i.e. psim is primal simplex, dsim is dual simplex
               and intpnt is interior-point.
*/
package com.mosek.fusion.examples;

import mosek.*;
import mosek.fusion.*;
import java.util.Formatter;
import java.util.*;
import java.lang.*;

public class callback {
  private static DataCallback makeUserCallback(final double maxtime,
                                               final Model mod) {
    return new DataCallback() {
      public int callback(callbackcode caller,
                          double[]     douinf,
                          int[]        intinf,
                          long[]       lintinf) 
      {
        double opttime = 0.0;
        int itrn;
        double pobj, dobj, stime;

        Formatter f = new Formatter(System.out);
        switch (caller) {
          case begin_intpnt:
            f.format("Starting interior-point optimizer\n");
            break;
          case intpnt:
            itrn    = intinf[iinfitem.intpnt_iter.value      ];
            pobj    = douinf[dinfitem.intpnt_primal_obj.value];
            dobj    = douinf[dinfitem.intpnt_dual_obj.value  ];
            stime   = douinf[dinfitem.intpnt_time.value      ];
            opttime = douinf[dinfitem.optimizer_time.value   ];
            f.format("Iterations: %-3d\n", itrn);
            f.format("  Elapsed time: %6.2f(%.2f)\n", opttime, stime);
            f.format("  Primal obj.: %-18.6e  Dual obj.: %-18.6e\n", pobj, dobj);
            break;
          case end_intpnt:
            f.format("Interior-point optimizer finished.\n");
            break;
          case begin_primal_simplex:
            f.format("Primal simplex optimizer started.\n");
            break;
          case update_primal_simplex:
            itrn    = intinf[iinfitem.sim_primal_iter.value  ];
            pobj    = douinf[dinfitem.sim_obj.value          ];
            stime   = douinf[dinfitem.sim_time.value         ];
            opttime = douinf[dinfitem.optimizer_time.value   ];

            f.format("Iterations: %-3d\n", itrn);
            f.format("  Elapsed time: %6.2f(%.2f)\n", opttime, stime);
            f.format("  Obj.: %-18.6e\n", pobj );
            break;
          case end_primal_simplex:
            f.format("Primal simplex optimizer finished.\n");
            break;
          case begin_dual_simplex:
            f.format("Dual simplex optimizer started.\n");
            break;
          case update_dual_simplex:
            itrn    = intinf[iinfitem.sim_dual_iter.value    ];
            pobj    = douinf[dinfitem.sim_obj.value          ];
            stime   = douinf[dinfitem.sim_time.value         ];
            opttime = douinf[dinfitem.optimizer_time.value   ];
            f.format("Iterations: %-3d\n", itrn);
            f.format("  Elapsed time: %6.2f(%.2f)\n", opttime, stime);
            f.format("  Obj.: %-18.6e\n", pobj);
            break;
          case end_dual_simplex:
            f.format("Dual simplex optimizer finished.\n");
            break;
          case begin_bi:
            f.format("Basis identification started.\n");
            break;
          case end_bi:
            f.format("Basis identification finished.\n");
            break;
          default:
        }
        System.out.flush();
        if (opttime >= maxtime) {
          f.format("MOSEK is spending too much time. Terminate it.\n");
          System.out.flush();
          return 1;
        }
        return 0;
      }
    };
  }

  private static Progress makeProgressCallback() {
    return new Progress() {
      public int progress(callbackcode caller) 
      {
        // Process the caller code here
        return 0;
      }
    };
  }
  

  public static void main(String[] args) {
    String slvr     = "intpnt";
    if (args.length < 1) {
      System.out.println("Usage: ( psim | dsim | intpnt ) ");
    }

    if (args.length >= 1) slvr     = args[0];

    // Create a random large linear problem
    int n = 150;
    int m = 700;
    double[] A = new double[m * n];
    double[] b = new double[m];
    double[] c = new double[n];

    Random rnd = new Random();
    for (int i = 0; i < m * n; i++) A[i] = rnd.nextDouble();
    for (int i = 0; i < m; i++)   b[i] = rnd.nextDouble();
    for (int i = 0; i < n; i++)   c[i] = rnd.nextDouble();

    try(Model M = new Model("callback")) {
      Variable x = M.variable(n, Domain.unbounded());
      M.constraint(Expr.mul(Matrix.dense(m, n, A), x), Domain.lessThan(b));
      M.objective(ObjectiveSense.Maximize, Expr.dot(c, x));

      if ( slvr == "psim")
        M.setSolverParam("optimizer", "primalSimplex");
      else if ( slvr == "dsim")
        M.setSolverParam("optimizer", "dualSimplex");
      else if ( slvr == "intpnt")
        M.setSolverParam("optimizer", "intpnt");

      double maxtime = 0.07;

      M.setDataCallbackHandler( makeUserCallback(maxtime, M) );

      M.solve();
    } catch (mosek.Exception e) {
      System.out.println ("An error/warning was encountered");
      System.out.println (e.toString());
      throw e;
    }
  }
}