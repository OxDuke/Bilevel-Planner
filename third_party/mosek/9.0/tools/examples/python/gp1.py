##
#   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#   File:      gp1.py
#
#   Purpose:   Demonstrates how to solve a simple Geometric Program (GP)
#              cast into conic form with exponential cones and log-sum-exp.
#
#              Example from
#                https://gpkit.readthedocs.io/en/latest/examples.html#maximizing-the-volume-of-a-box
#
from numpy import log, exp
from mosek import *
import sys

# Since the value of infinity is ignored, we define it solely
# for symbolic purposes
inf = 0.0

# Define a stream printer to grab output from MOSEK
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()

# maximize     h*w*d
# subjecto to  2*(h*w + h*d) <= Awall
#              w*d <= Afloor
#              alpha <= h/w <= beta
#              gamma <= d/w <= delta
#
# Variable substitutions:  h = exp(x), w = exp(y), d = exp(z).
#
# maximize     x+y+z
# subject      log( exp(x+y+log(2/Awall)) + exp(x+z+log(2/Awall)) ) <= 0
#                              y+z <= log(Afloor)
#              log( alpha ) <= x-y <= log( beta )
#              log( gamma ) <= z-y <= log( delta )
def max_volume_box(Aw, Af, alpha, beta, gamma, delta):
    # Basic dimensions of our problem
    numvar    = 3  # Variables in original problem
    numLinCon = 3  # Linear constraints in original problem
    numExp    = 2  # Number of exp-terms in the log-sum-exp constraint

    # Linear part of the problem
    cval  = [1, 1, 1]
    asubi = [0, 0, 1, 1, 2, 2]
    asubj = [1, 2, 0, 1, 2, 1]
    aval  = [1.0, 1.0, 1.0, -1.0, 1.0, -1.0]
    bkc   = [boundkey.up, boundkey.ra, boundkey.ra]
    blc   = [-inf, log(alpha), log(gamma)]
    buc   = [log(Af), log(beta), log(delta)]

    # Linear part setting up slack variables
    # for the linear expressions appearing inside exps
    # x_5 - x - y = log(2/Awall)
    # x_8 - x - z = log(2/Awall)
    # The slack indexes are convenient for defining exponential cones, see later
    a2subi = [3, 3, 3, 4, 4, 4]
    a2subj = [5, 0, 1, 8, 0, 2]
    a2val  = [1.0, -1.0, -1.0, 1.0, -1.0, -1.0]
    b2kc   = [boundkey.fx, boundkey.fx]
    b2luc  = [log(2/Aw), log(2/Aw)]

    with Env() as env:
        with env.Task(0, 0) as task:
            task.set_Stream(streamtype.log, streamprinter)
            
            # Add variables and constraints
            task.appendvars(numvar + 3*numExp)
            task.appendcons(numLinCon + numExp + 1)

            # Objective is the sum of three first variables
            task.putobjsense(objsense.maximize)
            task.putcslice(0, numvar, cval)
            task.putvarboundsliceconst(0, numvar, boundkey.fr, -inf, inf)

            # Add the three linear constraints
            task.putaijlist(asubi, asubj, aval)
            task.putconboundslice(0, numvar, bkc, blc, buc)

            # Add linear constraints for the expressions appearing in exp(...)
            task.putaijlist(a2subi, a2subj, a2val)
            task.putconboundslice(numLinCon, numLinCon+numExp, b2kc, b2luc, b2luc)

            c = numLinCon + numExp
            expStart = numvar
            # Add a single log-sum-exp constraint sum(log(exp(z_i))) <= 0
            # Assume numExp variable triples are ordered as (u0,t0,z0,u1,t1,z1...)
            # starting from variable with index expStart

            # sum(u_i) = 1 as constraint number c, u_i unbounded
            task.putarow(c, range(expStart, expStart + 3*numExp, 3), [1.0]*numExp)
            task.putconbound(c, boundkey.fx, 1.0, 1.0)
            task.putvarboundlistconst(range(expStart, expStart + 3*numExp, 3), 
                                      boundkey.fr, -inf, inf)

            # z_i unbounded
            task.putvarboundlistconst(range(expStart + 2, expStart + 2 + 3*numExp, 3), 
                                      boundkey.fr, -inf, inf)            

            # t_i = 1
            task.putvarboundlistconst(range(expStart + 1, expStart + 1 + 3*numExp, 3), 
                                      boundkey.fx, 1.0, 1.0)

            # Every triple is in an exponential cone
            task.appendconesseq([conetype.pexp]*numExp, [0.0]*numExp, [3]*numExp, expStart)

            # Solve and map to original h, w, d
            task.optimize()
            xyz = [0.0]*numvar
            task.getxxslice(soltype.itr, 0, numvar, xyz)
            return exp(xyz)

Aw, Af, alpha, beta, gamma, delta = 200.0, 50.0, 2.0, 10.0, 2.0, 10.0
h,w,d = max_volume_box(Aw, Af, alpha, beta, gamma, delta) 
print("h={0:.3f}, w={1:.3f}, d={2:.3f}".format(h, w, d))