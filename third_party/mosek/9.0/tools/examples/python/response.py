##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      response.py
#
#  Purpose :   This example demonstrates proper response handling
#              for problems solved with the interior-point optimizers.
#
##
import mosek
import sys

# A log message
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()

def main(args):
  filename = args[0] if len(args) >= 1 else "../data/cqo1.mps"

  try:
    # Create environment and task
    with mosek.Env() as env:
      with env.Task(0, 0) as task:
        # (Optional) set a log stream
        # task.set_Stream(mosek.streamtype.log, streamprinter)

        # (Optional) uncomment to see what happens when solution status is unknown
        # task.putintparam(mosek.iparam.intpnt_max_iterations, 1)

        # In this example we read data from a file
        task.readdata(filename)

        # Optimize
        trmcode = task.optimize()
        task.solutionsummary(mosek.streamtype.log)

        # We expect solution status OPTIMAL
        solsta = task.getsolsta(mosek.soltype.itr)

        if solsta == mosek.solsta.optimal:
          # Optimal solution. Fetch and print it.
          print("An optimal interior-point solution is located.")
          numvar = task.getnumvar()
          xx = [ 0.0 ] * numvar
          task.getxx(mosek.soltype.itr, xx)
          for i in range(numvar): 
            print("x[{0}] = {1}".format(i, xx[i]))

        elif solsta == mosek.solsta.dual_infeas_cer:
          print("Dual infeasibility certificate found.")

        elif solsta == mosek.solsta.prim_infeas_cer:
          print("Primal infeasibility certificate found.")
        
        elif solsta == mosek.solsta.unknown:
          # The solutions status is unknown. The termination code
          # indicates why the optimizer terminated prematurely.
          print("The solution status is unknown.")
          symname, desc = mosek.Env.getcodedesc(trmcode)
          print("   Termination code: {0} {1}".format(symname, desc))

        else:
          print("An unexpected solution status {0} is obtained.".format(str(solsta)))

  except mosek.Error as e:
      print("Unexpected error ({0}) {1}".format(e.errno, e.msg))

if __name__ == '__main__':
    main(sys.argv[1:])