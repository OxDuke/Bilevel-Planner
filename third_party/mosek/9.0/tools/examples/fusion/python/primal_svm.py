#
# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      primal_svm.py
#
# Purpose: Implements a simple soft-margin SVM
#          using the Fusion API.
#
import mosek
from mosek.fusion import *

import random 

def primal_svm(m,n,X,y,CC):

    print("Number of data    : %d"%m)
    print("Number of features: %d"%n)

    with Model() as M:

        w =  M.variable('w' , n, Domain.unbounded())
        t =  M.variable('t' , 1, Domain.unbounded())
        b =  M.variable('b' , 1, Domain.unbounded())
        xi = M.variable('xi', m, Domain.greaterThan(0.))

        M.constraint(
            Expr.add( 
                Expr.mulElm( y, 
                             Expr.sub( Expr.mul(X,w), Var.repeat(b,m) ) 
                         ),
                xi
            ), 
            Domain.greaterThan( 1. ) )

        M.constraint( Expr.vstack(1., t, w), Domain.inRotatedQCone() )

        print ('   c   |    b      |     w ')

        for C in CC:
            M.objective( ObjectiveSense.Minimize, Expr.add( t, Expr.mul(C, Expr.sum(xi) ) ) )
            M.solve()

            try:
                cb = '{0:6} | {1:8f} | '.format(C,b.level()[0]) 
                wstar =' '.join([ '{0:8f}'.format(wi) for wi in w.level()]) 
                print (cb+wstar)
            except:
                pass;             

if __name__ == '__main__':

    CC=[ 500.0*i for i in range(10)]

    m  = 50
    n  = 3
    seed= 0

    random.seed(seed)
    nump= random.randint(0,50)
    numm= m - nump

    y = [  1. for i in range(nump)] + \
        [ -1. for i in range(numm)]

    mean = 1.
    var = 1.

    X=  [ [ random.gauss( mean,var) for f in range(n)  ]  for i in range(nump)] + \
        [ [ random.gauss(-mean,var) for f in range(n)  ]  for i in range(numm)] 

    primal_svm(m,n,X,y,CC)


