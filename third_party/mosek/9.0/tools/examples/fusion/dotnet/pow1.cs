//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      pow1.cs
//
//   Purpose: Demonstrates how to solve the problem
//
//     maximize x^0.2*y^0.8 + z^0.4 - x
//           st x + y + 0.5z = 2
//              x,y,z >= 0
//
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class pow1
  {
    public static void Main(string[] args)
    {
      using (Model M = new Model("pow1"))
      {

        Variable x  = M.Variable("x", 3, Domain.Unbounded());
        Variable x3 = M.Variable();
        Variable x4 = M.Variable();

        // Create the linear constraint
        double[] aval = new double[] {1.0, 1.0, 0.5};
        M.Constraint(Expr.Dot(x, aval), Domain.EqualsTo(2.0));

        // Create the exponential conic constraint
        // Create the conic constraints
        M.Constraint(Var.Vstack(x.Slice(0,2), x3), Domain.InPPowerCone(0.2));
        M.Constraint(Expr.Vstack(x.Index(2), 1.0, x4), Domain.InPPowerCone(0.4));      

        // Set the objective function
        double[] cval = new double[] {1.0, 1.0, -1.0};
        M.Objective(ObjectiveSense.Maximize, Expr.Dot(cval, Var.Vstack(x3, x4, x.Index(0))));

        // Solve the problem
        M.Solve();

        // Get the linear solution values
        double[] solx = x.Level();
        Console.WriteLine("x,y,z = {0}, {1}, {2}", solx[0], solx[1], solx[2]);
      }
    }
  }
}