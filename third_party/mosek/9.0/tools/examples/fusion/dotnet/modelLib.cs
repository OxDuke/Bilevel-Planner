/*
  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File :      modelLib.cs

  Purpose: Library of simple building blocks in Mosek Fusion.
*/
using System;

namespace mosek.fusion.example 
{
  public class modelLib 
  {
    // Duplicate variables
    // x = y
    public static void dup(Model M, Variable x, Variable y) {
      M.Constraint(Expr.Sub(x,y), Domain.EqualsTo(0.0));
    }

    // Absolute value
    // t >= |x|, where t, x have the same shape
    public static void abs(Model M, Variable t, Variable x) {
      M.Constraint(Expr.Add(t,x), Domain.GreaterThan(0.0));
      M.Constraint(Expr.Sub(t,x), Domain.GreaterThan(0.0));
    }

    // 1-norm
    // t >= sum( |x_i| ), x is a vector Variable
    public static void norm1(Model M, Variable t, Variable x) {
      Variable u = M.Variable(x.GetShape(), Domain.Unbounded());
      abs(M, u, x);
      M.Constraint(Expr.Sub(t, Expr.Sum(u)), Domain.GreaterThan(0.0));
    }

    // Square
    // t >= x^2
    public static void sq(Model M, Variable t, Variable x) {
      M.Constraint(Expr.Hstack(0.5, t, x), Domain.InRotatedQCone());
    }

    // 2-norm
    // t >= sqrt(x_1^2 + ... + x_n^2) where x is a vector
    public static void norm2(Model M, Variable t, Variable x) {
      M.Constraint(Expr.Vstack(t, x), Domain.InQCone());
    }

    // Power with exponent > 1
    // t >= |x|^p (where p>1)
    public static void pow(Model M, Variable t, Variable x, double p) {
      M.Constraint(Expr.Hstack(t, 1, x), Domain.InPPowerCone(1.0/p));
    }

    // Inverse of power 
    // t >= 1/|x|^p, x>0 (where p>0)
    public static void pow_inv(Model M, Variable t, Variable x, double p) {
      M.Constraint(Expr.Hstack(t, x, 1), Domain.InPPowerCone(1.0/(1.0+p)));
    }

    // p-norm, p>1
    // t >= \|x\|_p (where p>1), x is a vector Variable
    public static void pnorm(Model M, Variable t, Variable x, double p) {
      int n = (int) x.GetSize();
      Variable r = M.Variable(n);
      M.Constraint(Expr.Sub(t, Expr.Sum(r)), Domain.EqualsTo(0.0));
      M.Constraint(Expr.Hstack(Var.Repeat(t,n), r, x), Domain.InPPowerCone(1.0-1.0/p));
    }

    // Geometric mean
    // |t| <= (x_1...x_n)^(1/n), x_i>=0, x is a vector Variable of length >= 1
    public static void geo_mean(Model M, Variable t, Variable x) {
      int n = (int) x.GetSize();
      if (n==1) {
          abs(M, x, t);
      }
      else {
          Variable t2 = M.Variable();
          M.Constraint(Expr.Hstack(t2, x.Index(n-1), t), Domain.InPPowerCone(1.0-1.0/n));
          geo_mean(M, t2, x.Slice(0,n-1));
      }
    }

    // Logarithm
    // t <= log(x), x>=0
    public static void log(Model M, Variable t, Variable x) {
      M.Constraint(Expr.Hstack(x, 1, t), Domain.InPExpCone());
    }

    // Exponential
    // t >= exp(x)
    public static void exp(Model M, Variable t, Variable x) {
      M.Constraint(Expr.Hstack(t, 1, x), Domain.InPExpCone());
    }

    // Entropy
    // t >= x * log(x), x>=0
    public static void ent(Model M, Variable t, Variable x) {
      M.Constraint(Expr.Hstack(1, x, Expr.Neg(t)), Domain.InPExpCone());
    }

