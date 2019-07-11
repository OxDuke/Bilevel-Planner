##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      opt_server_async.py
#
#  Purpose :   Demonstrates how to use MOSEK OptServer
#              to solve optimization problem asynchronously
##
import mosek
import sys
import time

def streamprinter(msg):
    sys.stdout.write(msg)
    sys.stdout.flush()

if len(sys.argv) != 5:
    print("Missing argument, syntax is:")
    print("  opt-server-async inputfile host port numpolls")
else:

    filename = sys.argv[1]
    host = sys.argv[2]
    port = sys.argv[3]
    numpolls = int(sys.argv[4])
    token = None

    with mosek.Env() as env:

        with env.Task(0, 0) as task:

            print("reading task from file")
            task.readdata(filename)

            print("Solve the problem remotely (async)")
            token = task.asyncoptimize(host, port)

        print("Task token: %s" % token)

        with env.Task(0, 0) as task:

            task.readdata(filename)

            task.set_Stream(mosek.streamtype.log, streamprinter)

            i = 0

            while i < numpolls:

                time.sleep(0.1)

                print("poll %d..." % i)
                respavailable, res, trm = task.asyncpoll(host,
                                                         port,
                                                         token)

                print("done!")

                if respavailable:
                    print("solution available!")

                    respavailable, res, trm = task.asyncgetresult(host,
                                                                  port,
                                                                  token)

                    task.solutionsummary(mosek.streamtype.log)
                    break

                i = i + 1

                if i == numpolls:
                    print("max number of polls reached, stopping host.")
                    task.asyncstop(host, port, token)