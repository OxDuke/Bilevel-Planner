////
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      logistic.java
//
// Purpose: Implements logistic regression with regulatization.
//
//          Demonstrates using the exponential cone and log-sum-exp in Fusion.

package com.mosek.fusion.examples;
import mosek.fusion.*;

public class logistic {
  // t >= log( 1 + exp(u) ) coordinatewise
  public static void softplus(Model      M,
                              Expression t,
                              Expression u)
  {
    int n = t.getShape()[0];
    Variable z1 = M.variable(n);
    Variable z2 = M.variable(n);
    M.constraint(Expr.add(z1, z2), Domain.equalsTo(1));
    M.constraint(Expr.hstack(z1, Expr.constTerm(n, 1.0), Expr.sub(u,t)), Domain.inPExpCone());
    M.constraint(Expr.hstack(z2, Expr.constTerm(n, 1.0), Expr.neg(t)), Domain.inPExpCone());
  }


  // Model logistic regression (regularized with full 2-norm of theta)
  // X - n x d matrix of data points
  // y - length n vector classifying training points
  // lamb - regularization parameter
  public static Model logisticRegression(double[][] X, 
                                         boolean[]  y,
                                         double     lamb)
  {
    int n = X.length;
    int d = X[0].length;       // num samples, dimension
    
    Model M = new Model();   

    Variable theta = M.variable("theta", d);
    Variable t     = M.variable(n);
    Variable reg   = M.variable();

    M.objective(ObjectiveSense.Minimize, Expr.add(Expr.sum(t), Expr.mul(lamb,reg)));
    M.constraint(Var.vstack(reg, theta), Domain.inQCone());

    double[] signs = new double[n];
    for(int i = 0; i < n; i++)
      if (y[i]) signs[i] = -1; else signs[i] = 1;

    softplus(M, t, Expr.mulElm(Expr.mul(X, theta), signs));

    return M;
  }

  public static void main(String[] args)
  throws SolutionError {
    // Test: detect and approximate a circle using degree 2 polynomials
    int n = 30;
    double[][] X = new double[n*n][6];
    boolean[] Y = new boolean[n*n];

    for(int i=0; i<n; i++) 
    for(int j=0; j<n; j++)
    {
      int k = i*n+j;
      double x = -1 + 2.0*i/(n-1);
      double y = -1 + 2.0*j/(n-1);
      X[k][0] = 1.0; X[k][1] = x; X[k][2] = y; X[k][3] = x*y;
      X[k][4] = x*x; X[k][5] = y*y;
      Y[k] = (x*x+y*y>=0.69);
    }

    Model M = logisticRegression(X, Y, 0.1);
    Variable theta = M.getVariable("theta");

    M.setLogHandler(new java.io.PrintWriter(System.out));
    M.solve();
    for(int i=0;i<6;i++)
      System.out.println(theta.level()[i]);
  }
}