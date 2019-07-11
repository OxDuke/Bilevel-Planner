//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:     alan.cs
//
//  Purpose: This file contains an implementation of the alan.gms (as
//  found in the GAMS online model collection) using Fusion/.NET.
//
//  The model is a simple portfolio choice model. The objective is to
//  invest in a number of assets such that we minimize the risk, while
//  requiring a certain expected return.
//
//  We operate with 4 assets (hardware,software, show-biz and treasure
//  bill). The risk is defined by the covariance matrix
//    Q = [[  4.0, 3.0, -1.0, 0.0 ],
//         [  3.0, 6.0,  1.0, 0.0 ],
//         [ -1.0, 1.0, 10.0, 0.0 ],
//         [  0.0, 0.0,  0.0, 0.0 ]]
//
//
//  We use the form Q = U^T * U, where U is a Cholesky factor of Q.
//

using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class alan
  {
    /////////////////////////////////////////////////////////////////////
    // Problem data.

    // Security names
    private static string[]
    securities = { "hardware", "software", "show-biz", "t-bills" };
    // Mean returns on securities
    private static double[]
    mean       = {        8.0,        9.0,       12.0,       7.0 };
    // Target mean return
    private static double
    target     = 10.0;

    // Factor of covariance matrix.
    private static Matrix U =
      Matrix.Dense(
        new double[,]
    { {  2.0       ,  1.5       , -0.5       , 0.0 },
      {  0.0       ,  1.93649167,  0.90369611, 0.0 },
      {  0.0       ,  0.0       ,  2.98886824, 0.0 },
      {  0.0       ,  0.0       ,  0.0       , 0.0 }
    });
    private static int numsec = securities.Length;
    public static void Main(String[] args)
    {
      using (Model M = new Model("alan"))
      {
        Variable x = M.Variable("x",        numsec, Domain.GreaterThan(0.0));
        Variable t = M.Variable("variance", Domain.GreaterThan(0.0));
        M.Objective("minvar", ObjectiveSense.Minimize, t.AsExpr());

        // sum securities to 1.0
        M.Constraint("wealth",  Expr.Sum(x), Domain.EqualsTo(1.0));
        // define target expected return
        M.Constraint("dmean", Expr.Dot(mean, x), Domain.GreaterThan(target));

        M.Constraint("q",Expr.Vstack(Expr.ConstTerm(1, 0.5),
                                 t.AsExpr(),
                                 Expr.Mul(U, x)),
                     Domain.InRotatedQCone());
        Console.WriteLine("Solve...");
        M.Solve();        
        Console.WriteLine("... Solved.");

        double[] solx = x.Level();

        Console.WriteLine("Primal solution = {0}", solx[0]);
        for (int i = 1; i < numsec; ++i)
          Console.Write(", {0}", solx[i]);
        Console.WriteLine("");        
      }
    }
  }
}
