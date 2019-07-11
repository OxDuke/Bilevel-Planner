/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      concurrent.cc

   Purpose: Demonstrates a simple implementation of a concurrent optimizer.
            
            The concurrent optimizer starts a few parallel optimizations
            of the same problem using different algorithms, and reports
            a solution when the first optimizer is ready.

            This example also demonstrates how to define a simple callback handler
            that stops the optimizer when requested.

            For cross-platform compatibility this is C++ code requiring C++11.
            This is due to the use of std::thread. The rest is pure C. It should be easily  
            adapted to C and any threading model by only modifying the "optimize"
            function.
*/
#include<stdio.h>
#include<stdlib.h>
#include<mosek.h>
#include<thread>

/* This function prints log output from MOSEK to the terminal. */
static void MSKAPI printstr(void       *handle,
                            const char str[])
{
  printf("%s", str);
}

/**
   Defines a Mosek callback function whose only function
   is to indicate if the optimizer should be stopped.
 */
int stop = 0;
int firstStop = -1;
MSKint32t MSKAPI callback (MSKtask_t task, MSKuserhandle_t usrptr, MSKcallbackcodee caller,
                           const MSKrealt * douinf, const MSKint32t * intinf, const MSKint64t * lintinf)
{
  return stop;
}


/** Takes a list of tasks and optimizes then in parallel. The
    response code and termination code from each optimization is
    stored in ``res`` and ``trm``.

    When one task completes with rescode == ok, others are terminated.

    Returns the index of the first optimizer which returned without error.
    Whether or not this is the task firstOK contains the
    most valuable answer, is for the caller to decide.
    Otherwise returns -1.
 */
void runTask(int         num, 
             MSKtask_t   task,
             MSKrescodee *res,
             MSKrescodee *trm) 
{
  *res = MSK_optimizetrm(task, trm);
  if (*res == MSK_RES_OK) {
    if (!stop) firstStop = num;
    stop = 1;
  }
}

int optimize(int         n,
             MSKtask_t   *tasks,
             MSKrescodee *res,
             MSKrescodee *trm)
{
  int i;
  std::thread * jobs = new std::thread[n];

  // Set a callback function and start optimization
  for(i = 0; i < n; ++i) {
    MSK_putcallbackfunc(tasks[i], callback, NULL);
    res[i] = trm[i] = MSK_RES_ERR_UNKNOWN;
    jobs[i] = std::thread(runTask, i, tasks[i], &(res[i]), &(trm[i]));
  }

  // Join all threads
  for(i = 0; i < n; ++i) jobs[i].join();
  delete[] jobs;

  // For debugging, print res and trm codes for all optimizers
  for(i = 0; i < n; ++i)
    printf("Optimizer  %d   res %d   trm %d\n", i, res[i], trm[i]);

  return firstStop;
}


/** 
    Given a continuous task, set up jobs to optimize it 
    with a list of different solvers.

    Returns an index, corresponding to the optimization
    task that is returned as winTask. This is the task
    with the best possible status of those that finished.
    If none task is considered successfull returns -1.
 */
int optimizeconcurrent(MSKtask_t            task, 
                       int                  n,
                       MSKoptimizertypee    *optimizers,
                       MSKtask_t            *winTask,
                       MSKrescodee          *winTrm,
                       MSKrescodee          *winRes)
{
  MSKtask_t   *tasks = new MSKtask_t[n];
  MSKrescodee *res   = new MSKrescodee[n];
  MSKrescodee *trm   = new MSKrescodee[n];

  // Clone tasks and choose various optimizers
  for (int i = 0; i < n; ++i)
  {
    MSK_clonetask(task, &(tasks[i]));
    MSK_putintparam(tasks[i], MSK_IPAR_OPTIMIZER, optimizers[i]);
  }

  // Solve tasks in parallel
  int firstOK = optimize(n, tasks, res, trm);

  if (firstOK >= 0) 
  {
    *winTask  = tasks[firstOK];
    *winTrm   = trm[firstOK]; 
    *winRes   = res[firstOK];
  }
  else
  {
    *winTask  = NULL; 
    *winTrm   = MSK_RES_ERR_UNKNOWN; 
    *winRes   = MSK_RES_ERR_UNKNOWN;        
  }

  // Cleanup 
  for (int i = 0; i < n; ++i)
    if (i != firstOK) MSK_deletetask(&(tasks[i]));

  delete[] tasks; delete[] res; delete[] trm;
  return firstOK;
}
 

/** 
    Given a mixed-integer task, set up jobs to optimize it 
    with different values of seed. That will lead to
    different execution paths of the optimizer.

    Returns an index, corresponding to the optimization
    task that is returned as winTask. This is the task
    with the best value of the objective function.
    If none task is considered successful returns -1.

    Typically, the input task would contain a time limit. The two
    major scenarios are:
    1. Some clone ends before time limit - then it has optimum.
    2. All clones reach time limit - pick the one with best objective.
 */
