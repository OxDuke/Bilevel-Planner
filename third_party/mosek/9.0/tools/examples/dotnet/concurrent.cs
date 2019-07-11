/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      concurrent.cs

   Purpose: Demonstrates a simple implementation of a concurrent optimizer.
            
            The concurrent optimizer starts a few parallel optimizations
            of the same problem using different algorithms, and reports
            a solution when the first optimizer is ready.

            This example also demonstrates how to define a simple callback handler
            that stops the optimizer when requested.
*/
using System.Threading.Tasks;
using System;

namespace mosek.example
{
  public class concurrent
  {
    /** Takes a list of tasks and optimizes then in parallel. The
        response code and termination code from each optimization is
        stored in ``res`` and ``trm``.

        When one task completes with rescode == ok, others are terminated.

        Returns true if some optimizer returned without error. In this case
        ``firstOK`` is the index of the first task that returned
        with rescode == ok. Whether or not this is the task firstOK contains the
        most valuable answer, is for the caller to decide.
     */
    public static bool optimize(mosek.Task[]    tasks,
                                mosek.rescode[] res,
                                mosek.rescode[] trm,
                                out int         firstOK)
    {
      var n = tasks.Length;
      var jobs = new System.Threading.Tasks.Task[n];
      int firstStop = -1;

      // Set a callback function 
      var cb = new CallbackProxy();
      for (var i = 0; i < n; ++i)
        tasks[i].set_Progress(cb);
      
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
          finally
          {
            // If this finished with success, inform other tasks to interrupt
            if (res[num] == mosek.rescode.ok)
            {
              if (!cb.Stop) firstStop = num;
              cb.Stop = true;
            }
          }
        } );
      }

      // Join all threads
      foreach (var j in jobs)
        j.Wait();

      // For debugging, print res and trm codes for all optimizers
      for (var i = 0; i < n; ++i)
        Console.WriteLine("Optimizer  {0}  res {1}   trm {2}", i, res[i], trm[i]);

      firstOK = firstStop;
      return cb.Stop;
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
                                          int[]                 optimizers,
                                          out mosek.Task        winTask,
                                          out mosek.rescode     winTrm,
                                          out mosek.rescode     winRes)
    {
      var n = optimizers.Length;
      var tasks = new mosek.Task[n];
      var res   = new mosek.rescode[n];
      var trm   = new mosek.rescode[n];

      // Clone tasks and choose various optimizers
      for (var i = 0; i < n; ++i)
      {
        tasks[i] = new mosek.Task(task);
        tasks[i].putintparam(mosek.iparam.optimizer, optimizers[i]);
      }

      // Solve tasks in parallel
      bool success;
      int firstOK;
      success = optimize(tasks, res, trm, out firstOK);

      if (success) 
      {
        winTask  = tasks[firstOK]; 
        winTrm   = trm[firstOK]; 
        winRes   = res[firstOK];
        return firstOK;
      }
      else
      {
        winTask  = null; 
        winTrm   = 0; 
        winRes   = 0;        
        return -1;
      }
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
                                             out mosek.Task        winTask,
                                             out mosek.rescode     winTrm,
                                             out mosek.rescode     winRes)
    {
      var n = seeds.Length;
      var tasks = new mosek.Task[n];
      var res   = new mosek.rescode[n];
      var trm   = new mosek.rescode[n];

      // Clone tasks and choose various seeds for the optimizer
      for (var i = 0; i < n; ++i)
      {
        tasks[i] = new mosek.Task(task);
        tasks[i].putintparam(mosek.iparam.mio_seed, seeds[i]);
      }

      // Solve tasks in parallel
      bool success;
      int firstOK;
      success = optimize(tasks, res, trm, out firstOK);

      if (success) 
      {
        // Pick the task that ended with res = ok
        // and contains an integer solution with best objective value
        mosek.objsense sense = task.getobjsense();
        double bestObj = (sense == mosek.objsense.minimize) ? 1.0e+10 : -1.0e+10;
        int bestPos = -1;

        for (var i = 0; i < n; ++i)
          Console.WriteLine("{0}   {1}   ", i, tasks[i].getprimalobj(mosek.soltype.itg));

        for (var i = 0; i < n; ++i)
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
          winTask  = tasks[bestPos]; 
          winTrm   = trm[bestPos]; 
          winRes   = res[bestPos];
          return bestPos;
        }
      }

      winTask  = null; 
      winTrm   = 0; 
      winRes   = 0;        
      return -1;
    }

    /** 
       This is an example of how one can use the methods
           optimizeconcurrent
           optimizeconcurrentMIO

       argv[0] : name of file with input problem
       argv[1]: (optional) time limit
     */
    public static void Main(string[] argv)
    {
      using (var env = new mosek.Env())
      {
        using (var task = new mosek.Task(env))
        {
          if (argv.Length >= 1) 
          {
            task.readdata(argv[0]);
          }
          else
          {
            task.readdata("../data/25fv47.mps");
          }

          mosek.rescode res, trm;
          mosek.Task    t;
          int           idx;
          int           numint;
          task.getnumintvar(out numint);

          // Optional time limit
          if (argv.Length >= 2) 
          {
            double timeLimit = double.Parse(argv[1]);
            task.putdouparam(mosek.dparam.optimizer_max_time, timeLimit);
          }

          if (numint == 0) 
          {
            /* If the problem is continuous
               optimize it with three continuous optimizers.
              (Simplex will fail for non-linear problems)
            */
            int[] optimizers = { 
              mosek.optimizertype.conic,
              mosek.optimizertype.dual_simplex,
              mosek.optimizertype.primal_simplex
            };

            idx = optimizeconcurrent(task, optimizers, out t, out trm, out res);
          }          
          else
          {
            /* Mixed-integer problem.
               Try various seeds.
            */
            int[] seeds = { 42, 13, 71749373 };

            idx = optimizeconcurrentMIO(task, seeds, out t, out trm, out res);
          }          

          // Check results and print the best answer
          if (idx >= 0) 
          {
            Console.WriteLine("Result from optimizer with index {0}: res {1}  trm {2}", idx, res, trm);
            t.set_Stream(mosek.streamtype.log, new msgclass(""));
            t.optimizersummary(mosek.streamtype.log);
            t.solutionsummary(mosek.streamtype.log);
          }
          else 
          {
            Console.WriteLine("All optimizers failed.");
          }
        }
      }
    }


    /**
       Defines a Mosek callback function whose only function
       is to indicate if the optimizer should be stopped.
     */
    private class CallbackProxy : mosek.Progress
    {
      private bool stop;
      public CallbackProxy()
      {
        stop = false;
      }

      public override int progressCB(mosek.callbackcode caller)
      {
        // Return non-zero implies terminate the optimizer
        return stop ? 1 : 0;
      }

      public bool Stop
      {
        get { return stop; }
        set { stop = value; }
      }
    }

    /**
       A simple stream handler.
     */
    class msgclass : mosek.Stream
    {
      public msgclass (string prfx) { }
      public override void streamCB (string msg)
      {
        Console.Write("{0}", msg);
      }
    }
  }
}