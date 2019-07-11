##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      blas_lapack.py
#
#  Purpose :   To demonstrate how to call BLAS/LAPACK routines
#              for whose MOSEK provides simplified interfaces.
##
import mosek

def print_matrix(x, r, c):
    for i in range(r):
        print([x[j * r + i] for j in range(c)])

with mosek.Env() as env:

    n = 3
    m = 2
    k = 3

    alpha = 2.0
    beta = 0.5

    x = [1.0, 1.0, 1.0]
    y = [1.0, 2.0, 3.0]
    z = [1.0, 1.0]
    v = [0.0, 0.0]
    #A has m=2 rows and k=3 cols
    A = [1.0, 1.0, 2.0, 2.0, 3., 3.]
    #B has k=3 rows and n=3 cols
    B = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]
    C = [0.0 for i in range(n * m)]
    D = [1.0, 1.0, 1.0, 1.0]
    Q = [1.0, 0.0, 0.0, 2.0]

# BLAS routines

    xy = env.dot(n, x, y)
    print("dot results= %f\n" % xy)

    env.axpy(n, alpha, x, y)
    print("\naxpy results is ")
    print_matrix(y, 1, len(y))

    env.gemv(mosek.transpose.no, m, n, alpha, A, x, beta, z)
    print("\ngemv results is ")
    print_matrix(z, 1, len(z))

    env.gemm(mosek.transpose.no, mosek.transpose.no,
             m, n, k, alpha, A, B, beta, C)
    print("\ngemm results is ")
    print_matrix(C, m, n)

    env.syrk(mosek.uplo.lo, mosek.transpose.no, m, k, alpha, A, beta, D)
    print("\nsyrk results is")
    print_matrix(D, m, m)

# LAPACK routines

    env.potrf(mosek.uplo.lo, m, Q)
    print("\npotrf results is ")
    print_matrix(Q, m, m)


    env.syeig(mosek.uplo.lo, m, Q, v)
    print("\nsyeig results is")
    print_matrix(v, 1, m)


    env.syevd(mosek.uplo.lo, m, Q, v)
    print("\nsyevd results is")
    print('v: ')
    print_matrix(v, 1, m)
    print('Q: ')
    print_matrix(Q, m, m)

    print("Exiting...")