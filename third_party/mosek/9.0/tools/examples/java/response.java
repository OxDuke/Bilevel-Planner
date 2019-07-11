/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      response.java

   Purpose :   This example demonstrates proper response handling
               for problems solved with the interior-point optimizers.
*/
package com.mosek.example;
import mosek.*;

public class response {
  public static void main(String[] argv) {
    StringBuffer symname = new StringBuffer();
    StringBuffer desc = new StringBuffer();

    String filename;
    if (argv.length >=1) filename = argv[0];
    else                 filename = "../data/cqo1.mps";

    // Create the task and environment
    try (Env  env  = new Env();
         Task task = new Task(env, 0, 0)) {
      
      // (Optionally) attach the log handler to receive log information
      /*
      task.set_Stream(
        streamtype.log,
        new mosek.Stream()
      { public void stream(String msg) { System.out.print(msg); }});
      */

      // (Optionally) uncomment this line to experience solution status Unknown
      // task.putintparam(iparam.intpnt_max_iterations, 1);

      // On this example we read an optimization problem from a file
      task.readdata(filename);

      // Perform optimization. 
      rescode trm = task.optimize();
      task.solutionsummary(streamtype.log);

      // Handle solution status. We expect Optimal
      solsta solsta = task.getsolsta(soltype.itr);

      switch ( solsta ) {
        case optimal:
          // Fetch and print the solution
          System.out.println("An optimal interior point solution is located.");
          int numvar = task.getnumvar();
          double[] xx = new double[numvar];
          task.getxx(soltype.itr, xx);
          for(int i = 0; i < numvar; i++) 
            System.out.println("x[" + i + "] = " + xx[i]);
          break;

        case dual_infeas_cer:
          System.out.println("Dual infeasibility certificate found.");
          break;

        case prim_infeas_cer:
          System.out.println("Primal infeasibility certificate found.");
          break;

        case unknown: 
          // The solutions status is unknown. The termination code
          // indicates why the optimizer terminated prematurely. 
          System.out.println("The solution status is unknown.");
          Env.getcodedesc(trm, symname, desc);
          System.out.printf("   Termination code: %s %s\n", symname, desc);
          break;

        default:
          System.out.println("Unexpected solution status " + solsta + "\n");
          break;
      } 
    }
    catch (mosek.Error e) {
      System.out.println("Unexpected error (" + e.code + ") " + e.msg);
    }
  }
}