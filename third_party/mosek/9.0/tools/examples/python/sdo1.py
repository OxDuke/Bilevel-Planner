##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      sdo1.py
#
#  Purpose :   Demonstrates how to solve a small mixed semidefinite and conic quadratic
#              optimization problem using the MOSEK Python API.
##
import sys
import mosek

# Since the value of infinity is ignored, we define it solely
# for symbolic purposes
inf = 0.0

# Define a stream printer to grab output from MOSEK
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()


def main():
    # Make mosek environment
    with mosek.Env() as env:

        # Create a task object and attach log stream printer
        with env.Task(0, 0) as task:
            task.set_Stream(mosek.streamtype.log, streamprinter)

            # Bound keys for constraints
            bkc = [mosek.boundkey.fx,
                   mosek.boundkey.fx]

            # Bound values for constraints
            blc = [1.0, 0.5]
            buc = [1.0, 0.5]

            # Below is the sparse representation of the A
            # matrix stored by row.
            asub = [[0],
                    [1, 2]]
            aval = [[1.0],
                    [1.0, 1.0]]

            conesub = [0, 1, 2]

            barci = [0, 1, 1, 2, 2]
            barcj = [0, 0, 1, 1, 2]
            barcval = [2.0, 1.0, 2.0, 1.0, 2.0]

            barai = [[0, 1, 2],
                     [0, 1, 2, 1, 2, 2]]
            baraj = [[0, 1, 2],
                     [0, 0, 0, 1, 1, 2]]
            baraval = [[1.0, 1.0, 1.0],
                       [1.0, 1.0, 1.0, 1.0, 1.0, 1.0]]

            numvar = 3
            numcon = len(bkc)
            BARVARDIM = [3]

            # Append 'numvar' variables.
            # The variables will initially be fixed at zero (x=0).
            task.appendvars(numvar)

            # Append 'numcon' empty constraints.
            # The constraints will initially have no bounds.
            task.appendcons(numcon)

            # Append matrix variables of sizes in 'BARVARDIM'.
            # The variables will initially be fixed at zero.
            task.appendbarvars(BARVARDIM)

            # Set the linear term c_0 in the objective.
            task.putcj(0, 1.0)

            for j in range(numvar):
                # Set the bounds on variable j
                # blx[j] <= x_j <= bux[j]
                task.putvarbound(j, mosek.boundkey.fr, -inf, +inf)

            for i in range(numcon):
                # Set the bounds on constraints.
                # blc[i] <= constraint_i <= buc[i]
                task.putconbound(i, bkc[i], blc[i], buc[i])

                # Input row i of A
                task.putarow(i,                  # Constraint (row) index.
                             asub[i],            # Column index of non-zeros in constraint i.
                             aval[i])            # Non-zero values of row i.

            # Add the quadratic cone constraint
            task.appendcone(mosek.conetype.quad,
                            0.0,
                            conesub)

            symc = task.appendsparsesymmat(BARVARDIM[0],
                                           barci,
                                           barcj,
                                           barcval)

            syma0 = task.appendsparsesymmat(BARVARDIM[0],
                                            barai[0],
                                            baraj[0],
                                            baraval[0])

            syma1 = task.appendsparsesymmat(BARVARDIM[0],
                                            barai[1],
                                            baraj[1],
                                            baraval[1])

            task.putbarcj(0, [symc], [1.0])

            task.putbaraij(0, 0, [syma0], [1.0])
            task.putbaraij(1, 0, [syma1], [1.0])

            # Input the objective sense (minimize/maximize)
            task.putobjsense(mosek.objsense.minimize)

            # Solve the problem and print summary
            task.optimize()
            task.solutionsummary(mosek.streamtype.msg)

            # Get status information about the solution
            prosta = task.getprosta(mosek.soltype.itr)
            solsta = task.getsolsta(mosek.soltype.itr)

            if (solsta == mosek.solsta.optimal):
                xx = [0.] * numvar
                task.getxx(mosek.soltype.itr, xx)

                lenbarvar = BARVARDIM[0] * (BARVARDIM[0] + 1) / 2
                barx = [0.] * int(lenbarvar)
                task.getbarxj(mosek.soltype.itr, 0, barx)

                print("Optimal solution:\nx=%s\nbarx=%s" % (xx, barx))
            elif (solsta == mosek.solsta.dual_infeas_cer or
                  solsta == mosek.solsta.prim_infeas_cer):
                print("Primal or dual infeasibility certificate found.\n")
            elif solsta == mosek.solsta.unknown:
                print("Unknown solution status")
            else:
                print("Other solution status")

# call the main function
try:
    main()
except mosek.MosekException as e:
    print("ERROR: %s" % str(e.errno))
    if e.msg is not None:
        print("\t%s" % e.msg)
        sys.exit(1)
except:
    import traceback
    traceback.print_exc()
    sys.exit(1)