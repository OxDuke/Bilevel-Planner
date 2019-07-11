//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      lpt.java
//
// Purpose:  Demonstrates how to solve the multi-processor
//           scheduling problem using the Fusion API.
//
package com.mosek.fusion.examples;

import java.util.Random;

import java.util.*;
import java.io.*;
import java.lang.*;
import java.lang.Math.*;
import java.lang.Number.*;
import mosek.*;
import mosek.fusion.*;

public class lpt {
  public static void main(String [] args) {
    int n = 30;             //Number of tasks
    int m = 6;              //Number of processors

    double lb = 1.;         //The range of lengths of short tasks
    double ub = 5.;

    double sh = 0.8;        //The proportion of short tasks
    int n_short = (int)(n * sh);
    int n_long = n - n_short;

    double[] T = new double[n];
    Random gen = new Random(0);
    for (int i = 0; i < n_short; i++)
      T[i] = gen.nextDouble() * (ub - lb) + lb;
    for (int i = n_short; i < n; i++)
      T[i] = 20 * (gen.nextDouble() * (ub - lb) + lb);
    Arrays.sort(T);

    Model M = new Model("Multi-processor scheduling");

    Variable x = M.variable("x", new int[] {m, n}, Domain.binary());
    Variable t = M.variable("t", 1, Domain.unbounded());

    M.constraint( Expr.sum(x, 0), Domain.equalsTo(1.) );
    M.constraint( Expr.sub( Var.repeat(t, m), Expr.mul(x, T)), Domain.greaterThan(0.) );

    M.objective( ObjectiveSense.Minimize, t );

    //Computing LPT solution
    double [] init    = new double[n * m];
    double [] schedule = new double[m];

    for (int i = n - 1; i >= 0; i--) {
      int next = 0;
      for (int j = 1; j < m; j++)
        if (schedule[j] < schedule[next]) next = j;
      schedule[next] += T[i];
      init[next * n + i] = 1;
    }

    //Comment this line to switch off feeding in the initial LPT solution
    x.setLevel(init);

    M.setLogHandler(new PrintWriter(System.out));

    M.setSolverParam("mioTolRelGap", .01);
    M.solve();

    try {
      System.out.printf("initial solution: \n");
      for (int i = 0; i < m; i++) {
        System.out.printf("M %d [", i);
        for (int y = 0; y < n; y++)
          System.out.printf( "%d, ", (int) init[i * n + y]);
        System.out.printf("]\n");
      }
      System.out.print("MOSEK solution:\n");
      for (int i = 0; i < m; i++) {
        System.out.printf("M %d [", i);
        for (int y = 0; y < n; y++)
          System.out.printf( "%d, ", (int)x.index(i, y).level()[0]);
        System.out.printf("]\n");
      }
    } catch (SolutionError e) {}
  }
}