//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      baker.java
//
// Purpose: Demonstrates a small linear problem.
//
// Source: "Linaer Algebra" by Knut Sydsaeter and Bernt Oeksendal.
//
// The problem: A baker has 150 kg flour, 22 kg sugar, 25 kg butter and two
// recipes:
//   1) Cakes, requiring 3.0 kg flour, 1.0 kg sugar and 1.2 kg butter per dozen.
//   2) Breads, requiring 5.0 kg flour, 0.5 kg sugar and 0.5 kg butter per dozen.
// Let the revenue per dozen cakes be $4 and the revenue per dozen breads be $6.
//
// We now wish to compute the combination of cakes and breads that will optimize
// the total revenue.

package com.mosek.fusion.examples;

import mosek.fusion.*;
import java.io.PrintWriter;

public class baker {
  private static String[] ingredientnames = { "Flour", "Sugar", "Butter" };
  private static double[] stock = { 150.0,   22.0,    25.0 };

  private static double[][] recipe_data = {
    { 3.0, 5.0 },
    { 1.0, 0.5 },
    { 1.2, 0.5 }
  };
  private static String[] productnames = { "Cakes", "Breads" };

  private static double[] revenue = { 4.0, 6.0 };
  public static void main(String[] args)
  throws SolutionError {
    Matrix recipe = Matrix.dense(recipe_data);
    Model M = new Model("Recipe");
    try {
      // "production" defines the amount of each product to bake.
      Variable production = M.variable("production",
                                       Set.make(productnames),
                                       Domain.greaterThan(0.0));
      // The objective is to maximize the total revenue.
      M.objective("revenue",
                  ObjectiveSense.Maximize,
                  Expr.dot(revenue, production));

      // The prodoction is constrained by stock:
      M.constraint(Expr.mul(recipe, production), Domain.lessThan(stock));
      M.setLogHandler(new PrintWriter(System.out));

      // We solve and fetch the solution:
      M.solve();
      double[] res = production.level();
      System.out.println("Solution:");
      for (int i = 0; i < res.length; ++i) {
        System.out.println(" Number of " + productnames[i] + " : " + res[i]);
      }
      System.out.println(" Revenue : $" +
                         (res[0] * revenue[0] + res[1] * revenue[1]));
    } finally {
      M.dispose();
    }
  }
}