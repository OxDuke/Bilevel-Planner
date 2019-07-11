//
//    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//    File:    mioinitsol.cs
//
//    Purpose:  Demonstrates how to solve a small mixed
//             integer linear optimization problem
//             providing an initial feasible solution.
//
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class mioinitsol
  {
    public static void Main(string[] args)
    {
      double[] c = { 7.0, 10.0, 1.0, 5.0 };
      int n = 4;
      using (Model M = new Model("mioinitsol"))
      {

        Variable x = M.Variable("x", n, Domain.GreaterThan(0.0));
        x.Slice(0,3).MakeInteger();

        // Create the constraint
        M.Constraint(Expr.Sum(x), Domain.LessThan(2.5));

        // Set the objective function to (c^T * x)
        M.Objective("obj", ObjectiveSense.Maximize, Expr.Dot(c, x));

        // Assign values to integer variables.
        // We only set a slice of x     
        double[] init_sol = { 1, 1, 0 };
        x.Slice(0,3).SetLevel( init_sol );

        // Solve the problem
        M.Solve();

        // Get the solution values
        double[] sol = x.Level();
        Console.Write("x = [");
        for(int i=0;i<n;i++)
        {
          Console.Write("{0}, ",sol[i]);
        }
        Console.WriteLine("]");

        // Was the initial solution used?
        int constr = M.GetSolverIntInfo("mioConstructSolution");
        double constrVal = M.GetSolverDoubleInfo("mioConstructSolutionObj");
        Console.WriteLine("Initial solution utilization: " + constr);
        Console.WriteLine("Initial solution objective: " +  constrVal);
      }
    }
  }
}