//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      gp1.cs
//
//   Purpose:   Demonstrates how to solve a simple Geometric Program (GP)
//              cast into conic form with exponential cones and log-sum-exp.
//
//              Example from
//                https://gpkit.readthedocs.io/en/latest/examples.html//maximizing-the-volume-of-a-box
//
using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class gp1
  {
    // Models log(sum(exp(Ax+b))) <= 0.
    // Each row of [A b] describes one of the exp-terms
    public static void logsumexp(Model      M, 
                                 double[,]  A, 
                                 Variable   x,
                                 double[]   b)
    {
      int k = A.GetLength(0);
      Variable u = M.Variable(k);
      M.Constraint(Expr.Sum(u), Domain.EqualsTo(1.0));
      M.Constraint(Expr.Hstack(u,
                               Expr.ConstTerm(k, 1.0),
                               Expr.Add(Expr.Mul(A, x), b)), Domain.InPExpCone());
    }

    // maximize     h*w*d
    // subjecto to  2*(h*w + h*d) <= Awall
    //              w*d <= Afloor
    //              alpha <= h/w <= beta
    //              gamma <= d/w <= delta
    //
    // Variable substitutions:  h = exp(x), w = exp(y), d = exp(z).
    //
    // maximize     x+y+z
    // subject      log( exp(x+y+log(2/Awall)) + exp(x+z+log(2/Awall)) ) <= 0
    //                              y+z <= log(Afloor)
    //              log( alpha ) <= x-y <= log( beta )
    //              log( gamma ) <= z-y <= log( delta )
    public static double[] max_volume_box(double Aw, double Af, 
                                          double alpha, double beta, double gamma, double delta)
    {
      using (Model M = new Model("max_vol_box"))
      {
        Variable xyz = M.Variable(3);
        M.Objective("Objective", ObjectiveSense.Maximize, Expr.Sum(xyz));
        
        logsumexp(M, 
                  new double[,] {{1,1,0}, {1,0,1}}, 
                  xyz, 
                  new double[] {System.Math.Log(2.0/Aw), System.Math.Log(2.0/Aw)});
        
        M.Constraint(Expr.Dot(new double[] {0,1,1}, xyz), Domain.LessThan(System.Math.Log(Af)));
        M.Constraint(Expr.Dot(new double[] {1,-1,0}, xyz), Domain.InRange(System.Math.Log(alpha),System.Math.Log(beta)));
        M.Constraint(Expr.Dot(new double[] {0,-1,1}, xyz), Domain.InRange(System.Math.Log(gamma),System.Math.Log(delta)));
        
        M.SetLogHandler(Console.Out);
        M.Solve();
        
        double[] xyzVal = xyz.Level();
        double[] hwdVal = new double[3];
        for(int i=0; i<3; i++) hwdVal[i] = System.Math.Exp(xyzVal[i]);

        return hwdVal;
      }
    }

    public static void Main(String[] args)
    {
      double Aw    = 200.0;
      double Af    = 50.0;
      double alpha = 2.0;
      double beta  = 10.0;
      double gamma = 2.0;
      double delta = 10.0;
      
      double[] hwd = max_volume_box(Aw, Af, alpha, beta, gamma, delta);

      Console.WriteLine("h={0:f4} w={1:f4} d={2:f4}", hwd[0], hwd[1], hwd[2]);
    }
  }
}