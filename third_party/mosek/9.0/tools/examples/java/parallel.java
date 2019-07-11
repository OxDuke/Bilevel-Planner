/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      parallel.java

   Purpose: Demonstrates parallel optimization
*/
package com.mosek.example;

public class parallel
{
  /** Takes a list of tasks and optimizes them in parallel threads. The
      response code and termination code from each optimization is
      stored in ``res`` and ``trm``.
   */
  public static void paropt(mosek.Task[]    tasks,
                            mosek.rescode[] res,
                            mosek.rescode[] trm)
  {
    int n = tasks.length;
    Thread[] jobs = new Thread[n];
    
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
      }};
      jobs[i].start();
    }

    // Join all threads
    try {
      for (Thread j: jobs)
        j.join();
    }
    catch (InterruptedException e) {}
  }


  /** Example of how to use ``paropt``. 
      Optimizes tasks whose names were read from command line.
   */
  public static void main(String[] argv)
  {
    int n = argv.length;
    mosek.Task[]  t      = new mosek.Task[n];
    mosek.rescode[] res  = new mosek.rescode[n];
    mosek.rescode[] trm  = new mosek.rescode[n];

    mosek.Env env = new mosek.Env();

    for(int i = 0; i < n; i++) 
    {
      t[i] = new mosek.Task(env);
      t[i].readdata(argv[i]);
      // Each task will be single-threaded
      t[i].putintparam(mosek.iparam.intpnt_multi_thread, mosek.onoffkey.off.value);      
    }

    paropt(t, res, trm);

    for(int i = 0; i < n; i++) 
      System.out.printf("Task  %d  res %s   trm %s   obj_val  %f  time %f\n", 
        i, 
        res[i], 
        trm[i],  
        t[i].getdouinf(mosek.dinfitem.intpnt_primal_obj),
        t[i].getdouinf(mosek.dinfitem.optimizer_time));
  }
}