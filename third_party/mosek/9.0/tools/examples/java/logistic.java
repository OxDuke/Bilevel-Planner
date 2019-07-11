////
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      logistic.java
//
// Purpose: Implements logistic regression with regulatization.
//
//          Demonstrates using the exponential cone and log-sum-exp in Optimizer API.

package com.mosek.example;
import mosek.*;

public class logistic {
  public static double inf = 0.0;

  // t >= log( 1 + exp(u) )
  // t_i >= log( 1 + exp(u_i) ), i = 0..n-1
  // Adds auxiliary variables and constraints
  public static void softplus(Task task, int t, int u, int n)
  {
    int nvar = task.getnumvar();
    int ncon = task.getnumcon();
    task.appendvars(6*n);
    task.appendcons(3*n);
    int z1 = nvar, z2 = nvar+n, v1 = nvar+2*n, v2 = nvar+3*n, q1 = nvar+4*n, q2 = nvar+5*n;
    int zcon = ncon, v1con = ncon+n, v2con=ncon+2*n;
    int[]    subi = new int[7*n];
    int[]    subj = new int[7*n];
    double[] aval = new double[7*n];
    int         k = 0;

    // Linear constraints
    for(int i = 0; i < n; i++)
    {
      // z1 + z2 = 1
      subi[k] = zcon+i;  subj[k] = z1+i;  aval[k] = 1;  k++;
      subi[k] = zcon+i;  subj[k] = z2+i;  aval[k] = 1;  k++;
      // u - t - v1 = 0
      subi[k] = v1con+i; subj[k] = u+i;   aval[k] = 1;  k++;
      subi[k] = v1con+i; subj[k] = t+i;   aval[k] = -1; k++;
      subi[k] = v1con+i; subj[k] = v1+i;  aval[k] = -1; k++;
      // - t - v2 = 0
      subi[k] = v2con+i; subj[k] = t+i;   aval[k] = -1; k++;
      subi[k] = v2con+i; subj[k] = v2+i;  aval[k] = -1; k++;
    }
    task.putaijlist(subi, subj, aval);
    task.putconboundsliceconst(ncon, ncon+n, boundkey.fx, 1, 1);
    task.putconboundsliceconst(ncon+n, ncon+3*n, boundkey.fx, 0, 0);

    // Bounds for variables
    task.putvarboundsliceconst(nvar, nvar+4*n, boundkey.fr, -inf, inf);
    task.putvarboundsliceconst(nvar+4*n, nvar+6*n, boundkey.fx, 1, 1);

    // Cones
    for(int i = 0; i < n; i++)
    {
      task.appendcone(conetype.pexp, 0.0, new int[]{z1+i, q1+i, v1+i});
      task.appendcone(conetype.pexp, 0.0, new int[]{z2+i, q2+i, v2+i});
    }
  }

  // Model logistic regression (regularized with full 2-norm of theta)
  // X - n x d matrix of data points
  // y - length n vector classifying training points
  // lamb - regularization parameter
  public static double[] logisticRegression(Env        env,
                                            double[][] X, 
                                            boolean[]  y,
                                            double     lamb)
  {
    int n = X.length;
    int d = X[0].length;       // num samples, dimension

    try (Task task = new Task(env, 0, 0))
    {    
      // Variables [r; theta; t; u]
      int nvar = 1+d+2*n;
      task.appendvars(nvar);
      task.putvarboundsliceconst(0, nvar, boundkey.fr, -inf, inf);
      int r = 0, theta = 1, t = 1+d, u = 1+d+n;
      
      // Constraints: theta'*X +/- u = 0
      task.appendcons(n);
      task.putconboundsliceconst(0, n, boundkey.fx, 0, 0);      
      
      // Objective lambda*r + sum(t)
      task.putcj(r, lamb);
      for(int i = 0; i < n; i++) 
        task.putcj(t+i, 1.0);

      // The X block in theta'*X +/- u = 0
      int[]    subi   = new int[d*n+n];
      int[]    subj   = new int[d*n+n];
      double[] aval   = new double[d*n+n];
      int         k   = 0;
      for(int i = 0; i < n; i++)
      {
        for(int j = 0; j < d; j++)
        {
          subi[k] = i; subj[k] = theta+j; aval[k] = X[i][j]; k++;
        }
        subi[d*n+i] = i; subj[d*n+i] = u+i;
        if (y[i]) aval[d*n+i] = 1; else aval[d*n+i] = -1;
      }
      task.putaijlist(subi, subj, aval);

      // Softplus function constraints
      softplus(task, t, u, n);

      // Regularization
      task.appendconeseq(conetype.quad, 0.0, 1+d, r);

      // Solution
      task.optimize();
      double[] xx = new double[d];
      task.getxxslice(soltype.itr, theta, theta+d, xx);

      return xx;
    }
  }

  public static void main(String[] args)
  {
    Env env = new Env();

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

    double[] theta = logisticRegression(env, X, Y, 0.1);

    for(int i=0;i<6;i++)
      System.out.println(theta[i]);
  }
}