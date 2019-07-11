/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:    milo1.cs

   Purpose:  Demonstrates how to solve a small mixed
             integer linear optimization problem using the MOSEK C# API.
 */

using System;

namespace mosek.example
{
  public class MsgClass : mosek.Stream
  {
    public MsgClass ()
    {
      /* Construct the object */
    }

    public override void streamCB (string msg)
    {
      Console.Write ("{0}", msg);
    }
  }

  public class milo1
  {
    public static void Main ()
    {
      const int numcon = 2;
      const int numvar = 2;

      // Since the value infinity is never used, we define
      // 'infinity' symbolic purposes only
      double infinity = 0;


      mosek.boundkey[] bkc = { mosek.boundkey.up,
                               mosek.boundkey.lo
                             };
      double[] blc = { -infinity,
                       -4.0
                     };
      double[] buc = { 250.0,
                       infinity
                     };

      mosek.boundkey[] bkx = { mosek.boundkey.lo,
                               mosek.boundkey.lo
                             };
      double[] blx = { 0.0,
                       0.0
                     };
      double[] bux = { infinity,
                       infinity
                     };

      double[] c   = {1.0, 0.64 };
      int[][] asub    = { new int[]  {0,   1},  new int[] {0,    1}   };
      double[][] aval = { new double[] {50.0, 3.0}, new double[] {31.0, -2.0} };

      double[] xx  = new double[numvar];

      mosek.Env env = null;
      mosek.Task task = null;

      try
      {
        // Make mosek environment.
        env  = new mosek.Env ();
        // Create a task object linked with the environment env.
        task = new mosek.Task (env, numcon, numvar);
        // Directs the log task stream to the user specified
        // method task_msg_obj.streamCB
        MsgClass task_msg_obj = new MsgClass ();
        task.set_Stream (mosek.streamtype.log, task_msg_obj);

        /* Give MOSEK an estimate of the size of the input data.
             This is done to increase the speed of inputting data.
             However, it is optional. */
        /* Append 'numcon' empty constraints.
             The constraints will initially have no bounds. */
        task.appendcons(numcon);

        /* Append 'numvar' variables.
             The variables will initially be fixed at zero (x=0). */
        task.appendvars(numvar);

        /* Optionally add a constant term to the objective. */
        task.putcfix(0.0);

        for (int j = 0; j < numvar; ++j)
        {
          /* Set the linear term c_j in the objective.*/
          task.putcj(j, c[j]);
          /* Set the bounds on variable j.
                   blx[j] <= x_j <= bux[j] */
          task.putvarbound(j, bkx[j], blx[j], bux[j]);
          /* Input column j of A */
          task.putacol(j,                     /* Variable (column) index.*/
                       asub[j],               /* Row index of non-zeros in column j.*/
                       aval[j]);              /* Non-zero Values of column j. */
        }
        /* Set the bounds on constraints.
               for i=1, ...,numcon : blc[i] <= constraint i <= buc[i] */
        for (int i = 0; i < numcon; ++i)
          task.putconbound(i, bkc[i], blc[i], buc[i]);

        /* Specify integer variables. */
        for (int j = 0; j < numvar; ++j)
          task.putvartype(j, mosek.variabletype.type_int);
        task.putobjsense(mosek.objsense.maximize);

        /* Set max solution time */
        task.putdouparam(mosek.dparam.mio_max_time, 60.0);

        task.optimize();


        // Print a summary containing information
        //   about the solution for debugging purposes
        task.solutionsummary(mosek.streamtype.msg);

        mosek.solsta solsta;
        /* Get status information about the solution */
        task.getsolsta(mosek.soltype.itg, out solsta);
        task.getxx(mosek.soltype.itg, // Integer solution.
                   xx);

        switch (solsta)
        {
          case mosek.solsta.optimal:
            Console.WriteLine ("Optimal primal solution\n");
            for (int j = 0; j < numvar; ++j)
              Console.WriteLine ("x[{0}]:", xx[j]);
            break;
          case mosek.solsta.prim_feas:
            Console.WriteLine ("Feasible primal solution\n");
            for (int j = 0; j < numvar; ++j)
              Console.WriteLine ("x[{0}]:", xx[j]);
            break;
          case mosek.solsta.unknown:
            mosek.prosta prosta;
            task.getprosta(mosek.soltype.itg, out prosta);
            switch (prosta)
            {
              case mosek.prosta.prim_infeas_or_unbounded:
                Console.WriteLine("Problem status Infeasible or unbounded");
                break;
              case mosek.prosta.prim_infeas:
                Console.WriteLine("Problem status Infeasible.");
                break;
              case mosek.prosta.unknown:
                Console.WriteLine("Problem status unknown.");
                break;
              default:
                Console.WriteLine("Other problem status.");
                break;
            }
            break;
          default:
            Console.WriteLine("Other solution status");
            break;
        }
      }
      catch (mosek.Exception e)
      {
        Console.WriteLine (e.Code);
        Console.WriteLine (e);
        throw;
      }
      finally
      {
        if (task != null) task.Dispose ();
        if (env  != null)  env.Dispose ();
      }
    }
  }
}