//
//    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//    File:    mico1.cs
//
//    Purpose:  Demonstrates how to solve a small mixed
//              integer conic optimization problem.
//
//              minimize    x^2 + y^2
//              subject to  x >= e^y + 3.8
//                          x, y - integer
//
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class mico1
  {
    public static void Main(string[] args)
    {
      using (Model M = new Model("mico1"))
      {
        Variable x = M.Variable(Domain.Integral(Domain.Unbounded()));
        Variable y = M.Variable(Domain.Integral(Domain.Unbounded()));
        Variable t = M.Variable();

        M.Constraint(Expr.Vstack(t, x, y), Domain.InQCone());
        M.Constraint(Expr.Vstack(Expr.Sub(x, 3.8), 1, y), Domain.InPExpCone());

        M.Objective(ObjectiveSense.Minimize, t);

        M.Solve();

        Console.WriteLine("x, y = {0}, {1}", x.Level()[0], y.Level()[0]);
      }
    }
  }
}