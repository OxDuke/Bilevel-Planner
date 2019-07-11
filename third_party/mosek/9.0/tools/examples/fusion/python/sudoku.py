#
# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      sudoku.py
#
# Purpose:  A MILP-based SUDOKU solver
#
#

import sys
import mosek
from mosek.fusion import *
import numpy as npy


def print_solution(m, x):
    n = m * m
    print("\n")
    for i in range(n):
        ss = ""
        for j in range(n):
            if j % m == 0:
                ss = ss + " |"

            for k in range(n):
                if x.index([i, j, k]).level() > 0.5:
                    ss = ss + " " + str(k + 1)
                    break

        print(ss + ' |')
        if (i + 1) % m == 0:
            print("\n")

def main():

    m = 3
    n = m * m

    hr_fixed = [[1, 5, 4],
                [2, 2, 5], [2, 3, 8], [2, 6, 3],
                [3, 2, 1], [3, 4, 2], [3, 5, 8], [3, 7, 9],
                [4, 2, 7], [4, 3, 3], [4, 4, 1], [4, 7, 8], [4, 8, 4],
                [6, 2, 4], [6, 3, 1], [6, 6, 9], [6, 7, 2], [6, 8, 7],
                [7, 3, 4], [7, 5, 6], [7, 6, 5], [7, 8, 8],
                [8, 4, 4], [8, 7, 1], [8, 8, 6],
                [9, 5, 9]
                ]

    fixed = [[f[0] - 1, f[1] - 1, f[2] - 1] for f in hr_fixed]

    with Model('SUDOKU') as M:
        x = M.variable("x",[n, n, n], Domain.binary())

        #each value only once per dimension
        for d in range(m):
            M.constraint("row_sum(%d)" % d, Expr.sum(x, d), Domain.equalsTo(1.))

        #each number must appear only once in a block

        for k in range(n):
            for i in range(m):
                for j in range(m):
                    M.constraint("blocksum(%d,%d,k=%d)" % (i,j,k),
                                 Expr.sum(x.slice([i * m, j * m, k], [(i + 1) * m, (j + 1) * m, k + 1])),
                                 Domain.equalsTo(1.))

        M.constraint("fix",x.pick(fixed), Domain.equalsTo(1.0))

        M.setLogHandler(sys.stdout)

        M.solve()

        M.writeTask("sudoku.task")

        #print the solution, if any...
        if M.getPrimalSolutionStatus() in [SolutionStatus.Optimal]:
            print_solution(m, x)
        else:
            print("No solution found!")

if __name__ == '__main__':
    main()