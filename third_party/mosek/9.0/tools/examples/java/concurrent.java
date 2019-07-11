/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      concurrent.java

   Purpose: Demonstrates a simple implementation of a concurrent optimizer.
            
            The concurrent optimizer starts a few parallel optimizations
            of the same problem using different algorithms, and reports
            a solution when the first optimizer is ready.

            This example also demonstrates how to define a simple callback handler
            that stops the optimizer when requested.
*/
package com.mosek.example;

public class concurrent
{
  /** Takes a list of tasks and optimizes then in parallel. The
      response code and termination code from each optimization is
      stored in ``res`` and ``trm``.

      When one task completes with rescode == ok, others are terminated.

      Returns the index of the first task that returned
      with rescode == ok. Whether or not this task contains the
      most valuable answer, is for the caller to decide. If none
      completed without error return -1.
   */
  public static int optimize(mosek.Task[]    tasks,
                             mosek.rescode[] res,
                             mosek.rescode[] trm)
  {
    int n = tasks.length;
    Thread[] jobs = new Thread[n];

    // Set a callback function 
    final CallbackProxy cb = new CallbackProxy();
    for (int i = 0; i < n; ++i)
      tasks[i].set_Progress(cb);
    
    // Initialize
    for (int i = 0; i < n; ++i) 
    {
      res[i] = mosek.rescode.err_unknown;
      trm[i] = mosek.rescode.err_unknown;
    }

    // Start parallel optimizations, one per task
    for (int i = 0; i < n; ++i)
    {
      int num = i;
      jobs[i] = new Thread() { public void run() {
        try
        {
          trm[num] = tasks[num].optimize();
          res[num] = mosek.rescode.ok;
        }
        catch (mosek.Exception e)
        {
          trm[num] = mosek.rescode.err_unknown;
          res[num] = e.code;
        }
        finally
        {
          // If this finished with success, inform other tasks to interrupt
          if (res[num] == mosek.rescode.ok)
          {
            if (!cb.stop) cb.firstStop = num;
            cb.stop = true;
          }
        }
      }};
      jobs[i].start();
    }

    // Join all threads
    try {
      for (Thread j: jobs)
        j.join();
    }
    catch (InterruptedException e) {}

    // For debugging, print res and trm codes for all optimizers
    for (int i = 0; i < n; ++i)
      System.out.println("Optimizer  " + i + "  res " + res[i] + "   trm " + trm[i]);

    return cb.firstStop;
  }


