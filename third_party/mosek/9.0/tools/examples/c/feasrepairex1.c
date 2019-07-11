
/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      feasrepairex1.c

  Purpose:   To demonstrate how to use the MSK_primalrepair function to
             repair an infeasible problem.
*/

#include <math.h>
#include <stdio.h>

#include "mosek.h"

static void MSKAPI printstr(void *handle,
                            const char str[])
{
  fputs(str, stdout);
} /* printstr */

static const char * feasrepair_lp =
"minimize                                    \n"
" obj: - 10 x1 - 9 x2                        \n"
"st                                          \n"
" c1: + 7e-01 x1 + x2 <= 630                 \n"
" c2: + 5e-01 x1 + 8.333333333e-01 x2 <= 600 \n"
" c3: + x1 + 6.6666667e-01 x2 <= 708         \n"
" c4: + 1e-01 x1 + 2.5e-01 x2 <= 135         \n"
"bounds                                      \n"
"x2 >= 650                                   \n"
"end                                         \n";

int main(int argc, const char *argv[])
{
  MSKenv_t    env;
  MSKrescodee r, trmcode;
  MSKtask_t   task;

  r = MSK_makeenv(&env, NULL);

  if (r == MSK_RES_OK)
    r = MSK_makeemptytask(env, &task);

  if (r == MSK_RES_OK)
    MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

  if (r == MSK_RES_OK)
    r = MSK_readlpstring(task,feasrepair_lp); /* Read problem from string */

  if (r == MSK_RES_OK)
    r = MSK_putintparam(task, MSK_IPAR_LOG_FEAS_REPAIR, 3);

  if (r == MSK_RES_OK)
  {
    /* Weights are NULL implying all weights are 1. */
    r = MSK_primalrepair(task, NULL, NULL, NULL, NULL);
  }

  if (r == MSK_RES_OK)
  {
    double sum_viol;

    r = MSK_getdouinf(task, MSK_DINF_PRIMAL_REPAIR_PENALTY_OBJ, &sum_viol);

    if (r == MSK_RES_OK)
    {
      printf("Minimized sum of violations = %e\n", sum_viol);

      r = MSK_optimizetrm(task, &trmcode); /* Optimize the repaired task. */

      MSK_solutionsummary(task, MSK_STREAM_MSG);
    }
  }

  printf("Return code: %d\n", r);

  return (r);
}