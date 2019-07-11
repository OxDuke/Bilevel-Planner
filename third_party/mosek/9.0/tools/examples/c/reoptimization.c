/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      reoptimization.c

   Purpose:   To demonstrate how to solve a  linear
              optimization problem using the MOSEK API
              and modify and re-optimize the problem.
 */
#include <stdio.h>

#define DEBUG 1    /* Set to zero to turn debugging off. */

#include "mosek.h" /* Include the MOSEK definition file. */

static void MSKAPI printstr(void *handle,
                            const char str[])
{
  printf("%s", str);
} /* printstr */

void printres(int n, const double x[]) {
  int i;
  for (i = 0; i < n; i++) printf("%.3f ", x[i]);
  printf("\n");
}

int main(int argc, char *argv[])
{
  MSKint32t       numvar = 3,
                  numcon = 3;
  MSKint32t       i, j;
  double          c[]    = {1.5, 2.5, 3.0};
  MSKint32t       ptrb[] = {0, 3, 6},
                  ptre[] = {3, 6, 9},
                  asub[] = { 0, 1, 2,
                             0, 1, 2,
                             0, 1, 2
                           };

  double          aval[] = { 2.0, 3.0, 2.0,
                             4.0, 2.0, 3.0,
                             3.0, 3.0, 2.0
                           };

  MSKboundkeye    bkc[]  = {MSK_BK_UP, MSK_BK_UP, MSK_BK_UP    };
  double          blc[]  = { -MSK_INFINITY, -MSK_INFINITY, -MSK_INFINITY};
  double          buc[]  = {100000, 50000, 60000};

  MSKboundkeye    bkx[]  = {MSK_BK_LO,     MSK_BK_LO,    MSK_BK_LO};
  double          blx[]  = {0.0,           0.0,          0.0,};
  double          bux[]  = { +MSK_INFINITY, +MSK_INFINITY, +MSK_INFINITY};

  double          *xx = NULL;
  MSKenv_t        env;
  MSKtask_t       task;
  MSKint32t       varidx, conidx;
  MSKrescodee     r,lr;

  /* Create the mosek environment. */
  r = MSK_makeenv(&env,DEBUG ? "" :  NULL);

  if (r == MSK_RES_OK)
  {
    /* Create the optimization task. */
    r = MSK_maketask(env, numcon, numvar, &task);

    /* Append the constraints. */
    if (r == MSK_RES_OK)
      r = MSK_appendcons(task, numcon);

    /* Append the variables. */
    if (r == MSK_RES_OK)
      r = MSK_appendvars(task, numvar);

    /* Put C. */
    if (r == MSK_RES_OK)
      r = MSK_putcfix(task, 0.0);

    if (r == MSK_RES_OK)
      for (j = 0; j < numvar; ++j)
        r = MSK_putcj(task, j, c[j]);

    /* Put constraint bounds. */
    if (r == MSK_RES_OK)
      for (i = 0; i < numcon; ++i)
        r = MSK_putconbound(task, i, bkc[i], blc[i], buc[i]);

    /* Put variable bounds. */
    if (r == MSK_RES_OK)
      for (j = 0; j < numvar; ++j)
        r = MSK_putvarbound(task, j, bkx[j], blx[j], bux[j]);

    /* Put A. */
    if (r == MSK_RES_OK)
      if (numcon > 0)
        for (j = 0; j < numvar; ++j)
          r = MSK_putacol(task,
                          j,
                          ptre[j] - ptrb[j],
                          asub + ptrb[j],
                          aval + ptrb[j]);

    if (r == MSK_RES_OK)
      r = MSK_putobjsense(task,
                          MSK_OBJECTIVE_SENSE_MAXIMIZE);

    if (r == MSK_RES_OK)
      r = MSK_optimizetrm(task, NULL);

    if (r == MSK_RES_OK)
    {
      xx = calloc(numvar, sizeof(double));
      if (!xx)
        r = MSK_RES_ERR_SPACE;
    }

    if (r == MSK_RES_OK)
      r = MSK_getxx(task,
                    MSK_SOL_BAS,       /* Basic solution.       */
                    xx);

    printres(numvar, xx);

    /******************** Make a change to the A matrix **********/
    if (r == MSK_RES_OK)
      r = MSK_putaij(task, 0, 0, 3.0);
    if (r == MSK_RES_OK)
      r = MSK_optimizetrm(task, NULL);
    if (r == MSK_RES_OK)
      r = MSK_getxx(task,
                    MSK_SOL_BAS,       /* Basic solution.       */
                    xx);
    printres(numvar, xx);
    if (xx)
      free(xx);

    /*********************** Add a new variable ******************/
    /* Get index of new variable, this should be 3 */
    if (r == MSK_RES_OK)
      r = MSK_getnumvar(task, &varidx);
    /* Append a new variable x_3 to the problem */
    if (r == MSK_RES_OK)
    {
      r = MSK_appendvars(task, 1);
      numvar++;
    }
    /* Set bounds on new variable */
    if (r == MSK_RES_OK)
      r = MSK_putvarbound(task,
                          varidx,
                          MSK_BK_LO,
                          0,
                          +MSK_INFINITY);

    /* Change objective */
    if (r == MSK_RES_OK)
      r = MSK_putcj(task, varidx, 1.0);

    /* Put new values in the A matrix */
    if (r == MSK_RES_OK)
    {
      MSKint32t acolsub[] = {0,   2};
      double    acolval[] =  {4.0, 1.0};

      r = MSK_putacol(task,
                      varidx, /* column index */
                      2, /* num nz in column*/
                      acolsub,
                      acolval);
    }

    /* Change optimizer to free simplex and reoptimize */
    if (r == MSK_RES_OK)
      r = MSK_putintparam(task, MSK_IPAR_OPTIMIZER, MSK_OPTIMIZER_FREE_SIMPLEX);

    if (r == MSK_RES_OK)
      r = MSK_optimizetrm(task, NULL);
    if (r == MSK_RES_OK)
    {
      xx = calloc(numvar, sizeof(double));
      if (!xx)
        r = MSK_RES_ERR_SPACE;
    }

    if (r == MSK_RES_OK)
      r = MSK_getxx(task,
                    MSK_SOL_BAS,       /* Basic solution.       */
                    xx);

    printres(numvar, xx);

    /* **************** Add a new constraint ******************* */
    /* Get index of new constraint*/
    if (r == MSK_RES_OK)
      r = MSK_getnumcon(task, &conidx);

    /* Append a new constraint */
    if (r == MSK_RES_OK)
    {
      r = MSK_appendcons(task, 1);
      numcon++;
    }

    /* Set bounds on new constraint */
    if (r == MSK_RES_OK)
      r = MSK_putconbound(task,
                          conidx,
                          MSK_BK_UP,
                          -MSK_INFINITY,
                          30000);

    /* Put new values in the A matrix */
    if (r == MSK_RES_OK)
    {
      MSKint32t arowsub[] = {0,   1,   2,   3  };
      double    arowval[] =  {1.0, 2.0, 1.0, 1.0};

      r = MSK_putarow(task,
                      conidx, /* row index */
                      4,      /* num nz in row*/
                      arowsub,
                      arowval);
    }
    if (r == MSK_RES_OK)
      r = MSK_optimizetrm(task, NULL);

    if (r == MSK_RES_OK)
      r = MSK_getxx(task,
                    MSK_SOL_BAS,       /* Basic solution.       */
                    xx);

    printres(numvar, xx);


    /* **************** Change constraint bounds ******************* */
    if (r == MSK_RES_OK)
    {
      MSKboundkeye newbkc[]  = { MSK_BK_UP, MSK_BK_UP, MSK_BK_UP, MSK_BK_UP };
      double       newblc[]  = { -MSK_INFINITY, -MSK_INFINITY, -MSK_INFINITY, -MSK_INFINITY };
      double       newbuc[]  = { 80000, 40000, 50000, 22000 };

      r = MSK_putconboundslice(task, 0, numcon, newbkc, newblc, newbuc);
    }
    
    if (r == MSK_RES_OK)
      r = MSK_optimizetrm(task, NULL);

    if (r == MSK_RES_OK)
      r = MSK_getxx(task,
                    MSK_SOL_BAS,       /* Basic solution.       */
                    xx);

    printres(numvar, xx);

    if (xx)
      free(xx);
    
    lr = MSK_deletetask(&task);
    if (r == MSK_RES_OK)
      r = lr;

  }
  lr = MSK_deleteenv(&env);
  if (r == MSK_RES_OK)
    r = lr; 

  printf("Return code: %d (0 means no error occured.)\n", r);

  return (r);
} /* main */