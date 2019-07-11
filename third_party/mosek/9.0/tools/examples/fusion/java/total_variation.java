//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      total_variation.java
//
// Purpose:   Demonstrates how to solve a total
//            variation problem using the Fusion API.
//

package com.mosek.fusion.examples;
import java.util.Random;
import mosek.fusion.*;

public class total_variation {
  public static void main(String[] args)
  throws SolutionError {
    int ncols = 50;
    int nrows = 50;
    int seed  =  0;
    double sigma = 1.0;
    int ncells = nrows * ncols;
    Random randGen = new Random(seed);

    double [] f = new double[ncells];

    //Random signal with Gaussian noise
    for (int i = 0; i < ncells; i++)
      f[i] = Math.max( Math.min(1.0, randGen.nextDouble() + randGen.nextGaussian() * sigma), .0 );


    Model M = new Model("TV");
    try {
      Variable u = M.variable(new int[] {nrows + 1, ncols + 1}, Domain.inRange(0., 1.0));
      Variable t = M.variable(new int[] {nrows, ncols}, Domain.unbounded());

      Variable ucore = u.slice(new int[] {0, 0}, new int[] {nrows, ncols});

      Expression deltax = Expr.sub( u.slice( new int[] {1, 0}, new int[] {nrows + 1, ncols} ), ucore );
      Expression deltay = Expr.sub( u.slice( new int[] {0, 1}, new int[] {nrows, ncols + 1} ), ucore );

      M.constraint( Expr.stack(2, t, deltax, deltay), Domain.inQCone().axis(2) );

      M.constraint( Expr.vstack(sigma, Expr.flatten( Expr.sub( Matrix.dense(nrows, ncols, f),  ucore ) ) ),
                    Domain.inQCone() );

      M.objective( ObjectiveSense.Minimize, Expr.sum(t) );

      M.setLogHandler(new java.io.PrintWriter(System.out));
      M.solve();
    } finally {
      M.dispose();
    }
  }
}