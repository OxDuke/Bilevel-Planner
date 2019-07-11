##
#    Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#    File:    lpt.py
#
#    Purpose:  Demonstrates how to solve the multi-processor
#              scheduling problem using the Fusion API.
##


import sys
import random
from mosek.fusion import *


def main():
    #Parameters:
    n = 30               #Number of tasks
    m = 6                #Number of processors

    lb = 1.              #Range of short task lengths
    ub = 5.

    sh = 0.8             #Proportion of short tasks
    n_short = int(n * sh)
    n_long = n - n_short

    random.seed(0)
    T = sorted([random.uniform(lb, ub) for i in range(n_short)]
               + [random.uniform(20 * lb, 20 * ub) for i in range(n_long)], reverse=True)

    print("# jobs(n)   : ", n)
    print("# machine(m): ", m)

    with Model('Multi-processor scheduling') as M:

        x = M.variable('x', [m, n], Domain.binary())
        t = M.variable('t', 1, Domain.unbounded())

        M.constraint(Expr.sum(x, 0), Domain.equalsTo(1.))
        M.constraint(Expr.sub(Var.repeat(t, m), Expr.mul(x, T)),
                     Domain.greaterThan(0.))

        M.objective(ObjectiveSense.Minimize, t)

        #LPT heuristic
        schedule = [0. for i in range(m)]
        init = [0. for i in range(n * m)]

        for i in range(n):
            mm = schedule.index(min(schedule))
            schedule[mm] += T[i]
            init[n * mm + i] = 1.

        #Comment this line to switch off feeding in the initial LPT solution
        x.setLevel(init)

        M.setLogHandler(sys.stdout)
        M.setSolverParam("mioTolRelGap", .01)
        M.solve()

        print('initial solution:')
        for i in range(m):
            print('M', i, init[i * n:(i + 1) * n])

        print('MOSEK solution:')
        for i in range(m):
            print('M', i, [y for y in x.slice([i, 0], [i + 1, n]).level()])

if __name__ == '__main__':
    main()