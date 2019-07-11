/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      milo1.c

   Purpose:   To demonstrate how to solve a small mixed
              integer linear optimization problem using
              the MOSEK API.
*/
#include <stdio.h>
#include "mosek.h" /* Include the MOSEK definition file. */

static void MSKAPI printstr(void *handle,
                            const char str[])
{
  printf("%s", str);
} /* printstr */

int main(int argc, char *argv[])
{
  const MSKint32t numvar = 2,
                  numcon = 2;

  double       c[]   = {  1.0, 0.64 };
  MSKboundkeye bkc[] = { MSK_BK_UP,    MSK_BK_LO };
  double       blc[] = { -MSK_INFINITY, -4.0 };
  double       buc[] = { 250.0,        MSK_INFINITY };

  MSKboundkeye bkx[] = { MSK_BK_LO,    MSK_BK_LO };
  double       blx[] = { 0.0,          0.0 };
  double       bux[] = { MSK_INFINITY, MSK_INFINITY };


  MSKint32t    aptrb[] = { 0, 2 },
               aptre[] = { 2, 4 },
               asub[] = { 0,    1,   0,    1 };
  double       aval[] = { 50.0, 3.0, 31.0, -2.0 };
  MSKint32t    i, j;

  MSKenv_t     env = NULL;
  MSKtask_t    task = NULL;
  MSKrescodee  r;

  /* Create the mosek environment. */
  r = MSK_makeenv(&env, NULL);

  /* Check if return code is ok. */
  if (r == MSK_RES_OK)
  {
    /* Create the optimization task. */
    r = MSK_maketask(env, 0, 0, &task);

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

    /* Optionally add a constant term to the objective. */
    if (r == MSK_RES_OK)
      r = MSK_putcfix(task, 0.0);
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

      /* Input column j of A */
      if (r == MSK_RES_OK)
        r = MSK_putacol(task,
                        j,                 /* Variable (column) index.*/
                        aptre[j] - aptrb[j], /* Number of non-zeros in column j.*/
                        asub + aptrb[j],   /* Pointer to row indexes of column j.*/
                        aval + aptrb[j]);  /* Pointer to Values of column j.*/

    }

    /* Set the bounds on constraints.
       for i=1, ...,numcon : blc[i] <= constraint i <= buc[i] */
    for (i = 0; i < numcon && r == MSK_RES_OK; ++i)
      r = MSK_putconbound(task,
                          i,           /* Index of constraint.*/
                          bkc[i],      /* Bound key.*/
                          blc[i],      /* Numerical value of lower bound.*/
                          buc[i]);     /* Numerical value of upper bound.*/

    /* Specify integer variables. */
    for (j = 0; j < numvar && r == MSK_RES_OK; ++j)
      r = MSK_putvartype(task, j, MSK_VAR_TYPE_INT);

    if (r == MSK_RES_OK)
      r =  MSK_putobjsense(task,
                           MSK_OBJECTIVE_SENSE_MAXIMIZE);

    if (r == MSK_RES_OK)
      /* Set max solution time */
      r = MSK_putdouparam(task,
                          MSK_DPAR_MIO_MAX_TIME,
                          60.0);

    if (r == MSK_RES_OK)
    {
      MSKrescodee trmcode;

      /* Run optimizer */
      r = MSK_optimizetrm(task, &trmcode);

      /* Print a summary containing information
         about the solution for debugging purposes*/
      MSK_solutionsummary(task, MSK_STREAM_MSG);

      if (r == MSK_RES_OK)
      {
        MSKint32t  j;
        MSKsolstae solsta;
        double     *xx = NULL;

        MSK_getsolsta(task, MSK_SOL_ITG, &solsta);

        xx = calloc(numvar, sizeof(double));
        if (xx)
        {
          switch (solsta)
          {
            case MSK_SOL_STA_INTEGER_OPTIMAL:
              MSK_getxx(task,
                        MSK_SOL_ITG,    /* Request the integer solution. */
                        xx);

              printf("Optimal solution.\n");
              for (j = 0; j < numvar; ++j)
                printf("x[%d]: %e\n", j, xx[j]);
              break;
            case MSK_SOL_STA_PRIM_FEAS:
              /* A feasible but not necessarily optimal solution was located. */
              MSK_getxx(task, MSK_SOL_ITG, xx);

              printf("Feasible solution.\n");
              for (j = 0; j < numvar; ++j)
                printf("x[%d]: %e\n", j, xx[j]);
              break;
            case MSK_SOL_STA_UNKNOWN:
              {
                MSKprostae prosta;
                MSK_getprosta(task, MSK_SOL_ITG, &prosta);
                switch (prosta)
                {
                  case MSK_PRO_STA_PRIM_INFEAS_OR_UNBOUNDED:
                    printf("Problem status Infeasible or unbounded\n");
                    break;
                  case MSK_PRO_STA_PRIM_INFEAS:
                    printf("Problem status Infeasible.\n");
                    break;
                  case MSK_PRO_STA_UNKNOWN:
                    printf("Problem status unknown. Termination code %d.\n", trmcode);
                    break;
                  default:
                    printf("Other problem status.");
                    break;
                }
              }
              break;
            default:
              printf("Other solution status.");
              break;
          }
        }
        else
        {
          r = MSK_RES_ERR_SPACE;
        }
        free(xx);
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

    MSK_deletetask(&task);
  }
  MSK_deleteenv(&env);

  printf("Return code: %d.\n", r);
  return (r);
} /* main */