/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      solutionquality.java

   Purpose :   To demonstrate how to examine the quality of a solution.
*/
package com.mosek.example;
import mosek.*;

public class solutionquality {
  public static void main (String[] args) {
    if (args.length == 0) {
      System.out.println ("Missing argument, syntax is:");
      System.out.println ("  solutionquality inputfile");
    } else {
      try (Env  env  = new Env();
           Task task = new Task(env, 0, 0)) {
        task.set_Stream (mosek.streamtype.log,
        new mosek.Stream() {
          public void stream(String msg) { System.out.print(msg); }
        });
        // We assume that a problem file was given as the first command
        // line argument (received in `args')
        task.readdata (args[0]);

        // Solve the problem
        task.optimize ();

        // System.Out.Println (a summary of the solution
        task.solutionsummary (mosek.streamtype.log);

        mosek.solsta solsta[] = new mosek.solsta[1];
        task.getsolsta(mosek.soltype.bas, solsta);

        double pobj[] = new double[1];
        double pviolcon[] = new double[1];
        double pviolvar[] = new double[1];
        double pviolbarvar[] = new double[1];
        double pviolcones[] = new double[1];
        double pviolitg[] = new double[1];
        double dobj[] = new double[1];
        double dviolcon[] = new double[1];
        double dviolvar[] = new double[1];
        double dviolbarvar[] = new double[1];
        double dviolcones[] = new double[1];

        task.getsolutioninfo(mosek.soltype.bas,
                             pobj, pviolcon, pviolvar, pviolbarvar, pviolcones, pviolitg,
                             dobj, dviolcon, dviolvar, dviolbarvar, dviolcones);

        switch (solsta[0]) {
          case optimal:

            double abs_obj_gap     = Math.abs(dobj[0] - pobj[0]);
            double rel_obj_gap     = abs_obj_gap / (1.0 + Math.min(Math.abs(pobj[0]), Math.abs(dobj[0])));
            double max_primal_viol = Math.max(pviolcon[0], pviolvar[0]);
            max_primal_viol = Math.max(max_primal_viol  , pviolbarvar[0]);
            max_primal_viol = Math.max(max_primal_viol  , pviolcones[0]);

            double max_dual_viol   = Math.max(dviolcon[0], dviolvar[0]);
            max_dual_viol   = Math.max(max_dual_viol  , dviolbarvar[0]);
            max_dual_viol   = Math.max(max_dual_viol  , dviolcones[0]);

            // Assume the application needs the solution to be within
            //    1e-6 ofoptimality in an absolute sense. Another approach
            //   would be looking at the relative objective gap

            System.out.println ("Customized solution information.\n");
            System.out.println ("  Absolute objective gap: " + abs_obj_gap);
            System.out.println ("  Relative objective gap: " + rel_obj_gap);
            System.out.println ("  Max primal violation  : " + max_primal_viol);
            System.out.println ("  Max dual violation    : " + max_dual_viol);

            boolean accepted = true;

            if ( rel_obj_gap > 1e-6 ) {
              System.out.println ("Warning: The relative objective gap is LARGE.");
              accepted = false;
            }

            // We will accept a primal infeasibility of 1e-8 and
            // dual infeasibility of 1e-6. These number should chosen problem
            // dependent.
            if ( max_primal_viol > 1e-8 ) {
              System.out.println ("Warning: Primal violation is too LARGE");
              accepted = false;
            }

            if (max_dual_viol > 1e-6 ) {
              System.out.println ("Warning: Dual violation is too LARGE.");
              accepted = false;
            }

            if ( accepted ) {
              int numvar = task.getnumvar();
              System.out.println ("Optimal primal solution");
              double xj[] = new double[1];
              for (int j = 0; j < numvar; j++) {
                task.getxxslice(mosek.soltype.bas, j, j + 1, xj);
                System.out.println ("x[" + j + "]: " + xj[0]);
              }
            } else {
              // print etailed information about the solution
              task.analyzesolution(mosek.streamtype.log, mosek.soltype.bas);
            }
            break;

          case dual_infeas_cer:
          case prim_infeas_cer:
            System.out.println ("Primal or dual infeasibility certificate found.");
            break;
          case unknown:
            System.out.println ("The status of the solution is unknown.");
            break;
          default:
            System.out.println ("Other solution status");
        }
      } catch (mosek.Exception e) {
        System.out.println ("An error/warning was encountered");
        System.out.println (e.toString());
        throw e;
      }
    }
  }
}