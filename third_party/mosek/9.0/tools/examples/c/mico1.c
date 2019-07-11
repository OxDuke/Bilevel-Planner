/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      mico1.c

   Purpose :   Demonstrates how to solve a small mixed
               integer conic optimization problem.

               minimize    x^2 + y^2
               subject to  x >= e^y + 3.8
                           x, y - integer
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
  MSKboundkeye bkc[] = { MSK_BK_FX, MSK_BK_FX, MSK_BK_FX };
  double       bc[]  = { -3.8, 1, 0 };

  MSKvariabletypee  vart[] = { MSK_VAR_TYPE_INT, MSK_VAR_TYPE_INT };
  MSKint32t       intsub[] = { 1, 2 };

  MSKint32t    asubi[] = {0, 0, 1, 2, 2},
               asubj[] = {1, 3, 4, 2, 5};
  MSKrealt     aval[]  = {-1, 1, 1, 1, -1};
  MSKint32t    i, j;

  MSKenv_t     env = NULL;
  MSKtask_t    task = NULL;
  MSKrescodee  r, trm;

  r = MSK_makeenv(&env, NULL);

  if (r == MSK_RES_OK)
  {
    r = MSK_maketask(env, 0, 0, &task);

    if (r == MSK_RES_OK)
      r = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

    if (r == MSK_RES_OK)
      r = MSK_appendvars(task, 6);

    if (r == MSK_RES_OK)
      r = MSK_appendcons(task, 3);

    if (r == MSK_RES_OK)
      r = MSK_putvarboundsliceconst(task, 0, 6, MSK_BK_FR, -0.0, 0.0);    

    /* Integrality constraints */
    if (r == MSK_RES_OK)
      r = MSK_putvartypelist(task, 2, intsub, vart);        

    /* Linear part of the problem */
    if (r == MSK_RES_OK)
      r = MSK_putaijlist(task, 5, asubi, asubj, aval);        
    if (r == MSK_RES_OK)
      r = MSK_putconboundslice(task, 0, 3, bkc, bc, bc);        

    /* Objective */
    if (r == MSK_RES_OK)
      r = MSK_putobjsense(task, MSK_OBJECTIVE_SENSE_MINIMIZE);
    if (r == MSK_RES_OK)
      r = MSK_putcj(task, 0, 1);

    /* Conic part of the problem */
    if (r == MSK_RES_OK)
      r = MSK_appendconeseq(task, MSK_CT_QUAD, 0, 3, 0);
    if (r == MSK_RES_OK)
      r = MSK_appendconeseq(task, MSK_CT_PEXP, 0, 3, 3);

    /* Optimize the problem */
    if (r == MSK_RES_OK)
      r = MSK_optimizetrm(task, &trm);

    if (r == MSK_RES_OK)
      r = MSK_solutionsummary(task, MSK_STREAM_MSG);

    if (r == MSK_RES_OK)
    {
      MSKrealt xx[] = {0, 0};

      r = MSK_getxxslice(task, MSK_SOL_ITG, 1, 3, xx);
      
      if (r == MSK_RES_OK)
        printf("x = %.2f, y = %.2f\n", xx[0], xx[1]);
    }

    if (task) MSK_deletetask(&task);
  }

  if (env) MSK_deleteenv(&env);
  return r;
} 