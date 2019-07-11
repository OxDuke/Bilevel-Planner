##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      cqo1.py
#
#  Purpose :   Demonstrates how to solve small conic quadratic
#              optimization problem using the MOSEK Python API.
##
import sys
import mosek

# Since the actual value of Infinity is ignores, we define it solely
# for symbolic purposes:
inf = 0.0

# Define a stream printer to grab output from MOSEK
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()


def main():
    # Make a MOSEK environment
    with mosek.Env() as env:
        # Attach a printer to the environment
        env.set_Stream(mosek.streamtype.log, streamprinter)

        # Create a task
        with env.Task(0, 0) as task:
            # Attach a printer to the task
            task.set_Stream(mosek.streamtype.log, streamprinter)

            bkc = [mosek.boundkey.fx]
            blc = [1.0]
            buc = [1.0]

            c = [0.0, 0.0, 0.0,
                 1.0, 1.0, 1.0]
            bkx = [mosek.boundkey.lo, mosek.boundkey.lo, mosek.boundkey.lo,
                   mosek.boundkey.fr, mosek.boundkey.fr, mosek.boundkey.fr]
            blx = [0.0, 0.0, 0.0,
                   -inf, -inf, -inf]
            bux = [inf, inf, inf,
                   inf, inf, inf]

            asub = [[0], [0], [0]]
            aval = [[1.0], [1.0], [2.0]]

            numvar = len(bkx)
            numcon = len(bkc)
            NUMANZ = 4

            # Append 'numcon' empty constraints.
            # The constraints will initially have no bounds.
            task.appendcons(numcon)

            #Append 'numvar' variables.
            # The variables will initially be fixed at zero (x=0).
            task.appendvars(numvar)

            for j in range(numvar):
              # Set the linear term c_j in the objective.
                task.putcj(j, c[j])
              # Set the bounds on variable j
              # blx[j] <= x_j <= bux[j]
                task.putvarbound(j, bkx[j], blx[j], bux[j])

            for j in range(len(aval)):
              # Input column j of A
                task.putacol(j,                  # Variable (column) index.
                             # Row index of non-zeros in column j.
                             asub[j],
                             aval[j])            # Non-zero Values of column j.
            for i in range(numcon):
                task.putconbound(i, bkc[i], blc[i], buc[i])

                # Input the cones
            task.appendcone(mosek.conetype.quad,
                            0.0,
                            [3, 0, 1])
            task.appendcone(mosek.conetype.rquad,
                            0.0,
                            [4, 5, 2])

            # Input the objective sense (minimize/maximize)
            task.putobjsense(mosek.objsense.minimize)

            # Optimize the task
            task.optimize()
            # Print a summary containing information
            # about the solution for debugging purposes
            task.solutionsummary(mosek.streamtype.msg)
            prosta = task.getprosta(mosek.soltype.itr)
            solsta = task.getsolsta(mosek.soltype.itr)

            # Output a solution
            xx = [0.] * numvar
            task.getxx(mosek.soltype.itr,
                       xx)

            if solsta == mosek.solsta.optimal:
                print("Optimal solution: %s" % xx)
            elif solsta == mosek.solsta.dual_infeas_cer:
                print("Primal or dual infeasibility.\n")
            elif solsta == mosek.solsta.prim_infeas_cer:
                print("Primal or dual infeasibility.\n")
            elif mosek.solsta.unknown:
                print("Unknown solution status")
            else:
                print("Other solution status")

# call the main function
try:
    main()
except mosek.MosekException as e:
    print("ERROR: %s" % str(e.errno))
    print("\t%s" % e.msg)
    sys.exit(1)
except:
    import traceback
    traceback.print_exc()
    sys.exit(1)