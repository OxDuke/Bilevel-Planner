/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      simple.java

   Purpose :   Demonstrates a very simple example using MOSEK by
               reading a problem file, solving the problem and
               writing the solution to a file.
*/
package com.mosek.example;
import mosek.*;

public class simple {
  public static void main (String[] args) {
    if (args.length == 0) {
      System.out.println ("Missing argument, syntax is:");
      System.out.println ("  simple inputfile [ solutionfile ]");
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

        // Print a summary of the solution
        task.solutionsummary (mosek.streamtype.log);

        // If an output file was specified, save problem to file
        if (args.length >= 2) {
          // If using OPF format, these parameters will specify what to include in output
          task.putintparam (mosek.iparam.opf_write_solutions,  mosek.onoffkey.on.value);
          task.putintparam (mosek.iparam.opf_write_problem,    mosek.onoffkey.on.value);
          task.putintparam (mosek.iparam.opf_write_hints,      mosek.onoffkey.off.value);
          task.putintparam (mosek.iparam.opf_write_parameters, mosek.onoffkey.off.value);

          task.writedata (args[1]);
        }
      }
    }
  }
}