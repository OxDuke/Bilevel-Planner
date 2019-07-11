/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      mioinitsol.c

   Purpose:   To demonstrate how to solve a MIP with a start guess.

 */

#include "mosek.h"
#include <stdio.h>

static void MSKAPI printstr(void        *handle,
                            const char str[])
{
  printf("%s", str);
} /* printstr */


int main(int argc, char *argv[])
{
  char         buffer[512];

  const MSKint32t numvar    = 4,
                  numcon    = 1,
                  numintvar = 3;

  MSKrescodee  r;

  MSKenv_t     env;
  MSKtask_t    task;

  double       c[] = { 7.0, 10.0, 1.0, 5.0 };

  MSKboundkeye bkc[] = {MSK_BK_UP};
  double       blc[] = { -MSK_INFINITY};
  double       buc[] = {2.5};

  MSKboundkeye bkx[] = {MSK_BK_LO, MSK_BK_LO, MSK_BK_LO, MSK_BK_LO};
  double       blx[] = {0.0,       0.0,       0.0,      0.0      };
  double       bux[] = {MSK_INFINITY, MSK_INFINITY, MSK_INFINITY, MSK_INFINITY};

  MSKint32t    ptrb[] = {0, 1, 2, 3},
               ptre[] = {1, 2, 3, 4},
               asub[] = {0,   0,   0,   0  };

  double       aval[] = {1.0, 1.0, 1.0, 1.0};
  MSKint32t    intsub[] = {0, 1, 2};
  MSKint32t    j;

  double       xx[4];

  r = MSK_makeenv(&env, NULL);

  if (r == MSK_RES_OK)
    r = MSK_maketask(env, 0, 0, &task);

  if (r == MSK_RES_OK)
    r = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

  if (r == MSK_RES_OK)
    r = MSK_inputdata(task,
                      numcon, numvar,
                      numcon, numvar,
                      c,
                      0.0,
                      ptrb,
                      ptre,
                      asub,
                      aval,
                      bkc,
                      blc,
                      buc,
                      bkx,
                      blx,
                      bux);

  if (r == MSK_RES_OK)
    r = MSK_putobjsense(task, MSK_OBJECTIVE_SENSE_MAXIMIZE);

  for (j = 0; j < numintvar && r == MSK_RES_OK; ++j)
    r = MSK_putvartype(task, intsub[j], MSK_VAR_TYPE_INT);

  if (r == MSK_RES_OK)
  {
    /* Assign values to integer variables 
       (we only set a slice of xx) */
    double       xxInit[] = {1.0, 1.0, 0.0};
    r = MSK_putxxslice(task, MSK_SOL_ITG, 0, 3, xxInit);
  }
  /* solve */

  if (r == MSK_RES_OK)
  {
    MSKrescodee trmcode;
    r = MSK_optimizetrm(task, &trmcode);
    MSK_solutionsummary(task, MSK_STREAM_LOG);
  }

  /* Read back the solution */
  r = MSK_getxx(task, MSK_SOL_ITG, xx);

  if (r == MSK_RES_OK) 
  {
    int constr;
    double constr_obj;

    printf("Solution:\n");
    for(j = 0; j < numvar; j++)
      printf("%f ", xx[j]);
    printf("\n");
    
    /*  Was the initial guess used?     */
    MSK_getintinf(task, MSK_IINF_MIO_CONSTRUCT_SOLUTION, &constr);
    MSK_getdouinf(task, MSK_DINF_MIO_CONSTRUCT_SOLUTION_OBJ, &constr_obj);
    printf("Initial solution utilization: %d\nInitial solution objective: %.3f\n", constr, constr_obj);
  }
  else
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
  MSK_deleteenv(&env);

  return (r);
}