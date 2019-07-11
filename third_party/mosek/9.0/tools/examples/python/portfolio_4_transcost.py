##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      portfolio_4_transcost.py
#
#  Purpose :   Implements a basic portfolio optimization model
#              with fixed setup costs and transaction costs
#              as a mixed-integer problem.
##
import mosek

def streamprinter(text):
    print("%s" % text),

if __name__ == '__main__':

    n = 3
    gamma = 0.05
    mu = [0.1073, 0.0737, 0.0627]
    GT = [[0.1667, 0.0232, 0.0013],
          [0.0000, 0.1033, -0.0022],
          [0.0000, 0.0000, 0.0338]]
    x0 = [0.0, 0.0, 0.0]
    w = 1.0
    f = [0.01, 0.01, 0.01]
    g = [0.001, 0.001, 0.001]

    # This value has no significance.
    inf = 0.0

    with mosek.Env() as env:
        with env.Task(0, 0) as task:
            task.set_Stream(mosek.streamtype.log, streamprinter)

            # Total wealth
            U = w + sum(x0)

            # Constraints.
            task.appendcons(1 + 4 * n)
            task.putconbound(0, mosek.boundkey.fx, U, U)
            task.putconname(0, "budget")

            task.putconboundlist(range(1 + 0, 1 + n), n *
                                 [mosek.boundkey.fx], n * [0.0], n * [0.0])
            for j in range(1, 1 + n):
                task.putconname(j, "GT[%d]" % j)

            task.putconboundlist(range(
                1 + n, 1 + 2 * n), n * [mosek.boundkey.lo], [-x0[j] for j in range(0, n)], n * [inf])
            for i in range(0, n):
                task.putconname(1 + n + i, "zabs1[%d]" % (1 + i))

            task.putconboundlist(range(1 + 2 * n, 1 + 3 * n),
                                 n * [mosek.boundkey.lo], x0, n * [inf])
            for i in range(0, n):
                task.putconname(1 + 2 * n + i, "zabs2[%d]" % (1 + i))

            task.putconboundlist(range(1 + 3 * n, 1 + 4 * n),
                                 n * [mosek.boundkey.up], n * [-inf], n * [0.0])
            for i in range(0, n):
                task.putconname(1 + 3 * n + i, "ind[%d]" % (1 + i))

            # Offset of variables into the API variable.
            offsetx = 0
            offsets = n
            offsett = n + 1
            offsetz = 2 * n + 1
            offsety = 3 * n + 1

            # Variables.
            task.appendvars(1 + 4 * n)

            # x variables.
            task.putclist(range(offsetx + 0, offsetx + n), mu)
            task.putaijlist(
                n * [0], range(offsetx + 0, offsetx + n), n * [1.0])
            for j in range(0, n):
                task.putaijlist(
                    n * [1 + j], range(offsetx + 0, offsetx + n), GT[j])
                task.putaij(1 + n + j, offsetx + j, -1.0)
                task.putaij(1 + 2 * n + j, offsetx + j, 1.0)

            task.putvarboundlist(
                range(offsetx + 0, offsetx + n), n * [mosek.boundkey.lo], n * [0.0], n * [inf])
            for j in range(0, n):
                task.putvarname(offsetx + j, "x[%d]" % (1 + j))

            # s variable.
            task.putvarbound(offsets + 0, mosek.boundkey.fx, gamma, gamma)
            task.putvarname(offsets + 0, "s")

            # t variables.
            task.putaijlist(range(1, n + 1), range(offsett +
                                                   0, offsett + n), n * [-1.0])
            task.putvarboundlist(range(offsett + 0, offsett + n),
                                 n * [mosek.boundkey.fr], n * [-inf], n * [inf])
            for j in range(0, n):
                task.putvarname(offsett + j, "t[%d]" % (1 + j))

            # z variables.
            task.putaijlist(n * [0], range(offsetz, offsetz + n), g)
            task.putaijlist(range(1 + 1 * n, 1 + 2 * n),
                            range(offsetz, offsetz + n), n * [1.0])
            task.putaijlist(range(1 + 2 * n, 1 + 3 * n),
                            range(offsetz, offsetz + n), n * [1.0])
            task.putaijlist(range(1 + 3 * n, 1 + 4 * n),
                            range(offsetz, offsetz + n), n * [1.0])
            task.putvarboundlist(range(offsetz, offsetz + n),
                                 n * [mosek.boundkey.fr], n * [-inf], n * [inf])
            for j in range(0, n):
                task.putvarname(offsetz + j, "z[%d]" % (1 + j))

            # y variables.
            task.putaijlist(n * [0], range(offsety, offsety + n), f)            
            task.putaijlist(range(1 + 3 * n, 1 + 4 * n),
                            range(offsety, offsety + n), n * [-U])
            task.putvarboundlist(range(offsety, offsety + n),
                                 n * [mosek.boundkey.ra], n * [0.0], n * [1.0])            
            task.putvartypelist(range(offsety, offsety + n), n * [mosek.variabletype.type_int])
            for j in range(0, n):
                task.putvarname(offsety + j, "y[%d]" % (1 + j))

            # quadratic cone
            task.appendcone(mosek.conetype.quad, 0.0, [
                            offsets] + list(range(offsett, offsett + n)))
            task.putconename(0, "stddev")

            task.putobjsense(mosek.objsense.maximize)

            # Turn all log output off.
            # task.putintparam(mosek.iparam.log,0)

            # Dump the problem to a human readable OPF file.
            # task.writedata("dump.opf")

            task.optimize()

            # Display the solution summary for quick inspection of results.
            task.solutionsummary(mosek.streamtype.msg)

            expret = 0.0
            x = [0.] * n
            task.getxxslice(mosek.soltype.itg, offsetx + 0, offsetx + n, x)
            for j in range(0, n):
                expret += mu[j] * x[j]

            stddev = [0.]
            task.getxxslice(mosek.soltype.itg, offsets +
                            0, offsets + 1, stddev)

            print("\nExpected return %e for gamma %e\n" % (expret, stddev[0]))