/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:    solutionquality.cs

  Purpose: To demonstrate how to examine the quality of a solution.
*/
using System;

namespace mosek.example
{
  class msgclass : mosek.Stream
  {
    string prefix;
    public msgclass(string prfx)
    {
      prefix = prfx;
    }

    public override void streamCB(string msg)
    {
      Console.Write("{0}{1}", prefix, msg);
    }
  }

  public class solutionquality
  {
    public static void Main(String[] args)
    {
      if (args.Length == 0)
      {
        Console.WriteLine("Missing argument, syntax is:");
        Console.WriteLine("  solutionquality inputfile");
      }
      else
      {
        using (mosek.Env env = new mosek.Env())
        {
          // Create a task object.
          using (mosek.Task task = new mosek.Task(env, 0, 0))
          {
            task.set_Stream(mosek.streamtype.log, new msgclass(""));
            try
            {

              // We assume that a problem file was given as the first command
              // line argument (received in `args')
              task.readdata(args[0]);

              // Solve the problem
              task.optimize();

              // Console.WriteLine (a summary of the solution
              task.solutionsummary(mosek.streamtype.log);

              mosek.solsta solsta;
              task.getsolsta(mosek.soltype.bas, out solsta);

              double pobj, pviolcon, pviolvar, pviolbarvar, pviolcones, pviolitg;
              double dobj, dviolcon, dviolvar, dviolbarvar, dviolcones;

              task.getsolutioninfo(mosek.soltype.bas,
                                   out pobj, out pviolcon, out pviolvar, out pviolbarvar, out pviolcones, out pviolitg,
                                   out dobj, out dviolcon, out dviolvar, out dviolbarvar, out dviolcones);

              switch (solsta)
              {
                case mosek.solsta.optimal:

                  double abs_obj_gap = Math.Abs(dobj - pobj);
                  double rel_obj_gap = abs_obj_gap / (1.0 + Math.Min(Math.Abs(pobj), Math.Abs(dobj)));
                  double max_primal_viol = Math.Max(pviolcon, pviolvar);
                  max_primal_viol = Math.Max(max_primal_viol, pviolbarvar);
                  max_primal_viol = Math.Max(max_primal_viol, pviolcones);

                  double max_dual_viol = Math.Max(dviolcon, dviolvar);
                  max_dual_viol = Math.Max(max_dual_viol, dviolbarvar);
                  max_dual_viol = Math.Max(max_dual_viol, dviolcones);

                  // Assume the application needs the solution to be within
                  //    1e-6 ofoptimality in an absolute sense. Another approach
                  //   would be looking at the relative objective gap

                  Console.WriteLine("Customized solution information.\n");
                  Console.WriteLine("  Absolute objective gap: " + abs_obj_gap);
                  Console.WriteLine("  Relative objective gap: " + rel_obj_gap);
                  Console.WriteLine("  Max primal violation  : " + max_primal_viol);
                  Console.WriteLine("  Max dual violation    : " + max_dual_viol);

                  bool accepted = true;

                  if (rel_obj_gap > 1e-6)
                  {
                    Console.WriteLine("Warning: The relative objective gap is LARGE.");
                    accepted = false;
                  }

                  // We will accept a primal infeasibility of 1e-8 and
                  // dual infeasibility of 1e-6. These number should chosen problem
                  // dependent.
                  if (max_primal_viol > 1e-8)
                  {
                    Console.WriteLine("Warning: Primal violation is too LARGE");
                    accepted = false;
                  }

                  if (max_dual_viol > 1e-6)
                  {
                    Console.WriteLine("Warning: Dual violation is too LARGE.");
                    accepted = false;
                  }

                  if (accepted)
                  {
                    int numvar = task.getnumvar();
                    double[] xx = new double[numvar];
                    Console.WriteLine("Optimal primal solution");
                    task.getxx(mosek.soltype.bas, xx);
                    for (int j = 0; j < numvar; j++)
                      Console.WriteLine("x[{0}]: {1}", j, xx[j]);
                  }
                  else
                  {
                    // print etailed information about the solution
                    task.analyzesolution(mosek.streamtype.log, mosek.soltype.bas);
                  }
                  break;

                case mosek.solsta.dual_infeas_cer:
                case mosek.solsta.prim_infeas_cer:
                  Console.WriteLine("Primal or dual infeasibility certificate found.");
                  break;

                case mosek.solsta.unknown:
                  Console.WriteLine("The status of the solution is unknown.");
                  break;

                default:
                  Console.WriteLine("Other solution status");
                  break;
              }
            }
            catch (mosek.Exception e)
            {
              Console.WriteLine("\nAn error occourred: " + e.Code);
              Console.WriteLine(e);
              //throw;
            }
          }
        }
      }
    }
  }
}