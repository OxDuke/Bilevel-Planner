//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      gp1.java
//
//   Purpose:   Demonstrates how to solve a simple Geometric Program (GP)
//              cast into conic form with exponential cones and log-sum-exp.
//
//              Example from
//                https://gpkit.readthedocs.io/en/latest/examples.html//maximizing-the-volume-of-a-box
//
package com.mosek.fusion.examples;
import mosek.fusion.*;
import java.lang.Math;

public class gp1 {
  // Models log(sum(exp(Ax+b))) <= 0.
  // Each row of [A b] describes one of the exp-terms
  public static void logsumexp(Model      M, 
                               double[][] A, 
                               Variable   x,
                               double[]   b)
  {
    int k = A.length;
    Variable u = M.variable(k);
    M.constraint(Expr.sum(u), Domain.equalsTo(1.0));
    M.constraint(Expr.hstack(u,
                             Expr.constTerm(k, 1.0),
                             Expr.add(Expr.mul(A, x), b)), Domain.inPExpCone());
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
  throws SolutionError
  {
    Model M = new Model("max_vol_box");
    try {
      Variable xyz = M.variable(3);
      M.objective("Objective", ObjectiveSense.Maximize, Expr.sum(xyz));
      
      logsumexp(M, 
                new double[][] {{1,1,0}, {1,0,1}}, 
                xyz, 
                new double[] {Math.log(2.0/Aw), Math.log(2.0/Aw)});
      
      M.constraint(Expr.dot(new double[] {0,1,1}, xyz), Domain.lessThan(Math.log(Af)));
      M.constraint(Expr.dot(new double[] {1,-1,0}, xyz), Domain.inRange(Math.log(alpha),Math.log(beta)));
      M.constraint(Expr.dot(new double[] {0,-1,1}, xyz), Domain.inRange(Math.log(gamma),Math.log(delta)));
      
      M.setLogHandler(new java.io.PrintWriter(System.out));
      M.solve();
      
      double[] xyzVal = xyz.level();
      double[] hwdVal = new double[3];
      for(int i=0; i<3; i++) hwdVal[i] = Math.exp(xyzVal[i]);

      return hwdVal;
    } finally {
      M.dispose();
    }
  }

  public static void main(String[] args)
  throws SolutionError
  {
    double Aw    = 200.0;
    double Af    = 50.0;
    double alpha = 2.0;
    double beta  = 10.0;
    double gamma = 2.0;
    double delta = 10.0;
    
    double[] hwd = max_volume_box(Aw, Af, alpha, beta, gamma, delta);

    System.out.format("h=%.4f w=%.4f d=%.4f\n", hwd[0], hwd[1], hwd[2]);
  }
}