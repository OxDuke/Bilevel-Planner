////
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      sospoly.java
//
//  Purpose:
//  Models the cone of nonnegative polynomials and nonnegative trigonometric
//  polynomials using Nesterov's framework  [1].
//
//  Given a set of coefficients (x0, x1, ..., xn), the functions model the
//  cone of nonnegative polynomials
//
//  P_m = { x \in R^{n+1} | x0 + x1*t + ... xn*t^n >= 0, forall t \in I }
//
//  where I can be the entire real axis, the semi-infinite interval [0,inf), or
//  a finite interval I = [a, b], respectively.
//
//  References:
//
//  [1] "Squared Functional Systems and Optimization Problems",
//      Y. Nesterov, in High Performance Optimization,
//      Kluwer Academic Publishers, 2000.
package com.mosek.fusion.examples;

import mosek.fusion.*;

class sospoly {
  // Creates a Hankel matrix of dimension n+1, where
  // H_lk = a if l+k=i, and 0 otherwise.
  public static Matrix Hankel(int n, int i, double a) {
    if (i < n + 1)
      return Matrix.sparse(n + 1, n + 1, range(i, -1, -1), range(i + 1), fill(i + 1, a));
    else
      return Matrix.sparse(n + 1, n + 1, range(n, i - n - 1, -1), range(i - n, n + 1), fill(2 * n + 1 - i, a));
  }

  // Models the cone of nonnegative polynomials on the real axis
  public static void nn_inf(Model M, Variable x) {
    int m = (int)x.getSize() - 1;
    int n = (m / 2); // degree of polynomial is 2*n

    //assert(m == 2*n)

    // Setup variables
    Variable X = M.variable(Domain.inPSDCone(n + 1));

    // x_i = Tr H(n, i) * X  i=0,...,m
    for (int i = 0; i < m + 1; ++i)
      M.constraint( Expr.sub(x.index(i), Expr.dot(Hankel(n, i, 1.0), X)), Domain.equalsTo(0.0));
  }

  // Models the cone of nonnegative polynomials on the semi-infinite interval [0,inf)
  public static void nn_semiinf(Model M, Variable x) {
    int n = (int)x.getSize() - 1;
    int n1 = n / 2,
        n2 = (n - 1) / 2;

    // Setup variables
    Variable X1 = M.variable(Domain.inPSDCone(n1 + 1));
    Variable X2 = M.variable(Domain.inPSDCone(n2 + 1));

    // x_i = Tr H(n1, i) * X1 + Tr H(n2,i-1) * X2, i=0,...,n
    for (int i = 0; i < n + 1; ++i)
      M.constraint( Expr.sub(x.index(i),
                             Expr.add(Expr.dot(Hankel(n1, i, 1.0),  X1),
                                      Expr.dot(Hankel(n2, i - 1, 1.0), X2))), Domain.equalsTo(0.0) );
  }

  // Models the cone of nonnegative polynomials on the finite interval [a,b]
  public static void nn_finite(Model M, Variable x, double a, double b) {
    //assert(a < b)
    int m = (int)x.getSize() - 1;
    int n = m / 2;

    if (m == 2 * n) {
      Variable X1 = M.variable(Domain.inPSDCone(n + 1));
      Variable X2 = M.variable(Domain.inPSDCone(n));

      // x_i = Tr H(n,i)*X1 + (a+b)*Tr H(n-1,i-1) * X2 - a*b*Tr H(n-1,i)*X2 - Tr H(n-1,i-2)*X2, i=0,...,m
      for (int i = 0; i < m + 1; ++i)
        M.constraint( Expr.sub(x.index(i),
                               Expr.add(Expr.sub(Expr.dot(Hankel(n, i, 1.0),  X1), Expr.dot(Hankel(n - 1, i, a * b), X2)),
                                        Expr.sub(Expr.dot(Hankel(n - 1, i - 1, a + b), X2), Expr.dot(Hankel(n - 1, i - 2, 1.0),  X2)))),
                      Domain.equalsTo(0.0) );
    } else {
      Variable X1 = M.variable(Domain.inPSDCone(n + 1));
      Variable X2 = M.variable(Domain.inPSDCone(n + 1));

      // x_i = Tr H(n,i-1)*X1 - a*Tr H(n,i)*X1 + b*Tr H(n,i)*X2 - Tr H(n,i-1)*X2, i=0,...,m
      for (int i = 0; i < m + 1; ++i)
        M.constraint( Expr.sub(x.index(i),
                               Expr.add(Expr.sub(Expr.dot(Hankel(n, i - 1, 1.0),  X1), Expr.dot(Hankel(n, i, a), X1)),
                                        Expr.sub(Expr.dot(Hankel(n, i, b), X2), Expr.dot(Hankel(n, i - 1, 1.0),  X2)))),
                      Domain.equalsTo(0.0) );
    }
  }

