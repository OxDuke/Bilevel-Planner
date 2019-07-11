/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:      callback.cs

   Purpose:   To demonstrate how to use the progress 
              callback. 

              Use this script as follows:
              callback psim  25fv47.mps
              callback dsim  25fv47.mps
              callback intpnt 25fv47.mps

              The first argument tells which optimizer to use
              i.e. psim is primal simplex, dsim is dual simplex
              and intpnt is interior-point. 
*/
using mosek;
using System;

namespace mosek.example
{
  class myCallback : mosek.DataCallback
  {
    double maxtime;
    
    public myCallback(double maxtime_)
    {
      maxtime = maxtime_;
    }
    
    public override int callback( callbackcode caller, 
                                  double[]     douinf,
                                  int[]        intinf,
                                  long[]       lintinf )
    {
      double opttime = 0.0;
      int itrn;
      double pobj, dobj, stime;

      switch (caller)
      {
        case callbackcode.begin_intpnt:
          Console.WriteLine("Starting interior-point optimizer");
          break;
        case callbackcode.intpnt:
          itrn    = intinf[(int) iinfitem.intpnt_iter      ];
          pobj    = douinf[(int) dinfitem.intpnt_primal_obj];
          dobj    = douinf[(int) dinfitem.intpnt_dual_obj  ];
          stime   = douinf[(int) dinfitem.intpnt_time      ];
          opttime = douinf[(int) dinfitem.optimizer_time   ];
          
          Console.WriteLine("Iterations: {0,-3}",itrn);
          Console.WriteLine("  Elapsed: Time: {0,6:F2}({1:F2})",opttime,stime);
          Console.WriteLine("  Primal obj.: {0,-18:E6}  Dual obj.: {1,018:E6}e",pobj,dobj);
          break;
        case callbackcode.end_intpnt:
          Console.WriteLine("Interior-point optimizer finished.");
          break;
        case callbackcode.begin_primal_simplex:
          Console.WriteLine("Primal simplex optimizer started.");
          break;
        case callbackcode.update_primal_simplex:
          itrn    = intinf[(int) iinfitem.sim_primal_iter  ];
          pobj    = douinf[(int) dinfitem.sim_obj          ];
          stime   = douinf[(int) dinfitem.sim_time         ];
          opttime = douinf[(int) dinfitem.optimizer_time   ];
          
          Console.WriteLine("Iterations: {0,-3}}", itrn);
          Console.WriteLine("  Elapsed time: {0,6:F2}({1:F2})",opttime,stime);
          Console.WriteLine("  Obj.: {0,-18:E6}", pobj );
          break;
        case callbackcode.end_primal_simplex:
          Console.WriteLine("Primal simplex optimizer finished.");
          break;
        case callbackcode.begin_dual_simplex:
          Console.WriteLine("Dual simplex optimizer started.");
          break;
        case callbackcode.update_dual_simplex:
          itrn    = intinf[(int) iinfitem.sim_dual_iter    ];
          pobj    = douinf[(int) dinfitem.sim_obj          ];
          stime   = douinf[(int) dinfitem.sim_time         ];
          opttime = douinf[(int) dinfitem.optimizer_time   ];
          Console.WriteLine("Iterations: {0,-3}}", itrn);
          Console.WriteLine("  Elapsed time: {0,6:F2}({1:F2})",opttime,stime);
          Console.WriteLine("  Obj.: {0,-18:E6}", pobj );
          break;
        case callbackcode.end_dual_simplex:
          Console.WriteLine("Dual simplex optimizer finished.");
          break;
        case callbackcode.begin_bi:
          Console.WriteLine("Basis identification started.");
          break;
        case  callbackcode.end_bi:
          Console.WriteLine("Basis identification finished.");
          break;
        default:
          break;
      }

      if (opttime >= maxtime)
        // mosek is spending too much time. Terminate it.
        return 1;
                
      return 0;
    }
  }

  class myStream : Stream
  {
    public myStream () : base() { }
    
    public override void streamCB (string msg)
    {
      Console.Write ("{0}",msg);
    }
  }

  public class callback
  {
    public static void Main(string[] args)
    {
      string filename = "../data/25fv47.mps";
      string slvr     = "intpnt";

      if (args.Length < 2)
      {
          Console.WriteLine("Usage: callback ( psim | dsim | intpnt ) filename");
      }

      if (args.Length >= 1) slvr = args[0];
      if (args.Length >= 2) filename = args[1];

      using (Env env = new mosek.Env())
      {
        using (Task task = new Task(env,0,0))
        {
          task.readdata(filename);

          if (slvr == "psim")
            task.putintparam(iparam.optimizer, optimizertype.primal_simplex);
          else if (slvr == "dsim")
            task.putintparam(iparam.optimizer, optimizertype.dual_simplex);
          else if (slvr == "intpnt")
            task.putintparam(iparam.optimizer, optimizertype.intpnt);

          double maxtime = 0.06;
          task.set_InfoCallback(new myCallback(maxtime));
          task.optimize();

          task.solutionsummary(streamtype.msg);
        }
      }
    }
  }
}