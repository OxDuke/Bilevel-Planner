/*
  File : portfolio_1_basic.cs

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Implements a basic portfolio optimization model.
  
  Note:
    This example uses LINQ, which is only available in .NET Framework 3.5 and later.
*/

using System.IO;
using System;
using System.Linq;
using System.Globalization;

namespace mosek.fusion.example
{
  public class portfolio_1_basic
  {

    private static int        n      = 3;
    private static double     w      = 1.0;
    private static double[]   mu     = {0.1073, 0.0737, 0.0627};
    private static double[]   x0     = {0.0, 0.0, 0.0};
    private static double[]   gammas = {0.035, 0.040, 0.050, 0.060, 0.070, 0.080, 0.090};
    private static double[,] GT     = {
      { 0.166673333200005, 0.0232190712557243 ,  0.0012599496030238 },
      { 0.0              , 0.102863378954911  , -0.00222873156550421},
      { 0.0              , 0.0                ,  0.0338148677744977 }
    };
    public static double sum(double[] x)
    {
      double r = 0.0;
      for (int i = 0; i < x.Length; ++i) r += x[i];
      return r;
    }

    public static double dot(double[] x, double[] y)
    {
      double r = 0.0;
      for (int i = 0; i < x.Length; ++i) r += x[i] * y[i];
      return r;
    }

    /*
    Purpose:
        Computes the optimal portfolio for a given risk

    Input:
        n: Number of assets
        mu: An n dimmensional vector of expected returns
        GT: A matrix with n columns so (GT")*GT  = covariance matrix"
        x0: Initial holdings
        w: Initial cash holding
        gamma: Maximum risk (=std. dev) accepted

    Output:
        Optimal expected return and the optimal portfolio
    */
    public static double BasicMarkowitz
    ( int n,
      double[] mu,
      double[,]GT,
      double[] x0,
      double   w,
      double   gamma)
    {

      using( Model M = new Model("Basic Markowitz"))
      {

        // Redirect log output from the solver to stdout for debugging.
        // if uncommented.
        //M.SetLogHandler(Console.Out);

        // Defines the variables (holdings). Shortselling is not allowed.
        Variable x = M.Variable("x", n, Domain.GreaterThan(0.0));

        //  Maximize expected return
        M.Objective("obj", ObjectiveSense.Maximize, Expr.Dot(mu, x));

        // The amount invested  must be identical to intial wealth
        M.Constraint("budget", Expr.Sum(x), Domain.EqualsTo(w + sum(x0)));

        // Imposes a bound on the risk
        M.Constraint("risk", Expr.Vstack(gamma, Expr.Mul(GT, x)), Domain.InQCone());

        // Solves the model.
        M.Solve();

        return dot(mu, x.Level());
      }
    }


    /*
      The example. Reads in data and solves the portfolio models.
     */
    public static void Main(string[] argv)
    {

      Console.WriteLine("\n-------------------------------------------------------------------");
      Console.WriteLine("Basic Markowitz portfolio optimization");
      Console.WriteLine("---------------------------------------------------------------------");
      foreach (var gamma in gammas)
      {
        double res = BasicMarkowitz(n, mu, GT, x0, w, gamma);
        Console.WriteLine("Expected return: {0,-12:f4}  St deviation: {1,-12:f4} ", res, gamma);
      }
    }
  }
}
