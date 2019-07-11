##
#   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#   File:      cqo1.py
#
#   Purpose: Demonstrates how to solve the problem
#
#   minimize y1 + y2 + y3
#   such that
#            x1 + x2 + 2.0 x3  = 1.0
#                    x1,x2,x3 >= 0.0
#   and
#            (y1,x1,x2) in C_3,
#            (y2,y3,x3) in K_3
#
#   where C_3 and K_3 are respectively the quadratic and
#   rotated quadratic cone of size 3 defined as
#       C_3 = { z1,z2,z3 :      z1 >= sqrt(z2^2 + z3^2) }
#       K_3 = { z1,z2,z3 : 2 z1 z2 >= z3^2              }
##

from mosek.fusion import *

with Model('cqo1') as M:

    x = M.variable('x', 3, Domain.greaterThan(0.0))
    y = M.variable('y', 3, Domain.unbounded())

    # Create the aliases
    #      z1 = [ y[0],x[0],x[1] ]
    #  and z2 = [ y[1],y[2],x[2] ]
    z1 = Var.vstack(y.index(0), x.slice(0, 2))
    z2 = Var.vstack(y.slice(1, 3), x.index(2))

    # Create the constraint
    #      x[0] + x[1] + 2.0 x[2] = 1.0
    M.constraint("lc", Expr.dot([1.0, 1.0, 2.0], x), Domain.equalsTo(1.0))

    # Create the constraints
    #      z1 belongs to C_3
    #      z2 belongs to K_3
    # where C_3 and K_3 are respectively the quadratic and
    # rotated quadratic cone of size 3, i.e.
    #                 z1[0] >= sqrt(z1[1]^2 + z1[2]^2)
    #  and  2.0 z2[0] z2[1] >= z2[2]^2
    qc1 = M.constraint("qc1", z1, Domain.inQCone())
    qc2 = M.constraint("qc2", z2, Domain.inRotatedQCone())

    # Set the objective function to (y[0] + y[1] + y[2])
    M.objective("obj", ObjectiveSense.Minimize, Expr.sum(y))

    # Solve the problem
    M.solve()
    M.writeTask('cqo1.opf')

    # Get the linear solution values
    solx = x.level()
    soly = y.level()
    print('x1,x2,x3 = %s' % str(solx))
    print('y1,y2,y3 = %s' % str(soly))

    # Get conic solution of qc1
    qc1lvl = qc1.level()
    qc1sn = qc1.dual()
    print('qc1 levels                = %s' % str(qc1lvl))
    print('qc1 dual conic var levels = %s' % str(qc1sn))