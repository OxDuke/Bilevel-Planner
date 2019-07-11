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
from mosek.fusion import *
import sys

# Set up a small artificial conic problem to test with
def setupExample(M):
    x = M.variable('x', 3, Domain.greaterThan(0.0))
    y = M.variable('y', 3, Domain.unbounded())
    z1 = Var.vstack(y.index(0), x.slice(0, 2))
    z2 = Var.vstack(y.slice(1, 3), x.index(2))
    M.constraint("lc", Expr.dot([1.0, 1.0, 2.0], x), Domain.equalsTo(1.0))
    qc1 = M.constraint("qc1", z1, Domain.inQCone())
    qc2 = M.constraint("qc2", z2, Domain.inRotatedQCone())
    M.objective("obj", ObjectiveSense.Minimize, Expr.sum(y))

with Model() as M:
  # (Optional) set a log stream
  # M.setLoghandler(sys.stdout)

  # (Optional) uncomment to see what happens when solution status is unknown
  # M.setSolverParam("intpntMaxIterations", 1)

  # In this example we set up a small conic problem
  setupExample(M)

  # Optimize
  try:
    M.solve()

    # We expect solution status OPTIMAL (this is also default)
    M.acceptedSolutionStatus(AccSolutionStatus.Optimal)

    print("Optimal solution for x: {0}".format(M.getVariable('x').level()))
    print("Optimal primal objective: {0}".format(M.primalObjValue()))
    # .. continue analyzing the solution

  except OptimizeError as e:
    print("Optimization failed. Error: {0}".format(e))

  except SolutionError as e:
    # The solution with at least the expected status was not available.
    # We try to diagnoze why.
    print("Requested solution was not available.")
    prosta = M.getProblemStatus()

    if prosta == ProblemStatus.DualInfeasible:
      print("Dual infeasibility certificate found.")

    elif prosta == ProblemStatus.PrimalInfeasible:
      print("Primal infeasibility certificate found.")
          
    elif prosta == ProblemStatus.Unknown:
      # The solutions status is unknown. The termination code
      # indicates why the optimizer terminated prematurely.
      print("The solution status is unknown.")
      symname, desc = mosek.Env.getcodedesc(mosek.rescode(int(M.getSolverIntInfo("optimizeResponse"))))
      print("   Termination code: {0} {1}".format(symname, desc))

    else:
      print("Another unexpected problem status {0} is obtained.".format(prosta))

  except Exception as e:
    print("Unexpected error: {0}".format(e))