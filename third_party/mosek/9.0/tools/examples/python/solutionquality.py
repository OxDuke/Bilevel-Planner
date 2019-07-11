##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      solutionquality.py
#
#  Purpose :   To demonstrate how to examine the quality of a solution.
##
import sys
import mosek

def streamprinter(msg):
    sys.stdout.write(msg)
    sys.stdout.flush()

if len(sys.argv) <= 1:
    print("Missing argument, syntax is:")
    print("  solutionquality inputfile")
else:
    try:
        # Create the mosek environment.
        with mosek.Env() as env:
            # Create a task object linked with the environment env.
            # We create it with 0 variables and 0 constraints initially,
            # since we do not know the size of the problem.
            with env.Task(0, 0) as task:

                task.set_Stream(mosek.streamtype.log, streamprinter)

                # We assume that a problem file was given as the first command
                # line argument (received in `argv')
                task.readdata(sys.argv[1])

                # Solve the problem
                task.optimize()

                # Print a summary of the solution
                task.solutionsummary(mosek.streamtype.log)

                whichsol = mosek.soltype.bas

                solsta = task.getsolsta(whichsol)

                pobj, pviolcon, pviolvar, pviolbarvar, pviolcones, pviolitg, \
                dobj, dviolcon, dviolvar, dviolbarvar, dviolcones = \
                    task.getsolutioninfo(whichsol)

                if solsta in [mosek.solsta.optimal]:

                    abs_obj_gap = abs(dobj - pobj)
                    rel_obj_gap = abs_obj_gap / \
                        (1.0 + min(abs(pobj), abs(dobj)))
                    max_primal_viol = max(pviolcon, pviolvar)
                    max_primal_viol = max(max_primal_viol, pviolbarvar)
                    max_primal_viol = max(max_primal_viol, pviolcones)

                    max_dual_viol = max(dviolcon, dviolvar)
                    max_dual_viol = max(max_dual_viol, dviolbarvar)
                    max_dual_viol = max(max_dual_viol, dviolcones)

                    # Assume the application needs the solution to be within
                    #    1e-6 ofoptimality in an absolute sense. Another approach
                    #   would be looking at the relative objective gap

                    print("\n\n")
                    print("Customized solution information.\n")
                    print("  Absolute objective gap: %e\n" % abs_obj_gap)
                    print("  Relative objective gap: %e\n" % rel_obj_gap)
                    print("  Max primal violation  : %e\n" % max_primal_viol)
                    print("  Max dual violation    : %e\n" % max_dual_viol)

                    accepted = True

                    if rel_obj_gap > 1e-6:
                        print("Warning: The relative objective gap is LARGE.")
                        accepted = False

                    # We will accept a primal infeasibility of 1e-8 and
                    # dual infeasibility of 1e-6. These number should chosen problem
                    # dependent.
                    if max_primal_viol > 1e-8:
                        print("Warning: Primal violation is too LARGE")
                        accepted = False

                    if max_dual_viol > 1e-6:
                        print("Warning: Dual violation is too LARGE.")
                        accepted = False

                    if accepted:

                        numvar = task.getnumvar()
                        print("Optimal primal solution")
                        xj = [0.]
                        for j in range(numvar):
                            task.getxxslice(whichsol, j, j + 1, xj)
                            print("x[%d]: %e\n" % (j, xj[0]))

                    else:
                        #Print detailed information about the solution
                        task.analyzesolution(mosek.streamtype.log, whichsol)

                elif solsta in [mosek.solsta.dual_infeas_cer, mosek.solsta.prim_infeas_cer]:

                    print("Primal or dual infeasibility certificate found.")

                elif solsta == mosek.solsta.unkwown:
                    print("The status of the solution is unknown.")
                else:
                    print("Other solution status")

    except mosek.Error as e:
        print(e)