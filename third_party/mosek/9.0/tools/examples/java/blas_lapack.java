/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      blas_lapack.java

   Purpose :   To demonstrate how to call BLAS/LAPACK routines for whose MOSEK provides simplified interfaces.
*/
package com.mosek.example;

public class blas_lapack {
  static final int n = 3, m = 2, k = 3;

  public static void main (String[] args) {

    double alpha = 2.0, beta = 0.5;
    double[] x = {1., 1., 1.};
    double[] y = {1., 2., 3.};
    double[] z = {1.0, 1.0};

    /*A has m=2 rows and k=3 cols*/
    double[] A = {1., 1., 2., 2., 3., 3.};
    /*B has k=3 rows and n=3 cols*/
    double[] B = {1., 1., 1., 1., 1., 1., 1., 1., 1.};
    double[] C = { 1., 2., 3., 4., 5., 6.};

    double[] D = {1.0, 1.0, 1.0, 1.0};
    double[] Q = {1.0, 0.0, 0.0, 2.0};
    double[] v = new double[2];

    double[] xy = {0.};

    try (mosek.Env  env = new mosek.Env()) {
      /*  routines*/

      env.dot(n, x, y, xy);

      env.axpy(n, alpha, x, y);

      env.gemv(mosek.transpose.no, m, n, alpha, A, x, beta, z);

      env.gemm(mosek.transpose.no, mosek.transpose.no, m, n, k, alpha, A, B, beta, C);

      env.syrk(mosek.uplo.lo, mosek.transpose.no, m, k, alpha, A, beta, D);

      /* LAPACK routines*/

      env.potrf(mosek.uplo.lo, m, Q);

      env.syeig(mosek.uplo.lo, m, Q, v);

      env.syevd(mosek.uplo.lo, m, Q, v);

    } catch (mosek.Exception e) {
      System.out.println ("An error/warning was encountered");
      System.out.println (e.toString());
    }
  }
}