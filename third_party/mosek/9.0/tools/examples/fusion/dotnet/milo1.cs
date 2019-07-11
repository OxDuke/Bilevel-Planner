
//
//    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//    File:    milo1.cs
//
//    Purpose:  Demonstrates how to solve a small mixed
//              integer linear optimization problem.
//
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class milo1
  {
    public static void Main(string[] args)
    {
      double[][] A =
      { new double[] { 50.0, 31.0 },
        new double[] { 3.0,  -2.0 }
      };
      double[] c = { 1.0, 0.64 };
      using (Model M = new Model("milo1"))
      {
        Variable x = M.Variable("x", 2, Domain.Integral(Domain.GreaterThan(0.0)));

        // Create the constraints
        //      50.0 x[0] + 31.0 x[1] <= 250.0
        //       3.0 x[0] -  2.0 x[1] >= -4.0
        M.Constraint("c1", Expr.Dot(A[0], x), Domain.LessThan(250.0));
        M.Constraint("c2", Expr.Dot(A[1], x), Domain.GreaterThan(-4.0));

        // Set max solution time
        M.SetSolverParam("mioMaxTime", 60.0);
        // Set max relative gap (to its default value)
        M.SetSolverParam("mioTolRelGap", 1e-4);
        // Set max absolute gap (to its default value)
        M.SetSolverParam("mioTolAbsGap", 0.0);

        // Set the objective function to (c^T * x)
        M.Objective("obj", ObjectiveSense.Maximize, Expr.Dot(c, x));

        // Solve the problem
        M.Solve();

        // Get the solution values
        double[] sol = x.Level();
        Console.WriteLine("x1,x2 = {0}, {1}", sol[0], sol[1]);
        double miorelgap = M.GetSolverDoubleInfo("mioObjRelGap");
        double mioabsgap = M.GetSolverDoubleInfo("mioObjAbsGap");
        Console.WriteLine("MIP rel gap = {0} ({0})", miorelgap, mioabsgap);
      }
    }
  }
}