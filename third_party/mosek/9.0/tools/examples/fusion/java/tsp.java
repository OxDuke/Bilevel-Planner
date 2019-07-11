//
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      tsp.java
//
//  Purpose: Demonstrates a simple technique to the TSP
//           usign the Fusion API.
//
package com.mosek.fusion.examples;

import java.util.*;
import java.lang.*;
import java.io.*;
import java.lang.Math.*;
import mosek.*;
import mosek.fusion.*;


public class tsp {
  public static void main(String [] args) {

    int    []A_i = new int[]   {0, 1, 2, 3, 1, 0, 2, 0};
    int    []A_j = new int[]   {1, 2, 3, 0, 0, 2, 1, 3};
    double []C_v = new double[] {1., 1., 1., 1., 0.1, 0.1, 0.1, 0.1};

    int n = 4;

    Matrix costs = Matrix.sparse(n, n, A_i, A_j, C_v);
    Matrix adj   = Matrix.sparse(n, n, A_i, A_j, 1.);

    tsp_fusion(n, adj, costs, true, false);     //we do not remove loops of lenght 2
    tsp_fusion(n, adj, costs, true, true);      //we include all constraints
  }

  public static void tsp_fusion(int n,
                                Matrix A,
                                Matrix C,
                                boolean remove_loops,
                                boolean remove_2_hop_loops) {

    Model M = new Model();

    Variable x = M.variable(new int[] {n, n}, Domain.binary());

    M.constraint(Expr.sum(x, 0), Domain.equalsTo(1.0));
    M.constraint(Expr.sum(x, 1), Domain.equalsTo(1.0));
    M.constraint(x, Domain.lessThan( A ));

    M.objective(ObjectiveSense.Minimize, Expr.dot(C, x));

    if (remove_loops)
      M.constraint(x.diag(), Domain.equalsTo(0.));

    if (remove_2_hop_loops)
      M.constraint(Expr.add(x, x.transpose()), Domain.lessThan(1.0));

    int it = 0;

    while (true) {
      it = it + 1;
      M.solve();

      try {
        System.out.printf("\nit #%d - solution cost: %f\n", it, M.primalObjValue());
      } catch (SolutionError e) {}

      ArrayList<ArrayList< int[] > > cycles = new ArrayList<ArrayList< int[] > >();

      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          try {
            if ( x.level()[i * n + j] <= 0.5) continue;
          } catch (SolutionError e) {}

          boolean found = false;
          for (ArrayList<int[]> c : cycles) {
            for (int[] a : c)
              if ( i == a[0] || j == a[0] ||
                   i == a[1] || j == a[1]  ) {
                c.add( new int[] {i, j} );
                found = true;
                break;
              }
            if (found == true) break;
          }

          if (found != true) {
            ArrayList< int[] > ll = new ArrayList<int[]>();
            ll.add( new int[] {i, j} );
            cycles.add(ll);
          }
        }
      }

      System.out.printf("\ncycles:\n");
      for (ArrayList<int[]> c : cycles) {
        for (int[] a : c)
          System.out.printf("[%d,%d] - ", a[0], a[1]);
        System.out.printf("\n");
      }

      if (cycles.size() == 1) break;

      for (ArrayList<int []> c : cycles) {
        int [][] cc = new int[c.size()][2];
        c.toArray(cc);
        M.constraint(Expr.sum(x.pick(cc)), Domain.lessThan( 1.0 * c.size() - 1 ));
      }
    }

    try {
      System.out.printf("\nsolution:\n");
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
          System.out.printf(" %d ", ((int) x.level()[i * n + j]) );
        System.out.printf("\n");
      }
    } catch (SolutionError e) {}
  }
}
