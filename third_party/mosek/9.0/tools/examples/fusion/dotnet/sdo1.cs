//
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      sdo1.cs
//
//  Purpose:
//  Solves the mixed semidefinite and conic quadratic optimization problem
//
//                   [2, 1, 0]
//    minimize    Tr [1, 2, 1] * X + x0
//                   [0, 1, 2]
//
//                   [1, 0, 0]
//    subject to  Tr [0, 1, 0] * X + x0           = 1.0
//                   [0, 0, 1]
//
//                   [1, 1, 1]
//                Tr [1, 1, 1] * X      + x1 + x2 = 0.5
//                   [1, 1, 1]
//
//                   X >> 0,  x0 >= (x1^2 + x2^2) ^ (1/2)
//
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class sdo1
  {
    public static void Main(string[] args)
    {
      using (Model M  = new Model("sdo1"))
      {
        // Setting up the variables
        Variable X  = M.Variable("X", Domain.InPSDCone(3));
        Variable x  = M.Variable("x", Domain.InQCone(3));

        // Setting up constant coefficient matrices
        Matrix C  = Matrix.Dense ( new double[,] { {2, 1, 0}, {1, 2, 1}, {0, 1, 2}} );
        Matrix A1 = Matrix.Eye(3);
        Matrix A2 = Matrix.Ones(3,3);

        // Objective
        M.Objective(ObjectiveSense.Minimize, Expr.Add(Expr.Dot(C, X), x.Index(0)));

        // Constraints
        M.Constraint("c1", Expr.Add(Expr.Dot(A1, X), x.Index(0)), Domain.EqualsTo(1.0));
        M.Constraint("c2", Expr.Add(Expr.Dot(A2, X), Expr.Sum(x.Slice(1, 3))),
                     Domain.EqualsTo(0.5));

        M.Solve();

        Console.WriteLine("[{0}]", (new Utils.StringBuffer()).A(X.Level()).ToString());
        Console.WriteLine("[{0}]", (new Utils.StringBuffer()).A(x.Level()).ToString());
      }
    }
  }
}