# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      duality.py
#
# Purpose: Shows how to access the dual solution
import sys
import mosek
import mosek.fusion
from mosek.fusion import *


def main(args):

    A = [[-0.5, 1.0]]
    b = [1.0]
    c = [1.0, 1.0]

    with Model("duality1") as M:
        x = M.variable("x", 2, Domain.greaterThan(0.0))

        con = M.constraint(
            Expr.sub(Expr.mul(Matrix.dense(A), x), b), Domain.equalsTo(0.0))

        M.objective("obj", ObjectiveSense.Minimize, Expr.dot(c, x))
        M.solve()

        print("x =", x.level())
        print("s =", x.dual())
        print("y =", con.dual())

if __name__ == '__main__':
    main(sys.argv[1:])