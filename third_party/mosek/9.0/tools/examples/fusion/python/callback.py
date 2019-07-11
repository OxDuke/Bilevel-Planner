##
#   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#   File:      callback.py
#
#   Purpose:   To demonstrate how to use the progress
#              callback.
#
#
#              callback psim
#              callback dsim
#              callback intpnt
#              The first argument tells which optimizer to use
#              i.e. psim is primal simplex, dsim is dual simplex
#              and intpnt is interior-point.
##
import sys
import numpy
from mosek.fusion import *
from mosek import callbackcode, iinfitem, dinfitem, liinfitem

def makeUserCallback(model, maxtime):
    def userCallback(caller,
                     douinf,
                     intinf,
                     lintinf):
        opttime = 0.0

        if caller == callbackcode.begin_intpnt:
            print("Starting interior-point optimizer")
        elif caller == callbackcode.intpnt:
            itrn = intinf[iinfitem.intpnt_iter]
            pobj = douinf[dinfitem.intpnt_primal_obj]
            dobj = douinf[dinfitem.intpnt_dual_obj]
            stime = douinf[dinfitem.intpnt_time]
            opttime = douinf[dinfitem.optimizer_time]

            print("Iterations: %-3d" % itrn)
            print("  Elapsed time: %6.2f(%.2f) " % (opttime, stime))
            print("  Primal obj.: %-18.6e  Dual obj.: %-18.6e" % (pobj, dobj))
        elif caller == callbackcode.end_intpnt:
            print("Interior-point optimizer finished.")
        elif caller == callbackcode.begin_primal_simplex:
            print("Primal simplex optimizer started.")
        elif caller == callbackcode.update_primal_simplex:
            itrn = intinf[iinfitem.sim_primal_iter]
            pobj = douinf[dinfitem.sim_obj]
            stime = douinf[dinfitem.sim_time]
            opttime = douinf[dinfitem.optimizer_time]

            print("Iterations: %-3d" % itrn)
            print("  Elapsed time: %6.2f(%.2f)" % (opttime, stime))
            print("  Obj.: %-18.6e" % pobj)
        elif caller == callbackcode.end_primal_simplex:
            print("Primal simplex optimizer finished.")
        elif caller == callbackcode.begin_dual_simplex:
            print("Dual simplex optimizer started.")
        elif caller == callbackcode.update_dual_simplex:
            itrn = intinf[iinfitem.sim_dual_iter]
            pobj = douinf[dinfitem.sim_obj]
            stime = douinf[dinfitem.sim_time]
            opttime = douinf[dinfitem.optimizer_time]
            print("Iterations: %-3d" % itrn)
            print("  Elapsed time: %6.2f(%.2f)" % (opttime, stime))
            print("  Obj.: %-18.6e" % pobj)
        elif caller == callbackcode.end_dual_simplex:
            print("Dual simplex optimizer finished.")
        elif caller == callbackcode.begin_bi:
            print("Basis identification started.")
        elif caller == callbackcode.end_bi:
            print("Basis identification finished.")
        else:
            pass

        if opttime >= maxtime:
            # mosek is spending too much time. Terminate it.
            print("Too much time, terminating.")
            return 1
        return 0

    return userCallback

def userProgresCallback(caller):
    # Handle the caller code here
    pass

def main(slvr):

    if slvr not in ['psim', 'dsim', 'intpnt']:
        return

    # Big random linear optimziation problem
    n = 150
    m = 700
    A = numpy.random.uniform(0.0, 10.0, (m, n))
    c = numpy.random.uniform(0.0, 10.0, n)
    b = numpy.random.uniform(0.0, 10.0, m)

    with Model() as M:
        x = M.variable(n, Domain.unbounded())
        M.constraint(Expr.mul(A, x), Domain.lessThan(b))
        M.objective(ObjectiveSense.Maximize, Expr.dot(c, x))

        if slvr == 'psim':
            M.setSolverParam('optimizer', 'primalSimplex')
        elif slvr == 'dsim':
            M.setSolverParam('optimizer', 'dualSimplex')
        else:
            M.setSolverParam('optimizer', 'intpnt')

        userCallback = makeUserCallback(model=M, maxtime=0.07)
        M.setDataCallbackHandler(userCallback)
        M.solve()


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else 'intpnt')