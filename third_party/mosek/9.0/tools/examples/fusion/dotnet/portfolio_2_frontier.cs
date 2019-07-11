/*
  File : portfolio_2_frontier.cs

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Implements a basic portfolio optimization model.
                 Computes points on the efficient frontier.

  Note:
    This example uses LINQ, which is only available in .NET Framework 3.5 and later.
*/

using System.IO;
using System;
using System.Linq;
using System.Globalization;

namespace mosek.fusion.example
{
  public class portfolio_2_frontier
  {

    private static int        n      = 3;
    private static double     w      = 1.0;
    private static double[]   mu     = {0.1073, 0.0737, 0.0627};
    private static double[]   x0     = {0.0, 0.0, 0.0};
    private static double[,]  GT     = {
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
          Computes several portfolios on the optimal portfolios by

              for alpha in alphas:
                  maximize   expected return - alpha * variance
                  subject to the constraints

      Input:
          n: Number of assets
          mu: An n dimmensional vector of expected returns
          GT: A matrix with n columns so (GT")*GT  = covariance matrix"
          x0: Initial holdings
          w: Initial cash holding
          alphas: List of the alphas

      Output:
          The efficient frontier as list of tuples (alpha, expected return, variance)
     */
    public static void EfficientFrontier
    ( int n,
      double[]    mu,
      double [,]  GT,
      double[]    x0,
      double      w,
      double[]    alphas,
      double[]    frontier_mux,
      double[]    frontier_s)
    {
      using(Model M = new Model("Efficient frontier"))
      {
        //M.SetLogHandler(Console.Out);

        // Defines the variables (holdings). Shortselling is not allowed.
        Variable x = M.Variable("x", n, Domain.GreaterThan(0.0)); // Portfolio variables
        Variable s = M.Variable("s", 1, Domain.Unbounded());      // Variance variable

        M.Constraint("budget", Expr.Sum(x), Domain.EqualsTo(w + sum(x0)));

        // Computes the risk
        M.Constraint("variance", Expr.Vstack(s, 0.5, Expr.Mul(GT, x)), Domain.InRotatedQCone());

        Expression mudotx = Expr.Dot(mu, x);

        for (int i = 0; i < alphas.Length; ++i)
        {
          //  Define objective as a weighted combination of return and variance
          M.Objective("obj", ObjectiveSense.Maximize, Expr.Sub(mudotx, Expr.Mul(alphas[i], s)));

          M.Solve();

          frontier_mux[i] = dot(mu, x.Level());
          frontier_s[i]   = s.Level()[0];
        }
      }
    }


    /*
      The example. Reads in data and solves the portfolio models.
     */
    public static void Main(string[] argv)
    {

      // Some predefined alphas are chosen
      double[] alphas = { 0.0, 0.01, 0.1, 0.25, 0.30, 0.35, 0.4, 0.45, 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 10.0 };
      int      niter = alphas.Length;
      double[] frontier_mux = new double[niter];
      double[] frontier_s   = new double[niter];

      EfficientFrontier(n, mu, GT, x0, w, alphas, frontier_mux, frontier_s);
      Console.WriteLine("\n-------------------------------------------------------------------------");
      Console.WriteLine("Efficient frontier") ;
      Console.WriteLine("------------------------------------------------------------------------");
      Console.WriteLine("{0,-12}  {1,-12}  {2,-12}", "alpha", "return", "variance") ;
      for (int i = 0; i < frontier_mux.Length; ++i)
        Console.WriteLine("{0,-12:f4}  {1,-12:e4}  {2,-12:e4}", alphas[i], frontier_mux[i], frontier_s[i]);

    }
  }
}
