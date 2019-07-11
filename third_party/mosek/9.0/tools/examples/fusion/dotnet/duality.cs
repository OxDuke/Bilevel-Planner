/*
  File : duality.cs

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Shows how to read the dual value of a constraint
*/
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class duality
  {
    public static void Main(string[] args)
    {
      double[,] A =  new double[,] { { -0.5, 1.0 }  };
      double[]  b =  new double[] { 1.0 };
      double[]  c =  new double[] { 1.0, 1.0 };

      using (Model M = new Model("duality"))
      {
        Variable x = M.Variable("x", 2, Domain.GreaterThan(0.0));

        Constraint con = M.Constraint(Expr.Sub(b, Expr.Mul(Matrix.Dense(A), x)), Domain.EqualsTo(0.0));

        M.Objective("obj", ObjectiveSense.Minimize, Expr.Dot(c, x));

        M.Solve();
        double[] xsol = x.Level();
        double[] ysol = con.Dual();

        Console.WriteLine("x1,x2,y = %{0}, %{1}, %{2}\n", xsol[0], xsol[1], ysol[0]);
      }
    }
  }
}