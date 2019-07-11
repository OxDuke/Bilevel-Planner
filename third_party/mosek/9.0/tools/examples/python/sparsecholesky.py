##
#  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File:      sparsecholesky.py
#
#  Purpose: Demonstrate the sparse Cholesky factorization.
##

from __future__ import print_function
import mosek
import numpy

# Prints out a Cholesky factor presented in sparse form
def printsparse(n, perm, diag, lnzc, lptrc, lensubnval, lsubc, lvalc):
    print("P = ", perm)
    print("diag(D) = ", numpy.array(diag))
    print("L=")
    l = [[0.0 for j in range(n)] for i in range(n)]
    for j in range(n):
        for i in range(lptrc[j], lptrc[j] + lnzc[j]):
            l[lsubc[i]][j] = lvalc[i]
    print(numpy.array(l))


# Create the mosek environment.
with mosek.Env() as env:
    # The matrix from the manual
    # Observe that anzc, aptrc, asubc and avalc only specify the lower
    # triangular part.
    n = 4
    anzc = [4, 1, 1, 1]
    asubc = [0, 1, 2, 3, 1, 2, 3]
    aptrc = [0, 4, 5, 6]
    avalc = [4.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]
    b = [13.0, 3.0, 4.0, 5.0]

    try:
        perm, diag, lnzc, lptrc, lensubnval, lsubc, lvalc = env.computesparsecholesky(
            0,      #Disable multithread
            1,      #User reordering heuristic
            1.0e-14,#Singularity tolerance
            anzc, aptrc, asubc, avalc)

        printsparse(n, perm, diag, lnzc, lptrc, lensubnval, lsubc, lvalc)

        x = [b[p] for p in perm]   # Permuted b is stored as x.

        # Compute inv(L)*x.
        env.sparsetriangularsolvedense(mosek.transpose.no,
                                       lnzc, lptrc, lsubc, lvalc, x)

        # Compute inv(L^T)*x.
        env.sparsetriangularsolvedense(mosek.transpose.yes,
                                       lnzc, lptrc, lsubc, lvalc, x)

        print("\nSolution Ax=b: x = ", numpy.array(
            [x[j] for i in range(n) for j in range(n) if perm[j] == i]), "\n")
    except:
        raise

    #Example 2 - singular A
    n = 3
    anzc = [3, 2, 1]
    asubc = [0, 1, 2, 1, 2, 2]
    aptrc = [0, 3, 5]
    avalc = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0]

    try:
        perm, diag, lnzc, lptrc, lensubnval, lsubc, lvalc = env.computesparsecholesky(
            0,      #Disable multithread
            1,      #User reordering heuristic
            1.0e-14,#Singularity tolerance
            anzc, aptrc, asubc, avalc)

        printsparse(n, perm, diag, lnzc, lptrc, lensubnval, lsubc, lvalc)
    except:
        raise