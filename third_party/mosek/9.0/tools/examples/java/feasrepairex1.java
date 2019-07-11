/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      feasrepairex1.java

   Purpose :   To demonstrate how to use the MSK_relaxprimal function to
               locate the cause of an infeasibility.

   Syntax :     On command line

                   java  feasrepairex1.feasrepairex1 feasrepair.lp

                feasrepair.lp is located in mosek\<version>\tools\examples.
*/
package com.mosek.example;

import mosek.*;

public class feasrepairex1 {

  public static void main (String[] args) {
    String filename = "../data/feasrepair.lp";
    if (args.length >= 1) filename = args[0];

    try (Env  env  = new Env();
         Task task = new Task(env, 0, 0)) {
        task.set_Stream(
        mosek.streamtype.log,
        new mosek.Stream()
      { public void stream(String msg) { System.out.print(msg); }});

      task.readdata(filename);

      task.putintparam(mosek.iparam.log_feas_repair, 3);

      task.primalrepair(null, null, null, null);

      double sum_viol = task.getdouinf(mosek.dinfitem.primal_repair_penalty_obj);

      System.out.println("Minimized sum of violations = " + sum_viol);

      task.optimize();

      task.solutionsummary(mosek.streamtype.msg);
    }
  }
}