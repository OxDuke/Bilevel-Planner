////
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      logistic.cs
//
// Purpose: Implements logistic regression with regulatization.
//
//          Demonstrates using the exponential cone and log-sum-exp in Fusion.

using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  public class logistic
  {
    // t >= log( 1 + exp(u) ) coordinatewise
    public static void softplus(Model      M,
                                Expression t,
                                Expression u)
    {
      int n = t.GetShape()[0];
      Variable z1 = M.Variable(n);
      Variable z2 = M.Variable(n);
      M.Constraint(Expr.Add(z1, z2), Domain.EqualsTo(1));
      M.Constraint(Expr.Hstack(z1, Expr.ConstTerm(n, 1.0), Expr.Sub(u,t)), Domain.InPExpCone());
      M.Constraint(Expr.Hstack(z2, Expr.ConstTerm(n, 1.0), Expr.Neg(t)), Domain.InPExpCone());
    }

    // Model logistic regression (regularized with full 2-norm of theta)
    // X - n x d matrix of data points
    // y - length n vector classifying training points
    // lamb - regularization parameter
    public static Model logisticRegression(double[,]  X, 
                                           bool[]     y,
                                           double     lamb)
    {
      int n = X.GetLength(0);
      int d = X.GetLength(1);       // num samples, dimension
      
      Model M = new Model();   

      Variable theta = M.Variable("theta", d);
      Variable t     = M.Variable(n);
      Variable reg   = M.Variable();

      M.Objective(ObjectiveSense.Minimize, Expr.Add(Expr.Sum(t), Expr.Mul(lamb,reg)));
      M.Constraint(Var.Vstack(reg, theta), Domain.InQCone());
      
      double[] signs = new double[n];
      for(int i = 0; i < n; i++)
        if (y[i]) signs[i] = -1; else signs[i] = 1;

      softplus(M, t, Expr.MulElm(Expr.Mul(X, theta), signs));

      return M;
    }

    public static void Main(String[] args)
    {
      // Test: detect and approximate a circle using degree 2 polynomials
      int n = 30;
      double[,] X = new double[n*n, 6];
      bool[] Y     = new bool[n*n];

      for(int i=0; i<n; i++) 
      for(int j=0; j<n; j++)
      {
        int k = i*n+j;
        double x = -1 + 2.0*i/(n-1);
        double y = -1 + 2.0*j/(n-1);
        X[k,0] = 1.0; X[k,1] = x; X[k,2] = y; X[k,3] = x*y;
        X[k,4] = x*x; X[k,5] = y*y;
        Y[k] = (x*x+y*y>=0.69);
      }

      Model M = logisticRegression(X, Y, 0.1);
      Variable theta = M.GetVariable("theta");

      M.Solve();
      for(int i=0; i<6; i++)
        Console.WriteLine(theta.Level()[i]);
    }
  }
}