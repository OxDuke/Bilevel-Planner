//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      gp1.cs
//
//   Purpose:   Demonstrates how to solve a simple Geometric Program (GP)
//              cast into conic form with exponential cones and log-sum-exp.
//
//              Example from
//                https://gpkit.readthedocs.io/en/latest/examples.html//maximizing-the-volume-of-a-box
//
using System;
using mosek;

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

  public class gp1
  {
    // Since the value of infinity is ignored, we define it solely
    // for symbolic purposes
    static double inf = 0.0;

    // maximize     h*w*d
    // subjecto to  2*(h*w + h*d) <= Awall
    //              w*d <= Afloor
    //              alpha <= h/w <= beta
    //              gamma <= d/w <= delta
    //
    // Variable substitutions:  h = exp(x), w = exp(y), d = exp(z).
    //
    // maximize     x+y+z
    // subject      log( exp(x+y+log(2/Awall)) + exp(x+z+log(2/Awall)) ) <= 0
    //                              y+z <= log(Afloor)
    //              log( alpha ) <= x-y <= log( beta )
    //              log( gamma ) <= z-y <= log( delta )
    public static double[] max_volume_box(double Aw, double Af, 
                                          double alpha, double beta, double gamma, double delta)
    {
      // Basic dimensions of our problem
      int numvar    = 3;  // Variables in original problem
      int numLinCon = 3;  // Linear constraints in original problem
      int numExp    = 2;  // Number of exp-terms in the log-sum-exp constraint

      // Linear part of the problem
      double[] cval  = {1, 1, 1};
      int[]    asubi = {0, 0, 1, 1, 2, 2};
      int[]    asubj = {1, 2, 0, 1, 2, 1};
      double[] aval  = {1.0, 1.0, 1.0, -1.0, 1.0, -1.0};
      boundkey[] bkc = {boundkey.up, boundkey.ra, boundkey.ra};
      double[] blc   = {-inf, Math.Log(alpha), Math.Log(gamma)};
      double[] buc   = {Math.Log(Af), Math.Log(beta), Math.Log(delta)};

      // Linear part setting up slack variables
      // for the linear expressions appearing inside exps
      // x_5 - x - y = log(2/Awall)
      // x_8 - x - z = log(2/Awall)
      // The slack indexes are convenient for defining exponential cones, see later
      int[]    a2subi = {3, 3, 3, 4, 4, 4};
      int[]    a2subj = {5, 0, 1, 8, 0, 2};
      double[] a2val  = {1.0, -1.0, -1.0, 1.0, -1.0, -1.0};
      boundkey[] b2kc = {boundkey.fx, boundkey.fx};
      double[] b2luc  = {Math.Log(2/Aw), Math.Log(2/Aw)};

      using (Env env = new Env())
      {
        using (Task task = new Task(env, 0, 0))
        {
          // Directs the log task stream to the user specified
          // method task_msg_obj.stream
          task.set_Stream (mosek.streamtype.log, new msgclass (""));

          // Add variables and constraints
          task.appendvars(numvar + 3*numExp);
          task.appendcons(numLinCon + numExp + 1);

          // Objective is the sum of three first variables
          task.putobjsense(objsense.maximize);
          task.putcslice(0, numvar, cval);
          task.putvarboundsliceconst(0, numvar, boundkey.fr, -inf, inf);

          // Add the three linear constraints
          task.putaijlist(asubi, asubj, aval);
          task.putconboundslice(0, numvar, bkc, blc, buc);

          // Add linear constraints for the expressions appearing in exp(...)
          task.putaijlist(a2subi, a2subj, a2val);
          task.putconboundslice(numLinCon, numLinCon+numExp, b2kc, b2luc, b2luc);

          int c = numLinCon + numExp;
          int expStart = numvar;
          // Add a single log-sum-exp constraint sum(log(exp(z_i))) <= 0
          // Assume numExp variable triples are ordered as (u0,t0,z0,u1,t1,z1...)
          // starting from variable with index expStart
          double[]  val = new double[numExp];
          int[]     sub = new int[numExp];

          // sum(u_i) = 1 as constraint number c, u_i unbounded
          for(int i = 0; i < numExp; i++)
            { sub[i] = expStart + 3*i; val[i] = 1.0; }
          task.putarow(c, sub, val);
          task.putconbound(c, boundkey.fx, 1.0, 1.0);
          task.putvarboundlistconst(sub, boundkey.fr, -inf, inf);

          // z_i unbounded
          for(int i = 0; i < numExp; i++) sub[i] = expStart + 3*i + 2;
          task.putvarboundlistconst(sub, boundkey.fr, -inf, inf);

          // t_i = 1
          for(int i = 0; i < numExp; i++) sub[i] = expStart + 3*i + 1; 
          task.putvarboundlistconst(sub, boundkey.fx, 1, 1);

          // Every triple is in an exponential cone
          conetype[] ct = new conetype[numExp];
          int[]     len = new int[numExp];
          for(int i = 0; i < numExp; i++) 
            { ct[i] = conetype.pexp; val[i] = 0.0; len[i] = 3; }
          task.appendconesseq(ct, val, len, expStart);

          // Solve and map to original h, w, d
          task.optimize();
          double[] xyz = new double[numvar];
          double[] hwd = new double[numvar];
          task.getxxslice(soltype.itr, 0, numvar, xyz);
          for(int i = 0; i < numvar; i++) hwd[i] = Math.Exp(xyz[i]);
          return hwd;
        }
      }
    }
    
    public static void Main(String[] args)
    {
      double Aw    = 200.0;
      double Af    = 50.0;
      double alpha = 2.0;
      double beta  = 10.0;
      double gamma = 2.0;
      double delta = 10.0;
      
      double[] hwd = max_volume_box(Aw, Af, alpha, beta, gamma, delta);

      Console.WriteLine("h={0:f4} w={1:f4} d={2:f4}", hwd[0], hwd[1], hwd[2]);
    }
  }
}