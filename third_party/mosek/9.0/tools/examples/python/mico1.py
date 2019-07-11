##
#    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#    File:    mico1.py
#
#    Purpose:  Demonstrates how to solve a small mixed
#              integer conic optimization problem.
#
#              minimize    x^2 + y^2
#              subject to  x >= e^y + 3.8
#                          x, y - integer
##
import mosek, sys

# Define a stream printer to grab output from MOSEK
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()

with mosek.Env() as env:
    with env.Task(0, 0) as task:
        task.set_Stream(mosek.streamtype.log, streamprinter)

        task.appendvars(6)
        task.appendcons(3)
        task.putvarboundsliceconst(0, 6, mosek.boundkey.fr, -0.0, 0.0)

        # Integrality constraints
        task.putvartypelist([1,2], [mosek.variabletype.type_int]*2)

        # Set up the three auxiliary linear constraints
        task.putaijlist([0,0,1,2,2],
                        [1,3,4,2,5],
                        [-1,1,1,1,-1])
        task.putconboundslice(0, 3, [mosek.boundkey.fx]*3, [-3.8, 1, 0], [-3.8, 1, 0])

        # Objective
        task.putobjsense(mosek.objsense.minimize)
        task.putcj(0, 1)

        # Conic part of the problem
        task.appendconesseq([mosek.conetype.quad, mosek.conetype.pexp], [0, 0], [3, 3], 0)

        # Optimize the task
        task.optimize()
        task.solutionsummary(mosek.streamtype.msg)

        xx = [0, 0]
        task.getxxslice(mosek.soltype.itg, 1, 3, xx)
        print(xx)