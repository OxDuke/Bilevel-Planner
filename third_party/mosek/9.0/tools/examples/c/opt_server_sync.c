/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:    opt_server_sync.c

  Purpose :   Demonstrates how to use MOSEK OptServer
              to solve optimization problem synchronously
*/
#include "mosek.h"

static void MSKAPI printstr(void *handle, const char str[])
{
  printf("%s", str);
}

int main(int argc, const char * argv[])
{
  MSKenv_t    env  = NULL;
  MSKtask_t   task = NULL;
  MSKrescodee res  = MSK_RES_OK;
  MSKrescodee trm  = MSK_RES_OK;

  if (argc <= 3)
  {
    printf("Missing argument, syntax is:\n");
    printf("  opt_server_sync inputfile host port\n");
  }
  else
  {
    // Create the mosek environment.
    // The `NULL' arguments here, are used to specify customized
    // memory allocators and a memory debug file. These can
    // safely be ignored for now.
    res = MSK_makeenv(&env, NULL);

    // Create a task object linked with the environment env.
    // We create it with 0 variables and 0 constraints initially,
    // since we do not know the size of the problem.
    if (res == MSK_RES_OK)
      res = MSK_maketask(env, 0, 0, &task);

    // Direct the task log stream to a user specified function
    if (res == MSK_RES_OK)
      res = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

    // We assume that a problem file was given as the first command
    // line argument (received in `argv')
    if (res == MSK_RES_OK)
      res = MSK_readdata(task, argv[1]);

    // Solve the problem remotely
    if (res == MSK_RES_OK)
      res = MSK_optimizermt(task, argv[2], argv[3], &trm);

    // Print a summary of the solution.
    if (res == MSK_RES_OK)
      res = MSK_solutionsummary(task, MSK_STREAM_LOG);

    // If an output file was specified, write a solution
    if (res == MSK_RES_OK && argc >= 3)
    {
      // We define the output format to be OPF, and tell MOSEK to
      // leave out parameters and problem data from the output file.
      MSK_putintparam(task, MSK_IPAR_OPF_WRITE_SOLUTIONS,  MSK_ON);
      MSK_putintparam(task, MSK_IPAR_OPF_WRITE_HINTS,      MSK_OFF);
      MSK_putintparam(task, MSK_IPAR_OPF_WRITE_PARAMETERS, MSK_OFF);
      MSK_putintparam(task, MSK_IPAR_OPF_WRITE_PROBLEM,    MSK_OFF);

      res = MSK_writedata(task, argv[2]);
    }

    // Delete task and environment
    MSK_deletetask(&task);
    MSK_deleteenv(&env);
  }
  return res;
}