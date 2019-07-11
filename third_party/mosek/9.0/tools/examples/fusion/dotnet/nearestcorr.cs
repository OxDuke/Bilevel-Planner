/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      nearestcorr.cs

   Purpose:
   Solves the nearest correlation matrix problem

     minimize   || A - X ||_F   s.t.  diag(X) = e, X is PSD

   as the equivalent conic program

     minimize     t

     subject to   (t, vec(A-X)) in Q
                  diag(X) = e
                  X >= 0
*/
using mosek.fusion;
using System;

namespace mosek.fusion.example
{
  public class nearestcorr
  {
    /** Assuming that e is an NxN expression, return the lower triangular part as a vector.
    */
    public static Expression Vec(Expression e)
    {
      int N       = e.GetShape()[0];
      int[] msubi = new int[N * (N + 1) / 2],
      msubj = new int[N * (N + 1) / 2];
      double[] mcof = new double[N * (N + 1) / 2];

      for (int i = 0, k = 0; i < N; ++i)
        for (int j = 0; j < i + 1; ++j, ++k)
        {
          msubi[k] = k;
          msubj[k] = i * N + j;
          if (i == j) mcof[k] = 1.0;
          else        mcof[k] = Math.Sqrt(2);
        }

      var S = Matrix.Sparse(N * (N + 1) / 2, N * N, msubi, msubj, mcof);
      return Expr.Mul(S, Expr.Flatten(e));
    }

    public static void Main(string[] argv)
    {
      int N = 5;
      var A = Matrix.Dense( new double[,]
      { { 0.0,  0.5,  - 0.1,  -0.2,   0.5},
        { 0.5,  1.25, -0.05, -0.1,   0.25},
        { -0.1, -0.05,  0.51,  0.02, -0.05},
        { -0.2, -0.1,   0.02,  0.54, -0.1},
        { 0.5,  0.25, -0.05, -0.1,   1.25}
      });

      var gammas = Range(0.0, 2.0, 0.1);
      var res  = new double[gammas.Length];
      var rank = new int[gammas.Length];

      nearestcorr_frobenius(A);
      nearestcorr_nn(A, gammas, res, rank);

      for (int i = 0; i < gammas.Length; ++i)
        Console.WriteLine("gamma = {0:f1}, res={1:e3}, rank = {2}", gammas[i], res[i], rank[i]);
    }

    public static void nearestcorr_nn(Matrix A, double[] gammas, double[] res, int[] rank)
    {
      int N = A.NumRows();
      using (var M = new Model("NucNorm"))
      {
        // Setup variables
        var t = M.Variable("t", 1, Domain.Unbounded());
        var X = M.Variable("X", Domain.InPSDCone(N));
        var w = M.Variable("w", N, Domain.GreaterThan(0.0));

        // (t, vec (X + diag(w) - A)) in Q
        var D = Expr.MulElm( Matrix.Eye(N), Var.Repeat(w, 1, N) );
        M.Constraint( Expr.Vstack( t, Vec(Expr.Sub(Expr.Add(X, D), A)) ), Domain.InQCone() );

        for (var k = 0; k < gammas.Length; ++k)
        {
          // Objective: Minimize t + gamma*Tr(X)
          var gamm_trX = Expr.Mul( gammas[k], Expr.Sum(X.Diag()) );
          M.Objective(ObjectiveSense.Minimize, Expr.Add(t, gamm_trX));
          M.Solve();

          // Find the eigenvalues of X and approximate rank
          var d = new double[N];
          mosek.LinAlg.syeig(mosek.uplo.lo, N, X.Level(), d);
          var rnk = 0; foreach (var v in d) if (v > 1e-6) ++rnk;

          res[k] = t.Level()[0];
          rank[k] = rnk;
        }
      }
    }

    public static void nearestcorr_frobenius(Matrix A)
    {
      int N = A.NumRows();
      using (var M = new Model("NearestCorrelation"))
      {
        // Setting up the variables
        var X = M.Variable("X", Domain.InPSDCone(N));
        var t = M.Variable("t", 1, Domain.Unbounded());

        // (t, vec (A-X)) \in Q
        M.Constraint( Expr.Vstack(t, Vec(Expr.Sub(A, X))), Domain.InQCone() );

        // diag(X) = e
        M.Constraint(X.Diag(), Domain.EqualsTo(1.0));

        // Objective: Minimize t
        M.Objective(ObjectiveSense.Minimize, t);

        // Solve the problem
        M.Solve();

        // Get the solution values
        Console.WriteLine("X = \n{0}", mattostr(X.Level(), N));
        Console.WriteLine("t = {0}", mattostr(t.Level(), N));
      }
    }

    /* Utility methods */
    private static string mattostr(double[] a, int n)
    {
      var c = new System.Globalization.CultureInfo("en-US");
      var b = new System.Text.StringBuilder();
      if (a.Length == 1 )
      {
        b.Append(String.Format(c, "{0:e3}", a[0]));
      }
      else
      {
        for (int i = 0; i < a.Length; ++i)
        {
          if (  i  % n == 0 )
            b.Append("[");
          b.Append(" ").Append(String.Format(c, "{0:e3}", a[i]));
          if ( ( i + 1 ) % n == 0 )
            b.Append("]\n");
        }
      }
      return b.ToString();
    }

    public static double[] Range (double start, double stop, double step)
    {
      int len;
      if (start < stop && step > 0.0)
        len = 1 + (int)((stop - start - 1) / step);
      else if (stop < start && step < 0)
        len = 1 + (int)((start - stop - 1) / (- step));
      else
        len = 0;
      double[] res = new double[len];
      double v = start;
      for (int i = 0; i < len; ++i, v += step)
        res[i] = v;
      return res;
    }
  }
}