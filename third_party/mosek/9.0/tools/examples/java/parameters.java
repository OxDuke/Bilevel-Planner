/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      parameters.java

   Purpose :   Demonstrates a very simple example about how to get/set
               parameters with MOSEK Java API
*/

package com.mosek.example;
import mosek.*;

public class parameters {

  public static void main (String[] args) {
    try {
      mosek.Env  env = new Env();
      mosek.Task task = new Task(env, 0, 0);

      System.out.println("Test MOSEK parameter get/set functions");

      // Set log level (integer parameter)
      task.putintparam(mosek.iparam.log, 1);
      // Select interior-point optimizer... (integer parameter)
      task.putintparam(mosek.iparam.optimizer, mosek.optimizertype.intpnt.value);
      // ... without basis identification (integer parameter)
      task.putintparam(mosek.iparam.intpnt_basis, mosek.basindtype.never.value);
      // Set relative gap tolerance (double parameter)
      task.putdouparam(mosek.dparam.intpnt_co_tol_rel_gap, 1.0e-7);

      // The same using explicit string names 
      task.putparam     ("MSK_DPAR_INTPNT_CO_TOL_REL_GAP", "1.0e-7");      
      task.putnadouparam("MSK_DPAR_INTPNT_CO_TOL_REL_GAP",  1.0e-7 );      

      // Incorrect value
      try {
        task.putdouparam(mosek.dparam.intpnt_co_tol_rel_gap, -1.0);
      } 
      catch (mosek.Error e) {
        System.out.println("Wrong parameter value");
      }


      double param = task.getdouparam(mosek.dparam.intpnt_co_tol_rel_gap);
      System.out.println("Current value for parameter intpnt_co_tol_rel_gap = " + param);

      /* Define and solve an optimization problem here */
      /* task.optimize() */
      /* After optimization: */

      System.out.println("Get MOSEK information items");

      double tm = task.getdouinf(mosek.dinfitem.optimizer_time);
      int  iter = task.getintinf(mosek.iinfitem.intpnt_iter);     
       
      System.out.println("Time: " + tm);
      System.out.println("Iterations: " + iter);         
    } catch (mosek.Exception e) {
      System.out.println ("An error/warning was encountered");
      System.out.println (e.toString());
      throw e;
    }
  }
}