##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      feasrepairex1.py
#
#  Purpose :   To demonstrate how to use the MSK_relaxprimal function to
#              locate the cause of an infeasibility.
#
#  Syntax :    On command line
#              python feasrepairex1.py feasrepair.lp
#              feasrepair.lp is located in mosek\<version>\tools\examples.
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


def main(inputfile):
    # Make a MOSEK environment
    with mosek.Env() as env:
        with env.Task(0, 0) as task:
            # Attach a printer to the task
            task.set_Stream(mosek.streamtype.log, streamprinter)

            # Read data
            task.readdata(inputfile)

            task.putintparam(mosek.iparam.log_feas_repair, 3)

            task.primalrepair(None, None, None, None)

            sum_viol = task.getdouinf(mosek.dinfitem.primal_repair_penalty_obj)
            print("Minimized sum of violations = %e" % sum_viol)

            task.optimize()

            task.solutionsummary(mosek.streamtype.msg)

# call the main function
try:
    filename = "../data/feasrepair.lp"
    if len(sys.argv) > 1:
        filename = sys.argv[1]
    main(filename)
except Exception as e:
    print(e)
    raise