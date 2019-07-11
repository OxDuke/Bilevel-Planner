/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      blas_lapack.c

   Purpose: To demonstrate how to call BLAS/LAPACK routines for
            which MOSEK provides simplified interfaces.

 */
#include "mosek.h"
void print_matrix(MSKrealt* x, MSKint32t r, MSKint32t c)
{
  MSKint32t i, j;
  for (i = 0; i < r; i++)
  {
    for (j = 0; j < c; j++)
      printf("%f ", x[j * r + i]);

    printf("\n");
  }

}

int main(int argc, char*  argv[])
{

  MSKrescodee r   = MSK_RES_OK;
  MSKenv_t    env = NULL;

  const MSKint32t n = 3, m = 2, k = 3;

  MSKrealt alpha = 2.0, beta = 0.5;
  MSKrealt x[]    = {1.0, 1.0, 1.0};
  MSKrealt y[]    = {1.0, 2.0, 3.0};
  MSKrealt z[]    = {1.0, 1.0};
  MSKrealt A[]    = {1.0, 1.0, 2.0, 2.0, 3.0, 3.0};
  MSKrealt B[]    = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
  MSKrealt C[]    = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  MSKrealt D[]    = {1.0, 1.0, 1.0, 1.0};
  MSKrealt Q[]    = {1.0, 0.0, 0.0, 2.0};
  MSKrealt v[]    = {0.0, 0.0, 0.0};

  MSKrealt xy;

  /* BLAS routines*/
  r = MSK_makeenv(&env, NULL);
  printf("n=%d m=%d k=%d\n", m, n, k);
  printf("alpha=%f\n", alpha);
  printf("beta=%f\n", beta);

  r = MSK_dot(env, n, x, y, &xy);
  printf("dot results= %f r=%d\n", xy, r);


  print_matrix(x, 1, n);
  print_matrix(y, 1, n);

  r = MSK_axpy(env, n, alpha, x, y);
  puts("axpy results is");
  print_matrix(y, 1, n);


  r = MSK_gemv(env, MSK_TRANSPOSE_NO, m, n, alpha, A, x, beta, z);
  printf("gemv results is (r=%d) \n", r);
  print_matrix(z, 1, m);

  r = MSK_gemm(env, MSK_TRANSPOSE_NO, MSK_TRANSPOSE_NO, m, n, k, alpha, A, B, beta, C);
  printf("gemm results is (r=%d) \n", r);
  print_matrix(C, m, n);

  r = MSK_syrk(env, MSK_UPLO_LO, MSK_TRANSPOSE_NO, m, k, 1., A, beta, D);
  printf("syrk results is (r=%d) \n", r);
  print_matrix(D, m, m);

  /* LAPACK routines*/

  r = MSK_potrf(env, MSK_UPLO_LO, m, Q);
  printf("potrf results is (r=%d) \n", r);
  print_matrix(Q, m, m);

  r = MSK_syeig(env, MSK_UPLO_LO, m, Q, v);
  printf("syeig results is (r=%d) \n", r);
  print_matrix(v, 1, m);

  r = MSK_syevd(env, MSK_UPLO_LO, m, Q, v);
  printf("syevd results is (r=%d) \n", r);
  print_matrix(v, 1, m);
  print_matrix(Q, m, m);

  /* Delete the environment and the associated data. */
  MSK_deleteenv(&env);

  return r;
}