/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      parallel.cc

   Purpose: Demonstrates parallel optimization
*/
#include<stdio.h>
#include<stdlib.h>
#include<mosek.h>
#include<thread>

/** Runs a single task */
void runTask(int         num, 
             MSKtask_t   task,
             MSKrescodee *res,
             MSKrescodee *trm) 
{
  *res = MSK_optimizetrm(task, trm);
}

/** Takes a list of tasks and optimizes then in parallel. The
    response code and termination code from each optimization is
    stored in ``res`` and ``trm``.
 */
void paropt(int         n,
            MSKtask_t   *tasks,
            MSKrescodee *res,
            MSKrescodee *trm)
{
  int i;
  std::thread * jobs = new std::thread[n];

  // Initialize
  for(i = 0; i < n; ++i) {
    res[i] = trm[i] = MSK_RES_ERR_UNKNOWN;
    jobs[i] = std::thread(runTask, i, tasks[i], &(res[i]), &(trm[i]));
  }

  // Join all threads
  for(i = 0; i < n; ++i) jobs[i].join();
  delete[] jobs;
}

/** Example of how to use ``paropt``. 
    Optimizes tasks whose names were read from command line.
 */
int main(int argc, char **argv)
{
  MSKenv_t env;
  int n = argc - 1;
  MSKtask_t   *tasks = new MSKtask_t[n];
  MSKrescodee *res   = new MSKrescodee[n];
  MSKrescodee *trm   = new MSKrescodee[n];
  
  MSK_makeenv(&env, NULL);

  for (int i = 0; i < n; i++) {
    MSK_makeemptytask(env, &(tasks[i]));
    MSK_readdata(tasks[i], argv[i+1]);
    // Each task will be single-threaded
    MSK_putintparam(tasks[i], MSK_IPAR_INTPNT_MULTI_THREAD, MSK_OFF);
  }

  paropt(n, tasks, res, trm);

  for(int i = 0; i < n; i++) {
    double obj, tm;
    MSK_getdouinf(tasks[i], MSK_DINF_INTPNT_PRIMAL_OBJ, &obj);
    MSK_getdouinf(tasks[i], MSK_DINF_OPTIMIZER_TIME, &tm);

    printf("Task  %d  res %d   trm %d   obj_val  %.5f  time %.5f\n", 
            i, 
            res[i], 
            trm[i],  
            obj,
            tm);
  }

  for(int i = 0; i < n; i++)
    MSK_deletetask(&(tasks[i]));
  delete[] tasks;
  delete[] res;
  delete[] trm;
  MSK_deleteenv(&env);
  return 0;
}