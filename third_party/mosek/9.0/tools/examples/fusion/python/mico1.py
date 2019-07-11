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

import sys
from mosek.fusion import *

with Model('mico1') as M:

    x = M.variable(Domain.integral(Domain.unbounded()))
    y = M.variable(Domain.integral(Domain.unbounded()))
    t = M.variable()

    M.constraint(Expr.vstack(t, x, y), Domain.inQCone())
    M.constraint(Expr.vstack(Expr.sub(x, 3.8), 1, y), Domain.inPExpCone())

    M.objective(ObjectiveSense.Minimize, t)

    M.setLogHandler(sys.stdout)
    M.solve()

    print('Solution: x = {0}, y = {1}'.format(x.level()[0], y.level()[0]))