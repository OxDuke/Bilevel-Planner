/*
  File : diet.cs

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Implements a basic linear model.
*/
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class diet : Model
  {
    public class DataException : FusionException
    {
      public DataException(string msg) : base(msg) {  }
    }

    public double[] dailyAllowance;
    public Matrix nutrientValue;

    private Variable   dailyPurchase;
    private Constraint dailyNutrients;

    public diet(string    name,
                string[]  foods,
                string[]  nutrients,
                double[]  daily_allowance,
                double[,] nutritive_value)
    : base(name)
    {
      bool finished = false;
      try
      {
        dailyAllowance = daily_allowance;
        nutrientValue = (Matrix.Dense(nutritive_value)).Transpose();

        int M = foods.Length;
        int N = nutrients.Length;

        if (dailyAllowance.Length != N)
          throw new DataException(
            "Length of daily_allowance does not match the number of nutrients");
        if (nutrientValue.NumColumns() != M)
          throw new DataException(
            "Number of rows in nutrient_value does not match the number of foods");
        if (nutrientValue.NumRows() != N)
          throw new DataException(
            "Number of columns in nutrient_value does not match the number of nutrients");

        dailyPurchase = Variable("Daily Purchase",
                                 Set.Make(foods),
                                 Domain.GreaterThan(0.0));
        dailyNutrients =
          Constraint("Nutrient Balance",
                     Expr.Mul(nutrientValue, dailyPurchase),
                     Domain.GreaterThan(dailyAllowance));
        Objective(ObjectiveSense.Minimize, Expr.Sum(dailyPurchase));
        finished = true;
      }
      finally
      {
        if (! finished)
          Dispose();
      }
    }

    public double[] getDailyPurchase()
    {
      return dailyPurchase.Level();
    }

    public double[] getDailyNutrients()
    {
      return dailyNutrients.Level();
    }


    /* Main class with data definitions */
    public static void Main(string[] argv)
    {
      string[] nutrient_unit = {
        "thousands",  "grams",        "grams",
        "milligrams", "thousand ius", "milligrams",
        "milligrams", "milligrams",   "milligrams"
      };
      string[] nutrients = {
        "calorie",    "protein",      "calcium",
        "iron",       "vitamin a",    "vitamin b1",
        "vitamin b2", "niacin",       "vitamin c"
      };
      string[] foods = {
        "wheat",        "cornmeal", "cannedmilk", "margarine", "cheese",
        "peanut butter", "lard",     "liver",      "porkroast", "salmon",
        "greenbeans",   "cabbage",  "onions",     "potatoes",  "spinach",
        "sweet potatos", "peaches",  "prunes",     "limabeans", "navybeans"
      };
      double[,] nutritive_value = {
        //  calorie       calcium      vitamin a        vitamin b2      vitamin c
        //         protein        iron           vitamin b1      niacin
        {44.7,  1411,   2.0,   365,    0,      55.4,   33.3,  441,     0},  // wheat
        {36,     897,   1.7,    99,   30.9,    17.4,    7.9,  106,     0},  // cornmeal
        { 8.4,   422,  15.1,     9,   26,       3,     23.5,   11,    60},  // cannedmilk
        {20.6,    17,   0.6,     6,   55.8,     0.2,    0,      0,     0},  // margarine
        { 7.4,   448,  16.4,    19,   28.1,     0.8,   10.3,    4,     0},  // cheese
        {15.7,   661,   1,      48,     0,      9.6,    8.1,  471,     0},  // peanut butter
        {41.7,     0,   0,       0,    0.2,     0,      0.5,    5,     0},  // lard
        { 2.2,   333,   0.2,   139,  169.2,     6.4,   50.8,  316,   525},  // liver
        { 4.4,   249,   0.3,    37,    0,      18.2,    3.6,   79,     0},  // porkroast
        { 5.8,   705,   6.8,    45,    3.5,     1,      4.9,  209,     0},  // salmon
        { 2.4,   138,   3.7,    80,   69,       4.3,    5.8,   37,   862},  // greenbeans
        { 2.6,   125,   4,      36,    7.2,     9,      4.5,   26,  5369},  // cabbage
        { 5.8,   166,   3.8,    59,   16.6,     4.7,    5.9,   21,  1184},  // onions
        {14.3,   336,   1.8,   118,    6.7,    29.4,    7.1,  198,  2522},  // potatoes
        { 1.1,   106,   0.0,   138,  918.4,     5.7,   13.8,   33,  2755},  // spinach
        { 9.6,   138,   2.7,    54,  290.7,     8.4,    5.4,   83,  1912},  // sweet potatos
        { 8.5,    87,   1.7,   173,   86.8,     1.2,    4.3,   55,    57},  // peaches
        {12.8,    99,   2.5,   154,   85.7,     3.9,    4.3,   65,   257},  // prunes
        {17.4,  1055,   3.7,   459,    5.1,    26.9,   38.2,   93,     0},  // limabeans
        {26.9,  1691,  11.4,   792,    0,      38.4,   24.6,  217,     0}
      };// navybeans
      double[] daily_allowance =
      {   3.0,    70.0, 0.8,    12.0,  5.0,     1.8,    2.7,   18.0,  75.0 };

      using (diet M = new diet("Stinglers Diet Problem",
                               foods,
                               nutrients,
                               daily_allowance,
                               nutritive_value))
      {
        M.Solve();

        try
        {
          Console.WriteLine("Solution (x 365):");
          double[] x = M.getDailyPurchase();
          for (int i = 0; i < x.Length; ++i)
            Console.WriteLine("  {0,15} : {1,10:0.00} $", foods[i], 365 * x[i]);

          Console.WriteLine("Nutrients (daily):");
          double[] y = M.getDailyNutrients();
          for (int i = 0; i < y.Length; ++i)
            Console.WriteLine("  {0,15} : {1,10:0.00} {2,-12} ({3})",
                              nutrients[i],
                              y[i],
                              nutrient_unit[i],
                              daily_allowance[i]);
        }
        catch (SolutionError e)
        {
          Console.WriteLine("Solution error : {0}", e.ToString());
        }
      }
    }
  }
}