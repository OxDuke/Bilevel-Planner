//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      callback.cs
//
//   Purpose:   To demonstrate how to use the progress
//              callback.
//
//              Use this script as follows:
//              callback psim
//              callback dsim
//              callback intpnt
//
//              The first argument tells which optimizer to use
//              i.e. psim is primal simplex, dsim is dual simplex
//              and intpnt is interior-point.
//

using System;
using mosek.fusion;

namespace mosek.fusion.example
{
  class myCallback : mosek.DataCallback
  {
    double maxtime;
    Model M;

    public myCallback( double maxtime_,
                       Model M_)
    {
      maxtime = maxtime_;
      M = M_;
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

          Console.WriteLine("Iterations: {0,-3}", itrn);
          Console.WriteLine("  Elapsed: Time: {0,6:F2}({1:F2})", opttime, stime);
          Console.WriteLine("  Primal obj.: {0,-18:E6}  Dual obj.: {1,018:E6}e", pobj, dobj);
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
          Console.WriteLine("  Elapsed time: {0,6:F2}({1:F2})", opttime, stime);
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
          Console.WriteLine("  Elapsed time: {0,6:F2}({1:F2})", opttime, stime);
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
      {
        Console.WriteLine("MOSEK is spending too much time. Terminate it.");
        return 1;
      }
      return 0;
    }
  }

  class myProgressCallback : mosek.Progress
  {
    public override int progressCB( callbackcode caller )
    {
      //Handle caller code here
      return 0;
    }
  }

  public class callback
  {
    public static void Main(string[] args)
    {
      string slvr     = "intpnt";

      if (args.Length < 1)
      {
        Console.WriteLine("Usage: callback ( psim | dsim | intpnt )");
      }

      if (args.Length >= 1) slvr = args[0];

      // We create a large linear problem
      int n = 150;
      int m = 700;
      double[] A = new double[m * n];
      double[] b = new double[m];
      double[] c = new double[n];

      Random rnd = new Random();
      for (int i = 0; i < m * n; i++) A[i] = rnd.NextDouble();
      for (int i = 0; i < m; i++)   b[i] = rnd.NextDouble();
      for (int i = 0; i < n; i++)   c[i] = rnd.NextDouble();

      double maxtime = 0.07;

      Model M = new Model("callback");

      Variable x = M.Variable(n, Domain.Unbounded());
      M.Constraint(Expr.Mul(Matrix.Dense(m, n, A), x), Domain.LessThan(b));
      M.Objective(ObjectiveSense.Maximize, Expr.Dot(c, x));

      if ( slvr == "psim")
        M.SetSolverParam("optimizer", "primalSimplex");
      else if ( slvr == "dsim")
        M.SetSolverParam("optimizer", "dualSimplex");
      else if ( slvr == "intpnt")
        M.SetSolverParam("optimizer", "intpnt");

      M.SetDataCallbackHandler( new myCallback(maxtime, M) );

      M.Solve();
    }
  }
}