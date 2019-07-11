/*
  File : parallel.cs

  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  Description :  Demonstrates parallel optimization
*/
using System.Threading.Tasks;
using System;

namespace mosek.example
{
  public class Parallel
  {
    /** Takes a list of tasks and optimizes them in parallel threads. The
        response code and termination ocde from each optimization is
        stored in ``res`` and ``trm``.
     */
    public static void paropt(mosek.Task[]    tasks,
                              mosek.rescode[] res,
                              mosek.rescode[] trm)
    {
      var n = tasks.Length;
      var jobs = new System.Threading.Tasks.Task[n];
     
      // Initialize
      for (var i = 0; i < n; ++i) 
      {
        res[i] = mosek.rescode.err_unknown;
        trm[i] = mosek.rescode.err_unknown;
      }

      // Start parallel optimizations, one per task
      for (var i = 0; i < n; ++i)
      {
        int num = i;
        jobs[i] = System.Threading.Tasks.Task.Factory.StartNew( () => {
          try
          {
            trm[num] = tasks[num].optimize();
            res[num] = mosek.rescode.ok;
          }
          catch (mosek.Exception e)
          {
            trm[num] = mosek.rescode.err_unknown;
            res[num] = e.Code;
          }
        } );
      }

      // Join all threads
      foreach (var j in jobs)
        j.Wait();
    }

    /** Example of how to use ``paropt``. 
        Optimizes tasks whose names were read from command line.
     */
    public static void Main(string[] argv)
    {
      int n = argv.Length;
      mosek.Task[]  t      = new mosek.Task[n];
      mosek.rescode[] res  = new mosek.rescode[n];
      mosek.rescode[] trm  = new mosek.rescode[n];
  
      using (var env = new mosek.Env())
      {
        for(int i = 0; i < n; i++) 
        {
          t[i] = new mosek.Task(env);
          t[i].readdata(argv[i]);
          // Each task will be single-threaded
          t[i].putintparam(mosek.iparam.intpnt_multi_thread, mosek.onoffkey.off);
        }

        paropt(t, res, trm);

        for(int i = 0; i < n; i++) 
          Console.WriteLine("Task  {0}  res {1}   trm {2}   obj_val  {3}  time {4}", 
            i, 
            res[i], 
            trm[i],  
            t[i].getdouinf(mosek.dinfitem.intpnt_primal_obj),
            t[i].getdouinf(mosek.dinfitem.optimizer_time));
      }
    }
  }
}