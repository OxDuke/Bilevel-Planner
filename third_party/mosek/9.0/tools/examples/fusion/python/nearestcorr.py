##
#  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File:      nearestcorr.py
#
#  Purpose:
#  Solves the nearest correlation matrix problem
#
#    minimize   || A - X ||_F   s.t.  diag(X) = e, X is PSD
#
#  as the equivalent conic program
#
#    minimize     t
#
#    subject to   (t, vec(A-X)) in Q
#                 diag(X) = e
#                 X >= 0
##

import sys
import mosek
import mosek.fusion
from mosek.fusion import *
from mosek import LinAlg

"""
 Assuming that e is an NxN expression, return the lower triangular part as a vector.
"""
def vec(e):
    N = e.getShape()[0]

    msubi = range(N * (N + 1) // 2)
    msubj = [i * N + j for i in range(N) for j in range(i + 1)]
    mcof  = [2.0**0.5 if i !=
             j else 1.0 for i in range(N) for j in range(i + 1)]

    S = Matrix.sparse(N * (N + 1) // 2, N * N, msubi, msubj, mcof)
    return Expr.mul(S, Expr.flatten(e))

def nearestcorr(A):
    N = A.numRows()

    # Create a model
    with Model("NearestCorrelation") as M:
        # Setting up the variables
        X = M.variable("X", Domain.inPSDCone(N))
        t = M.variable("t", 1, Domain.unbounded())

        # (t, vec (A-X)) \in Q
        v = vec(Expr.sub(A, X))
        M.constraint("C1", Expr.vstack(t, v), Domain.inQCone())

        # diag(X) = e
        M.constraint("C2", X.diag(), Domain.equalsTo(1.0))

        # Objective: Minimize t
        M.objective(ObjectiveSense.Minimize, t)
        M.writeTask('nearcor.task')
        M.writeTask('nearcor.cbf')
        M.solve()

        return X.level(), t.level()

def nearestcorr_nucnorm(A, gammas):
    N = A.numRows()
    with Model("NucNorm") as M:
        # Setup variables
        t = M.variable("t", 1, Domain.unbounded())
        X = M.variable("X", Domain.inPSDCone(N))
        w = M.variable("w", N, Domain.greaterThan(0.0))

        # D = diag(w)
        D = Expr.mulElm(Matrix.eye(N), Var.repeat(w, 1, N))
        # (t, vec (X + D - A)) in Q
        M.constraint(Expr.vstack(t, vec(Expr.sub(Expr.add(X, D), A))),
                     Domain.inQCone())

        result = []
        for g in gammas:
            # Objective: Minimize t + gamma*Tr(X)
            M.objective(ObjectiveSense.Minimize, Expr.add(
                t, Expr.mul(g, Expr.sum(X.diag()))))
            M.solve()

            # Find eigenvalues of X and compute its rank
            d = [0.0] * int(N)
            LinAlg.syeig(mosek.uplo.lo, N, X.level(), d)
            result.append(
                (g, t.level(), sum([d[i] > 1e-6 for i in range(N)]), X.level()))

        return result

if __name__ == '__main__':
    N = 5
    A = Matrix.dense(N, N, [0.0, 0.5, -0.1, -0.2, 0.5,
                            0.5, 1.25, -0.05, -0.1, 0.25,
                            -0.1, -0.05, 0.51, 0.02, -0.05,
                            -0.2, -0.1, 0.02, 0.54, -0.1,
                            0.5, 0.25, -0.05, -0.1, 1.25])

    gammas = [0.1 * i for i in range(11)]

    X, t = nearestcorr(A)

    print("--- Nearest Correlation ---")
    print("X = ")
    print(X.reshape((N, N)))
    print("t = ", t)

    print("--- Nearest Correlation with Nuclear Norm---")
    for g, res, rank, X in nearestcorr_nucnorm(A, gammas):
        print("gamma=%f, res=%e, rank=%d" % (g, res, rank))