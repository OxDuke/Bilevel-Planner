/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      mioinitsol.cs

   Purpose:   Demonstrates how to solve a MIP with a start guess.

 */
using System;

namespace mosek.example
{
  class msgclass : mosek.Stream
  {
    string prefix;
    public msgclass (string prfx)
    {
      prefix = prfx;
    }

    public override void streamCB (string msg)
    {
      Console.Write ("{0}{1}", prefix, msg);
    }
  }

  public class mioinitsol
  {
    public static void Main ()
    {
      mosek.Env env = null;
      mosek.Task task = null;
      // Since the value infinity is never used, we define
      // 'infinity' symbolic purposes only
      double infinity = 0;

      int numvar = 4;
      int numcon = 1;
      int NUMINTVAR = 3;

      double[] c = { 7.0, 10.0, 1.0, 5.0 };

      mosek.boundkey[] bkc = {mosek.boundkey.up};
      double[] blc = { -infinity};
      double[] buc = {2.5};
      mosek.boundkey[] bkx = {mosek.boundkey.lo,
                              mosek.boundkey.lo,
                              mosek.boundkey.lo,
                              mosek.boundkey.lo
                             };
      double[] blx = {0.0,
                      0.0,
                      0.0,
                      0.0
                     };
      double[] bux = {infinity,
                      infinity,
                      infinity,
                      infinity
                     };

      int[] ptrb = {0, 1, 2, 3};
      int[] ptre = {1, 2, 3, 4};
      double[] aval = {1.0, 1.0, 1.0, 1.0};
      int[] asub = {0, 0, 0, 0};
      int[] intsub = {0, 1, 2};
      double[] xx  = new double[numvar];

      try
      {
        // Make mosek environment.
        env  = new mosek.Env ();
        // Create a task object linked with the environment env.
        task = new mosek.Task (env, numcon, numvar);
        // Directs the log task stream to the user specified
        // method task_msg_obj.streamCB
        task.set_Stream (mosek.streamtype.log, new msgclass ("[task]"));
        task.inputdata(numcon, numvar,
                       c,
                       0.0,
                       ptrb,
                       ptre,
                       asub,
                       aval,
                       bkc,
                       blc,
                       buc,
                       bkx,
                       blx,
                       bux);

        for (int j = 0 ; j < NUMINTVAR ; ++j)
          task.putvartype(intsub[j], mosek.variabletype.type_int);
        task.putobjsense(mosek.objsense.maximize);

        // Assign values to integer variables.
        // We only set a slice of xx
        double[] values = {1.0, 1.0, 0.0};
        task.putxxslice(mosek.soltype.itg, 0, 3, values);

        try
        {
          task.optimize();
          task.solutionsummary(mosek.streamtype.log);
        }
        catch (mosek.Warning w)
        {
          Console.WriteLine("Mosek warning:");
          Console.WriteLine (w.Code);
          Console.WriteLine (w);
        }
        
        task.getxx(mosek.soltype.itg, xx);

        Console.WriteLine("Solution:");
        for (int j = 0; j < numvar; ++j)
          Console.WriteLine ("x[{0}]:{1}", j, xx[j]);

        // Was the initial solution used?
        int constr = task.getintinf(mosek.iinfitem.mio_construct_solution);
        double constrVal = task.getdouinf(mosek.dinfitem.mio_construct_solution_obj);
        Console.WriteLine("Initial solution utilization: " + constr);
        Console.WriteLine("Initial solution objective: " +  constrVal);
      }
      catch (mosek.Exception e)
      {
        Console.WriteLine (e.Code);
        Console.WriteLine (e);
        throw;
      }

      if (task != null) task.Dispose ();
      if (env  != null)  env.Dispose ();
    }
  }
}