/*
  File : portfolio_4_transcost.cs

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Implements a basic portfolio optimization model
                 with fixed setup costs and transaction costs
                 as a mixed-integer problem.

  Note:
    This example uses LINQ, which is only available in .NET Framework 3.5 and later.
*/

using System.IO;
using System;
using System.Linq;
using System.Globalization;

namespace mosek.fusion.example
{
  public class portfolio_4_transcost
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
            GT: A matrix with n columns so (GT")*GT  = covariance matrix"
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
      double[,]GT,
      double[] x0,
      double   w,
      double   gamma,
      double[] f,
      double[] g)
    {

      // Upper bound on the traded amount
      double[] u = new double[n];
      {
        double v = w + sum(x0);
        for (int i = 0; i < n; ++i) u[i] = v;
      }

      using( Model M = new Model("Markowitz portfolio with transaction costs") )
      {
        //M.SetLogHandler(Console.Out);

        // Defines the variables. No shortselling is allowed.
        Variable x = M.Variable("x", n, Domain.GreaterThan(0.0));

        // Addtional "helper" variables
        Variable z = M.Variable("z", n, Domain.Unbounded());
        // Binary varables
        Variable y = M.Variable("y", n, Domain.Binary());

        //  Maximize expected return
        M.Objective("obj", ObjectiveSense.Maximize, Expr.Dot(mu, x));

        // Invest amount + transactions costs = initial wealth
        M.Constraint("budget", Expr.Add(Expr.Add(Expr.Sum(x), Expr.Dot(f, y)), Expr.Dot(g, z)),
                     Domain.EqualsTo(w + sum(x0)));

        // Imposes a bound on the risk
        M.Constraint("risk", Expr.Vstack(gamma, Expr.Mul(GT, x)), Domain.InQCone());

        // z >= |x-x0|
        M.Constraint("buy",  Expr.Sub(z, Expr.Sub(x, x0)), Domain.GreaterThan(0.0));
        M.Constraint("sell", Expr.Sub(z, Expr.Sub(x0, x)), Domain.GreaterThan(0.0));

        //M.constraint("trade", Expr.hstack(z,Expr.sub(x,x0)), Domain.inQcone())"

        // Consraints for turning y off and on. z-diag(u)*y<=0 i.e. z_j <= u_j*y_j
        M.Constraint("y_on_off", Expr.Sub(z, Expr.Mul(Matrix.Diag(u), y)), Domain.LessThan(0.0));

        // Integer optimization problems can be very hard to solve so limiting the
        // maximum amount of time is a valuable safe guard
        M.SetSolverParam("mioMaxTime", 180.0);
        M.Solve();
        return x.Level();
      }
    }


    /*
      The example. Reads in data and solves the portfolio models.
     */
    public static void Main(string[] argv)
    {

      double[] f = new double[n]; for (var i = 0; i < n; ++i) f[i] = 0.01;
      double[] g = new double[n]; for (var i = 0; i < n; ++i) g[i] = 0.001;
      double gamma = gammas[0];

      double[] x = MarkowitzWithTransactionsCost(n, mu, GT, x0, w, gamma, f, g);
      Console.WriteLine("\n-------------------------------------------------------------------------");
      Console.WriteLine("Markowitz portfolio optimization with transaction cost");
      Console.WriteLine("------------------------------------------------------------------------");
      Console.WriteLine("Expected return: {0:e4}", dot(mu, x));

    }
  }
}
