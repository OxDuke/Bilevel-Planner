#
#  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File:      concurrent.py
#
#  Purpose: Demonstrates a simple implementation of a concurrent optimizer.
#           
#           The concurrent optimizer starts a few parallel optimizations
#           of the same problem using different algorithms, and reports
#           a solution when the first optimizer is ready.
#
#           This example also demonstrates how to define a simple callback handler
#           that stops the optimizer when requested.
#
import mosek, sys
from threading import Thread

# Defines a Mosek callback function whose only function
# is to indicate if the optimizer should be stopped.
stop = False
firstStop = -1
def cbFun(code):
  return 1 if stop else 0

# Stream handling method
def streamwriter(s):
  sys.stdout.write('{0}'.format(s))

# firstOK, res, trm = optimize(tasks)
#
# Takes a list of tasks and optimizes then in parallel. The
# response code and termination code from each optimization is
# returned in ``res`` and ``trm``.
#
# When one task completes with rescode == ok, others are terminated.
#
# firstOK is the index of the first task that returned
# with rescode == ok. Whether or not this task contains the
# most valuable answer, is for the caller to decide. If none
# completed without error returns -1.
def runTask(num, task, res, trm):
  global stop
  global firstStop
  try:
    trm[num] = task.optimize();
    res[num] = mosek.rescode.ok
  except mosek.MosekException as e:
    trm[num] = mosek.rescode.err_unknown
    res[num] = e.errno
  finally:
    # If this finished with success, inform other tasks to interrupt
    if res[num] == mosek.rescode.ok:
      if not stop:
        firstStop = num
      stop = True

def optimize(tasks):
  n = len(tasks)
  res = [ mosek.rescode.err_unknown ] * n
  trm = [ mosek.rescode.err_unknown ] * n  

  # Set a callback function 
  for t in tasks:
    t.set_Progress(cbFun)
  
  # Start parallel optimizations, one per task
  jobs = [ Thread(target=runTask, args=(i, tasks[i], res, trm)) for i in range(n) ]
  for j in jobs:
    j.start()
  for j in jobs:
    j.join()

  # For debugging, print res and trm codes for all optimizers
  for i in range(n):
    print("Optimizer  {0}   res {1}   trm {2}".format(i, res[i], trm[i]))

  return firstStop, res, trm

#
# idx, winTask, winTrm, winRes = optimizeconcurrent(task, optimizers)
#
# Given a continuous task, set up jobs to optimize it 
# with a list of different solvers.
#
# Returns an index, corresponding to the optimization
# task that is returned as winTask. This is the task
# with the best possible status of those that finished.
# If none task is considered successful returns -1.
def optimizeconcurrent(task, optimizers):
  n = len(optimizers)
  tasks = [ mosek.Task(task) for _ in range(n) ]

  # Choose various optimizers for cloned tasks
  for i in range(n):
    tasks[i].putintparam(mosek.iparam.optimizer, optimizers[i])

  # Solve tasks in parallel
  firstOK, res, trm = optimize(tasks)

  if firstOK >= 0:
    return firstOK, tasks[firstOK], trm[firstOK], res[firstOK]
  else:
    return -1, None, None, None

#
# idx, winTask, winTrm, winRes = optimizeconcurrent(task, optimizers)
#
# Given a mixed-integer task, set up jobs to optimize it 
# with different values of seed. That will lead to
# different execution paths of the optimizer.
#
# Returns an index, corresponding to the optimization
# task that is returned as winTask. This is the task
# with the best value of the objective function.
# If none task is considered successful returns -1.
#
# Typically, the input task would contain a time limit. The two
# major scenarios are:
# 1. Some clone ends before time limit - then it has optimum.
# 2. All clones reach time limit - pick the one with best objective.
def optimizeconcurrentMIO(task, seeds):
  n = len(seeds)
  tasks = [ mosek.Task(task) for _ in range(n) ]  
  
  # Choose various seeds for cloned tasks
  for i in range(n):
    tasks[i].putintparam(mosek.iparam.mio_seed, seeds[i])

  # Solve tasks in parallel
  firstOK, res, trm = optimize(tasks)

  if firstOK >= 0:
    # Pick the task that ended with res = ok
    # and contains an integer solution with best objective value
    sense = task.getobjsense();
    bestObj = 1.0e+10 if sense == mosek.objsense.minimize else -1.0e+10
    bestPos = -1

    for i in range(n):
      print("{0}   {1}".format(i,tasks[i].getprimalobj(mosek.soltype.itg)))

    for i in range(n):
      if ((res[i] == mosek.rescode.ok) and
          (tasks[i].getsolsta(mosek.soltype.itg) == mosek.solsta.prim_feas or
           tasks[i].getsolsta(mosek.soltype.itg) == mosek.solsta.integer_optimal) and
         ((tasks[i].getprimalobj(mosek.soltype.itg) < bestObj) 
           if (sense == mosek.objsense.minimize) else 
          (tasks[i].getprimalobj(mosek.soltype.itg) > bestObj))):
        bestObj = tasks[i].getprimalobj(mosek.soltype.itg)
        bestPos = i

    if bestPos >= 0:
      return bestPos, tasks[bestPos], trm[bestPos], res[bestPos]

  return -1, None, None, None

 
# This is an example of how one can use the methods
#       optimizeconcurrent
#       optimizeconcurrentMIO
#
#   argv[0] : name of file with input problem
#   argv[1]: (optional) time limit
def main(argv):
  with mosek.Env() as env:
    with mosek.Task(env, 0, 0) as task:
      if len(argv) >= 2:
        task.readdata(argv[1])
      else:
        task.readdata("../data/25fv47.mps")

      # Optional time limit
      if len(argv) >= 3:
        task.putdouparam(mosek.dparam.optimizer_max_time, float(argv[2]))

      if (task.getnumintvar() == 0):
        # If the problem is continuous
        # optimize it with three continuous optimizers.
        # (Simplex will fail for non-linear problems)
        optimizers = [
          mosek.optimizertype.conic,
          mosek.optimizertype.dual_simplex,
          mosek.optimizertype.primal_simplex
        ]

        idx, t, trm, res = optimizeconcurrent(task, optimizers)
      else:
        # Mixed-integer problem.
        # Try various seeds.
        seeds = [ 42, 13, 71749373 ]

        idx, t, trm, res = optimizeconcurrentMIO(task, seeds)

      # Check results and print the best answer
      if idx >= 0:
        print("Result from optimizer with index {0}:  res {1}  trm {2}".format(idx, res, trm))
        t.set_Stream(mosek.streamtype.log, streamwriter)
        t.optimizersummary(mosek.streamtype.log)
        t.solutionsummary(mosek.streamtype.log);
      else:
        print("All optimizers failed.")


if __name__ == "__main__":
    main(sys.argv)