  /** 
      Given a continuous task, set up jobs to optimize it 
      with a list of different solvers.

      Returns an index, corresponding to the optimization
      task that is returned as winTask. This is the task
      with the best possible status of those that finished.
      If none task is considered successful returns -1.
   */
  public static int  optimizeconcurrent(mosek.Task            task, 
                                        mosek.optimizertype[] optimizers,
                                        mosek.Task[]          winTask,
                                        mosek.rescode[]       winTrm,
                                        mosek.rescode[]       winRes)
  {
    int n = optimizers.length;
    mosek.Task[] tasks  = new mosek.Task[n];
    mosek.rescode[] res = new mosek.rescode[n];
    mosek.rescode[] trm = new mosek.rescode[n];

    // Clone tasks and choose various optimizers
    for (int i = 0; i < n; ++i)
    {
      tasks[i] = new mosek.Task(task);
      tasks[i].putintparam(mosek.iparam.optimizer, optimizers[i].value);
    }

    // Solve tasks in parallel
    int firstOK = optimize(tasks, res, trm);

    if (firstOK >= 0) 
    {
      winTask[0]  = tasks[firstOK]; 
      winTrm[0]   = trm[firstOK]; 
      winRes[0]   = res[firstOK];
    }
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
  public static int  optimizeconcurrentMIO(mosek.Task            task, 
                                           int[]                 seeds,
                                           mosek.Task[]          winTask,
                                           mosek.rescode[]       winTrm,
                                           mosek.rescode[]       winRes)
  {
    int n = seeds.length;
    mosek.Task[] tasks  = new mosek.Task[n];
    mosek.rescode[] res = new mosek.rescode[n];
    mosek.rescode[] trm = new mosek.rescode[n];

    // Clone tasks and choose various seeds for the optimizer
    for (int i = 0; i < n; ++i)
    {
      tasks[i] = new mosek.Task(task);
      tasks[i].putintparam(mosek.iparam.mio_seed, seeds[i]);
    }

    // Solve tasks in parallel
    int firstOK = optimize(tasks, res, trm);

    if (firstOK >= 0) 
    {
      // Pick the task that ended with res = ok
      // and contains an integer solution with best objective value
      mosek.objsense sense = task.getobjsense();
      double bestObj = (sense == mosek.objsense.minimize) ? 1.0e+10 : -1.0e+10;
      int bestPos = -1;

      for (int i = 0; i < n; ++i)
        System.out.println(i + "    " + tasks[i].getprimalobj(mosek.soltype.itg));

      for (int i = 0; i < n; ++i)
        if ((res[i] == mosek.rescode.ok) &&
            (tasks[i].getsolsta(mosek.soltype.itg) == mosek.solsta.prim_feas ||
             tasks[i].getsolsta(mosek.soltype.itg) == mosek.solsta.integer_optimal) &&
            ((sense == mosek.objsense.minimize) ? 
                (tasks[i].getprimalobj(mosek.soltype.itg) < bestObj) :
                (tasks[i].getprimalobj(mosek.soltype.itg) > bestObj)   )   )
        {
          bestObj = tasks[i].getprimalobj(mosek.soltype.itg);
          bestPos = i;
        }

      if (bestPos != -1)
      {
        winTask[0]  = tasks[bestPos]; 
        winTrm[0]   = trm[bestPos]; 
        winRes[0]   = res[bestPos];
        return bestPos;
      }
    }
  
    return -1;
  }

  /** 
     This is an example of how one can use the methods
         optimizeconcurrent
         optimizeconcurrentMIO

     argv[0] : name of file with input problem
     argv[1]: (optional) time limit
   */
  public static void main(String[] argv)
  {
    try (mosek.Env  env  = new mosek.Env();
         mosek.Task task = new mosek.Task(env, 0, 0)) {

      if (argv.length>=1)
      {
        task.readdata(argv[0]);
      }
      else
      {
        task.readdata("../data/25fv47.mps");
      }

      mosek.rescode[]  res = { mosek.rescode.ok }, trm = { mosek.rescode.ok };
      mosek.Task[]     t = new mosek.Task[1];
      int              idx;
      int[]            numint = { 0 };
      task.getnumintvar(numint);

      // Optional time limit
      if (argv.length >= 2) {
        double timeLimit = Double.parseDouble(argv[1]);
        task.putdouparam(mosek.dparam.optimizer_max_time, timeLimit);
      }

      if (numint[0] == 0) 
      {
        /* If the problem is continuous
           optimize it with three continuous optimizers.
          (Simplex will fail for non-linear problems)
        */
        mosek.optimizertype[] optimizers = { 
          mosek.optimizertype.conic,
          mosek.optimizertype.dual_simplex,
          mosek.optimizertype.primal_simplex
        };

        idx = optimizeconcurrent(task, optimizers, t, trm, res);
      }          
      else
      {
        /* Mixed-integer problem.
           Try various seeds.
        */
        int[] seeds = { 42, 13, 71749373 };

        idx = optimizeconcurrentMIO(task, seeds, t, trm, res);
      }          

      // Check results and print the best answer
      if (idx >= 0) 
      {
        System.out.println("Result from optimizer with index " + idx + ": res " + res[0] + "  trm " + trm[0]);
        t[0].set_Stream(mosek.streamtype.log, new mosek.Stream() { public void stream(String s) { System.out.print(s); }});
        t[0].optimizersummary(mosek.streamtype.log);
        t[0].solutionsummary(mosek.streamtype.log);
      }
      else 
      {
        System.out.println("All optimizers failed.");
      }
    }
  }

  /**
     Defines a Mosek callback function whose only function
     is to indicate if the optimizer should be stopped.
   */
  public static class CallbackProxy extends mosek.Progress
  {
    public boolean stop;
    public int firstStop;
    public CallbackProxy()
    {
      stop = false;
      firstStop = -1;
    }

    public int progress(mosek.callbackcode caller)
    {
      // Return non-zero implies terminate the optimizer
      return stop ? 1 : 0;
    }
  }
}