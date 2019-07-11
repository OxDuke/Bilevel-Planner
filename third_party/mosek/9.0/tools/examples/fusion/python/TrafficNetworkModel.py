##
# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      TrafficNetworkModel.py
#
# Purpose:   Demonstrates a traffic network problem as a conic quadratic problem.
#
# Source:    Robert Fourer, "Convexity Checking in Large-Scale Optimization",
#            OR 53 --- Nottingham 6-8 September 2011.
#
# The problem:
#            Given a directed graph representing a traffic network
#            with one source and one sink, we have for each arc an
#            associated capacity, base travel time and a
#            sensitivity. Travel time along a specific arc increases
#            as the flow approaches the capacity.
#
#            Given a fixed inflow we now wish to find the
#            configuration that minimizes the average travel time.
##

from mosek.fusion import *
import numpy as np
import sys


def main(args):
    n = 4
    arc_i = [0,    0,    2,    1,    2]
    arc_j = [1,    2,    1,    3,    3]
    arc_base = [4.0,  1.0,  2.0,  1.0,  6.0]
    arc_cap = [10.0, 12.0, 20.0, 15.0, 10.0]
    arc_sens = [0.1,  0.7,  0.9,  0.5,  0.1]

    T = 20.0
    source_idx = 0
    sink_idx = 3

    with Model() as M:
        narcs = len(arc_i)

        NxN = Set.make(n, n)
        sens = Matrix.sparse(n, n, arc_i, arc_j, arc_sens)
        cap = Matrix.sparse(n, n, arc_i, arc_j, arc_cap)
        basetime = Matrix.sparse(n, n, arc_i, arc_j, arc_base)

        cs_inv_matrix = \
            Matrix.sparse(n, n, arc_i, arc_j,
                          [1.0 / (arc_sens[i] * arc_cap[i]) for i in range(narcs)])
        s_inv_matrix = \
            Matrix.sparse(n, n, arc_i, arc_j,
                          [1.0 / arc_sens[i] for i in range(narcs)])

        sparsity = [ [ i, j] for i,j in zip(arc_i,arc_j) ]

        x = M.variable("traffic_flow", [n,n], Domain.greaterThan(0.0).sparse(sparsity+[ [sink_idx, source_idx] ]))
        t = M.variable("travel_time",  [n,n], Domain.greaterThan(0.0).sparse(sparsity))
        d = M.variable("d",            [n,n], Domain.greaterThan(0.0).sparse(sparsity))
        z = M.variable("z",            [n,n], Domain.greaterThan(0.0).sparse(sparsity))

        # Set the objective:
        M.objective("Average travel time",
                    ObjectiveSense.Minimize,
                    Expr.mul(1.0 / T, Expr.add(Expr.dot(basetime, x), Expr.sum(d))))

        # Set up constraints
        # Constraint (1a)
        numnz = len(arc_sens)

        v = Var.hstack([ d.pick(sparsity),
                         z.pick(sparsity),
                         x.pick(sparsity) ])

        M.constraint("(1a)", v, Domain.inRotatedQCone(narcs,3))

        # Constraint (1b)

        M.constraint("(1b)",
                    Expr.condense(
                       Expr.sub(
                         Expr.add(z, Expr.mulElm(x, cs_inv_matrix)),
                         s_inv_matrix)),
                     Domain.equalsTo(0.0))
        # Constraint (2)
        M.constraint("(2)",
                     Expr.sub( Expr.sum(x,0),
                               Expr.sum(x,1) ),
                     Domain.equalsTo(0.0))
        # Constraint (3)
        M.constraint("(3)", x.index(sink_idx, source_idx), Domain.equalsTo(T))

        M.writeTask("TrafficNetwork.opf")
        M.setLogHandler(sys.stdout);
        M.solve()

        flow = x.pick(sparsity).level()

        print("Optimal flow:")
        for i, j, v in zip(arc_i, arc_j, flow):
            print("\tflow node%d->node%d = %f" % (i, j, v))


main(sys.argv[1:])