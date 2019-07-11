//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      breaksolver.java
//
//  Purpose: Show how to break a long-running task.
//
//  Requires a parameter defining a timeout in seconds.
//


package com.mosek.fusion.examples;
import mosek.fusion.*;
import java.util.Random;

public class breaksolver {
  private static void nshuffle(Random R, int[] a, int n) {
    for (int i = 0; i < n; ++i) {
      int idx = (int)(R.nextDouble() * (a.length - i)) + i;
      int tmp = a[i]; a[i] = a[idx]; a[idx] = tmp;
    }
  }

  public static void main(String[] args)
  throws java.lang.InterruptedException, SolutionError {
    long timeout = 5;

    int n = 200;   // number of binary variables
    int m = n / 5; // number of constraints
    int p = n / 5; // Each constraint picks p variables and requires that half of them are 1
    Random R = new Random(1234);

    System.out.println("Build problem...");
    try (Model M = new Model("SolveBinary")) {
      M.setLogHandler(new java.io.PrintWriter(System.out));

      Variable x = M.variable("x", n, Domain.binary());
      M.objective(ObjectiveSense.Minimize, Expr.sum(x));

      int[] idxs = new int[n];
      for (int i = 0; i < n; ++i)
        idxs[i] = i;

      for (int i = 0; i < m; ++i) {
        nshuffle(R, idxs, p);
        M.constraint(Expr.sum(x.pick(java.util.Arrays.copyOf(idxs, p))),
                     Domain.equalsTo(p / 2));
      }

      System.out.println("Start thread...");
      Thread T = new Thread() { public void run() { M.solve(); } };
      T.start();

      long T0 = System.currentTimeMillis();
      while (true) {
        if (System.currentTimeMillis() - T0 > timeout * 1000) {
          System.out.println("Solver terminated due to timeout!");
          M.breakSolver();
          T.join();
          break;
        }
        if (! T.isAlive()) {
          System.out.println("Solver terminated before anything happened!");
          T.join();
          break;
        }
      }
    } finally {
      System.out.println("End.");
    }
  }
}