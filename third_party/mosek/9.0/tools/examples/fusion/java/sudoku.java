//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      sudoku.java
//
// Purpose:  A MILP-based SUDOKU solver
//
//

package com.mosek.fusion.examples;

import java.lang.*;
import java.util.*;
import java.io.*;
import mosek.fusion.*;

public class sudoku {
  //fixed cells in human readable (i.e. 1-based) format
  private static final int [][] hr_fixed = { 
    {1, 5, 4},
    {2, 2, 5}, {2, 3, 8}, {2, 6, 3},
    {3, 2, 1}, {3, 4, 2}, {3, 5, 8}, {3, 7, 9},
    {4, 2, 7}, {4, 3, 3}, {4, 4, 1}, {4, 7, 8}, {4, 8, 4},
    {6, 2, 4}, {6, 3, 1}, {6, 6, 9}, {6, 7, 2}, {6, 8, 7},
    {7, 3, 4}, {7, 5, 6}, {7, 6, 5}, {7, 8, 8},
    {8, 4, 4}, {8, 7, 1}, {8, 8, 6},
    {9, 5, 9}
  };

  public static void main(String[] args)
  throws SolutionError {

    int m = 3;
    int n = m * m;

    int nf = hr_fixed.length;

    int [][] fixed = new int[nf][3];

    for (int i = 0; i < nf; i++)
      for (int d = 0; d < m; d++)
        fixed[i][d] =  hr_fixed[i][d] - 1;

    try(Model M = new Model("SUDOKU")) {
      M.setLogHandler(new PrintWriter(System.out));
      Variable x = M.variable(new int[] {n, n, n}, Domain.binary());

      //each value only once per dimension
      for (int d = 0; d < m; d++)
        M.constraint( Expr.sum(x, d), Domain.equalsTo(1.) );

      //each number must appear only once in a block
      for (int k = 0; k < n ; k++)
        for (int i = 0; i < m ; i++)
          for (int j = 0; j < m ; j++)
            M.constraint( Expr.sum( x.slice( new int[] {i * m, j * m, k},
                                             new int[] {(i + 1) * m, (j + 1) * m, k + 1}) ),
                          Domain.equalsTo(1.) );

      M.constraint( x.pick(fixed) , Domain.equalsTo(1.0) ) ;

      M.solve();

      //print the solution, if any...
      if ( M.getPrimalSolutionStatus() == SolutionStatus.Optimal )
        print_solution(m, x);
      else
        System.out.println("No solution found!\n");

    }
  }


  static void print_solution(int m, Variable x) {
    int n = m * m;
    System.out.println("\n");
    for (int i = 0; i < n; i++) {
      String s = "";
      for (int j = 0; j < n; j++) {
        if (j % m == 0) s += " |";
        for (int k = 0; k < n; k++)
          try {
            double sol[] = x.index( new int[] {i, j, k}).level();
            if (sol[0] > 0.5) {
              s += " " + (k + 1);
              break;
            }
          } catch (SolutionError se) {}
      }
      System.out.println(s + " |");

      if ((i + 1) % m == 0)
        System.out.println("\n");
    }
  }
}