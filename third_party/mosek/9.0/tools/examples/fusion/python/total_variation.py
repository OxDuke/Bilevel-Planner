##
# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      total_variation.py
#
# Purpose:   Demonstrates how to solve a total 
#            variation problem using the Fusion API.
##
import sys
import mosek
from mosek.fusion import *
import numpy as np

def total_var(n,m,f,sigma):
    with Model('TV') as M:

        u= M.variable( [n+1,m+1], Domain.inRange(0.,1.0) )
        t= M.variable( [n,m], Domain.unbounded() )

        ucore=  u.slice( [0,0], [n,m] )

        deltax= Expr.sub( u.slice( [1,0], [n+1,m] ), ucore)
        deltay= Expr.sub( u.slice( [0,1], [n,m+1] ), ucore)

        M.constraint( Expr.stack(2, t, deltax, deltay), Domain.inQCone().axis(2) )

        fmat = Matrix.dense(n,m,f)
        M.constraint( Expr.vstack(sigma, Expr.flatten( Expr.sub( fmat, ucore ) ) ),
                      Domain.inQCone() )

        M.objective( ObjectiveSense.Minimize, Expr.sum(t) )
        M.setLogHandler(sys.stdout)
        M.solve()

        return ucore.level()

#Display
def show(n,m,grid):
    try:
        import matplotlib
        matplotlib.use('Agg')   #Remove to go interactive
        import matplotlib.pyplot as plt
        import matplotlib.cm as cm
        plt.imshow(np.reshape(grid, (n,m)), extent=(0,m,0,n),
                   interpolation='nearest', cmap=cm.jet,
                   vmin=0, vmax=1)
        plt.show()
    except:
        print (grid)

#Handling a single example
def example(n,m,signal,noise,rel_sigma):
    f = signal+noise
    show(n,m,signal)
    show(n,m,f)
    u = total_var(n,m,f,rel_sigma*n*m)
    show(n,m,u)

if __name__ == '__main__':
    np.random.seed(0)
    
    #Example: Linear signal with Gaussian noise
    n,m=100,200
    u = example(n,m,
        np.reshape([[1.0*(i+j)/(n+m) for i in range(m)] for j in range(n)], n*m),
        np.random.normal(0., 0.08, n*m),
        0.0006
    )