//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      lpt.cs
//
// Purpose:  Demonstrates how to solve the multi-processor
//           scheduling problem using the Fusion API.
//


using System;
using System.Collections.Generic;

using mosek.fusion;

namespace mosek.fusion.example
{
  public class lpt
  {
    public static void Main(string [] args)
    {
      int n = 30;           //Number of tasks
      int m = 6;              //Number of processors

      double lb = 1.0;         //The range of lengths of short tasks
      double ub = 5.0;

      double sh = 0.8;        //The proportion of short tasks
      int n_short = (int)(n * sh);
      int n_long = n - n_short;

      double[] T = new double[n];
      Random gen = new Random(0);
      for (int i = 0; i < n_short; i++)
        T[i] = gen.NextDouble() * (ub - lb) + lb;
      for (int i = n_short; i < n; i++)
        T[i] = 20 * (gen.NextDouble() * (ub - lb) + lb);
      Array.Sort<double>(T);

      Model M = new Model("Multi-processor scheduling");

      Variable x = M.Variable("x", new int[] {m, n}, Domain.Binary());
      Variable t = M.Variable("t", 1, Domain.Unbounded());

      M.Constraint( Expr.Sum(x, 0), Domain.EqualsTo(1.0) );
      M.Constraint( Expr.Sub( Var.Repeat(t, m), Expr.Mul(x, T)), Domain.GreaterThan(0.0) );

      M.Objective( ObjectiveSense.Minimize, t );

      //Computing LPT solution
      double [] init    = new double[n * m];
      double [] schedule = new double[m];

      for (int i = n - 1; i >= 0; i--)
      {
        int next = 0;
        for (int j = 1; j < m; j++)
          if (schedule[j] < schedule[next]) next = j;
        schedule[next] += T[i];
        init[next * n + i] = 1;
      }

      //Comment this line to switch off feeding in the initial LPT solution
      x.SetLevel(init);

      M.SetLogHandler(Console.Out);

      M.SetSolverParam("mioTolRelGap", 0.01);
      M.Solve();

      try
      {
        Console.Write("initial solution: \n");
        for (int i = 0; i < m; i++)
        {
          Console.Write("M {0} [", i);
          for (int y = 0; y < n; y++)
            Console.Write( "{0}, ", (int) init[i * n + y]);
          Console.Write("]\n");
        }
        Console.Write("MOSEK solution:\n");
        for (int i = 0; i < m; i++)
        {
          Console.Write("M {0} [", i);
          for (int y = 0; y < n; y++)
            Console.Write( "{0}, ", (int)x.Index(i, y).Level()[0]);
          Console.Write("]\n");
        }
      }
      catch (SolutionError e) {}
    }
  }
}