/**
  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File :      reoptimization.cs

  Purpose:   Demonstrates how to solve a  linear
             optimization problem using the MOSEK API
             and modify and re-optimize the problem.
*/
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class reoptimization
  {
    public static void printsol(double[] x) {
      Console.Write("x = ");
      foreach(double val in x)
          Console.Write(String.Format("{0:F4} ", val));
      Console.WriteLine();
    }

    public static void Main(string[] args)
    {
      using (Model M = new Model())
      {
        double[]   c = new double[] { 1.5, 2.5, 3.0 };
        double[,]  A = new double[,] { {2, 4, 3},
                                       {3, 2, 3},
                                       {2, 3, 2} };
        double[]   b = new double[] { 100000.0, 50000.0, 60000.0 };
        int   numvar = c.Length;
        int   numcon = b.Length;

        // Create a model and input data
        Variable x = M.Variable(numvar, Domain.GreaterThan(0.0));
        Constraint con = M.Constraint(Expr.Mul(A, x), Domain.LessThan(b));
        M.Objective(ObjectiveSense.Maximize, Expr.Dot(c, x));
        // Solve the problem
        M.Solve();
        printsol(x.Level());

        /************** Change an element of the A matrix ****************/
        con.Index(0).Update(Expr.Mul(3.0, x.Index(0)), x.Index(0));
        M.Solve();
        printsol(x.Level());

        /*************** Add a new variable ******************************/
        // Create a variable and a compound view of all variables
        Variable x3 = M.Variable(Domain.GreaterThan(0.0));
        Variable xNew = Var.Vstack(x, x3);
        // Add to the exising constraint
        con.Update(Expr.Mul(x3, new double[]{4, 0, 1}), x3);
        // Change the objective to include x3
        M.Objective(ObjectiveSense.Maximize, Expr.Dot(new double[]{1.5,2.5,3.0,1.0}, xNew));
        M.Solve();
        printsol(xNew.Level());

        /**************** Add a new constraint *****************************/
        Constraint con2 = M.Constraint(Expr.Dot(xNew, new double[]{1, 2, 1, 1}), Domain.LessThan(30000.0));
        M.Solve();
        printsol(xNew.Level());

        /**************** Change constraint bounds *****************************/
        // Assemble all constraints we previously defined into one
        Constraint cAll = Constraint.Vstack(con, con2);
        // Change bounds by effectively updating fixed terms with the difference
        cAll.Update(new double[]{20000, 10000, 10000, 8000});
        M.Solve();
        printsol(xNew.Level());
      }
    }
  }
}