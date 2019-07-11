/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:      parameters.cs

 Purpose: Demonstrates a very simple example about how to get/set
 parameters with MOSEK .NET API

*/

using System;

namespace mosek.example
{
  public class parameters
  {
    public static void Main()
    {
      using (mosek.Env env = new mosek.Env())
      {
        using (mosek.Task task = new mosek.Task(env, 0, 0))
        {
          Console.WriteLine("Test MOSEK parameter get/set functions");

          // Set log level (integer parameter)
          task.putintparam(mosek.iparam.log, 1);
          // Select interior-point optimizer... (integer parameter)
          task.putintparam(mosek.iparam.optimizer, mosek.optimizertype.intpnt);
          // ... without basis identification (integer parameter)
          task.putintparam(mosek.iparam.intpnt_basis, mosek.basindtype.never);
          // Set relative gap tolerance (double parameter)
          task.putdouparam(mosek.dparam.intpnt_co_tol_rel_gap, 1.0e-7);

          // The same using explicit string names 
          task.putparam     ("MSK_DPAR_INTPNT_CO_TOL_REL_GAP", "1.0e-7");      
          task.putnadouparam("MSK_DPAR_INTPNT_CO_TOL_REL_GAP",  1.0e-7 );      

          // Incorrect value
          try 
          {
            task.putdouparam(mosek.dparam.intpnt_co_tol_rel_gap, -1.0);
          } 
          catch (mosek.Error) 
          {
            Console.WriteLine("Wrong parameter value"); 
          }


          double param = task.getdouparam(mosek.dparam.intpnt_co_tol_rel_gap);
          Console.WriteLine("Current value for parameter intpnt_co_tol_rel_gap = " + param);

          /* Define and solve an optimization problem here */
          /* task.optimize() */
          /* After optimization: */

          Console.WriteLine("Get MOSEK information items");

          double tm = task.getdouinf(mosek.dinfitem.optimizer_time);
          int  iter = task.getintinf(mosek.iinfitem.intpnt_iter);     
           
          Console.WriteLine("Time: " + tm);
          Console.WriteLine("Iterations: " + iter);         
        }
      }
    }
  }
}