/*
  File : portfolio_4_transcost.java

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Purpose :   Implements a basic portfolio optimization model
              with fixed setup costs and transaction costs
              as a mixed-integer problem.
*/
package com.mosek.fusion.examples;

import mosek.fusion.*;
import java.io.FileReader;
import java.io.BufferedReader;
import java.util.ArrayList;

public class portfolio_4_transcost {
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
      Description:
          Extends the basic Markowitz model with a market cost term.

      Input:
          n: Number of assets
          mu: An n dimmensional vector of expected returns
          GT: A matrix with n columns so (GT')*GT  = covariance matrix
          x0: Initial holdings
          w: Initial cash holding
          gamma: Maximum risk (=std. dev) accepted
          f: If asset j is traded then a fixed cost f_j must be paid
          g: If asset j is traded then a cost g_j must be paid for each unit traded

      Output:
         Optimal expected return and the optimal portfolio

  */
  public static double[] MarkowitzWithTransactionsCost
  ( int n,
    double[] mu,
    double[][] GT,
    double[] x0,
    double   w,
    double   gamma,
    double[] f,
    double[] g)
  throws mosek.fusion.SolutionError {

    // Upper bound on the traded amount
    double[] u = new double[n];
    {
      double v = w + sum(x0);
      for (int i = 0; i < n; ++i) u[i] = v;
    }

    Model M = new Model("Markowitz portfolio with transaction costs");
    try {
      //M.setLogHandler(new java.io.PrintWriter(System.out));

      // Defines the variables. No shortselling is allowed.
      Variable x = M.variable("x", n, Domain.greaterThan(0.0));

      // Addtional "helper" variables
      Variable z = M.variable("z", n, Domain.unbounded());
      // Binary varables
      Variable y = M.variable("y", n, Domain.binary());

      //  Maximize expected return
      M.objective("obj", ObjectiveSense.Maximize, Expr.dot(mu, x));

      // Invest amount + transactions costs = initial wealth
      M.constraint("budget", Expr.add(Expr.add(Expr.sum(x), Expr.dot(f, y)), Expr.dot(g, z)),
                   Domain.equalsTo(w + sum(x0)));

      // Imposes a bound on the risk
      M.constraint("risk", Expr.vstack( gamma, Expr.mul(GT, x)),
                   Domain.inQCone());

      // z >= |x-x0|
      M.constraint("buy", Expr.sub(z, Expr.sub(x, x0)), Domain.greaterThan(0.0));
      M.constraint("sell", Expr.sub(z, Expr.sub(x0, x)), Domain.greaterThan(0.0));

      //M.constraint("trade", Expr.hstack(z,Expr.sub(x,x0)), Domain.inQcone())"

      // Consraints for turning y off and on. z-diag(u)*y<=0 i.e. z_j <= u_j*y_j
      M.constraint("y_on_off", Expr.sub(z, Expr.mul(Matrix.diag(u), y)), Domain.lessThan(0.0));

      // Integer optimization problems can be very hard to solve so limiting the
      // maximum amount of time is a valuable safe guard
      M.setSolverParam("mioMaxTime", 180.0);
      M.solve();

      return x.level();
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

    double[] f = new double[n]; java.util.Arrays.fill(f, 0.01);
    double[] g = new double[n]; java.util.Arrays.fill(g, 0.001);
    double gamma = gammas[0];

    System.out.println("\n-----------------------------------------------------------------------------------");
    System.out.println("Markowitz portfolio optimization with transaction cost");
    System.out.println("-----------------------------------------------------------------------------------\n");

    double[] x = new double[n];
    x = MarkowitzWithTransactionsCost(n, mu, GT, x0, w, gamma, f, g);
    System.out.println("Optimal portfolio: \n");
    for ( int i = 0; i < x.length; ++i)
      System.out.format("\tx[%-2d]  %-12.4e\n", i, x[i]);

  }
}