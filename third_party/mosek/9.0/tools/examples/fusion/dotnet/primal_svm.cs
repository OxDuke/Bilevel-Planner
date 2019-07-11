//
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      primal_svm.cs
//
//  Purpose: Implements a simple soft-margin SVM
//          using the Fusion API.

using System;
using System.Collections.Generic;

using mosek.fusion;

namespace mosek.fusion.example
{
  public class primal_svm
  {
    public static void Main(string[] args)
    {
      int nc = 10;
      int m = 50;
      int n = 3;
      int seed = 1;
      double variance = 1.0;
      double mean = 1.0;

      Random gen = new Random(seed);

      int nump = gen.Next(m);

      double [] y = new double[m];

      for (int i = 0; i < nump; i++)  y[i] = 1.0;
      for (int i = nump; i < m; i++) y[i] = -1.0;

      double [,] X = new double[m, n];

      for (int i = 0; i < nump; i++)
        for (int j = 0; j < n; j++) {
          double xx = Math.Sqrt( -2.0 * Math.Log(gen.NextDouble()) );
          double yy = Math.Sin( 2.0 * Math.PI * gen.NextDouble() );
          X[i, j] = (xx * yy) * variance + mean;
        }
      Model M = new Model("Primal SVM");
      try {
        Console.WriteLine("Number of data    : {0}\n", m);
        Console.WriteLine("Number of features: {0}\n", n);

        Variable w =  M.Variable( n, Domain.Unbounded() );
        Variable t =  M.Variable( 1, Domain.Unbounded() );
        Variable b =  M.Variable( 1, Domain.Unbounded() );
        Variable xi = M.Variable( m, Domain.GreaterThan(0.0) );

        M.Constraint( Expr.Add(Expr.MulElm( y, Expr.Sub( Expr.Mul(X, w), Var.Repeat(b, m) ) ), xi ),
                      Domain.GreaterThan( 1.0 ) );
        M.Constraint( Expr.Vstack(1.0, t, w) , Domain.InRotatedQCone());

        Console.WriteLine(" c     | b      | w");
        for (int i = 0; i < nc; i++) {
          double c = 500.0 * i;
          M.Objective(ObjectiveSense.Minimize, Expr.Add( t, Expr.Mul(c, Expr.Sum(xi) ) ) );
          M.Solve();

          Console.Write("{0} | {1} |", c, b.Level()[0] );
          for (int j = 0; j < n; j++)
            Console.Write(" {0}", w.Level()[j] );
          Console.WriteLine();
        }
      }
      finally {
        M.Dispose();
      }
    }
  }
}