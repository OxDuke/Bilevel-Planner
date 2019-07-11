// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      baker.cs
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

using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class baker
  {
    private static string[] ingredientnames = { "Flour", "Sugar", "Butter" };
    private static double[] stock = { 150.0,   22.0,    25.0 };

    private static double[,] recipe_data =
    { { 3.0, 5.0 },
      { 1.0, 0.5 },
      { 1.2, 0.5 }
    };
    private static string[] productnames = { "Cakes", "Breads" };

    private static double[] revenue = { 4.0, 6.0 };

    public static void Main(string[] args)
    {
      Matrix recipe = Matrix.Dense(recipe_data);
      using (Model M = new Model("Recipe"))
      {
        // "production" defines the amount of each product to bake.
        Variable production = M.Variable("production",
                                         Set.Make(productnames),
                                         Domain.GreaterThan(0.0));
        // The objective is to maximize the total revenue.
        M.Objective("revenue",
                    ObjectiveSense.Maximize,
                    Expr.Dot(revenue, production));

        // The prodoction is constrained by stock:
        M.Constraint(Expr.Mul(recipe, production), Domain.LessThan(stock));
        M.SetLogHandler(Console.Out);

        // We solve and fetch the solution:
        M.Solve();
        double[] res = production.Level();
        Console.WriteLine("Solution:");
        for (int i = 0; i < res.Length; ++i)
        {
          Console.WriteLine(" Number of {0} : {1}", productnames[i], res[i]);
        }
        Console.WriteLine(" Revenue : ${0}",
                          res[0] * revenue[0] + res[1] * revenue[1]);
      }
    }
  }
}