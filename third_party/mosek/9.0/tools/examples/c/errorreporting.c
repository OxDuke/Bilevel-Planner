/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File: errorreporting.c

   Purpose:   To demonstrate how the error reporting can be customized.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosek.h"

static MSKrescodee MSKAPI handleresponse(MSKuserhandle_t handle,
                                         MSKrescodee     r,
                                         const char      msg[])
/* A custom response handler. */
{
  /* Find out if we had an error or warning */
  MSKrescodetypee rct;
  MSK_getresponseclass(r, &rct);

  if (r == MSK_RES_OK)
  {
    /* Do nothing */
  }
  else if (rct == MSK_RESPONSE_WRN)
  {
    printf("MOSEK reports warning number %d: %s\n", r, msg);
    r = MSK_RES_OK;
  }
  else if (rct == MSK_RESPONSE_ERR)
  {
    printf("MOSEK reports error number %d: %s\n", r, msg);
  }
  else
  {
    printf("Unexpected response %d: %s\n", r, msg);
  }

  return (r);
} /* handlerespone */


int main(int argc, char *argv[])
{
  MSKenv_t    env;
  MSKrescodee r;
  MSKtask_t   task;

  r = MSK_makeenv(&env, NULL);

  if (r == MSK_RES_OK)
  {
    r = MSK_makeemptytask(env, &task);
    if (r == MSK_RES_OK)
    {
      /*
       * Input a custom warning and error handler function.
       */

      MSK_putresponsefunc(task, handleresponse, NULL);

      /* User defined code goes here */
      /* This will provoke an error */

      if (r == MSK_RES_OK)
        r = MSK_putaij(task, 10, 10, 1.0);

    }
    MSK_deletetask(&task);
  }
  MSK_deleteenv(&env);

  printf("Return code - %d\n", r);

  if (r == MSK_RES_ERR_INDEX_IS_TOO_LARGE)
    return MSK_RES_OK;
  else
    return 1;
} /* main */