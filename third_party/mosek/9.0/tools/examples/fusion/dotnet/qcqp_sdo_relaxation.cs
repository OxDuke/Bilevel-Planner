/*
* Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
*
* File:      qcqp_sdo_relaxation.cs
*
* Purpose:   Demonstrate how to use SDP to solve
*            convex relaxation of a mixed-integer QCQP
**/
using System;

using mosek.fusion;
using mosek;

namespace mosek.fusion.example
{
  public class qcqp_sdo_relaxation
  {
    public static void Main(string[] args)
    {
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
        for (int i = 0; i < m; i++) {
          double u1 = rnd.NextDouble(); //these are uniform(0,1) random doubles
          double u2 = rnd.NextDouble();
          A[i * n + j] = Math.Sqrt(-2.0 * Math.Log(u1)) * Math.Sin(2.0 * Math.PI * u2);
        }
        c[j] = rnd.NextDouble();
      }

      // P = A^T A
      LinAlg.syrk(mosek.uplo.lo, mosek.transpose.yes,
                  n, m, 1.0, A, 0.0, P);
      for (int j = 0; j < n; j++) for (int i = j + 1; i < n; i++) P[i * n + j] = P[j * n + i];

      // q = -P c, b = A c
      LinAlg.gemv(mosek.transpose.no, n, n, -1.0, P, c, 0.0, q);
      LinAlg.gemv(mosek.transpose.no, m, n,  1.0, A, c, 0.0, b);

      // Solve the problems
      Model M = miqcqp_sdo_relaxation(n, Matrix.Dense(n, n, P), q);
      Model Mint = int_least_squares(n, Matrix.Dense(n, m, A).Transpose(), b);
      M.Solve();
      Mint.Solve();

      // rounded and optimal solution
      double[] xRound = M.GetVariable("Z").Slice(new int[] {0, n}, new int[] {n, n + 1}).Level();
      for (int i = 0; i < n; i++) xRound[i] = Math.Round(xRound[i]);
      double[] yRound = (double[]) b.Clone();
      double[] xOpt   = Mint.GetVariable("x").Level();
      double[] yOpt   = (double[]) b.Clone();
      LinAlg.gemv(mosek.transpose.no, m, n,  1.0, A, xRound, -1.0, yRound);        // Ax_round-b
      LinAlg.gemv(mosek.transpose.no, m, n,  1.0, A, xOpt, -1.0, yOpt);            // Ax_opt-b

      Console.WriteLine(M.GetSolverDoubleInfo("optimizerTime") + " " + Mint.GetSolverDoubleInfo("optimizerTime"));
      Console.WriteLine(Math.Sqrt(LinAlg.dot(m, yRound, yRound)) + " " +
                        Math.Sqrt(LinAlg.dot(m, yOpt, yOpt)));
    }

    public static Model miqcqp_sdo_relaxation(int n, Matrix P, double[] q) {
      Model M = new Model();

      Variable Z = M.Variable("Z", Domain.InPSDCone(n + 1));

      Variable X = Z.Slice(new int[] {0, 0}, new int[] {n, n});
      Variable x = Z.Slice(new int[] {0, n}, new int[] {n, n + 1});

      M.Constraint( Expr.Sub(X.Diag(), x), Domain.GreaterThan(0.0) );
      M.Constraint( Z.Index(n, n), Domain.EqualsTo(1.0) );

      M.Objective( ObjectiveSense.Minimize, Expr.Add(
                     Expr.Sum( Expr.MulElm( P, X ) ),
                     Expr.Mul( 2.0, Expr.Dot(x, q) )
                   ) );

      return M;
    }

    // A direct integer model for minimizing |Ax-b|
    public static Model int_least_squares(int n, Matrix A, double[] b) {
      Model M = new Model();

      Variable x = M.Variable("x", n, Domain.Integral(Domain.Unbounded()));
      Variable t = M.Variable("t", 1, Domain.Unbounded());

      M.Constraint( Expr.Vstack(t, Expr.Sub(Expr.Mul(A, x), b)), Domain.InQCone() );
      M.Objective( ObjectiveSense.Minimize, t );

      return M;
    }
  }
}