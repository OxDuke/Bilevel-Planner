##
#  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File:      tsp.py
#
#  Purpose: Demonstrates a simple technique to the TSP
#           usign the Fusion API.
#
##

from __future__ import print_function 
import mosek
from mosek.fusion import *
import sys

def tsp(n, A, C, remove_selfloops, remove_2_hop_loops):
    with Model() as M:

        x = M.variable([n,n], Domain.binary())

        M.constraint(Expr.sum(x,0), Domain.equalsTo(1.0))
        M.constraint(Expr.sum(x,1), Domain.equalsTo(1.0))
        M.constraint(x, Domain.lessThan( A ))

        M.objective(ObjectiveSense.Minimize, Expr.dot(C ,x))

        if remove_2_hop_loops:
            M.constraint(Expr.add(x, x.transpose()), Domain.lessThan(1.0))

        if remove_selfloops:
            M.constraint(x.diag(), Domain.equalsTo(0.))

        it = 1

        while True:
            print("\n\n--------------------\nIteration",it)
            M.solve()

            print('\nsolution cost:', M.primalObjValue())
            print('\nsolution:')
              
            cycles = []
        
            for i in range(n):
                xi = x.slice([i,0],[i+1,n])
                print(xi.level())

                for j in range(n):
                    if xi.level()[j] <= 0.5 : continue

                    found = False
                    for c in cycles:
                        if len( [ a for a in c if i in a or j in a ] )> 0:
                            c.append( [i,j] )
                            found = True
                            break

                    if not found:
                        cycles.append([ [ i,j ]])

            print('\ncycles:')
            print([c for c in cycles])

            if len(cycles)==1:
                break;

            for c in cycles:
                M.constraint(Expr.sum(x.pick(c)), Domain.lessThan( 1.0 * len(c) - 1 ))
            it = it +1
            
        return x.level(), c
    
    return [],[]

def main():
    A_i = [0,1,2,3,1,0,2,0]
    A_j = [1,2,3,0,0,2,1,3]
    C_v = [1.,1.,1.,1.,0.1,0.1,0.1,0.1]
    n = max(max(A_i),max(A_j))+1
    costs = Matrix.sparse(n,n,A_i,A_j,C_v)
    x,c = tsp(n, Matrix.sparse(n,n,A_i,A_j,1.), costs , True, True)
    x,c = tsp(n, Matrix.sparse(n,n,A_i,A_j,1.), costs , True, False)
if __name__ == '__main__':
    main()