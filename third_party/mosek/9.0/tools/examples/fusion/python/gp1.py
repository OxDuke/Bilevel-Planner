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
from numpy import log, exp, array
from mosek.fusion import *
import sys

# Models log(sum(exp(Ax+b))) <= 0.
# Each row of [A b] describes one of the exp-terms
def logsumexp(M, A, x, b):    
    k = int(A.shape[0])
    u = M.variable(k)
    M.constraint(Expr.sum(u), Domain.equalsTo(1.0))
    M.constraint(Expr.hstack(u,
                             Expr.constTerm(k, 1.0),
                             Expr.add(Expr.mul(A, x), b)), Domain.inPExpCone())

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
    with Model('max_vol_box') as M:
        xyz = M.variable(3)
        M.objective('Objective', ObjectiveSense.Maximize, Expr.sum(xyz))
        
        logsumexp(M, array([[1,1,0],[1,0,1]]), xyz, array([log(2.0/Aw), log(2.0/Aw)]))
        
        M.constraint(Expr.dot([0, 1, 1], xyz), Domain.lessThan(log(Af)))
        M.constraint(Expr.dot([1,-1, 0], xyz), Domain.inRange(log(alpha),log(beta)))
        M.constraint(Expr.dot([0,-1, 1], xyz), Domain.inRange(log(gamma),log(delta)))
        
        M.setLogHandler(sys.stdout)
        M.solve()
        
        return exp(xyz.level())

Aw, Af, alpha, beta, gamma, delta = 200.0, 50.0, 2.0, 10.0, 2.0, 10.0
h,w,d = max_volume_box(Aw, Af, alpha, beta, gamma, delta) 
print("h={0:.3f}, w={1:.3f}, d={2:.3f}".format(h, w, d))