int optimizeconcurrentMIO(MSKtask_t            task, 
                          int                  n,
                          int                  *seeds,
                          MSKtask_t            *winTask,
                          MSKrescodee          *winTrm,
                          MSKrescodee          *winRes)
{
  MSKtask_t   *tasks = new MSKtask_t[n];
  MSKrescodee *res   = new MSKrescodee[n];
  MSKrescodee *trm   = new MSKrescodee[n];
  *winTask  = NULL; 
  *winTrm   = MSK_RES_ERR_UNKNOWN; 
  *winRes   = MSK_RES_ERR_UNKNOWN;        
  int bestPos = -1;

  // Clone tasks and choose various optimizers
  for (int i = 0; i < n; ++i)
  {
    MSK_clonetask(task, &(tasks[i]));
    MSK_putintparam(tasks[i], MSK_IPAR_MIO_SEED, seeds[i]);
  }

  // Solve tasks in parallel
  int firstOK = optimize(n, tasks, res, trm);

  if (firstOK >= 0) 
  {
    // Pick the task that ended with res = ok
    // and contains an integer solution with best objective value
    MSKobjsensee sense;
    double bestObj;

    MSK_getobjsense(task, &sense);
    bestObj = (sense == MSK_OBJECTIVE_SENSE_MINIMIZE) ? 1.0e+10 : -1.0e+10;

    for (int i = 0; i < n; ++i) {
      double priObj;
      MSK_getprimalobj(tasks[i], MSK_SOL_ITG, &priObj);
      printf("%d      %f\n", i, priObj);
    }

    for (int i = 0; i < n; ++i) {
      double priObj;
      MSKsolstae solsta;
      MSK_getprimalobj(tasks[i], MSK_SOL_ITG, &priObj);
      MSK_getsolsta(tasks[i], MSK_SOL_ITG, &solsta);

      if ((res[i] == MSK_RES_OK) &&
          (solsta == MSK_SOL_STA_PRIM_FEAS ||
           solsta == MSK_SOL_STA_INTEGER_OPTIMAL) &&
          ((sense == MSK_OBJECTIVE_SENSE_MINIMIZE) ? 
              (priObj < bestObj) : (priObj > bestObj) ) )
      {
        bestObj = priObj;
        bestPos = i;
      }
    }

    if (bestPos != -1)
    {
      *winTask  = tasks[bestPos];
      *winTrm   = trm[bestPos]; 
      *winRes   = res[bestPos];
    }
  }

  // Cleanup 
  for (int i = 0; i < n; ++i)
    if (i != bestPos) MSK_deletetask(&(tasks[i]));

  delete[] tasks; delete[] res; delete[] trm;
  return bestPos;
}


/** 
   This is an example of how one can use the methods
       optimizeconcurrent
       optimizeconcurrentMIO

   argv[0] : name of file with input problem
   argv[1]: (optional) time limit
 */
int main(int argc, char **argv)
{

  MSKenv_t env;
  MSKtask_t task;
  int numint;

  MSKtask_t t;
  MSKrescodee res, trm;
  int idx;

  MSK_makeenv(&env, NULL);
  MSK_makeemptytask(env, &task);
  if (argc >= 2)
    MSK_readdata(task, argv[1]);
  else
    MSK_readdata(task, "../data/25fv47.mps");

  if (argc >= 3) 
    MSK_putdouparam(task, MSK_DPAR_OPTIMIZER_MAX_TIME, atof(argv[2]));

  MSK_getnumintvar(task, &numint);

  if (numint == 0) 
  {
    /* If the problem is continuous
       optimize it with three continuous optimizers.
      (Simplex will fail for non-linear problems)
    */
    MSKoptimizertypee optimizers[3] = { 
      MSK_OPTIMIZER_CONIC,
      MSK_OPTIMIZER_DUAL_SIMPLEX,
      MSK_OPTIMIZER_PRIMAL_SIMPLEX
    };

    idx = optimizeconcurrent(task, 3, optimizers, &t, &trm, &res);
  }          
  else
  {
    /* Mixed-integer problem.
       Try various seeds.
    */
    int seeds[3] = { 42, 13, 71749373 };

    idx = optimizeconcurrentMIO(task, 3, seeds, &t, &trm, &res);
  }          

  // Check results and print the best answer
  if (idx >= 0) 
  {
    printf("Result from optimizer with index %d: res %d  trm %d\n", idx, res, trm);
    MSK_linkfunctotaskstream(t, MSK_STREAM_LOG, NULL, printstr);
    MSK_optimizersummary(t, MSK_STREAM_LOG);
    MSK_solutionsummary(t, MSK_STREAM_LOG);
    MSK_deletetask(&t);
  }
  else 
  {
    printf("All optimizers failed.");
  }

  MSK_deletetask(&task);
  MSK_deleteenv(&env);
  return 0;
}