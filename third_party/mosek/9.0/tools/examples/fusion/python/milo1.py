##
#    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#    File:    milo1.py
#
#    Purpose:  Demonstrates how to solve a small mixed
#              integer linear optimization problem.
##

import sys
from mosek.fusion import *

def main(args):
    A = [[50.0, 31.0],
         [3.0, -2.0]]
    c = [1.0, 0.64]

    with Model('milo1') as M:

        x = M.variable('x', 2, Domain.integral(Domain.greaterThan(0.0)))

        # Create the constraints
        #      50.0 x[0] + 31.0 x[1] <= 250.0
        #       3.0 x[0] -  2.0 x[1] >= -4.0
        M.constraint('c1', Expr.dot(A[0], x), Domain.lessThan(250.0))
        M.constraint('c2', Expr.dot(A[1], x), Domain.greaterThan(-4.0))

        # Set max solution time
        M.setSolverParam('mioMaxTime', 60.0)
        # Set max relative gap (to its default value)
        M.setSolverParam('mioTolRelGap', 1e-4)
        # Set max absolute gap (to its default value)
        M.setSolverParam('mioTolAbsGap', 0.0)

        # Set the objective function to (c^T * x)
        M.objective('obj', ObjectiveSense.Maximize, Expr.dot(c, x))

        # Solve the problem
        M.solve()

        # Get the solution values
        print('[x0, x1] = ', x.level())
        print("MIP rel gap = %.2f (%f)" % (M.getSolverDoubleInfo(
            "mioObjRelGap"), M.getSolverDoubleInfo("mioObjAbsGap")))

if __name__ == '__main__':
    main(sys.argv[1:])