//
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      tsp.cs
//
//  Purpose: Demonstrates a simple technique to the TSP
//           usign the Fusion API.
//
using System;
using System.Collections.Generic;
using System.Collections;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class tsp {
    public static void Main(string [] args) {
      int []A_i = new int[] {0, 1, 2, 3, 1, 0, 2, 0};
      int []A_j = new int[] {1, 2, 3, 0, 0, 2, 1, 3};
      double []C_v =  new double[] {1.0, 1.0, 1.0, 1.0, 0.1, 0.1, 0.1, 0.1};

      int n = 4;
      Matrix adj = Matrix.Sparse(n, n, A_i, A_j, 1.0);
      Matrix costs = Matrix.Sparse(n, n, A_i, A_j, C_v);

      tsp_fusion(n, adj, costs , true, true);
      tsp_fusion(n, adj, costs , true, false);
    }

    public static void tsp_fusion(int n, Matrix A, Matrix C,
                                  bool remove_loops,
                                  bool remove_2_hop_loops) {
      Model M = new Model();

      Variable x = M.Variable(new int[] {n, n}, Domain.Binary());

      M.Constraint(Expr.Sum(x, 0), Domain.EqualsTo(1.0));
      M.Constraint(Expr.Sum(x, 1), Domain.EqualsTo(1.0));
      M.Constraint(x, Domain.LessThan( A ));

      M.Objective(ObjectiveSense.Minimize, Expr.Dot(C, x));

      if (remove_2_hop_loops)
        M.Constraint(Expr.Add(x, x.Transpose()), Domain.LessThan(1.0));

      if (remove_loops)
        M.Constraint(x.Diag(), Domain.EqualsTo(0.0));

      for (int it = 0; true; it++)
      {
        M.Solve();
        try
        {
          Console.WriteLine("\nit #{0} - solution cost: {1}", it, M.PrimalObjValue());
        } catch (SolutionError e) {}

        ArrayList cycles = new ArrayList();

        for (int i = 0; i < n; i++) {
          for (int j = 0; j < n; j++) {
            try {
              if ( x.Level()[i * n + j] <= 0.5) continue;
            } catch (SolutionError e) {}

            bool found = false;
            foreach (ArrayList c in cycles) {
              foreach (int[] a in c)
                if ( i == a[0] || j == a[0] ||
                     i == a[1] || j == a[1]  )
                {
                  c.Add( new int[] {i, j} );
                  found = true;
                  break;
                }
              if (found == true) break;
            }

            if (found != true) {
              ArrayList ll = new ArrayList();
              ll.Add( new int[] {i, j} );
              cycles.Add(ll);
            }
          }
        }

        Console.WriteLine("\ncycles:");
        foreach (ArrayList c in cycles) {
          foreach (int[] a in c)
            Console.Write("[{0},{1}] - ", a[0], a[1]);
          Console.Write("\n");
        }

        if (cycles.Count == 1) break;

        foreach (ArrayList c in cycles) {
          int [,] cc = new int[c.Count, 2];
          int i = 0;
          foreach (int[] a in c) {
            cc[i, 0] = a[0];
            cc[i, 1] = a[1];
            i++;
          }
          M.Constraint(Expr.Sum(x.Pick(cc)), Domain.LessThan( 1.0 * c.Count - 1 ));
        }
      }

      try {
        Console.WriteLine("\nsolution:");
        for (int i = 0; i < n; i++) {
          for (int j = 0; j < n; j++)
            Console.Write(" {0} ", ((int) x.Level()[i * n + j]));
          Console.Write("\n");
        }
      } catch (SolutionError e) {}
    }
  }
}

