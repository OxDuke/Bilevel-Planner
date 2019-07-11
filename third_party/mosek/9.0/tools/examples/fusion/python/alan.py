##
# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File:      alan.py
#
#  Purpose: This file contains an implementation of the alan.gms (as
#  found in the GAMS online model collection) using Java/Fusion.
#
#  The model is a simple portfolio choice model. The objective is to
#  invest in a number of assets such that we minimize the risk, while
#  requiring a certain expected return.
#
#  We operate with 4 assets (hardware,software, show-biz and treasure
#  bill). The risk is defined by the covariance matrix
#    Q = [[  4.0, 3.0, -1.0, 0.0 ],
#         [  3.0, 6.0,  1.0, 0.0 ],
#         [ -1.0, 1.0, 10.0, 0.0 ],
#         [  0.0, 0.0,  0.0, 0.0 ]]
#
#
#  We use the form Q = U^T * U, where U is a Cholesky factor of Q.
##

import sys
from mosek.fusion import *

###########################################################
### Problem data:
#
# Security names
securities = ["hardware", "software", "show-biz", "t-bills"]
# Mean returns on securities
mean = [8.0, 9.0, 12.0, 7.0]
# Target mean return
target = 10.0
# Factor of covariance matrix.
U = Matrix.dense([[2.0, 1.5, -0.5, 0.0],
                  [0.0, 1.93649167, 0.90369611, 0.0],
                  [0.0, 0.0, 2.98886824, 0.0],
                  [0.0, 0.0, 0.0, 0.0]])

numsec = len(securities)

###########################################################


def main(args):
    with Model('alan') as M:
        x = M.variable("x", numsec, Domain.greaterThan(0.0))
        t = M.variable("t", 1, Domain.greaterThan(0.0))

        M.objective("minvar", ObjectiveSense.Minimize, t)

        # sum securities to 1.0
        M.constraint("wealth", Expr.sum(x), Domain.equalsTo(1.0))
        # define target expected return
        M.constraint("dmean", Expr.dot(mean, x), Domain.greaterThan(target))

        M.constraint("t > ||Ux||",
                     Expr.vstack(0.5,
                                 t,
                                 Expr.mul(U, x)), Domain.inRotatedQCone())
        M.setLogHandler(sys.stdout)
        print("Solve...")
        M.solve()
        M.writeTask('alan.opf')
        print("-------------Summary----------")
        print("... Solved")
        solx = x.level()
        print("Solution = " + ','.join([str(v) for v in solx]))


if __name__ == '__main__':
    main(sys.argv[1:])