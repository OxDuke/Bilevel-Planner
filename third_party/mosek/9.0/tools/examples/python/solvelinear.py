##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      solvelinear.py
#
#  Purpose :   To demonstrate the usage of MSK_solvewithbasis
#              when solving the linear system:
#
#                1.0  x1             = b1
#               -1.0  x0  +  1.0  x1 = b2
#
#              with two different right hand sides
#
#                 b = (1.0, -2.0)
#
#              and
#
#                 b = (7.0, 0.0)
##
import mosek

def setup(task,
          aval,
          asub,
          ptrb,
          ptre,
          numvar,
          basis):
    # Since the value infinity is never used, we define
    # 'infinity' symbolic purposes only
    infinity = 0

    skx = [mosek.stakey.bas] * numvar
    skc = [mosek.stakey.fix] * numvar

    task.appendvars(numvar)
    task.appendcons(numvar)

    for i in range(len(asub)):
        task.putacol(i, asub[i], aval[i])

    for i in range(numvar):
        task.putconbound(i, mosek.boundkey.fx, 0.0, 0.0)

    for i in range(numvar):
        task.putvarbound(i,
                         mosek.boundkey.fr,
                         -infinity,
                         infinity)

    # Define a basic solution by specifying
    # status keys for variables & constraints.
    task.deletesolution(mosek.soltype.bas);

    task.putskcslice(mosek.soltype.bas, 0, numvar, skc);
    task.putskxslice(mosek.soltype.bas, 0, numvar, skx);

    task.initbasissolve(basis);


def main():
    numcon = 2
    numvar = 2

    aval = [[-1.0],
            [1.0, 1.0]]
    asub = [[1],
            [0, 1]]

    ptrb = [0, 1]
    ptre = [1, 3]

    #int[]       bsub  = new int[numvar];
    #double[]    b     = new double[numvar];
    #int[]       basis = new int[numvar];

    with mosek.Env() as env:
        with mosek.Task(env) as task:
            # Directs the log task stream to the user specified
            # method task_msg_obj.streamCB
            task.set_Stream(mosek.streamtype.log,
                            lambda msg: sys.stdout.write(msg))
            # Put A matrix and factor A.
            # Call this function only once for a given task.

            basis = [0] * numvar
            b = [0.0, -2.0]
            bsub = [0, 1]

            setup(task,
                  aval,
                  asub,
                  ptrb,
                  ptre,
                  numvar,
                  basis)

            # now solve rhs
            b = [1, -2]
            bsub = [0, 1]
            nz = task.solvewithbasis(0, 2, bsub, b)
            print("\nSolution to Bx = b:\n")

            # Print solution and show correspondents
            # to original variables in the problem
            for i in range(nz):
                if basis[bsub[i]] < numcon:
                    print("This should never happen")
                else:
                    print("x%d = %d" % (basis[bsub[i]] - numcon, b[bsub[i]]))

            b[0] = 7
            bsub[0] = 0

            nz = task.solvewithbasis(0, 1, bsub, b)

            print("\nSolution to Bx = b:\n")
            # Print solution and show correspondents
            # to original variables in the problem
            for i in range(nz):
                if basis[bsub[i]] < numcon:
                    print("This should never happen")
                else:
                    print("x%d = %d" % (basis[bsub[i]] - numcon, b[bsub[i]]))

if __name__ == "__main__":
    try:
        main()
    except:
        import traceback
        traceback.print_exc()