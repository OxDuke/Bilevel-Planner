/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      sparsecholesky.cs

   Purpose:   Demonstrate the sparse Cholesky factorization.

 */

using System;

namespace mosek.example
{
  public class Sparsecholesky
  {

    public static void printsparse(int      n,
                                   int[]    perm,
                                   double[] diag,
                                   int[]    lnzc,
                                   long[]   lptrc,
                                   long     lensubnval,
                                   int[]    lsubc,
                                   double[] lvalc)
    {
      long i, j;
      Console.Write("P = [ ");
      for (i = 0; i < n; i++) Console.Write("{0} ", perm[i]);
      Console.WriteLine("]");
      Console.Write("diag(D) = [ ");
      for (i = 0; i < n; i++) Console.Write("{0} ", diag[i]);
      Console.WriteLine("]");
      double[] l = new double[n * n];
      for (j = 0; j < n; j++)
        for (i = lptrc[j]; i < lptrc[j] + lnzc[j]; i++)
          l[lsubc[i]*n + j] = lvalc[i];
      Console.WriteLine("L=");
      for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) Console.Write("{0} ", l[i * n + j]);
        Console.WriteLine("");
      }
    }


    public static void Main ()
    {
      /* Create the mosek environment. */
      using (mosek.Env env = new mosek.Env())
      {
        {
          //Example from the manual
          //Observe that anzc, aptrc, asubc and avalc only specify the lower triangular part.
          const int n           = 4;
          int[] anzc            = {4, 1, 1, 1};
          int[] asubc           = {0, 1, 2, 3, 1, 2, 3};
          long[] aptrc          = {0, 4, 5, 6};
          double[] avalc        = {4.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
          double[] b            = {13.0, 3.0, 4.0, 5.0};

          int[] perm, lnzc, lsubc;
          long lensubnval;
          double[] diag, lvalc;
          long[] lptrc;

          env.computesparsecholesky(0,        //Disable multithreading
                                    1,        //Apply reordering heuristic
                                    1.0e-14,  //Singularity tolerance
                                    anzc, aptrc, asubc, avalc,
                                    out perm, out diag,
                                    out lnzc, out lptrc, out lensubnval, out lsubc, out lvalc);

          printsparse(n, perm, diag, lnzc, lptrc, lensubnval, lsubc, lvalc);

          /* Permuted b is stored as x. */
          double[] x = new double[n];
          for (int i = 0; i < n; i++) x[i] = b[perm[i]];

          /*Compute  inv(L)*x.*/
          env.sparsetriangularsolvedense(mosek.transpose.no, lnzc, lptrc, lsubc, lvalc, x);
          /*Compute  inv(L^T)*x.*/
          env.sparsetriangularsolvedense(mosek.transpose.yes, lnzc, lptrc, lsubc, lvalc, x);

          Console.Write("\nSolution A x = b, x = [ ");
          for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) if (perm[j] == i) Console.Write("{0} ", x[j]);
          Console.WriteLine("]\n");
        }
        {
          const int n           = 3;
          int[] anzc            = {3, 2, 1};
          int[] asubc           = {0, 1, 2, 1, 2, 2};
          long[] aptrc          = {0, 3, 5, };
          double[] avalc        = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

          int[] perm, lnzc, lsubc;
          long lensubnval;
          double[] diag, lvalc;
          long[] lptrc;

          env.computesparsecholesky(0,        //Disable multithreading
                                    1,        //Apply reordering heuristic
                                    1.0e-14,  //Singularity tolerance
                                    anzc, aptrc, asubc, avalc,
                                    out perm, out diag,
                                    out lnzc, out lptrc, out lensubnval, out lsubc, out lvalc);

          printsparse(n, perm, diag, lnzc, lptrc, lensubnval, lsubc, lvalc);
        }
      }
    }
  }
}