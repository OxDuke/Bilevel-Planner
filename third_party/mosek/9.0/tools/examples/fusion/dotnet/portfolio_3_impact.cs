/*
  File : portfolio_3_impact.cs

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Implements a basic portfolio optimization model
                 with transaction costs of type x^(3/2)

  Note:
    This example uses LINQ, which is only available in .NET Framework 3.5 and later.
*/

using System.IO;
using System;
using System.Linq;
using System.Globalization;

namespace mosek.fusion.example
{
  public class portfolio_3_impact
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
        Description:
            Extends the basic Markowitz model with a market cost term.

        Input:
            n: Number of assets
            mu: An n dimmensional vector of expected returns
            GT: A matrix with n columns so (GT')*GT  = covariance matrix'
            x0: Initial holdings
            w: Initial cash holding
            gamma: Maximum risk (=std. dev) accepted
            m: It is assumed that  market impact cost for the j'th asset is
               m_j|x_j-x0_j|^3/2

        Output:
           Optimal expected return and the optimal portfolio

    */
    public static void MarkowitzWithMarketImpact
    ( int n,
      double[] mu,
      double[,]GT,
      double[] x0,
      double   w,
      double   gamma,
      double[] m,
      double[] xsol,
      double[] tsol)
    {
      using(Model M = new Model("Markowitz portfolio with market impact"))
      {

        //M.SetLogHandler(Console.Out);

        // Defines the variables. No shortselling is allowed.
        Variable x = M.Variable("x", n, Domain.GreaterThan(0.0));

        // Variables computing market impact
        Variable t = M.Variable("t", n, Domain.Unbounded());

        //  Maximize expected return
        M.Objective("obj", ObjectiveSense.Maximize, Expr.Dot(mu, x));

        // Invested amount + slippage cost = initial wealth
        M.Constraint("budget", Expr.Add(Expr.Sum(x), Expr.Dot(m, t)), Domain.EqualsTo(w + sum(x0)));

        // Imposes a bound on the risk
        M.Constraint("risk", Expr.Vstack(gamma, Expr.Mul(GT, x)), Domain.InQCone());

        // t >= |x-x0|^1.5 using a power cone
        M.Constraint("tz", Expr.Hstack(t, Expr.ConstTerm(n, 1.0), Expr.Sub(x,x0)), Domain.InPPowerCone(2.0/3.0));

        M.Solve();

        if (xsol != null)
          Array.Copy(x.Level(), xsol, n);
        if (tsol != null)
          Array.Copy(t.Level(), tsol, n);
      }
    }


    /*
      The example. Reads in data and solves the portfolio models.
     */
    public static void Main(string[] argv)
    {
      // Somewhat arbirtrary choice of m
      double[] m = new double[n]; for (int i = 0; i < n; ++i) m[i] = 1.0e-2;
      double[] x = new double[n];
      double[] t = new double[n];
      double gamma = gammas[0];

      MarkowitzWithMarketImpact(n, mu, GT, x0, w, gamma, m, x, t);
      Console.WriteLine("\n-----------------------------------------------------------------------");
      Console.WriteLine("Markowitz portfolio optimization with market impact cost");
      Console.WriteLine("------------------------------------------------------------------------");
      Console.WriteLine("Expected return: {0:e4} St deviation: {1:e4} Market impact cost: {2:e4}\n",
                        dot(mu, x),
                        gamma,
                        dot(m, t));

    }
  }
}
