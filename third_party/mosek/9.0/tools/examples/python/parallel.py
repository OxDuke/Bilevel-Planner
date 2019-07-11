#
#  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File:      parallel.py
#
#  Purpose: Demonstrates parallel optimization
#
import mosek, sys
from threading import Thread

# A run method to optimize a single task
def runTask(num, task, res, trm):
  try:
    trm[num] = task.optimize();
    res[num] = mosek.rescode.ok
  except mosek.MosekException as e:
    trm[num] = mosek.rescode.err_unknown
    res[num] = e.errno

# Takes a list of tasks and optimizes them in parallel threads. The
# response code and termination code from each optimization is
#  stored in ``res`` and ``trm``.
def paropt(tasks):
  n = len(tasks)
  res = [ mosek.rescode.err_unknown ] * n
  trm = [ mosek.rescode.err_unknown ] * n  

  # Start parallel optimizations, one per task
  jobs = [ Thread(target=runTask, args=(i, tasks[i], res, trm)) for i in range(n) ]
  for j in jobs:
    j.start()
  for j in jobs:
    j.join()

  return res, trm

 
# Example of how to use ``paropt``. 
# Optimizes tasks whose names were read from command line.
def main(argv):
  n = len(argv) - 1
  tasks = []

  with mosek.Env() as env:
    for i in range(n):
      t = mosek.Task(env, 0, 0)
      t.readdata(argv[i+1])
      # Each task will be single-threaded
      t.putintparam(mosek.iparam.intpnt_multi_thread, mosek.onoffkey.off)
      tasks.append(t)

    res, trm = paropt(tasks)

    for i in range(n):
      print("Task  {0}  res {1}   trm {2}   obj_val  {3}  time {4}".format(
             i, 
             res[i], 
             trm[i],
             tasks[i].getdouinf(mosek.dinfitem.intpnt_primal_obj),
             tasks[i].getdouinf(mosek.dinfitem.optimizer_time)))

if __name__ == "__main__":
    main(sys.argv)