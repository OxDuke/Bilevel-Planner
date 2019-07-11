/*
  File : portfolio_1_basic.java

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Purpose :   Implements a basic portfolio optimization model.
*/
package com.mosek.fusion.examples;

import mosek.fusion.*;
import java.io.FileReader;
import java.io.BufferedReader;
import java.util.ArrayList;

public class portfolio_1_basic {
  public static double sum(double[] x) {
    double r = 0.0;
    for (int i = 0; i < x.length; ++i) r += x[i];
    return r;
  }

  public static double dot(double[] x, double[] y) {
    double r = 0.0;
    for (int i = 0; i < x.length; ++i) r += x[i] * y[i];
    return r;
  }

  /*
  Purpose:
      Computes the optimal portfolio for a given risk

  Input:
      n: Number of assets
      mu: An n dimmensional vector of expected returns
      GT: A matrix with n columns so (GT')*GT  = covariance matrix
      x0: Initial holdings
      w: Initial cash holding
      gamma: Maximum risk (=std. dev) accepted

  Output:
      Optimal expected return and the optimal portfolio
  */
  public static double BasicMarkowitz
  ( int n,
    double[] mu,
    double[][] GT,
    double[] x0,
    double   w,
    double   gamma)
  throws mosek.fusion.SolutionError {

    Model M = new Model("Basic Markowitz");
    try {
      // Redirect log output from the solver to stdout for debugging.
      // if uncommented.
      //M.setLogHandler(new java.io.PrintWriter(System.out));

      // Defines the variables (holdings). Shortselling is not allowed.
      Variable x = M.variable("x", n, Domain.greaterThan(0.0));

      //  Maximize expected return
      M.objective("obj", ObjectiveSense.Maximize, Expr.dot(mu, x));

      // The amount invested  must be identical to intial wealth
      M.constraint("budget", Expr.sum(x), Domain.equalsTo(w + sum(x0)));

      // Imposes a bound on the risk
      M.constraint("risk", Expr.vstack(gamma, Expr.mul(GT, x)), Domain.inQCone());

      // Solves the model.
      M.solve();

      return dot(mu, x.level());
    } finally {
      M.dispose();
    }
  }

  /*
    The example. Reads in data and solves the portfolio models.
   */
  public static void main(String[] argv)
  throws java.io.IOException,
         java.io.FileNotFoundException,
         mosek.fusion.SolutionError {

    int        n      = 3;
    double     w      = 1.0;
    double[]   mu     = {0.1073, 0.0737, 0.0627};
    double[]   x0     = {0.0, 0.0, 0.0};
    double[]   gammas = {0.035, 0.040, 0.050, 0.060, 0.070, 0.080, 0.090};
    double[][] GT     = {
      { 0.166673333200005, 0.0232190712557243 ,  0.0012599496030238 },
      { 0.0              , 0.102863378954911  , -0.00222873156550421},
      { 0.0              , 0.0                ,  0.0338148677744977 }
    };


    System.out.println("\n-----------------------------------------------------------------------------------");
    System.out.println("Basic Markowitz portfolio optimization");
    System.out.println("-----------------------------------------------------------------------------------\n");
    for ( int i = 0; i < gammas.length; ++i) {
      double expret = BasicMarkowitz( n, mu, GT, x0, w, gammas[i]);
      System.out.format("Expected return: %.4e Std. deviation: %.4e\n",
                        expret,
                        gammas[i]);
    }
  }
}