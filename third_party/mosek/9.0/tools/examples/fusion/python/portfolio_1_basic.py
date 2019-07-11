##
# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      portfolio_1_basic.py
#
#  Purpose :   Implements a basic portfolio optimization model.
#
##

from mosek.fusion import *
import numpy as np

"""
Purpose:
    Computes the optimal portfolio for a given risk 
 
Input:
    n: Number of assets
    mu: An n dimensional vector of expected returns
    GT: A matrix with n columns so (GT')*GT  = covariance matrix
    x0: Initial holdings 
    w: Initial cash holding
    gamma: Maximum risk (=std. dev) accepted
 
Output:
    Optimal expected return and the optimal portfolio     
""" 
def BasicMarkowitz(n,mu,GT,x0,w,gamma):
    
    with  Model("Basic Markowitz") as M:

        # Redirect log output from the solver to stdout for debugging. 
        # if uncommented.
        # M.setLogHandler(sys.stdout) 
        
        # Defines the variables (holdings). Shortselling is not allowed.
        x = M.variable("x", n, Domain.greaterThan(0.0))
        
        #  Maximize expected return
        M.objective('obj', ObjectiveSense.Maximize, Expr.dot(mu,x))
        
        # The amount invested  must be identical to initial wealth
        M.constraint('budget', Expr.sum(x), Domain.equalsTo(w+sum(x0)))
        
        # Imposes a bound on the risk
        M.constraint('risk', Expr.vstack( gamma,Expr.mul(GT,x)), Domain.inQCone())

        # Solves the model.
        M.solve()

        return np.dot(mu,x.level())


if __name__ == '__main__':    

    n      = 3;
    w      = 1.0;   
    mu     = [0.1073,0.0737,0.0627]
    x0     = [0.0,0.0,0.0]
    gammas = [0.035,0.040,0.050,0.060,0.070,0.080,0.090]
    GT     = [
        [ 0.166673333200005, 0.0232190712557243 ,  0.0012599496030238 ],
        [ 0.0              , 0.102863378954911  , -0.00222873156550421],
        [ 0.0              , 0.0                ,  0.0338148677744977 ]
    ]


    print("\n-----------------------------------------------------------------------------------");
    print('Basic Markowitz portfolio optimization')
    print("-----------------------------------------------------------------------------------\n");
    for gamma in gammas:
        er = BasicMarkowitz(n,mu,GT,x0,w,gamma)
        print('Expected return: %.4e Std. deviation: %.4e' % (er,gamma))