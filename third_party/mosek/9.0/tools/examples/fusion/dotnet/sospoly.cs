////
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      sospoly.cs
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
using mosek.fusion;
using System;

namespace mosek.fusion.example
{
  class sospoly
  {
    // Creates a Hankel matrix of dimension n+1, where
    // H_lk = a if l+k=i, and 0 otherwise.
    public static Matrix Hankel(int n, int i, double a)
    {
      if (i < n + 1)
        return Matrix.Sparse(n + 1, n + 1, Range(i, -1, -1), Range(i + 1), a);
      else
        return Matrix.Sparse(n + 1, n + 1, Range(n, i - n - 1, -1), Range(i - n, n + 1), a);
    }

    // Models the cone of nonnegative polynomials on the real axis
    public static void nn_inf(Model M, Variable x)
    {
      int m = (int)x.GetSize() - 1;
      int n = (m / 2); // degree of polynomial is 2*n

      //assert(m == 2*n)

      // Setup variables
      Variable X = M.Variable(Domain.InPSDCone(n + 1));

      // x_i = Tr H(n, i) * X  i=0,...,m
      for (int i = 0; i < m + 1; ++i)
        M.Constraint( Expr.Sub(x.Index(i), Expr.Dot(Hankel(n, i, 1.0), X)), Domain.EqualsTo(0.0));
    }

    // Models the cone of nonnegative polynomials on the semi-infinite interval [0,inf)
    public static void nn_semiinf(Model M, Variable x)
    {
      int n = (int)x.GetSize() - 1;
      int n1 = n / 2,
          n2 = (n - 1) / 2;

      // Setup variables
      Variable X1 = M.Variable(Domain.InPSDCone(n1 + 1));
      Variable X2 = M.Variable(Domain.InPSDCone(n2 + 1));

      // x_i = Tr H(n1, i) * X1 + Tr H(n2,i-1) * X2, i=0,...,n
      for (int i = 0; i < n + 1; ++i)
        M.Constraint( Expr.Sub(x.Index(i),
                               Expr.Add(Expr.Dot(Hankel(n1, i, 1.0),  X1),
                                        Expr.Dot(Hankel(n2, i - 1, 1.0), X2))), Domain.EqualsTo(0.0) );
    }

    // Models the cone of nonnegative polynomials on the finite interval [a,b]
    public static void nn_finite(Model M, Variable x, double a, double b)
    {
      //assert(a < b)
      int m = (int)x.GetSize() - 1;
      int n = m / 2;

      if (m == 2 * n)
      {
        Variable X1 = M.Variable(Domain.InPSDCone(n + 1));
        Variable X2 = M.Variable(Domain.InPSDCone(n));

        // x_i = Tr H(n,i)*X1 + (a+b)*Tr H(n-1,i-1) * X2 - a*b*Tr H(n-1,i)*X2 - Tr H(n-1,i-2)*X2, i=0,...,m
        for (int i = 0; i < m + 1; ++i)
          M.Constraint( Expr.Sub(x.Index(i),
                                 Expr.Add(Expr.Sub(Expr.Dot(Hankel(n, i, 1.0),      X1), Expr.Dot(Hankel(n - 1, i, a * b), X2)),
                                          Expr.Sub(Expr.Dot(Hankel(n - 1, i - 1, a + b), X2), Expr.Dot(Hankel(n - 1, i - 2, 1.0),  X2)))),
                        Domain.EqualsTo(0.0) );
      }
      else
      {
        Variable X1 = M.Variable(Domain.InPSDCone(n + 1));
        Variable X2 = M.Variable(Domain.InPSDCone(n + 1));

        // x_i = Tr H(n,i-1)*X1 - a*Tr H(n,i)*X1 + b*Tr H(n,i)*X2 - Tr H(n,i-1)*X2, i=0,...,m
        for (int i = 0; i < m + 1; ++i)
          M.Constraint( Expr.Sub(x.Index(i),
                                 Expr.Add(Expr.Sub(Expr.Dot(Hankel(n, i - 1, 1.0),  X1), Expr.Dot(Hankel(n, i, a), X1)),
                                          Expr.Sub(Expr.Dot(Hankel(n, i, b),     X2), Expr.Dot(Hankel(n, i - 1, 1.0),  X2)))),
                        Domain.EqualsTo(0.0) );
      }
    }

