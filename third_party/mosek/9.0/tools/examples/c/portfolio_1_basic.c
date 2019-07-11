/*
  File : portfolio_1_basic.c

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Implements a basic portfolio optimization model.
*/
#include <math.h>
#include <stdio.h>
#include "mosek.h"

#define MOSEKCALL(_r,_call)  if ( (_r)==MSK_RES_OK ) (_r) = (_call)

static void MSKAPI printstr(void *handle,
                            const char str[])
{
  printf("%s", str);
} /* printstr */

int main(int argc, const char **argv)
{
  char            buf[128];

  double          expret  = 0.0,
                  stddev  = 0.0,
                  xj;

  const MSKint32t n       = 3;
  const MSKrealt  gamma   = 0.05;
  const MSKrealt  mu[]    = {0.1073,  0.0737,  0.0627};
  const MSKrealt  GT[][3] = {{0.1667,  0.0232,  0.0013},
                             {0.0000,  0.1033, -0.0022},
                             {0.0000,  0.0000,  0.0338}};
  const MSKrealt  x0[3]   = {0.0, 0.0, 0.0};
  const MSKrealt  w       = 1.0;
  MSKrealt        rtemp;
  MSKenv_t        env;
  MSKint32t       k, i, j, offsetx, offsets, offsett, *sub;
  MSKrescodee     res = MSK_RES_OK, trmcode;
  MSKtask_t       task;

  /* Initial setup. */
  env  = NULL;
  task = NULL;
  MOSEKCALL(res, MSK_makeenv(&env, NULL));
  MOSEKCALL(res, MSK_maketask(env, 0, 0, &task));
  MOSEKCALL(res, MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr));

  /* Total budget */
  rtemp = w;
  for (j = 0; j < n; ++j)
    rtemp += x0[j];

  /* Constraints. */
  MOSEKCALL(res, MSK_appendcons(task, 1 + n));

  /* Total budget constraint - set bounds l^c = u^c */
  MOSEKCALL(res, MSK_putconbound(task, 0, MSK_BK_FX, rtemp, rtemp));
  sprintf(buf, "%s", "budget");
  MOSEKCALL(res, MSK_putconname(task, 0, buf));

  /* The remaining constraints GT * x - t = 0 - set bounds l^c = u^c*/
  for (i = 0; i < n; ++i)
  {
    MOSEKCALL(res, MSK_putconbound(task, 1 + i, MSK_BK_FX, 0.0, 0.0));
    sprintf(buf, "GT[%d]", 1 + i);
    MOSEKCALL(res, MSK_putconname(task, 1 + i, buf));
  }

  /* Variables. */
  MOSEKCALL(res, MSK_appendvars(task, 1 + 2 * n));

  offsetx = 0;   /* Offset of variable x into the API variable. */
  offsets = n;   /* Offset of variable x into the API variable. */
  offsett = n + 1; /* Offset of variable t into the API variable. */

  /* x variables. */
  for (j = 0; j < n; ++j)
  {
    /* Return of asset j in the objective */
    MOSEKCALL(res, MSK_putcj(task, offsetx + j, mu[j]));
    /* Coefficients in the first row of A */
    MOSEKCALL(res, MSK_putaij(task, 0, offsetx + j, 1.0));
    /* No short-selling - x^l = 0, x^u = inf */
    MOSEKCALL(res, MSK_putvarbound(task, offsetx + j, MSK_BK_LO, 0.0, MSK_INFINITY));
    sprintf(buf, "x[%d]", 1 + j);
    MOSEKCALL(res, MSK_putvarname(task, offsetx + j, buf));
  }

  /* s variable is a constant equal to gamma. */
  MOSEKCALL(res, MSK_putvarbound(task, offsets + 0, MSK_BK_FX, gamma, gamma));
  sprintf(buf, "s");
  MOSEKCALL(res, MSK_putvarname(task, offsets + 0, buf));

  /* t variables (t = GT*x). */
  for (j = 0; j < n; ++j)
  {
    /* Copying the GT matrix in the appropriate block of A */
    for (k = 0; k < n; ++k)
      if ( GT[k][j] != 0.0 )
        MOSEKCALL(res, MSK_putaij(task, 1 + k, offsetx + j, GT[k][j]));
    /* Diagonal -1 entries in a block of A */
    MOSEKCALL(res, MSK_putaij(task, 1 + j, offsett + j, -1.0));
    /* Free - no bounds */
    MOSEKCALL(res, MSK_putvarbound(task, offsett + j, MSK_BK_FR, -MSK_INFINITY, MSK_INFINITY));
    sprintf(buf, "t[%d]", 1 + j);
    MOSEKCALL(res, MSK_putvarname(task, offsett + j, buf));
  }

  if ( res == MSK_RES_OK )
  {
    /* Define the cone spanned by variables (s, t), i.e. dimension = n + 1 */
    MSKint32t *sub = (MSKint32t *) MSK_calloctask(task, n + 1, sizeof(MSKint32t));

    if ( sub )
    {
      /* Copy indices of variables involved in the conic constraint */
      sub[0] = offsets + 0;
      for (j = 0; j < n; ++j)
        sub[j + 1] = offsett + j;

      MOSEKCALL(res, MSK_appendcone(task, MSK_CT_QUAD, 0.0, n + 1, sub));
      MOSEKCALL(res, MSK_putconename(task, 0, "stddev"));

      MSK_freetask(task, sub);
    }
    else
      res = MSK_RES_ERR_SPACE;
  }

  MOSEKCALL(res, MSK_putobjsense(task, MSK_OBJECTIVE_SENSE_MAXIMIZE));

#if 0
  /* No log output */
  MOSEKCALL(res, MSK_putintparam(task, MSK_IPAR_LOG, 0));
#endif

#if 0
  /* Dump the problem to a human readable OPF file. */
  MOSEKCALL(res, MSK_writedata(task, "dump.opf"));
#endif

  MOSEKCALL(res, MSK_optimizetrm(task, &trmcode));

#if 1
  /* Display the solution summary for quick inspection of results. */
  MSK_solutionsummary(task, MSK_STREAM_MSG);
#endif

  if ( res == MSK_RES_OK )
  {
    expret = 0.0;
    stddev = 0.0;

    /* Read the x variables one by one and compute expected return. */
    /* Can also be obtained as value of the objective. */
    for (j = 0; j < n; ++j)
    {
      MOSEKCALL(res, MSK_getxxslice(task, MSK_SOL_ITR, offsetx + j, offsetx + j + 1, &xj));
      expret += mu[j] * xj;
    }

    /* Read the value of s. This should be gamma. */
    MOSEKCALL(res, MSK_getxxslice(task, MSK_SOL_ITR, offsets + 0, offsets + 1, &stddev));

    printf("\nExpected return %e for gamma %e\n", expret, stddev);
  }

  if ( task != NULL )
    MSK_deletetask(&task);

  if ( env != NULL )
    MSK_deleteenv(&env);

  return ( 0 );
}