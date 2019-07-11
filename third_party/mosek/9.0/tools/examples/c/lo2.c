/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      lo2.c

  Purpose:   To demonstrate how to solve a small linear
             optimization problem using the MOSEK C API,
             and handle the solver result and the problem
             solution.
*/
#include <stdio.h>
#include "mosek.h"

/* This function prints log output from MOSEK to the terminal. */
static void MSKAPI printstr(void *handle,
                            const char str[])
{
  printf("%s", str);
} /* printstr */

int main(int argc, const char *argv[])
{
  const int numvar = 4,
            numcon = 3;

  double       c[]     = {3.0, 1.0, 5.0, 1.0};
  /* Below is the sparse representation of the A
     matrix stored by row. */
  MSKint32t    aptrb[] = {0, 3, 7};
  MSKint32t    aptre[] = {3, 7, 9};
  MSKint32t    asub[]  = { 0, 1, 2,
                           0, 1, 2, 3,
                           1, 3
                         };
  double       aval[]  = { 3.0, 1.0, 2.0,
                           2.0, 1.0, 3.0, 1.0,
                           2.0, 3.0
                         };

  /* Bounds on constraints. */
  MSKboundkeye bkc[]  = {MSK_BK_FX, MSK_BK_LO,     MSK_BK_UP    };
  double       blc[]  = {30.0,      15.0,          -MSK_INFINITY};
  double       buc[]  = {30.0,      +MSK_INFINITY, 25.0         };
  /* Bounds on variables. */
  MSKboundkeye bkx[]  = {MSK_BK_LO,     MSK_BK_RA, MSK_BK_LO,     MSK_BK_LO     };
  double       blx[]  = {0.0,           0.0,       0.0,           0.0           };
  double       bux[]  = { +MSK_INFINITY, 10.0,      +MSK_INFINITY, +MSK_INFINITY };
  MSKenv_t     env  = NULL;
  MSKtask_t    task = NULL;
  MSKrescodee  r;
  MSKint32t    i, j;

  /* Create the mosek environment. */
  r = MSK_makeenv(&env, NULL);

  if (r == MSK_RES_OK)
  {
    /* Create the optimization task. */
    r = MSK_maketask(env, numcon, numvar, &task);

    /* Directs the log task stream to the 'printstr' function. */
    if (r == MSK_RES_OK)
      r = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

    /* Append 'numcon' empty constraints.
     The constraints will initially have no bounds. */
    if (r == MSK_RES_OK)
      r = MSK_appendcons(task, numcon);

    /* Append 'numvar' variables.
     The variables will initially be fixed at zero (x=0). */
    if (r == MSK_RES_OK)
      r = MSK_appendvars(task, numvar);

    for (j = 0; j < numvar && r == MSK_RES_OK; ++j)
    {
      /* Set the linear term c_j in the objective.*/
      if (r == MSK_RES_OK)
        r = MSK_putcj(task, j, c[j]);

      /* Set the bounds on variable j.
       blx[j] <= x_j <= bux[j] */
      if (r == MSK_RES_OK)
        r = MSK_putvarbound(task,
                            j,           /* Index of variable.*/
                            bkx[j],      /* Bound key.*/
                            blx[j],      /* Numerical value of lower bound.*/
                            bux[j]);     /* Numerical value of upper bound.*/
    }

    /* Set the bounds on constraints.
       for i=1, ...,numcon : blc[i] <= constraint i <= buc[i] */
    for (i = 0; i < numcon && r == MSK_RES_OK; ++i)
    {
      r = MSK_putconbound(task,
                          i,           /* Index of constraint.*/
                          bkc[i],      /* Bound key.*/
                          blc[i],      /* Numerical value of lower bound.*/
                          buc[i]);     /* Numerical value of upper bound.*/

      /* Input row i of A */
      if (r == MSK_RES_OK)
        r = MSK_putarow(task,
                        i,                 /* Row index.*/
                        aptre[i] - aptrb[i], /* Number of non-zeros in row i.*/
                        asub + aptrb[i],   /* Pointer to column indexes of row i.*/
                        aval + aptrb[i]);  /* Pointer to values of row i.*/
    }

    /* Maximize objective function. */
    if (r == MSK_RES_OK)
      r = MSK_putobjsense(task, MSK_OBJECTIVE_SENSE_MAXIMIZE);

    if (r == MSK_RES_OK)
    {
      MSKrescodee trmcode;

      /* Run optimizer */
      r = MSK_optimizetrm(task, &trmcode);

      /* Print a summary containing information
         about the solution for debugging purposes. */
      MSK_solutionsummary(task, MSK_STREAM_LOG);

      if (r == MSK_RES_OK)
      {
        MSKsolstae solsta;

        if (r == MSK_RES_OK)
          r = MSK_getsolsta(task, MSK_SOL_BAS, &solsta);
        switch (solsta)
        {
          case MSK_SOL_STA_OPTIMAL:
            {
              double *xx = (double*) calloc(numvar, sizeof(double));
              if (xx)
              {
                MSK_getxx(task,
                          MSK_SOL_BAS,    /* Request the basic solution. */
                          xx);

                printf("Optimal primal solution\n");
                for (j = 0; j < numvar; ++j)
                  printf("x[%d]: %e\n", j, xx[j]);
              }
              else
              {
                r = MSK_RES_ERR_SPACE;
              }

              free(xx);
              break;
            }
          case MSK_SOL_STA_DUAL_INFEAS_CER:
          case MSK_SOL_STA_PRIM_INFEAS_CER:
            printf("Primal or dual infeasibility certificate found.\n");
            break;
          case MSK_SOL_STA_UNKNOWN:
            {
              char symname[MSK_MAX_STR_LEN];
              char desc[MSK_MAX_STR_LEN];

              /* If the solutions status is unknown, print the termination code
                 indicating why the optimizer terminated prematurely. */

              MSK_getcodedesc(trmcode,
                              symname,
                              desc);

              printf("The solutuion status is unknown.\n");
              printf("The optimizer terminitated with code: %s\n", symname);
              break;
            }
          default:
            printf("Other solution status.\n");
            break;
        }
      }
    }

    if (r != MSK_RES_OK)
    {
      /* In case of an error print error code and description. */
      char symname[MSK_MAX_STR_LEN];
      char desc[MSK_MAX_STR_LEN];

      printf("An error occurred while optimizing.\n");
      MSK_getcodedesc(r,
                      symname,
                      desc);
      printf("Error %s - '%s'\n", symname, desc);
    }

    /* Delete the task and the associated data. */
    MSK_deletetask(&task);
  }

  /* Delete the environment and the associated data. */
  MSK_deleteenv(&env);

  return r;
}