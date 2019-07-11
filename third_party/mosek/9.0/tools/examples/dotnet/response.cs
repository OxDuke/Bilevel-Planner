/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      response.cs

  Purpose:   This example demonstrates proper response handling
             for problems solved with the interior-point optimizers.
*/
using System;
using mosek;
using System.Text;

namespace mosek.example
{
  // A log handler class
  class msgclass : mosek.Stream
  {
    public msgclass () {}
    public override void streamCB (string msg) { Console.Write ("{0}", msg); }
  }

  public class response
  {
    public static void Main(string[] argv)
    {
      StringBuilder symname = new StringBuilder();
      StringBuilder desc = new StringBuilder();

      string filename;
      if (argv.Length >= 1) filename = argv[0];
      else                  filename = "../data/cqo1.mps";

      // Define environment and task
      using (Env env = new Env())
      {
        using (Task task = new Task(env, 0, 0))
        {
          try 
          {
            // (Optionally) set a log handler
            // task.set_Stream (streamtype.log, new msgclass ());

            // (Optionally) uncomment this to get solution status unknown
            // task.putintparam(iparam.intpnt_max_iterations, 1);

            // In this example we read data from a file
            task.readdata(filename);

            // Perform optimization
            rescode trm = task.optimize();
            task.solutionsummary(streamtype.log);

            // Handle solution status. We expect Optimal
            solsta solsta = task.getsolsta(soltype.itr);

            switch ( solsta )
            {
              case solsta.optimal:
                // Optimal solution. Print variable values
                Console.WriteLine("An optimal interior-point solution is located.");
                int numvar = task.getnumvar();
                double[] xx = new double[numvar];
                task.getxx(soltype.itr, xx);
                for(int i = 0; i < numvar; i++) 
                  Console.WriteLine("x[" + i + "] = " + xx[i]);
                break;

              case solsta.dual_infeas_cer:
                Console.WriteLine("Dual infeasibility certificate found.");
                break;

              case solsta.prim_infeas_cer:
                Console.WriteLine("Primal infeasibility certificate found.");
                break;

              case solsta.unknown:
                /* The solutions status is unknown. The termination code
                   indicates why the optimizer terminated prematurely. */
                Console.WriteLine("The solution status is unknown.");
                Env.getcodedesc(trm, symname, desc);
                Console.WriteLine("  Termination code: {0} {1}", symname, desc);
                break;

              default:
                Console.WriteLine("An unexpected solution status " + solsta);
                break;
            }
          }
          catch (mosek.Error e)
          {
            Console.WriteLine("Unexpected optimization error ({0}) {1}", e.Code, e.Message);
          }
        }
      }
    }
  }
}