##
#  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File:      facility_location.py
#
#  Purpose: Demonstrates a small one-facility location problem.
#
#  Given 10 customers placed in a grid we wish to place a facility
#  somewhere so that the total sum of distances to customers is
#  minimized.
#
#  The problem is formulated as a conic optimization problem as follows.
#  Let f=(fx,fy) be the (unknown) location of the facility, and let
#  c_i=(cx_i,cy_i) be the (known) customer locations; then we wish to
#  minimize
#      sum_i || f - c_i ||
#  where
#      ||.||
#  denotes the euclidian norm.
#  This is formulated as
#
#  minimize   sum(d_i)
#  such that  d_i ^ 2 > tx_i ^ 2 + ty_i ^ 2, for all i (a.k.a. (d_i,tx_i,ty_i) in C^3_r)
#             tx_i = cx_i - fx, for all i
#             ty_i = cy_i - fy, for all i
#             d_i > 0, for all i
##
import sys
from mosek.fusion import *

# Customer locations
customerloc = Matrix.dense([[12.0, 2.0],
                            [15.0, 13.0],
                            [10.0, 8.0],
                            [0.0, 10.0],
                            [6.0, 13.0],
                            [5.0, 8.0],
                            [10.0, 12.0],
                            [4.0, 6.0],
                            [5.0, 2.0],
                            [1.0, 10.0]])
N = customerloc.numRows()

with Model('FacilityLocation') as M:
    # Variable holding the facility location
    f = M.variable("facility", Set.make(1, 2), Domain.unbounded())
# Variable defining the euclidian distances to each customer
    d = M.variable("dist", Set.make(N, 1), Domain.greaterThan(0.0))
# Variable defining the x and y differences to each customer
    t = M.variable("t", Set.make(N, 2), Domain.unbounded())
    M.constraint('dist measure', Var.hstack(d, t), Domain.inQCone())

    fxy = Var.repeat(f, N)

    M.constraint("xy diff", Expr.add(t, fxy), Domain.equalsTo(customerloc))

    M.objective("total_dist", ObjectiveSense.Minimize, Expr.sum(d))

    M.solve()
    M.writeTask("facility_location.task")
    print('Facility location =', f.level())