/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      lownerjohn_ellipsoid.cs

   Purpose:
   Computes the Lowner-John inner and outer ellipsoidal
   approximations of a polytope.


   The inner ellipsoidal approximation to a polytope

      S = { x \in R^n | Ax < b }.

   maximizes the volume of the inscribed ellipsoid,

      { x | x = C*u + d, || u ||_2 <= 1 }.

   The volume is proportional to det(C)^(1/n), so the
   problem can be solved as

     maximize         t
     subject to       t       <= det(C)^(1/n)
                 || C*ai ||_2 <= bi - ai^T * d,  i=1,...,m
                   C is PSD

   which is equivalent to a mixed conic quadratic and semidefinite
   programming problem.


   The outer ellipsoidal approximation to a polytope given
   as the convex hull of a set of points

       S = conv{ x1, x2, ... , xm }

   minimizes the volume of the enclosing ellipsoid,

     { x | || P*(x-c) ||_2 <= 1 }

   The volume is proportional to det(P)^{-1/n}, so the problem can
   be solved as

     minimize         t
     subject to       t       >= det(P)^(-1/n)
                 || P*xi + c ||_2 <= 1,  i=1,...,m
                   P is PSD.

   References:
   [1] "Lectures on Modern Optimization", Ben-Tal and Nemirovski, 2000.
 */

using System;

namespace mosek.fusion.example
{
  public class lownerjohn_ellipsoid
  {
    /**
        Purpose: Models the convex set

          S = { (x, t) \in R^n x R | x >= 0, t <= (x1 * x2 * ... *xn)^(1/n) }.

        using three-dimensional power cones
    */
    public static void geometric_mean(Model M, Variable x, Variable t)
    {
      int n = (int)x.GetSize();
      if (n==1)
        M.Constraint(Expr.Sub(t, x), Domain.LessThan(0.0));
      else
      {
        Variable t2 = M.Variable();
        M.Constraint(Var.Hstack(t2, x.Index(n-1), t), Domain.InPPowerCone(1-1.0/n));
        geometric_mean(M, x.Slice(0,n-1), t2);
      }
    }

    /**
        Purpose: Models the hypograph of the n-th power of the
        determinant of a positive definite matrix.

        The convex set (a hypograph)

          C = { (X, t) \in S^n_+ x R |  t <= det(X)^{1/n} },

        can be modeled as the intersection of a semidefinite cone

          [ X, Z; Z^T Diag(Z) ] >= 0

        and a number of rotated quadratic cones and affine hyperplanes,

          t <= (Z11*Z22*...*Znn)^{1/n}  (see geometric_mean).

        References:
        [1] "Lectures on Modern Optimization", Ben-Tal and Nemirovski, 2000.
     */
    public static Variable det_rootn(Model M, Variable t, int n)
    {
      // Setup variables
      Variable Y = M.Variable(Domain.InPSDCone(2 * n));

      Variable X   = Y.Slice(new int[]{0, 0}, new int[]{n, n});
      Variable Z   = Y.Slice(new int[]{0, n}, new int[]{n, 2 * n});
      Variable DZ  = Y.Slice(new int[]{n, n}, new int[]{2 * n, 2 * n});

      // Z is lower-triangular
      int[,] low_tri = new int[n*(n-1)/2,2];
      int k = 0;
      for(int i = 0; i < n; i++)
        for(int j = i+1; j < n; j++)
          { low_tri[k,0] = i; low_tri[k,1] = j; ++k; }
      M.Constraint(Z.Pick(low_tri), Domain.EqualsTo(0.0));
      // DZ = Diag(Z)
      M.Constraint(Expr.Sub(DZ, Expr.MulElm(Z, Matrix.Eye(n))), Domain.EqualsTo(0.0));

      // t^n <= (Z11*Z22*...*Znn)
      geometric_mean(M, DZ.Diag(), t);

      // Return an n x n PSD variable which satisfies t <= det(X)^(1/n)
      return X;
    }

    public static Tuple<double[], double[]> lownerjohn_inner(double[][] A, double[] b)
    {
      using( Model M = new Model("lownerjohn_inner"))
      {
        int m = A.Length;
        int n = A[0].Length;

        // Setup variables
        Variable t = M.Variable("t", 1, Domain.GreaterThan(0.0));
        Variable C = det_rootn(M, t, n);
        Variable d = M.Variable("d", n, Domain.Unbounded());

        // (bi - ai^T*d, C*ai) \in Q
        for (int i = 0; i < m; ++i)
          M.Constraint("qc" + i, Expr.Vstack(Expr.Sub(b[i], Expr.Dot(A[i], d)), Expr.Mul(C, A[i])),
                       Domain.InQCone().Axis(0) );

        // Objective: Maximize t
        M.Objective(ObjectiveSense.Maximize, t);
        M.Solve();
        M.WriteTask("LownerJohnInner.ptf");

        return Tuple.Create(C.Level(), d.Level());
      }
    }

    public static Tuple<double[], double[]> lownerjohn_outer(double[][] x)
    {
      using( Model M = new Model("lownerjohn_outer") )
      {
        int m = x.Length;
        int n = x[0].Length;

        // Setup variables
        Variable t = M.Variable("t", 1, Domain.GreaterThan(0.0));
        Variable P = det_rootn(M, t, n);
        Variable c = M.Variable("c", n, Domain.Unbounded());

        // (1, P(*xi+c)) \in Q
        for (int i = 0; i < m; ++i)
          M.Constraint("qc" + i, Expr.Vstack(Expr.Ones(1), Expr.Sub(Expr.Mul(P, x[i]), c)),
                       Domain.InQCone().Axis(0) );

        // Objective: Maximize t
        M.Objective(ObjectiveSense.Maximize, t);
        M.Solve();
        M.WriteTask("LownerJohnOuter.ptf");

        return Tuple.Create(P.Level(), c.Level());
      }
    }

    public static void Main(string[] argv)
    {
      double[][] p = new double[][] { new double[] {0.0, 0.0},
               new double[] {1.0, 3.0},
               new double[] {5.5, 4.5},
               new double[] {7.0, 4.0},
               new double[] {7.0, 1.0},
               new double[] {3.0, -2.0}
      };
      int n = p.Length;
      double[][] A = new double[n][];
      double[]   b = new double[n];
      for (int i = 0; i < n; ++i)
      {
        A[i] = new double[] { -p[i][1] + p[(i - 1 + n) % n][1], p[i][0] - p[(i - 1 + n) % n][0] };
        b[i] = A[i][0] * p[i][0] + A[i][1] * p[i][1];
      }

      Tuple<double[], double[]> ResInn = lownerjohn_inner(A, b);
      Tuple<double[], double[]> ResOut = lownerjohn_outer(p);
      double[] Ci = ResInn.Item1;
      double[] di = ResInn.Item2;
      double[] Po = ResOut.Item1;
      double[] co = ResOut.Item2;
      Console.WriteLine("Inner:");
      Console.WriteLine("  C = [{0}]", string.Join(", ", Ci));
      Console.WriteLine("  d = [{0}]", string.Join(", ", di));
      Console.WriteLine("Outer:");
      Console.WriteLine("  P = [{0}]", string.Join(", ", Po));
      Console.WriteLine("  c = [{0}]", string.Join(", ", co));
    }
  }
}

