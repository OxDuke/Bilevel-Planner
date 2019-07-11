//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      ceo1.cs
//
//   Purpose: Demonstrates how to solve the problem
//
//   minimize x1 + x2
//   such that
//            x1 + x2 + x3  = 1.0
//                x1,x2    >= 0.0
//   and      x1 >= x2 * exp(x3/x2)
//
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class ceo1
  {
    public static void Main(string[] args)
    {
      using (Model M = new Model("ceo1"))
      {

        Variable x = M.Variable("x", 3, Domain.Unbounded());

        // Create the constraint
        //      x[0] + x[1] + x[2] = 1.0
        M.Constraint("lc", Expr.Sum(x), Domain.EqualsTo(1.0));

        // Create the exponential conic constraint
        Constraint expc = M.Constraint("expc", x, Domain.InPExpCone());

        // Set the objective function to (x[0] + x[1])
        M.Objective("obj", ObjectiveSense.Minimize, Expr.Sum(x.Slice(0,2)));

        // Solve the problem
        M.Solve();

        // Get the linear solution values
        double[] solx = x.Level();
        Console.WriteLine("x1,x2,x3 = {0}, {1}, {2}", solx[0], solx[1], solx[2]);

        // Get conic solution of expc
        double[] expclvl = expc.Level();
        double[] expcsn  = expc.Dual();
        
        Console.Write("expc levels = {0}", expclvl[0]);
        for (int i = 1; i < expclvl.Length; ++i)
          Console.Write(", {0}", expclvl[i]);
        Console.WriteLine();

        Console.Write("expc dual conic var levels = {0}", expcsn[0]);
        for (int i = 1; i < expcsn.Length; ++i)
          Console.Write(", {0}", expcsn[i]);
        Console.WriteLine();
      }
    }
  }
}