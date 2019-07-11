/*
  File : portfolio_4_transcost.c

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Implements a basic portfolio optimization model
                 with fixed setup costs and transaction costs
                 as a mixed-integer problem.
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
  const MSKint32t n       = 3;
  const double    w       = 1.0,
                  x0[]    = {0.0, 0.0, 0.0},
                  gamma   = 0.05,
                  mu[]    = {0.1073,  0.0737,  0.0627},
                  f[]     = {0.01, 0.01, 0.01},
                  g[]     = {0.001, 0.001, 0.001},
                  GT[][3] = {{0.1667,  0.0232,  0.0013},
                             {0.0000,  0.1033, -0.0022},
                             {0.0000,  0.0000,  0.0338}
                            };
  double          U,
                  expret,
                  stddev,
                  xj;
  MSKenv_t        env;
  MSKint32t       k, i, j,
                  offsetx, offsets, offsett, offsetz, offsety;
  MSKrescodee     res = MSK_RES_OK, trmcode;
  MSKtask_t       task;

  /* Initial setup. */
  env  = NULL;
  task = NULL;
  MOSEKCALL(res, MSK_makeenv(&env, NULL));
  MOSEKCALL(res, MSK_maketask(env, 0, 0, &task));
  MOSEKCALL(res, MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr));

  /* Compute total wealth */
  U = w;
  for (k = 0; k < n; ++k) U += x0[k];

  /* Constraints. */
  MOSEKCALL(res, MSK_appendcons(task, 1 + 4 * n));
  MOSEKCALL(res, MSK_putconbound(task, 0, MSK_BK_FX, w, w));
  sprintf(buf, "%s", "budget");
  MOSEKCALL(res, MSK_putconname(task, 0, buf));

  for (i = 0; i < n; ++i)
  {
    MOSEKCALL(res, MSK_putconbound(task, 1 + i, MSK_BK_FX, 0.0, 0.0));
    sprintf(buf, "GT[%d]", 1 + i);
    MOSEKCALL(res, MSK_putconname(task, 1 + i, buf));
  }

  for (i = 0; i < n; ++i)
  {
    MOSEKCALL(res, MSK_putconbound(task, 1 + n + i, MSK_BK_LO, -x0[i], MSK_INFINITY));
    sprintf(buf, "zabs1[%d]", 1 + i);
    MOSEKCALL(res, MSK_putconname(task, 1 + n + i, buf));
  }

  for (i = 0; i < n; ++i)
  {
    MOSEKCALL(res, MSK_putconbound(task, 1 + 2 * n + i, MSK_BK_LO, x0[i], MSK_INFINITY));
    sprintf(buf, "zabs2[%d]", 1 + i);
    MOSEKCALL(res, MSK_putconname(task, 1 + 2 * n + i, buf));
  }

  for (i = 0; i < n; ++i)
  {
    MOSEKCALL(res, MSK_putconbound(task, 1 + 3 * n + i, MSK_BK_UP, -MSK_INFINITY, 0.0));
    sprintf(buf, "indicator[%d]", 1 + i);
    MOSEKCALL(res, MSK_putconname(task, 1 + 3 * n + i, buf));
  }

  /* Offsets of variables into the (serialized) API variable. */
  offsetx = 0;
  offsets = n;
  offsett = n + 1;
  offsetz = 2 * n + 1;
  offsety = 3 * n + 1;

  /* Variables. */
  MOSEKCALL(res, MSK_appendvars(task, 4 * n + 1));

  /* x variables. */
  for (j = 0; j < n; ++j)
  {
    MOSEKCALL(res, MSK_putcj(task, offsetx + j, mu[j]));
    MOSEKCALL(res, MSK_putaij(task, 0, offsetx + j, 1.0));
    for (k = 0; k < n; ++k)
      if (GT[k][j] != 0.0)
        MOSEKCALL(res, MSK_putaij(task, 1 + k, offsetx + j, GT[k][j]));
    MOSEKCALL(res, MSK_putaij(task, 1 + n + j, offsetx + j, -1.0));
    MOSEKCALL(res, MSK_putaij(task, 1 + 2 * n + j, offsetx + j, 1.0));

    MOSEKCALL(res, MSK_putvarbound(task, offsetx + j, MSK_BK_LO, 0.0, MSK_INFINITY));
    sprintf(buf, "x[%d]", 1 + j);
    MOSEKCALL(res, MSK_putvarname(task, offsetx + j, buf));
  }

  /* s variable. */
  MOSEKCALL(res, MSK_putvarbound(task, offsets + 0, MSK_BK_FX, gamma, gamma));
  sprintf(buf, "s");
  MOSEKCALL(res, MSK_putvarname(task, offsets + 0, buf));

  /* t variables. */
  for (j = 0; j < n; ++j)
  {
    MOSEKCALL(res, MSK_putaij(task, 1 + j, offsett + j, -1.0));
    MOSEKCALL(res, MSK_putvarbound(task, offsett + j, MSK_BK_FR, -MSK_INFINITY, MSK_INFINITY));
    sprintf(buf, "t[%d]", 1 + j);
    MOSEKCALL(res, MSK_putvarname(task, offsett + j, buf));
  }

  /* z variables. */
  for (j = 0; j < n; ++j)
  {
    MOSEKCALL(res, MSK_putaij(task, 0, offsetz + j, g[j]));    
    MOSEKCALL(res, MSK_putaij(task, 1 + 1 * n + j, offsetz + j, 1.0));
    MOSEKCALL(res, MSK_putaij(task, 1 + 2 * n + j, offsetz + j, 1.0));
    MOSEKCALL(res, MSK_putaij(task, 1 + 3 * n + j, offsetz + j, 1.0));    
    MOSEKCALL(res, MSK_putvarbound(task, offsetz + j, MSK_BK_FR, -MSK_INFINITY, MSK_INFINITY));
    sprintf(buf, "z[%d]", 1 + j);
    MOSEKCALL(res, MSK_putvarname(task, offsetz + j, buf));
  }

  /* y variables. */
  for (j = 0; j < n; ++j)
  {
    MOSEKCALL(res, MSK_putaij(task, 0, offsety + j, f[j]));    
    MOSEKCALL(res, MSK_putaij(task, 1 + 3 * n + j, offsety + j, -U));    
    MOSEKCALL(res, MSK_putvarbound(task, offsety + j, MSK_BK_RA, 0.0, 1.0));
    MOSEKCALL(res, MSK_putvartype(task, offsety + j, MSK_VAR_TYPE_INT));
    sprintf(buf, "y[%d]", 1 + j);
    MOSEKCALL(res, MSK_putvarname(task, offsety + j, buf));
  }

  if (res == MSK_RES_OK)
  {
    /* sub should be n+1 long i.e. the dimmension of the cone. */
    MSKint32t *sub = (MSKint32t *) MSK_calloctask(task, 3 >= n + 1 ? 3 : n + 1, sizeof(MSKint32t));

    if (sub)
    {
      /* Add quadratic cone */
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
  /* no log output. */
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
  
  if (res == MSK_RES_OK)
  {
    expret = 0.0;
    stddev = 0.0;

    for (j = 0; j < n; ++j)
    {
      MOSEKCALL(res, MSK_getxxslice(task, MSK_SOL_ITG, offsetx + j, offsetx + j + 1, &xj));
      expret += mu[j] * xj;
    }

    MOSEKCALL(res, MSK_getxxslice(task, MSK_SOL_ITG, offsets + 0, offsets + 1, &stddev));

    printf("\nExpected return %e for gamma %e\n", expret, stddev);
  }

  MSK_deletetask(&task);
  MSK_deleteenv(&env);

  return (0);
}