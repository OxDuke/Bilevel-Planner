/*
* Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
*
* File:      qcqp_sdo_relaxation.java
*
* Purpose:   Demonstrate how to use SDP to solve
*            convex relaxation of a mixed-integer QCQP
**/
package com.mosek.fusion.examples;

import java.util.*;
import java.lang.*;
import java.lang.Math.*;
import java.io.*;

import mosek.*;
import mosek.fusion.*;

public class qcqp_sdo_relaxation {
  public static void main(String [] args)
  throws SolutionError {
    Random rnd = new Random();
    // problem dimensions
    int n = 20;
    int m = 2 * n;

    // problem data
    double[] A = new double[m * n];
    double[] b = new double[m];
    double[] c = new double[n];
    double[] P = new double[n * n];
    double[] q = new double[n];

    for (int j = 0; j < n; j++) {
      for (int i = 0; i < m; i++)
        A[i * n + j] = rnd.nextGaussian();
      c[j] = rnd.nextDouble();
    }

    // P = A^T A
    LinAlg.syrk(mosek.uplo.lo, mosek.transpose.yes,
                n, m, 1.0, A, 0., P);
    for (int j = 0; j < n; j++) for (int i = j + 1; i < n; i++) P[i * n + j] = P[j * n + i];

    // q = -P c, b = A c
    LinAlg.gemv(mosek.transpose.no, n, n, -1.0, P, c, 0., q);
    LinAlg.gemv(mosek.transpose.no, m, n,  1.0, A, c, 0., b);

    // Solve the problems
    Model M = miqcqp_sdo_relaxation(n, Matrix.dense(n, n, P), q);
    Model Mint = int_least_squares(n, Matrix.dense(n, m, A).transpose(), b);
    M.solve();
    Mint.solve();

    // rounded and optimal solution
    double[] xRound = M.getVariable("Z").slice(new int[] {0, n}, new int[] {n, n + 1}).level();
    for (int i = 0; i < n; i++) xRound[i] = java.lang.Math.round(xRound[i]);
    double[] yRound = b.clone();
    double[] xOpt   = Mint.getVariable("x").level();
    double[] yOpt   = b.clone();
    LinAlg.gemv(mosek.transpose.no, m, n,  1.0, A, xRound, -1.0, yRound);        // Ax_round-b
    LinAlg.gemv(mosek.transpose.no, m, n,  1.0, A, xOpt, -1.0, yOpt);            // Ax_opt-b

    System.out.println(M.getSolverDoubleInfo("optimizerTime") + " " + Mint.getSolverDoubleInfo("optimizerTime"));
    System.out.println(java.lang.Math.sqrt(LinAlg.dot(m, yRound, yRound)) + " " +
                       java.lang.Math.sqrt(LinAlg.dot(m, yOpt, yOpt)));
  }

  static Model miqcqp_sdo_relaxation(int n, Matrix P, double[] q) {
    Model M = new Model();

    Variable Z = M.variable("Z", Domain.inPSDCone(n + 1));

    Variable X = Z.slice(new int[] {0, 0}, new int[] {n, n});
    Variable x = Z.slice(new int[] {0, n}, new int[] {n, n + 1});

    M.constraint( Expr.sub(X.diag(), x), Domain.greaterThan(0.) );
    M.constraint( Z.index(n, n), Domain.equalsTo(1.) );

    M.objective( ObjectiveSense.Minimize, Expr.add(
                   Expr.sum( Expr.mulElm( P, X ) ),
                   Expr.mul( 2.0, Expr.dot(x, q) )
                 ) );

    return M;
  }

  // A direct integer model for minimizing |Ax-b|
  static Model int_least_squares(int n, Matrix A, double[] b) {
    Model M = new Model();

    Variable x = M.variable("x", n, Domain.integral(Domain.unbounded()));
    Variable t = M.variable("t", 1, Domain.unbounded());

    M.constraint( Expr.vstack(t, Expr.sub(Expr.mul(A, x), b)), Domain.inQCone() );
    M.objective( ObjectiveSense.Minimize, t );

    return M;
  }
}