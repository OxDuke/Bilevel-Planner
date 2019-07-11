//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      sudoku.cs
//
// Purpose:  A MILP-based SUDOKU solver
//
using System;
using System.Collections.Generic;

using mosek.fusion;

namespace mosek.fusion.example
{
  public class sudoku
  {
    public static void Main(string[] args)
    {
      int m = 3;
      int n = m * m;

      //fixed cells in human readable (i.e. 1-based) format
      int[,] hr_fixed = {
        {1, 5, 4},
        {2, 2, 5}, {2, 3, 8}, {2, 6, 3},
        {3, 2, 1}, {3, 4, 2}, {3, 5, 8}, {3, 7, 9},
        {4, 2, 7}, {4, 3, 3}, {4, 4, 1}, {4, 7, 8}, {4, 8, 4},
        {6, 2, 4}, {6, 3, 1}, {6, 6, 9}, {6, 7, 2}, {6, 8, 7},
        {7, 3, 4}, {7, 5, 6}, {7, 6, 5}, {7, 8, 8},
        {8, 4, 4}, {8, 7, 1}, {8, 8, 6},
        {9, 5, 9}
      };
      int nf = hr_fixed.Length / 3;
      int[,] fixed_cells = new int[nf, 3];
      for (int i = 0; i < nf; i++)
        for (int d = 0; d < m; d++)
          fixed_cells[i, d] = hr_fixed[i, d] - 1;

      using (Model M = new Model("SUDOKU")) {
        M.SetLogHandler(Console.Out);
        Variable x = M.Variable(new int[] { n, n, n }, Domain.Binary());

        //each value only once per dimension
        for (int d = 0; d < m; d++)
          M.Constraint(Expr.Sum(x, d), Domain.EqualsTo(1.0));

        //each number must appears only once in a block
        for (int k = 0; k < n; k++)
          for (int i = 0; i < m; i++)
            for (int j = 0; j < m; j++)
            {
              Variable block = x.Slice(new int[] { i * m, j * m, k },
                                       new int[] { (i + 1)*m, (j + 1)*m, k + 1 });
              M.Constraint(Expr.Sum(block), Domain.EqualsTo(1.0));
            }

        M.Constraint(x.Pick(fixed_cells), Domain.EqualsTo(1.0));

        M.Solve();

        //print the solution, if any...
        if (M.GetPrimalSolutionStatus() == SolutionStatus.Optimal)
          print_solution(m, x);
        else
          Console.WriteLine("No solution found!\n");
      }
    }

    public static void print_solution(int m, Variable x)
    {
      int n = m * m;
      Console.WriteLine("\n");
      try
      {
        for (int i = 0; i < n; i++)
        {
          String s = "";
          for (int j = 0; j < n; j++)
          {
            if (j % m == 0) s += " |";
            for (int k = 0; k < n; k++)
            {
              double[] sol = x.Index(new int[] { i, j, k }).Level();
              if (sol[0] > 0.5)
              {
                s += " " + (k + 1);
                break;
              }
            }
          }
          Console.WriteLine(s + " |");
          if ((i + 1) % m == 0)
            Console.WriteLine("\n");
        }
      }
      catch (SolutionError se) { }
    }
  }
}