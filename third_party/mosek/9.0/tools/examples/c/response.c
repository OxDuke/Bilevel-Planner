/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      response.c

  Purpose:   This example demonstrates proper response handling
             for problems solved with the interior-point optimizers.
*/

#include <stdio.h>
#include "mosek.h"

/* Log handler */
void MSKAPI printlog(void *ptr,
                     const char s[])
{
  printf("%s", s);
}

int main(int argc, char const *argv[])
{
  MSKenv_t    env;
  MSKtask_t   task;
  MSKrescodee r;
  char        symname[MSK_MAX_STR_LEN];
  char        desc[MSK_MAX_STR_LEN];
  int         i, numvar;
  double      *xx = NULL;
  const char  *filename;

  if (argc >= 2) filename = argv[1];
  else           filename = "../data/cqo1.mps";

  // Create the environment
  r = MSK_makeenv(&env, NULL);

  if (r == MSK_RES_OK)
  {
    // Create the task
    r = MSK_makeemptytask(env, &task);

    // (Optionally) attach the log handler to receive log information
    // if ( r == MSK_RES_OK ) MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printlog);

    // (Optionally) uncomment this line to most likely see solution status Unknown
    // MSK_putintparam(task, MSK_IPAR_INTPNT_MAX_ITERATIONS, 1);

    // In this example we read an optimization problem from a file
    r = MSK_readdata(task, filename);

    if (r == MSK_RES_OK)
    {
      MSKrescodee trmcode;
      MSKsolstae  solsta;

      // Do the optimization, and exit in case of error
      r = MSK_optimizetrm(task, &trmcode);

      if (r != MSK_RES_OK) {
        MSK_getcodedesc(r, symname, desc);
        printf("Error during optimization: %s %s\n", symname, desc);
        exit(r);
      }

      MSK_solutionsummary(task, MSK_STREAM_LOG);

      /* Expected result: The solution status of the interiot-point solution is optimal. */

      if (MSK_RES_OK == MSK_getsolsta(task, MSK_SOL_ITR, &solsta))
      {
        switch (solsta)
        {
          case MSK_SOL_STA_OPTIMAL:
            printf("An optimal interior-point solution is located.\n");

            /* Read and print the variable values in the solution */
            MSK_getnumvar(task, &numvar);
            xx = calloc(numvar, sizeof(double));
            MSK_getxx(task, MSK_SOL_ITR, xx);
            for (i = 0; i < numvar; i++)
              printf("xx[%d] = %.4lf\n", i, xx[i]);
            free(xx);
            break;

          case MSK_SOL_STA_DUAL_INFEAS_CER:
            printf("Dual infeasibility certificate found.\n");
            break;

          case MSK_SOL_STA_PRIM_INFEAS_CER:
            printf("Primal infeasibility certificate found.\n");
            break;

          case MSK_SOL_STA_UNKNOWN:
            /* The solutions status is unknown. The termination code
               indicating why the optimizer terminated prematurely. */
            printf("The solution status is unknown.\n");
            /* No-error cause of termination e.g. an iteration limit is reached.  */
            MSK_getcodedesc(trmcode, symname, desc);
            printf("  Termination code: %s %s\n", symname, desc);
            break;

          default:
            MSK_solstatostr(task, solsta, desc);
            printf("An unexpected solution status %s with code %d is obtained.\n", desc, solsta);
            break;
        }
      }
      else
        printf("Could not obtain the solution status for the requested solution.\n");
    }
    else {
      MSK_getcodedesc(r, symname, desc);
      printf("Optimization was not started because of error %s(%d): %s\n", symname, r, desc);
    }

    MSK_deletetask(&task);
  }

  MSK_deleteenv(&env);
  return r;
}