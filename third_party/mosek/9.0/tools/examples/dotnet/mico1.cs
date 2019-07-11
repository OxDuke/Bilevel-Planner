/*
   Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File :      mico1.cs

   Purpose :   Demonstrates how to solve a small mixed
               integer conic optimization problem.

               minimize    x^2 + y^2
               subject to  x >= e^y + 3.8
                           x, y - integer
*/
using System;

namespace mosek.example
{
  public class MsgClass : mosek.Stream
  {
    public MsgClass () {}
    public override void streamCB (string msg)
    {
      Console.Write ("{0}", msg);
    }
  }

  public class mico1
  {
    public static void Main ()
    {
      mosek.Env env  = new mosek.Env ();
      mosek.Task task = new mosek.Task(env, 0, 0);
     
      // Directs the log task stream to the user specified
      // method task_msg_obj.streamCB
      MsgClass task_msg_obj = new MsgClass ();
      task.set_Stream (mosek.streamtype.log, task_msg_obj);

      task.appendvars(6);
      task.appendcons(3);
      task.putvarboundsliceconst(0, 6, mosek.boundkey.fr, -0.0, 0.0);

      // Integrality constraints
      task.putvartypelist(new int[]{1,2}, 
                          new mosek.variabletype[]{mosek.variabletype.type_int, mosek.variabletype.type_int});

      // Set up the three auxiliary linear constraints
      task.putaijlist(new int[]{0,0,1,2,2},
                      new int[]{1,3,4,2,5},
                      new double[]{-1,1,1,1,-1});
      task.putconboundslice(0, 3, 
                            new mosek.boundkey[]{mosek.boundkey.fx, mosek.boundkey.fx, mosek.boundkey.fx},
                            new double[]{-3.8, 1, 0}, new double[]{-3.8, 1, 0});

      // Objective
      task.putobjsense(mosek.objsense.minimize);
      task.putcj(0, 1);

      // Conic part of the problem
      task.appendconeseq(mosek.conetype.quad, 0, 3, 0);
      task.appendconeseq(mosek.conetype.pexp, 0, 3, 3);

      // Optimize the task
      task.optimize();
      task.solutionsummary(mosek.streamtype.msg);

      double[] xx = {0, 0};
      task.getxxslice(mosek.soltype.itg, 1, 3, xx);
      Console.WriteLine ("x = {0}, y = {1}", xx[0], xx[1]);
    }
  }
}