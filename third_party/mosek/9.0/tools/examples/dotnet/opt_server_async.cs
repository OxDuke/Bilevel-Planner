/*
   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

   File:       opt_server_async.cs

   Purpose :   Demonstrates how to use MOSEK OptServer
               to solve optimization problem asynchronously

*/
using System;
using System.Threading;

namespace mosek.example
{
  class msgclass : mosek.Stream
  {
    public override void streamCB (string msg)
    {
      Console.Write ("{0}", msg);
    }
  }

  public class opt_server_async
  {
    public static void Main (string[] args)
    {
      if (args.Length == 0)
      {
        Console.WriteLine ("Missing argument, syntax is:");
        Console.WriteLine ("  opt_server inputfile host port numpolls");
      }
      else
      {

        string inputfile = args[0];
        string host      = args[1];
        string port      = args[2];
        int numpolls     = Convert.ToInt32(args[3]);

        using (mosek.Env env = new mosek.Env())
        {

          string token;

          using (mosek.Task task = new mosek.Task(env))
          {
            task.readdata (inputfile);
            token = task.asyncoptimize (host, port);
          }

          using (mosek.Task task = new mosek.Task(env))
          {
            task.readdata (inputfile);
            task.set_Stream (mosek.streamtype.log, new msgclass ());
            Console.WriteLine("Starting polling loop...");

            int i = 0;

            while ( true )
            {
              Thread.Sleep(500);
              Console.WriteLine("poll {0}...\n", i);

              mosek.rescode trm;
              mosek.rescode resp;

              int respavailable = task.asyncpoll( host,
                                                  port,
                                                  token,
                                                  out resp,
                                                  out trm);


              Console.WriteLine("polling done");

              if (respavailable != 0)
              {
                Console.WriteLine("solution available!");

                task.asyncgetresult(host,
                                    port,
                                    token,
                                    out resp,
                                    out trm);

                task.solutionsummary (mosek.streamtype.log);
                break;
              }

              if (i == numpolls)
              {
                Console.WriteLine("max num polls reached, stopping host.");
                task.asyncstop (host, port, token);
                break;
              }
              i++;
            }

          }
        }
      }
    }
  }
}