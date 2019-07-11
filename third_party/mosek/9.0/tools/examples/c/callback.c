/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      callback.c

   Purpose:   To demonstrate how to use the progress
              callback.

              Compile and link the file with  MOSE, then
              use as follows:

              callback psim 25fv47.mps
              callback dsim 25fv47.mps
              callback intpnt 25fv47.mps

              The first argument tells which optimizer to use
              i.e. psim is primal simplex, dsim is dual simplex
              and intpnt is interior-point.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosek.h"

#define CONT_EXAMPLE

/* Type of user data used in callback functions */
typedef struct {
  double maxtime;   // Time limit for the optimizer
  int numvars;
  double *xx;       // For storing the integer solution when mixed integer optimizer is used
} cbdata;
typedef cbdata* cbdata_t;

/* Note: This function is declared using MSKAPI,
         so the correct calling convention is
         employed. */
static int MSKAPI usercallback(MSKtask_t            task,
                               MSKuserhandle_t      handle,
                               MSKcallbackcodee     caller,
                               const MSKrealt  * douinf,
                               const MSKint32t * intinf,
                               const MSKint64t * lintinf)
{
  cbdata_t data = (cbdata_t) handle;
  double maxtime = data->maxtime;
  MSKrescodee r;

  switch (caller)
  {
    case MSK_CALLBACK_BEGIN_INTPNT:
      printf("Starting interior-point optimizer\n");
      break;
    case MSK_CALLBACK_INTPNT:
      printf("Iterations: %-3d  Time: %6.2f(%.2f)  ",
             intinf[MSK_IINF_INTPNT_ITER],
             douinf[MSK_DINF_OPTIMIZER_TIME],
             douinf[MSK_DINF_INTPNT_TIME]);

      printf("Primal obj.: %-18.6e  Dual obj.: %-18.6e\n",
             douinf[MSK_DINF_INTPNT_PRIMAL_OBJ],
             douinf[MSK_DINF_INTPNT_DUAL_OBJ]);
      break;
    case MSK_CALLBACK_END_INTPNT:
      printf("Interior-point optimizer finished.\n");
      break;
    case MSK_CALLBACK_BEGIN_PRIMAL_SIMPLEX:
      printf("Primal simplex optimizer started.\n");
      break;
    case MSK_CALLBACK_UPDATE_PRIMAL_SIMPLEX:
      printf("Iterations: %-3d  ",
             intinf[MSK_IINF_SIM_PRIMAL_ITER]);
      printf("  Elapsed time: %6.2f(%.2f)\n",
             douinf[MSK_DINF_OPTIMIZER_TIME],
             douinf[MSK_DINF_SIM_TIME]);
      printf("Obj.: %-18.6e\n",
             douinf[MSK_DINF_SIM_OBJ]);
      break;
    case MSK_CALLBACK_END_PRIMAL_SIMPLEX:
      printf("Primal simplex optimizer finished.\n");
      break;
    case MSK_CALLBACK_BEGIN_DUAL_SIMPLEX:
      printf("Dual simplex optimizer started.\n");
      break;
    case MSK_CALLBACK_UPDATE_DUAL_SIMPLEX:
      printf("Iterations: %-3d  ", intinf[MSK_IINF_SIM_DUAL_ITER]);
      printf("  Elapsed time: %6.2f(%.2f)\n",
             douinf[MSK_DINF_OPTIMIZER_TIME],
             douinf[MSK_DINF_SIM_TIME]);
      printf("Obj.: %-18.6e\n", douinf[MSK_DINF_SIM_OBJ]);
      break;
    case MSK_CALLBACK_END_DUAL_SIMPLEX:
      printf("Dual simplex optimizer finished.\n");
      break;
    case MSK_CALLBACK_NEW_INT_MIO:
      printf("New integer solution has been located.\n");

      r = MSK_getxx(task, MSK_SOL_ITG, data->xx);
      if (r == MSK_RES_OK) {
        int i;
        printf("xx = ");
        for (i = 0; i < data->numvars; i++) printf("%lf ", data->xx[i]);
        printf("\nObj.: %f\n", douinf[MSK_DINF_MIO_OBJ_INT]);
      }
    default:
      break;
  }

  if (douinf[MSK_DINF_OPTIMIZER_TIME] >= maxtime)
  {
    /* mosek is spending too much time.
       Terminate it. */
    return (1);
  }

  return (0);
} /* usercallback */

