/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      feasrepairex1.cs

  Purpose:    To demonstrate how to use the MSK_relaxprimal function to
              locate the cause of an infeasibility.

  Syntax: On command line
          feasrepairex1 feasrepair.lp
          feasrepair.lp is located in mosek\<version>\tools\examples.
*/
using System;

namespace mosek.example
{
  class msgclass : mosek.Stream
  {
    public msgclass () {}

    public override void streamCB (string msg)
    {
      Console.Write ("{1}", msg);
    }
  }

  public class feasrepairex1
  {
    public static void Main (String[] args)
    {
      string filename = "../data/feasrepair.lp";
      if (args.Length >= 1) filename = args[0];

      using (mosek.Env env = new mosek.Env())
      {
        using (mosek.Task task = new mosek.Task(env, 0, 0))
        {
          task.set_Stream (mosek.streamtype.log, new msgclass());

          task.readdata(filename);

          task.putintparam(mosek.iparam.log_feas_repair, 3);

          task.primalrepair(null, null, null, null);

          double sum_viol = task.getdouinf(mosek.dinfitem.primal_repair_penalty_obj);

          Console.WriteLine("Minimized sum of violations = %{0}", sum_viol);

          task.optimize();
          task.solutionsummary(mosek.streamtype.msg);
        }
      }
    }
  }
}