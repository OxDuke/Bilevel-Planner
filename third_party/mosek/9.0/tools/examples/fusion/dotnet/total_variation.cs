//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      total_variation.cs
//
// Purpose:   Demonstrates how to solve a total
//            variation problem using the Fusion API.using System;
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class total_variation
  {
    public static void Main(string[] args)
    {
      int ncols = 50;
      int nrows = 50;
      int seed  =  0;
      double sigma = 1.0;
      int ncells = nrows * ncols;
      Random gen = new Random(seed);

      double[] f = new double[ncells];

      //Random signal with Gaussian noise
      for (int i = 0; i < ncells; i++)
      {
        double xx = Math.Sqrt(-2.0 * Math.Log(gen.NextDouble()));
        double yy = Math.Sin(2.0 * Math.PI * gen.NextDouble());
        f[i] = Math.Max(Math.Min(1.0, xx * yy) , .0);
      }


      Model M = new Model("TV");
      try
      {
        Variable u = M.Variable(new int[] {nrows + 1, ncols + 1},
                                Domain.InRange(0.0, 1.0));
        Variable t = M.Variable(new int[] {nrows, ncols}, Domain.Unbounded());

        Variable ucore = u.Slice(new int[] {0, 0}, new int[] {nrows, ncols});

        Expression deltax = Expr.Sub(u.Slice(new int[] {1, 0},
                                             new int[] {nrows + 1, ncols}),
                                     ucore);
        Expression deltay = Expr.Sub(u.Slice(new int[] {0, 1},
                                             new int[] {nrows, ncols + 1}),
                                     ucore);

        M.Constraint(Expr.Stack(2, t, deltax, deltay), Domain.InQCone().Axis(2));

        Matrix mat_f = Matrix.Dense(nrows, ncols, f);
        M.Constraint(Expr.Vstack( sigma, Expr.Flatten( Expr.Sub(ucore, mat_f)) ),
                     Domain.InQCone());

        M.SetLogHandler(Console.Out);

        M.Objective(ObjectiveSense.Minimize, Expr.Sum(t));

        M.Solve();
      }
      finally
      {
        M.Dispose();
      }
    }
  }
}