    // returns variables u representing the derivative of
    //  x(0) + x(1)*t + ... + x(n)*t^n,
    // with u(0) = x(1), u(1) = 2*x(2), ..., u(n-1) = n*x(n).
    public static Variable diff(Model M, Variable x)
    {
      int n = (int)x.GetSize() - 1;
      Variable u = M.Variable(n, Domain.Unbounded());
      M.Constraint(Expr.Sub(u, Expr.MulElm(Matrix.Dense(n, 1, Range(1.0, n + 1)), x.Slice(1, n + 1))), Domain.EqualsTo(0.0));
      return u;
    }

    public static double[] fitpoly(double[,] data, int n)
    {
      using (var M = new Model("smooth poly"))
      {
        int m = data.GetLength(0);
        double[] Adata = new double[m * (n + 1)];
        for (int j = 0, k = 0; j < m; ++j)
          for (int i = 0; i < n + 1; ++i, ++k)
            Adata[k] = Math.Pow(data[j, 0], i);

        Matrix A = Matrix.Dense(m, n + 1, Adata);
        double[] b = new double[m];
        for (int j = 0; j < m; ++j)
          b[j] = data[j, 1];

        Variable x  = M.Variable("x", n + 1, Domain.Unbounded());
        Variable z  = M.Variable("z", 1,   Domain.Unbounded());
        Variable dx = diff(M, x);

        M.Constraint(Expr.Mul(A, x), Domain.EqualsTo(b));

        // z - f'(t) >= 0, for all t \in [a, b]
        Variable ub = M.Variable(n, Domain.Unbounded());
        M.Constraint(Expr.Sub(ub,
                              Expr.Vstack(Expr.Sub(z, dx.Index(0)), Expr.Neg(dx.Slice(1, n)))),
                     Domain.EqualsTo(0.0));
        nn_finite(M, ub, data[0, 0], data[m - 1, 0]);

        // f'(t) + z >= 0, for all t \in [a, b]
        Variable lb = M.Variable(n, Domain.Unbounded());
        M.Constraint(Expr.Sub(lb,
                              Expr.Vstack(Expr.Add(z, dx.Index(0)), dx.Slice(1, n).AsExpr())),
                     Domain.EqualsTo(0.0));
        nn_finite(M, lb, data[0, 0], data[m - 1, 0]);

        M.Objective(ObjectiveSense.Minimize, z);
        M.Solve();
        return x.Level();
      }
    }
    public static void Main(string[] argv)
    {

      double[,] data =
      { { -1.0, 1.0 },
        {  0.0, 0.0 },
        {  1.0, 1.0 }
      };

      double[] x2 = fitpoly(data, 2);
      double[] x4 = fitpoly(data, 4);
      double[] x8 = fitpoly(data, 8);

      Console.WriteLine("fitpoly(data,2) -> {0}", arrtostr(x2));
      Console.WriteLine("fitpoly(data,4) -> {0}", arrtostr(x4));
      Console.WriteLine("fitpoly(data,8) -> {0}", arrtostr(x8));
    }



    // Some utility functions to make things look nicer
    public static int[] Range (int start, int stop, int step)
    {
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
    public static int[] Range (int start, int stop) { return Range(start, stop, 1); }
    public static int[] Range (int stop) { return Range(0, stop, 1); }
    public static double[] Range (double start, double stop, double step)
    {
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
    public static double[] Range (double start, double stop) { return Range(start, stop, 1.0); }
    public static double[] Range (double stop) { return Range(0.0, stop, 1.0); }

    public static double[] Fill (int num, double v) { double[] r = new double[num]; for (int i = 0; i < num; ++i) r[i] = v; return r;}

    private static string arrtostr(double[] a)
    {
      var c = new System.Globalization.CultureInfo("en-US");
      var b = new System.Text.StringBuilder("[");
      if (a.Length > 0) b.Append(String.Format(c, "{0:e3}", a[0]));
      for (int i = 1; i < a.Length; ++i) b.Append(",").Append(String.Format(c, "{0:e3}", a[i]));
      b.Append("]");
      return b.ToString();
    }
  }
}