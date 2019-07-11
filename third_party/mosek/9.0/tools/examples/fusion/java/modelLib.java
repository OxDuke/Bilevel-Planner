/*
  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File :      modelLib.java

  Purpose: Library of simple building blocks in Mosek Fusion.
*/
package com.mosek.fusion.examples;
import mosek.*;
import mosek.fusion.*;

public class modelLib {
  // Duplicate variables
  // x = y
  public static void dup(Model M, Variable x, Variable y) {
    M.constraint(Expr.sub(x,y), Domain.equalsTo(0.0));
  }

  // Absolute value
  // t >= |x|, where t, x have the same shape
  public static void abs(Model M, Variable t, Variable x) {
    M.constraint(Expr.add(t,x), Domain.greaterThan(0.0));
    M.constraint(Expr.sub(t,x), Domain.greaterThan(0.0));
  }

  // 1-norm
  // t >= sum( |x_i| ), x is a vector Variable
  public static void norm1(Model M, Variable t, Variable x) {
    Variable u = M.variable(x.getShape(), Domain.unbounded());
    abs(M, u, x);
    M.constraint(Expr.sub(t, Expr.sum(u)), Domain.greaterThan(0.0));
  }

  // Square
  // t >= x^2
  public static void sq(Model M, Variable t, Variable x) {
    M.constraint(Expr.hstack(0.5, t, x), Domain.inRotatedQCone());
  }

  // 2-norm
  // t >= sqrt(x_1^2 + ... + x_n^2) where x is a vector
  public static void norm2(Model M, Variable t, Variable x) {
    M.constraint(Expr.vstack(t, x), Domain.inQCone());
  }

  // Power with exponent > 1
  // t >= |x|^p (where p>1)
  public static void pow(Model M, Variable t, Variable x, double p) {
    M.constraint(Expr.hstack(t, 1, x), Domain.inPPowerCone(1.0/p));
  }

  // Inverse of power 
  // t >= 1/|x|^p, x>0 (where p>0)
  public static void pow_inv(Model M, Variable t, Variable x, double p) {
    M.constraint(Expr.hstack(t, x, 1), Domain.inPPowerCone(1.0/(1.0+p)));
  }

  // p-norm, p>1
  // t >= \|x\|_p (where p>1), x is a vector Variable
  public static void pnorm(Model M, Variable t, Variable x, double p) {
    int n = (int) x.getSize();
    Variable r = M.variable(n);
    M.constraint(Expr.sub(t, Expr.sum(r)), Domain.equalsTo(0.0));
    M.constraint(Expr.hstack(Var.repeat(t,n), r, x), Domain.inPPowerCone(1.0-1.0/p));
  }

  // Geometric mean
  // |t| <= (x_1...x_n)^(1/n), x_i>=0, x is a vector Variable of length >= 1
  public static void geo_mean(Model M, Variable t, Variable x) {
    int n = (int) x.getSize();
    if (n==1) {
        abs(M, x, t);
    }
    else {
        Variable t2 = M.variable();
        M.constraint(Expr.hstack(t2, x.index(n-1), t), Domain.inPPowerCone(1.0-1.0/n));
        geo_mean(M, t2, x.slice(0,n-1));
    }
  }

  // Logarithm
  // t <= log(x), x>=0
  public static void log(Model M, Variable t, Variable x) {
    M.constraint(Expr.hstack(x, 1, t), Domain.inPExpCone());
  }

  // Exponential
  // t >= exp(x)
  public static void exp(Model M, Variable t, Variable x) {
    M.constraint(Expr.hstack(t, 1, x), Domain.inPExpCone());
  }

  // Entropy
  // t >= x * log(x), x>=0
  public static void ent(Model M, Variable t, Variable x) {
    M.constraint(Expr.hstack(1, x, Expr.neg(t)), Domain.inPExpCone());
  }

  // Relative entropy
  // t >= x * log(x/y), x,y>=0
  public static void relent(Model M, Variable t, Variable x, Variable y) {
    M.constraint(Expr.hstack(y, x, Expr.neg(t)), Domain.inPExpCone());
  }

