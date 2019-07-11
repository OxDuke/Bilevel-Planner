//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      breaksolver.cs
//
//  Purpose: Show how to break a long-running task.
//
//  Requires a parameter defining a timeout in seconds.
//


using System;
using System.Threading;
using mosek.fusion;
using System.Diagnostics;

namespace mosek.fusion.example
{
  public class breaksolver
  {
    private static void nshuffle(Random R, int[] a, int n)
    {
      for (int i = 0; i < n; ++i)
      {
        int idx = R.Next(i, a.Length);
        int tmp = a[i]; a[i] = a[idx]; a[idx] = tmp;
      }
    }

    public static void Main(string[] args)
    {
      long timeout = 5;

      int n = 200;    // number of binary variables
      int m = n / 5; // number of constraints
      int p = n / 5; // Each constraint picks p variables and requires that half of them are 1
      Random R = new Random(1234);
      using (Model M = new Model("SolveBinary"))
      {
        M.SetLogHandler(System.Console.Out);

        //Variable x = M.Variable("x", n, Domain.InRange(0,1));
        Variable x = M.Variable("x", n, Domain.Binary());
        M.Objective(ObjectiveSense.Minimize, Expr.Sum(x));
        //M.SetSolverParam("numThreads",1);

        int[] idxs  = new int[n]; for (int i = 0; i < n; ++i) idxs[i] = i;
        int[] cidxs = new int[p];

        for (var i = 0; i < m; ++i)
        {
          nshuffle(R, idxs, p);
          Array.Copy(idxs, cidxs, p);
          M.Constraint(Expr.Sum(x.Pick(cidxs)), Domain.EqualsTo(p / 2));
        }

        var T = new Thread(new ThreadStart(M.Solve));
        T.Start();

        Stopwatch w = new Stopwatch(); w.Start();
        while (true)
        {
          if (w.ElapsedMilliseconds > timeout * 1000)
          {
            Console.WriteLine("Solver terminated due to timeout!");
            M.BreakSolver();
            T.Join();
            break;
          }
          if (! T.IsAlive) {
            Console.WriteLine("Solver terminated before anything happened!");
            T.Join();
            break;
          }
        }

        Console.WriteLine("End.");
      }
    }
  }
}