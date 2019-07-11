/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      solvebasis.c

  Purpose:   To demonstrate the usage of
             MSK_solvewithbasis on the problem:

             maximize  x0 + x1
             st.
                     x0 + 2.0 x1 <= 2
                     x0  +    x1 <= 6
                     x0 >= 0, x1>= 0

               The problem has the slack variables
               xc0, xc1 on the constraints
               and the variables x0 and x1.

               maximize  x0 + x1
               st.
                  x0 + 2.0 x1 -xc1       = 2
                  x0  +    x1       -xc2 = 6
                  x0 >= 0, x1>= 0,
                  xc1 <=  0 , xc2 <= 0

  Syntax:    solvebasis

 */
#include "mosek.h"

static void MSKAPI printstr(void *handle,
                            const char str[])
{
  printf("%s", str);
} /* printstr */

int main(int argc, const char *argv[])
{
  MSKenv_t     env;
  MSKtask_t    task;
  MSKint32t    numcon = 2, numvar = 2;
  double       c[]    = {1.0, 1.0};
  MSKint32t    ptrb[] = {0, 2},
                        ptre[] = {2, 3};
  MSKint32t    asub[] = {0, 1, 0, 1};
  double       aval[] = {1.0, 1.0, 2.0, 1.0};
  MSKboundkeye bkc[]  = { MSK_BK_UP, MSK_BK_UP };
  double       blc[]  = { -MSK_INFINITY, -MSK_INFINITY };
  double       buc[]  = {2.0, 6.0};

  MSKboundkeye bkx[]  = { MSK_BK_LO, MSK_BK_LO };
  double       blx[]  = {0.0, 0.0};
  double       bux[]  = { +MSK_INFINITY, +MSK_INFINITY};
  MSKrescodee  r      = MSK_RES_OK;
  MSKint32t    i, nz;
  double       w[]    = {2.0, 6.0};
  MSKint32t    sub[]  = {0, 1};
  MSKint32t    *basis;

  if (r == MSK_RES_OK)
    r = MSK_makeenv(&env, NULL);

  if (r == MSK_RES_OK)
    r = MSK_makeemptytask(env, &task);

  if (r == MSK_RES_OK)
    MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

  if (r == MSK_RES_OK)
    r = MSK_inputdata(task, numcon, numvar, numcon, numvar,
                      c, 0.0,
                      ptrb, ptre, asub, aval, bkc, blc, buc, bkx, blx, bux);

  if (r == MSK_RES_OK)
    r = MSK_putobjsense(task, MSK_OBJECTIVE_SENSE_MAXIMIZE);

  if (r == MSK_RES_OK)
    r = MSK_optimizetrm(task, NULL);

  if (r == MSK_RES_OK)
    basis = MSK_calloctask(task, numcon, sizeof(MSKint32t));

  if (r == MSK_RES_OK)
    r = MSK_initbasissolve(task, basis);

  /* List basis variables corresponding to columns of B */
  for (i = 0; i < numcon && r == MSK_RES_OK; ++i)
  {
    printf("basis[%d] = %d\n", i, basis[i]);
    if (basis[sub[i]] < numcon)
      printf("Basis variable no %d is xc%d.\n", i, basis[i]);
    else
      printf("Basis variable no %d is x%d.\n", i, basis[i] - numcon);
  }

  nz = 2;
  /* solve Bx = w */
  /* sub contains index of non-zeros in w.
     On return w contains the solution x and sub
     the index of the non-zeros in x.
   */
  if (r == MSK_RES_OK)
    r = MSK_solvewithbasis(task, 0, &nz, sub, w);

  if (r == MSK_RES_OK)
  {
    printf("\nSolution to Bx = w:\n\n");

    /* Print solution and b. */

    for (i = 0; i < nz; ++i)
    {
      if (basis[sub[i]] < numcon)
        printf("xc%d = %e\n", basis[sub[i]] , w[sub[i]]);
      else
        printf("x%d = %e\n", basis[sub[i]] - numcon , w[sub[i]]);
    }
  }

  /* Solve B^T y = w */
  nz     = 1;     /* Only one element in sub is nonzero. */
  sub[0] = 1;     /* Only w[1] is nonzero. */
  w[0]   = 0.0;
  w[1]   = 1.0;

  if (r == MSK_RES_OK)
    r = MSK_solvewithbasis(task, 1, &nz, sub, w);

  if (r == MSK_RES_OK)
  {
    printf("\nSolution to B^T y = w:\n\n");
    /* Print solution and y. */
    for (i = 0; i < nz; ++i)
      printf("y%d = %e\n", sub[i], w[sub[i]]);
  }

  return (r);
}/* main */