    // Relative entropy
    // t >= x * log(x/y), x,y>=0
    public static void relent(Model M, Variable t, Variable x, Variable y) {
      M.Constraint(Expr.Hstack(y, x, Expr.Neg(t)), Domain.InPExpCone());
    }

    // Log-sum-exp
    // log( sum_i(exp(x_i)) ) <= t, where x is a vector
    public static void logsumexp(Model M, Variable t, Variable x) {
      int n = (int) x.GetSize();
      Variable u = M.Variable(n);
      M.Constraint(Expr.Hstack(u, Expr.ConstTerm(n, 1.0), Expr.Sub(x, Var.Repeat(t, n))), Domain.InPExpCone());
      M.Constraint(Expr.Sum(u), Domain.LessThan(1.0));
    }

    // Semicontinuous variable
    // x = 0 or a <= x <= b
    public static void semicontinuous(Model M, Variable x, double a, double b) {
      Variable u = M.Variable(x.GetShape(), Domain.Binary());
      M.Constraint(Expr.Sub(x, Expr.Mul(a, u)), Domain.GreaterThan(0.0));
      M.Constraint(Expr.Sub(x, Expr.Mul(b, u)), Domain.LessThan(0.0));
    }

    // Indicator variable
    // x!=0 implies t=1. Assumes that |x|<=1 in advance.
    public static void indicator(Model M, Variable t, Variable x) {
      M.Constraint(t, Domain.InRange(0,1));
      t.MakeInteger();
      abs(M, t, x);
    }

    // Logical OR
    // x OR y, where x, y are binary
    public static void logic_or(Model M, Variable x, Variable y) {
      M.Constraint(Expr.Add(x, y), Domain.GreaterThan(1.0));
    }
    // x_1 OR ... OR x_n, where x is a binary vector
    public static void logic_or_vect(Model M, Variable x) {
      M.Constraint(Expr.Sum(x), Domain.GreaterThan(1.0));
    }

    // SOS1 (NAND)
    // at most one of x_1,...,x_n, where x is a binary vector (SOS1 Constraint)
    public static void logic_sos1(Model M, Variable x) {
      M.Constraint(Expr.Sum(x), Domain.LessThan(1.0));
    }
    // NOT(x AND y), where x, y are binary
    public static void logic_nand(Model M, Variable x, Variable y) {
      M.Constraint(Expr.Add(x, y), Domain.LessThan(1.0));
    }

    // Cardinality bound
    // At most k of entries in x are nonzero, assuming in advance |x_i|<=1.
    public static void card(Model M, Variable x, int k) {
      Variable t = M.Variable(x.GetShape(), Domain.Binary());
      abs(M, t, x);
      M.Constraint(Expr.Sum(t), Domain.LessThan(k));
    }

    // This is just a syntactic test without much sense
    public static void testModels() {
      Model M = new Model();
      Variable x = M.Variable();
      Variable y = M.Variable();
      Variable t = M.Variable();
      Variable p = M.Variable(5);
      Variable a = M.Variable(new int[]{10,2});
      Variable b = M.Variable(new int[]{10,2});
      Variable e = M.Variable(Domain.Binary());
      Variable f = M.Variable(Domain.Binary());

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
      Variable x = M.Variable();
      Variable y = M.Variable();
      Variable t = M.Variable(3);

      M.Constraint(Expr.Sub(x, y), Domain.GreaterThan(3.0));
      norm2(M, t.Index(0), Var.Vstack(x,y));
      log  (M, t.Index(1), y);
      pow  (M, t.Index(2), x, 1.5);

      M.Objective(ObjectiveSense.Maximize, Expr.Dot(t, new double[]{-1,1,-1}));

      M.SetLogHandler(Console.Out);
      M.Solve();
    }
      
    // Full test
    public static void Main(String[] argv) {
      testModels();
      testExample();
    }
  }
}