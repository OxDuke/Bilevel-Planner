/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      pow1.c

  Purpose: Demonstrates how to solve the problem

    maximize x^0.2*y^0.8 + z^0.4 - x
          st x + y + 0.5z = 2
             x,y,z >= 0
 */
#include <stdio.h>
#include "mosek.h" /* Include the MOSEK definition file. */

static void MSKAPI printstr(void *handle,
                            const char str[])
{
  printf("%s", str);
} /* printstr */

int main(int argc, const char *argv[])
{
  MSKrescodee  r;

  const MSKint32t numvar = 6,
                  numcon = 1;

  MSKboundkeye bkx[6];
  double       blx[6], bux[6];

  double       val[] = { 1.0, 1.0, -1.0 };
  MSKint32t    sub[] = { 3, 4, 0 };   

  double       aval[] = { 1.0, 1.0, 0.5 };
  MSKint32t    asub[] = { 0, 1, 2 };   

  MSKint32t   i, j;

  MSKenv_t    env  = NULL;
  MSKtask_t   task = NULL;

  /* Create the mosek environment. */
  r = MSK_makeenv(&env, NULL);

  if (r == MSK_RES_OK)
  {
    /* Create the optimization task. */
    r = MSK_maketask(env, numcon, numvar, &task);

    if (r == MSK_RES_OK)
    {
      MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

      /* Append 'numcon' empty constraints.
      The constraints will initially have no bounds. */
      if (r == MSK_RES_OK)
        r = MSK_appendcons(task, numcon);

      /* Append 'numvar' variables.
      The variables will initially be fixed at zero (x=0). */
      if (r == MSK_RES_OK)
        r = MSK_appendvars(task, numvar);

      /* Set up the linear part */
      MSK_putclist(task, 3, sub, val);
      MSK_putarow(task, 0, 3, asub, aval);
      MSK_putconbound(task, 0, MSK_BK_FX, 2.0, 2.0);
      for(i=0;i<5;i++)
        bkx[i] = MSK_BK_FR, blx[i] = -MSK_INFINITY, bux[i] = MSK_INFINITY;
      bkx[5] = MSK_BK_FX, blx[5] = bux[5] = 1.0;
      MSK_putvarboundslice(task, 0, numvar, bkx, blx, bux);

      if (r == MSK_RES_OK)
      {
        /* Append two power cones. */
        MSKint32t csub[2][3] = { {0, 1, 3}, {2, 5, 4} };
        r = MSK_appendcone(task, MSK_CT_PPOW, 0.2, 3, csub[0]);
        r = MSK_appendcone(task, MSK_CT_PPOW, 0.4, 3, csub[1]);
      }

      MSK_putobjsense(task, MSK_OBJECTIVE_SENSE_MAXIMIZE);

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
          MSKsolstae solsta;

          MSK_getsolsta(task, MSK_SOL_ITR, &solsta);

          switch (solsta)
          {
            case MSK_SOL_STA_OPTIMAL:
              {
                double *xx = NULL;

                xx = calloc(numvar, sizeof(double));
                if (xx)
                {
                  MSK_getxx(task,
                            MSK_SOL_ITR,    /* Request the interior solution. */
                            xx);

                  printf("Optimal primal solution\n");
                  for (j = 0; j < 3; ++j)
                    printf("x[%d]: %e\n", j, xx[j]);
                }
                else
                {
                  r = MSK_RES_ERR_SPACE;
                }
                free(xx);
              }
              break;
            case MSK_SOL_STA_DUAL_INFEAS_CER:
            case MSK_SOL_STA_PRIM_INFEAS_CER:
              printf("Primal or dual infeasibility certificate found.\n");
              break;
            case MSK_SOL_STA_UNKNOWN:
              printf("The status of the solution could not be determined. Termination code: %d.\n", trmcode);
              break;
            default:
              printf("Other solution status.");
              break;
          }
        }
        else
        {
          printf("Error while optimizing.\n");
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
    }
    /* Delete the task and the associated data. */
    MSK_deletetask(&task);
  }

  /* Delete the environment and the associated data. */
  MSK_deleteenv(&env);

  return (r);
} /* main */