static void MSKAPI printtxt(void       *info,
                            const char *buffer)
{ printf("%s", buffer);
} /* printtxt */

int main(int argc, const char *argv[])
{
  double    maxtime,
            *xx, *y;
  MSKrescodee r, trmr;
  int       j, i, numcon, numvar;
  FILE      *f;
  MSKenv_t  env;
  MSKtask_t task;
  cbdata    data;

#ifdef MIO_EXAMPLE
  /* To test the mixed-integer solution callback. */
  const char * slvr = "";
  const char * filename = "../data/milo1.lp";
#endif

#ifdef CONT_EXAMPLE
  /* To test interior-point/simplex optimizers on continuous problems */
  const char * slvr = "intpnt";
  const char * filename = "../data/25fv47.mps";
#endif

  printf("callback example\n");

  if (argc < 3)
  {
    printf("Usage: callback ( psim | dsim | intpnt ) filename\n");
    if (argc == 2)
      slvr = argv[1];
  }
  else
  {
    slvr = argv[1];
    filename = argv[2];
  }

  /* Create mosek environment. */
  /* "" should be replaced by NULL in a NON DEBUG setting. */
  r = MSK_makeenv(&env, ""); 

  /* Check the return code. */
  if (r == MSK_RES_OK)
  {
    /* Create an (empty) optimization task. */
    r = MSK_makeemptytask(env, &task);

    if (r == MSK_RES_OK)
    {
      MSK_linkfunctotaskstream(task, MSK_STREAM_MSG, NULL, printtxt);
      MSK_linkfunctotaskstream(task, MSK_STREAM_ERR, NULL, printtxt);
    }

    /* Specifies that data should be read from the file.
     */

    if (r == MSK_RES_OK)
      r = MSK_readdata(task, filename);

    if (r == MSK_RES_OK)
    {
      if (0 == strcmp(slvr, "psim"))
        MSK_putintparam(task, MSK_IPAR_OPTIMIZER, MSK_OPTIMIZER_PRIMAL_SIMPLEX);
      else  if (0 == strcmp(slvr, "dsim"))
        MSK_putintparam(task, MSK_IPAR_OPTIMIZER, MSK_OPTIMIZER_DUAL_SIMPLEX);
      else  if (0 == strcmp(slvr, "intpnt"))
        MSK_putintparam(task, MSK_IPAR_OPTIMIZER, MSK_OPTIMIZER_INTPNT);


      /* Tell mosek about the call-back function. */
      data.maxtime = 0.05;
      MSK_getnumvar(task, &data.numvars);
      data.xx = MSK_callocenv(env, data.numvars, sizeof(double));

      MSK_putcallbackfunc(task,
                          usercallback,
                          (void *) &data);

      /* Turn all MOSEK logging off. */
      MSK_putintparam(task,
                      MSK_IPAR_LOG,
                      0);

      r = MSK_optimizetrm(task, &trmr);
      printf("Return code - %d, termination code - %d\n", r, trmr);

      MSK_solutionsummary(task, MSK_STREAM_MSG);
      MSK_freeenv(env, data.xx);
    }


    {
      MSKrescodee lr = MSK_deletetask(&task);

      if ( r==MSK_RES_OK ) 
        r = lr;  
    }  
  }

  {
    MSKrescodee lr=MSK_deleteenv(&env);;

    if ( r==MSK_RES_OK ) 
      r = lr;  
  }  

  printf("Return code - %d\n", r);
  return (0);
} /* main */