  // Log-sum-exp
  // log( sum_i(exp(x_i)) ) <= t, where x is a vector
  public static void logsumexp(Model M, Variable t, Variable x) {
    int n = (int) x.getSize();
    Variable u = M.variable(n);
    M.constraint(Expr.hstack(u, Expr.constTerm(n, 1.0), Expr.sub(x, Var.repeat(t, n))), Domain.inPExpCone());
    M.constraint(Expr.sum(u), Domain.lessThan(1.0));
  }

  // Semicontinuous variable
  // x = 0 or a <= x <= b
  public static void semicontinuous(Model M, Variable x, double a, double b) {
    Variable u = M.variable(x.getShape(), Domain.binary());
    M.constraint(Expr.sub(x, Expr.mul(a, u)), Domain.greaterThan(0.0));
    M.constraint(Expr.sub(x, Expr.mul(b, u)), Domain.lessThan(0.0));
  }

  // Indicator variable
  // x!=0 implies t=1. Assumes that |x|<=1 in advance.
  public static void indicator(Model M, Variable t, Variable x) {
    M.constraint(t, Domain.inRange(0,1));
    t.makeInteger();
    abs(M, t, x);
  }

  // Logical OR
  // x OR y, where x, y are binary
  public static void logic_or(Model M, Variable x, Variable y) {
    M.constraint(Expr.add(x, y), Domain.greaterThan(1.0));
  }
  // x_1 OR ... OR x_n, where x is a binary vector
  public static void logic_or_vect(Model M, Variable x) {
    M.constraint(Expr.sum(x), Domain.greaterThan(1.0));
  }

  // SOS1 (NAND)
  // at most one of x_1,...,x_n, where x is a binary vector (SOS1 constraint)
  public static void logic_sos1(Model M, Variable x) {
    M.constraint(Expr.sum(x), Domain.lessThan(1.0));
  }
  // NOT(x AND y), where x, y are binary
  public static void logic_nand(Model M, Variable x, Variable y) {
    M.constraint(Expr.add(x, y), Domain.lessThan(1.0));
  }

  // Cardinality bound
  // At most k of entries in x are nonzero, assuming in advance |x_i|<=1.
  public static void card(Model M, Variable x, int k) {
    Variable t = M.variable(x.getShape(), Domain.binary());
    abs(M, t, x);
    M.constraint(Expr.sum(t), Domain.lessThan(k));
  }

  // This is just a syntactic test without much sense
  public static void testModels() {
    Model M = new Model();
    Variable x = M.variable();
    Variable y = M.variable();
    Variable t = M.variable();
    Variable p = M.variable(5);
    Variable a = M.variable(new int[]{10,2});
    Variable b = M.variable(new int[]{10,2});
    Variable e = M.variable(Domain.binary());
    Variable f = M.variable(Domain.binary());

    log(M, t, x);
    exp(M, t, x);
    ent(M, t, x);
    relent(M, t, x, y);
    logsumexp(M, t, p);
    abs(M, a, b);
    norm1(M, t, a);
    sq(M, t, x);
    norm2(M, t, p);
    pow(M, t, x, 1.5);
    pow_inv(M, t, x, 3.3);
    geo_mean(M, t, p);
    semicontinuous(M, y, 1.1, 2.2);
    indicator(M, e, y);
    logic_or(M, e, f);
    logic_nand(M, e, f);
    card(M, b, 5);
  }

  // A sample problem using functions from the library
  //
  // max -sqrt(x^2 + y^2) + log(y) - x^1.5
  //  st x >= y + 3
  //
  public static void testExample() {
    Model M = new Model();
    Variable x = M.variable();
    Variable y = M.variable();
    Variable t = M.variable(3);

    M.constraint(Expr.sub(x, y), Domain.greaterThan(3.0));
    norm2(M, t.index(0), Var.vstack(x,y));
    log  (M, t.index(1), y);
    pow  (M, t.index(2), x, 1.5);

    M.objective(ObjectiveSense.Maximize, Expr.dot(t, new double[]{-1,1,-1}));

    M.setLogHandler(new java.io.PrintWriter(System.out));
    M.solve();
  }
    
  // Full test
  public static void main(String[] argv) {
    testModels();
    testExample();
  }
}