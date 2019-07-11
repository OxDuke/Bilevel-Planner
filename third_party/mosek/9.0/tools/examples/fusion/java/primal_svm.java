//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      primal_svm.java
//
// Purpose: Implements a simple soft-margin SVM
//          using the Fusion API.

package com.mosek.fusion.examples;

import java.util.Random;
import java.util.*;
import java.io.*;
import java.lang.*;
import java.lang.Math.*;
import java.lang.Number.*;

import mosek.fusion.*;

public class primal_svm {

  private static final int nc = 10;
  private static final int m = 50;
  private static final int n = 3;
  private static final int seed = 0;
  private static final double var = 1.;
  private static final double mean = 1.;

  public static void main(String[] args)
  throws SolutionError {

    Random gen = new Random(seed);

    int nump = gen.nextInt(m);
    int numm = m - nump;

    double [] y = new double[m];

    Arrays.fill(y, 0, nump, 1.);
    Arrays.fill(y, nump, m, -1.);

    double [][] X = new double[m][n];

    for (int i = 0; i < nump; i++)
      for (int j = 0; j < n; j++)
        X[i][j] = gen.nextGaussian() + 1.;

    for (int i = nump; i < m; i++)
      for (int j = 0; j < n; j++)
        X[i][j] = gen.nextGaussian() - 1.;

    Model M = new Model("Primal SVM");
    try {
      System.out.format("Number of data    : %d\n", m);
      System.out.format("Number of features: %d\n", n);

      Variable w =  M.variable(n, Domain.unbounded());
      Variable t =  M.variable(1, Domain.unbounded());
      Variable b =  M.variable(1, Domain.unbounded());
      Variable xi = M.variable(m, Domain.greaterThan(0.));

      M.constraint(
        Expr.add( xi ,
                  Expr.mulElm( y,
                               Expr.sub( Expr.mul(X, w), Var.repeat(b, m) )
                             )
                ),
        Domain.greaterThan( 1. )
      );

      M.constraint( Expr.vstack(1., t, w), Domain.inRotatedQCone());

      System.out.println("   c   | b      | w");
      for (int i = 0; i < nc; i++) {
        double c = 500.0 * i;

        M.objective( ObjectiveSense.Minimize, Expr.add( t, Expr.mul(c, Expr.sum(xi) ) ) );
        M.solve();

        try {
          System.out.format("%4f | %8f", c, b.level()[0] );
          for (int j = 0; j < n; j++)
            System.out.format(" | %8f", w.level()[j] );

          System.out.print("\n");
        } catch (FusionException e) {}
      }
    }
    finally {
      M.dispose();
    }
  }
}