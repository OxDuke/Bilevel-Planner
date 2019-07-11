/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      parameters.c

 Purpose: Demonstrates a very simple example about how to get/set
 parameters with MOSEK C API
*/

#include "mosek.h"
#include "stdio.h"

int main()
{

  MSKenv_t env = NULL;
  MSKtask_t task = NULL;
  MSKrescodee res, trmcode;

  double param = 0.0;
  double      tm;
  MSKint32t   iter;

  /* Create an environment */
  res = MSK_makeenv(&env, NULL);

  if (res == MSK_RES_OK)
  {
    /* Create a task */
    res = MSK_maketask(env, 0, 0, &task);

    if (res == MSK_RES_OK)
    {
      printf("Test MOSEK parameter get/set functions\n");

      // Set log level (integer parameter)
      res = MSK_putintparam(task, MSK_IPAR_LOG, 1);
      // Select interior-point optimizer... (integer parameter)
      res = MSK_putintparam(task, MSK_IPAR_OPTIMIZER, MSK_OPTIMIZER_INTPNT);
      // ... without basis identification (integer parameter)
      res = MSK_putintparam(task, MSK_IPAR_INTPNT_BASIS, MSK_BI_NEVER);
      // Set relative gap tolerance (double parameter)
      res = MSK_putdouparam(task, MSK_DPAR_INTPNT_CO_TOL_REL_GAP, 1.0e-7);

      // The same using explicit string names
      res = MSK_putparam(task, "MSK_DPAR_INTPNT_CO_TOL_REL_GAP", "1.0e-7");
      res = MSK_putnadouparam(task, "MSK_DPAR_INTPNT_CO_TOL_REL_GAP",  1.0e-7);

      // Incorrect value
      res = MSK_putdouparam(task, MSK_DPAR_INTPNT_CO_TOL_REL_GAP,  -1.0);
      if (res != MSK_RES_OK)
        printf("Wrong parameter value\n");

      res = MSK_getdouparam(task, MSK_DPAR_INTPNT_CO_TOL_REL_GAP, &param);
      printf("Current value for parameter MSK_DPAR_INTPNT_CO_TOL_REL_GAP = %e\n", param);

      /* Define and solve an optimization problem here */
      /* MSK_optimizetrm(task, &trmcode) */
      /* After optimization: */

      printf("Get MOSEK information items\n");

      res = MSK_getdouinf(task, MSK_DINF_OPTIMIZER_TIME,     &tm);
      res = MSK_getintinf(task, MSK_IINF_INTPNT_ITER,        &iter);

      printf("Time: %f\nIterations: %d\n", tm, iter);

      MSK_deletetask(&task);
    }
    MSK_deleteenv(&env);
  }
  return 0;
}