//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      cqo1.cs
//
//   Purpose: Demonstrates how to solve the problem
//
//   minimize y1 + y2 + y3
//   such that
//            x1 + x2 + 2.0 x3  = 1.0
//                    x1,x2,x3 >= 0.0
//   and
//            (y1,x1,x2) in C_3,
//            (y2,y3,x3) in K_3
//
//   where C_3 and K_3 are respectively the quadratic and
//   rotated quadratic cone of size 3 defined as
//       C_3 = { z1,z2,z3 :      z1 >= sqrt(z2^2 + z3^2) }
//       K_3 = { z1,z2,z3 : 2 z1 z2 >= z3^2              }
//
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class cqo1
  {
    public static void Main(string[] args)
    {
      using (Model M = new Model("cqo1"))
      {

        Variable x = M.Variable("x", 3, Domain.GreaterThan(0.0));
        Variable y = M.Variable("y", 3, Domain.Unbounded());

        // Create the aliases
        //      z1 = [ y[0],x[0],x[1] ]
        //  and z2 = [ y[1],y[2],x[2] ]
        Variable z1 = Var.Vstack(y.Index(0),  x.Slice(0, 2));
        Variable z2 = Var.Vstack(y.Slice(1, 3), x.Index(2));

        // Create the constraint
        //      x[0] + x[1] + 2.0 x[2] = 1.0
        double[] aval = new double[] {1.0, 1.0, 2.0};
        M.Constraint("lc", Expr.Dot(aval, x), Domain.EqualsTo(1.0));

        // Create the constraints
        //      z1 belongs to C_3
        //      z2 belongs to K_3
        // where C_3 and K_3 are respectively the quadratic and
        // rotated quadratic cone of size 3, i.e.
        //                 z1[0] >= sqrt(z1[1]^2 + z1[2]^2)
        //  and  2.0 z2[0] z2[1] >= z2[2]^2
        Constraint qc1 = M.Constraint("qc1", z1.AsExpr(), Domain.InQCone());
        Constraint qc2 = M.Constraint("qc2", z2.AsExpr(), Domain.InRotatedQCone());

        // Set the objective function to (y[0] + y[1] + y[2])
        M.Objective("obj", ObjectiveSense.Minimize, Expr.Sum(y));

        // Solve the problem
        M.Solve();

        // Get the linear solution values
        double[] solx = x.Level();
        double[] soly = y.Level();
        Console.WriteLine("x1,x2,x3 = {0}, {1}, {2}", solx[0], solx[1], solx[2]);
        Console.WriteLine("y1,y2,y3 = {0}, {1}, {2}", soly[0], soly[1], soly[2]);

        // Get conic solution of qc1
        double[] qc1lvl = qc1.Level();
        double[] qc1sn  = qc1.Dual();
        
        Console.Write("qc1 levels = {0}", qc1lvl[0]);
        for (int i = 1; i < qc1lvl.Length; ++i)
          Console.Write(", {0}", qc1lvl[i]);
        Console.WriteLine();

        Console.Write("qc1 dual conic var levels = {0}", qc1sn[0]);
        for (int i = 1; i < qc1sn.Length; ++i)
          Console.Write(", {0}", qc1sn[i]);
        Console.WriteLine();
      }
    }
  }
}