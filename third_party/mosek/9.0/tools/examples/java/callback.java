/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      callback.java

   Purpose :   To demonstrate how to use the progress
               callback.

               Use this script as follows:

                 callback psim  25fv47.mps
                 callback dsim  25fv47.mps
                 callback intpnt 25fv47.mps

               The first argument tells which optimizer to use
               i.e. psim is primal simplex, dsim is dual simplex
               and intpnt is interior-point.
*/

package com.mosek.example;

import mosek.*;
import java.util.Formatter;

public class callback {
  private static DataCallback makeUserCallback(final double maxtime) {
    return new DataCallback() {
      public int callback(callbackcode caller,
                          double[]     douinf,
                          int[]        intinf,
                          long[]       lintinf) {
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
            f.format("  Time: %6.2f(%.2f) ", opttime, stime);
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
            f.format("  Elapsed time: %6.2f(%.2f\n", opttime, stime);
            f.format("  Obj.: %-18.6e", pobj );
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
        if (opttime >= maxtime)
          // mosek is spending too much time. Terminate it.
          return 1;

        return 0;
      }
    };
  }

  public static void main(String[] args) {
    String filename = "../data/25fv47.mps";
    String slvr     = "intpnt";
    if (args.length < 2) {
      System.out.println("Usage: ( psim | dsim | intpnt ) filename");
    }

    if (args.length >= 1) slvr     = args[0];
    if (args.length >= 2) filename = args[1];
    System.out.println("filename = " + filename);

    try (Env  env  = new Env();
         Task task = new Task(env, 0, 0)) {

      task.readdata(filename);

      if   (slvr == "psim")
        task.putintparam(iparam.optimizer, optimizertype.primal_simplex.value);
      else if (slvr == "dsim")
        task.putintparam(iparam.optimizer, optimizertype.dual_simplex.value);
      else if (slvr == "intpnt")
        task.putintparam(iparam.optimizer, optimizertype.intpnt.value);

      // Turn all MOSEK logging off (note that errors and other messages
      // are still sent through the log stream)

      double maxtime = 0.05;
      task.set_InfoCallback(makeUserCallback(maxtime));
      task.optimize();
      task.putintparam(iparam.log, 1);
      task.solutionsummary(streamtype.msg);

    } catch (mosek.Exception e) {
      System.out.println ("An error/warning was encountered");
      System.out.println (e.toString());
      throw e;
    }
  }
}