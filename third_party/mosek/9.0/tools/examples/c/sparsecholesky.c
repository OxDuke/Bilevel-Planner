/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      sparsecholesky.c

   Purpose: Demonstrate the sparse Cholesky factorization.
 */

#include <stdio.h>
#include "mosek.h"

/* Prints out a Cholesky factor presented in sparse form */
static void printsparse(MSKint32t       n,
                        MSKint32t       *perm,
                        MSKrealt        *diag,
                        MSKint32t       *lnzc,
                        MSKint64t       *lptrc,
                        MSKint64t       lensubnval,
                        MSKint32t       *lsubc,
                        MSKrealt        *lvalc)
{
  int i, j;
  MSKrealt*l;
  printf("P = [ ");
  for (i = 0; i < n; i++) printf("%d ", perm[i]);
  printf("]\ndiag(D) = [ ");
  for (i = 0; i < n; i++) printf("%.2e ", diag[i]);
  printf("]\nL=\n");
  l = (MSKrealt*) calloc(n * n, sizeof(MSKrealt));
  for (j = 0; j < n; j++)
    for (i = lptrc[j]; i < lptrc[j] + lnzc[j]; i++)
      l[lsubc[i]*n + j] = lvalc[i];
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) printf("%.2e ", l[n * i + j]);
    printf("\n");
  }
  free(l);
}


int main(int argc, const char *argv[])
{
  MSKenv_t    env;
  MSKrescodee r;

  /* The "" makes leak detection possible at a small cost.
     Otherwise use NULL instead of "". */
  r = MSK_makeenv(&env, "");

  if (r == MSK_RES_OK)
  {
    const MSKint32t n       = 4;      // Data from the example in the text
    //Observe that anzc, aptrc, asubc and avalc only specify the lower triangular part.
    const MSKint32t anzc[]  = {4, 1, 1, 1},
                    asubc[] = {0, 1, 2, 3, 1, 2, 3};
    const MSKint64t aptrc[] = {0, 4, 5, 6};
    const MSKrealt  avalc[] = {4.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                    b[]     = {13.0, 3.0, 4.0, 5.0};
    MSKint32t       *perm = NULL, *lnzc = NULL, *lsubc = NULL;
    MSKint64t       lensubnval, *lptrc = NULL;
    MSKrealt        *diag = NULL, *lvalc = NULL;

    printf("\nExample with positive definite A.\n");
    r = MSK_computesparsecholesky(env,
                                  0,         /* Disable multithreading */
                                  1,         /* Apply a reordering heuristic */
                                  1.0e-14,   /* Singularity tolerance */
                                  n, anzc, aptrc, asubc, avalc,
                                  &perm, &diag, &lnzc, &lptrc, &lensubnval, &lsubc, &lvalc);

    if (r == MSK_RES_OK)
    {
      MSKint32t i, j;
      MSKrealt  *x;
      printsparse(n, perm, diag, lnzc, lptrc, lensubnval, lsubc, lvalc);

      x = MSK_callocenv(env, n, sizeof(MSKrealt));
      if (x)
      {
        /* Permuted b is stored as x. */
        for (i = 0; i < n; ++i) x[i] = b[perm[i]];

        /* Compute inv(L)*x. */
        r = MSK_sparsetriangularsolvedense(env, MSK_TRANSPOSE_NO, n,
                                           lnzc, lptrc, lensubnval, lsubc, lvalc, x);

        if (r == MSK_RES_OK) {
          /* Compute inv(L^T)*x. */
          r = MSK_sparsetriangularsolvedense(env, MSK_TRANSPOSE_YES, n,
                                             lnzc, lptrc, lensubnval, lsubc, lvalc, x);
          printf("\nSolution A x = b, x = [ ");
          for (i = 0; i < n; i++)
            for (j = 0; j < n; j++) if (perm[j] == i) printf("%.2f ", x[j]);
          printf("]\n");
        }

        MSK_freeenv(env, x);
      }
      else
        printf("Out of space while creating x.\n");
    }
    else
      printf("Cholesky computation failed: %d\n", (int) r);

    MSK_freeenv(env, perm);
    MSK_freeenv(env, lnzc);
    MSK_freeenv(env, lsubc);
    MSK_freeenv(env, lptrc);
    MSK_freeenv(env, diag);
    MSK_freeenv(env, lvalc);
  }

  if (r == MSK_RES_OK)
  {
    const MSKint32t n       = 3;
    const MSKint32t anzc[]  = {3, 2, 1},
                    asubc[] = {0, 1, 2, 1, 2, 2};
    const MSKint64t aptrc[] = {0, 3, 5};
    const MSKrealt  avalc[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    MSKint32t       *perm = NULL, *lnzc = NULL, *lsubc = NULL;
    MSKint64t       lensubnval, *lptrc = NULL;
    MSKrealt        *diag = NULL, *lvalc = NULL;
    /* Let A be

           [1.0 1.0 1.0]
           [1.0 1.0 1.0]
           [1.0 1.0 1.0]

       then compute a sparse Cholesky factorization A. Observe A is NOT
       positive definite.

    */

    printf("\nExample with a semidefinite A.\n");
    r = MSK_computesparsecholesky(env, 0, 1, 1.0e-14,
                                  n, anzc, aptrc, asubc, avalc,
                                  &perm, &diag, &lnzc, &lptrc, &lensubnval, &lsubc, &lvalc);
    if (r == MSK_RES_OK)
      printsparse(n, perm, diag, lnzc, lptrc, lensubnval, lsubc, lvalc);

    MSK_freeenv(env, perm);
    MSK_freeenv(env, diag);
    MSK_freeenv(env, lnzc);
    MSK_freeenv(env, lptrc);
    MSK_freeenv(env, lsubc);
    MSK_freeenv(env, lvalc);
  }

  if (r == MSK_RES_OK)
    r = MSK_deleteenv(&env); /* Catch leaks */
  else
    MSK_deleteenv(&env);

  return (r);
} /* main */