  // returns variables u representing the derivative of
  //  x(0) + x(1)*t + ... + x(n)*t^n,
  // with u(0) = x(1), u(1) = 2*x(2), ..., u(n-1) = n*x(n).
  public static Variable diff(Model M, Variable x) {
    int n = (int)x.getSize() - 1;
    Variable u = M.variable(n, Domain.unbounded());
    M.constraint(Expr.sub(u, Expr.mulElm(Matrix.dense(n, 1, range(1.0, n + 1)), x.slice(1, n + 1))), Domain.equalsTo(0.0));
    return u;
  }

  public static double[] fitpoly(double[][] data, int n)
  throws SolutionError {
    Model M = new Model("smooth poly");

    try {
      int m = data.length;
      double[] Adata = new double[m * (n + 1)];
      for (int j = 0, k = 0; j < m; ++j)
        for (int i = 0; i < n + 1; ++i, ++k)
          Adata[k] = Math.pow(data[j][0], i);
      Matrix A = Matrix.dense(m, n + 1, Adata);
      double[] b = new double[m];
      for (int j = 0; j < m; ++j)
        b[j] = data[j][1];

      Variable x  = M.variable("x", n + 1, Domain.unbounded());
      Variable z  = M.variable("z", 1, Domain.unbounded());
      Variable dx = diff(M, x);

      M.constraint(Expr.mul(A, x), Domain.equalsTo(b));

      // z - f'(t) >= 0, for all t \in [a, b]
      Variable ub = M.variable(n, Domain.unbounded());
      M.constraint(Expr.sub(ub,
                            Expr.vstack(Expr.sub(z, dx.index(0)), Expr.neg(dx.slice(1, n)))),
                   Domain.equalsTo(0.0));
      nn_finite(M, ub, data[0][0], data[data.length - 1][0]);

      // f'(t) + z >= 0, for all t \in [a, b]
      Variable lb = M.variable(n, Domain.unbounded());
      M.constraint(Expr.sub(lb,
                            Expr.vstack(Expr.add(z, dx.index(0)), dx.slice(1, n))),
                   Domain.equalsTo(0.0));
      nn_finite(M, lb, data[0][0], data[data.length - 1][0]);

      M.objective(ObjectiveSense.Minimize, z);
      M.solve();
      return x.level();
    } finally {
      M.dispose();
    }
  }
  public static void main(String[] argv)
  throws SolutionError {

    double[][] data = {
      { -1.0, 1.0 },
      {  0.0, 0.0 },
      {  1.0, 1.0 }
    };

    double[] x2 = fitpoly(data, 2);
    double[] x4 = fitpoly(data, 4);
    double[] x8 = fitpoly(data, 8);

    System.out.println("fitpoly(data,2) -> " + arrtostr(x2));
    System.out.println("fitpoly(data,4) -> " + arrtostr(x4));
    System.out.println("fitpoly(data,8) -> " + arrtostr(x8));
  }



  // Some utility functions to make things look nicer
  public static int[] range (int start, int stop, int step) {
    int len;
    if (start < stop && step > 0)
      len = 1 + (stop - start - 1) / step;
    else if (stop < start && step < 0)
      len = 1 + (start - stop - 1) / (- step);
    else
      len = 0;
    int[] res = new int[len];
    for (int i = 0, v = start; i < len; ++i, v += step)
      res[i] = v;
    return res;
  }
  public static int[] range (int start, int stop) { return range(start, stop, 1); }
  public static int[] range (int stop) { return range(0, stop, 1); }
  public static double[] range (double start, double stop, double step) {
    int len;
    if (start < stop && step > 0.0)
      len = 1 + (int)((stop - start - 1) / step);
    else if (stop < start && step < 0)
      len = 1 + (int)((start - stop - 1) / (- step));
    else
      len = 0;
    double[] res = new double[len];
    double v = start;
    for (int i = 0; i < len; ++i, v += step)
      res[i] = v;
    return res;
  }
  public static double[] range (double start, double stop) { return range(start, stop, 1.0); }
  public static double[] range (double stop) { return range(0.0, stop, 1.0); }

  public static double[] fill (int num, double v) { double[] r = new double[num]; for (int i = 0; i < num; ++i) r[i] = v; return r;}

  private static String arrtostr(double[] a) {
    StringBuilder b = new StringBuilder("[");
    java.util.Formatter f = new java.util.Formatter(b, java.util.Locale.US);
    if (a.length > 0) f.format(", %.3e", a[0]);
    for (int i = 1; i < a.length; ++i) f.format(", %.3e", a[i]);
    b.append("]");
    return b.toString();
  }
}