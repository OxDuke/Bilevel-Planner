##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      lo2.py
#
#  Purpose :   Demonstrates how to solve small linear
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

            # Bound keys for constraints
            bkc = [mosek.boundkey.fx,
                   mosek.boundkey.lo,
                   mosek.boundkey.up]
            # Bound values for constraints
            blc = [30.0, 15.0, -inf]
            buc = [30.0, +inf, 25.0]
            # Bound keys for variables
            bkx = [mosek.boundkey.lo,
                   mosek.boundkey.ra,
                   mosek.boundkey.lo,
                   mosek.boundkey.lo]
            # Bound values for variables
            blx = [0.0, 0.0, 0.0, 0.0]
            bux = [+inf, 10.0, +inf, +inf]
            # Objective coefficients

            c = [3.0, 1.0, 5.0, 1.0]

            # We input the A matrix column-wise
            # asub contains row indexes
            asub = [[0, 1, 2],
                    [0, 1, 2, 3],
                    [0, 3]]
            # acof contains coefficients
            aval = [[3.0, 1.0, 2.0],
                    [2.0, 1.0, 3.0, 1.0],
                    [2.0, 3.0]]
            numvar = len(bkx)
            numcon = len(bkc)
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

            for i in range(numcon):
                task.putconbound(i, bkc[i], blc[i], buc[i])
                # Input row i of A
                task.putarow(i,                     # Row index.
                             # Column indexes of non-zeros in row i.
                             asub[i],
                             aval[i])              # Non-zero Values of row i.

            # Input the objective sense (minimize/maximize)
            task.putobjsense(mosek.objsense.maximize)

            # Optimize the task
            task.optimize()

            # Print a summary containing information
            #about the solution for debugging purposes
            task.solutionsummary(mosek.streamtype.msg)

            prosta = task.getprosta(mosek.soltype.bas)
            solsta = task.getsolsta(mosek.soltype.bas)

            # Output a solution
            xx = [0.] * numvar
            task.getxx(mosek.soltype.bas,
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
    if e.msg is not None:
        print("\t%s" % e.msg)
        sys.exit(1)
except:
    import traceback
    traceback.print_exc()
    sys.exit(1)