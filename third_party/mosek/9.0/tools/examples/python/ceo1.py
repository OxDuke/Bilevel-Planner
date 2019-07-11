##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      ceo1.py
#
#  Purpose :   Demonstrates how to solve small conic exponential
#              optimization problem using the MOSEK Python API.
##
import sys
import mosek

# Define a stream printer to grab output from MOSEK
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()

def main():

    # Only a symbolic constant
    inf = 0.0

    # Make a MOSEK environment
    with mosek.Env() as env:
        # Attach a printer to the environment
        env.set_Stream(mosek.streamtype.log, streamprinter)

        # Create a task
        with env.Task(0, 0) as task:
            # Attach a printer to the task
            task.set_Stream(mosek.streamtype.log, streamprinter)

            c = [1.0, 1.0, 0.0]
            a = [1.0, 1.0, 1.0]
            numvar, numcon = 3, 1

            # Append 'numcon' empty constraints.
            # The constraints will initially have no bounds.
            task.appendcons(numcon)

            # Append 'numvar' variables.
            # The variables will initially be fixed at zero (x=0).
            task.appendvars(numvar)

            # Set up the linear part of the problem
            task.putcslice(0, numvar, c)
            task.putarow(0, [0, 1, 2], a)
            task.putvarboundslice(0, numvar, [mosek.boundkey.fr] * numvar, [inf] * numvar, [inf] * numvar)
            task.putconbound(0, mosek.boundkey.fx, 1.0, 1.0)

            # Input the cones
            task.appendcone(mosek.conetype.pexp,
                            0.0,
                            [0, 1, 2])

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
    print("ERROR: %s" % str(e.code))
    if msg is not None:
        print("\t%s" % e.msg)
        sys.exit(1)
except:
    import traceback
    traceback.print_exc()
    sys.exit(1)