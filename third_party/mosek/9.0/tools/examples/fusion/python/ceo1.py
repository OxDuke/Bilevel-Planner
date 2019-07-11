##
#   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#   File:      ceo1.py
#
#   Purpose: Demonstrates how to solve the problem
#
#   minimize x1 + x2 
#   such that
#            x1 + x2 + x3  = 1.0
#                x1,x2    >= 0.0
#   and      x1 >= x2 * exp(x3/x2)
##

from mosek.fusion import *

with Model('ceo1') as M:

    x = M.variable('x', 3, Domain.unbounded())

    # Create the constraint
    #      x[0] + x[1] + x[2] = 1.0
    M.constraint("lc", Expr.sum(x), Domain.equalsTo(1.0))

    # Create the conic exponential constraint
    expc = M.constraint("expc", x, Domain.inPExpCone())

    # Set the objective function to (x[0] + x[1])
    M.objective("obj", ObjectiveSense.Minimize, Expr.sum(x.slice(0,2)))

    # Solve the problem
    M.solve()

    M.writeTask('ceo1.ptf')
    # Get the linear solution values
    solx = x.level()
    print('x1,x2,x3 = %s' % str(solx))

    # Get conic solution of expc
    expcval  = expc.level()
    expcdual = expc.dual()
    print('expc levels                = %s' % str(expcval))
    print('expc dual conic var levels = %s' % str(expcdual))