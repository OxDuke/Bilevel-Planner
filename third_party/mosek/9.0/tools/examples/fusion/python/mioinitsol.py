##
#    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#    File:    mioinitsol.py
#
#    Purpose:  Demonstrates how to solve a small mixed
#              integer linear optimization problem
#              providing an initial feasible solution.
##
import sys
from mosek.fusion import *

def main(args):
    c = [7.0, 10.0, 1.0, 5.0]

    with Model('mioinitsol') as M:
        n = 4

        x = M.variable('x', n, Domain.greaterThan(0.0))
        x.slice(0,3).makeInteger()

        M.constraint(Expr.sum(x), Domain.lessThan(2.5))

        # Set the objective function to (c^T * x)
        M.objective('obj', ObjectiveSense.Maximize, Expr.dot(c, x))

        # Assign values to integer variables.
        # We only set a slice of xx
        init_sol = [1.0, 1.0, 0.0]
        x.slice(0,3).setLevel(init_sol)

        # Solve the problem
        M.setLogHandler(sys.stdout)
        M.solve()

        # Get the solution values
        ss = M.getPrimalSolutionStatus()
        print("Solution status: {0}".format(ss))
        sol = x.level()
        print('x = {0}'.format(sol))

        # Was the initial solution used?
        constr = M.getSolverIntInfo("mioConstructSolution")
        constrVal = M.getSolverDoubleInfo("mioConstructSolutionObj")
        print("Initial solution utilization: {0}\nInitial solution objective: {1:.3f}\n".format(constr, constrVal))                    

if __name__ == '__main__':
    main(sys.argv[1:])