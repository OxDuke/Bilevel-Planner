##
#   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#   File:      pow1.py
#
#   Purpose: Demonstrates how to solve the problem
#
#     maximize x^0.2*y^0.8 + z^0.4 - x
#           st x + y + 0.5z = 2
#              x,y,z >= 0
#
##

from mosek.fusion import *

with Model('pow1') as M:

    x  = M.variable('x', 3, Domain.unbounded())
    x3 = M.variable()
    x4 = M.variable()

    # Create the linear constraint
    M.constraint(Expr.dot(x, [1.0, 1.0, 0.5]), Domain.equalsTo(2.0))

    # Create the power cone constraints
    M.constraint(Var.vstack(x.slice(0,2), x3), Domain.inPPowerCone(0.2))
    M.constraint(Expr.vstack(x.index(2), 1.0, x4), Domain.inPPowerCone(0.4))

    # Set the objective function
    M.objective(ObjectiveSense.Maximize, Expr.dot([1.0,1.0,-1.0], Var.vstack(x3, x4, x.index(0))))

    # Solve the problem
    M.solve()

    # Get the linear solution values
    solx = x.level()
    print('x,y,z = %s' % str(solx))