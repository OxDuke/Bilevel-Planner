##
# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      portfolio_5_card.py
#
#  Description :  Implements a basic portfolio optimization model
#                 with cardinality constraints on number of assets traded.
##

from mosek.fusion import *
import numpy as np 

"""
    Description:
        Extends the basic Markowitz model with cardinality constraints.

    Input:
        n: Number of assets
        mu: An n dimensional vector of expected returns
        GT: A matrix with n columns so (GT')*GT  = covariance matrix
        x0: Initial holdings 
        w: Initial cash holding
        gamma: Maximum risk (=std. dev) accepted
        k: Maximum number of assets on which we allow to change position.

    Output:
       Optimal expected return and the optimal portfolio     

"""
def MarkowitzWithCardinality(n,mu,GT,x0,w,gamma,k):
    # Upper bound on the traded amount
    w0 = w+sum(x0)
    u = n*[w0]

    with Model("Markowitz portfolio with cardinality bound") as M:
        #M.setLogHandler(sys.stdout)

        # Defines the variables. No shortselling is allowed.
        x = M.variable("x", n, Domain.greaterThan(0.0))

        # Additional "helper" variables 
        z = M.variable("z", n, Domain.unbounded())   
        # Binary variables  - do we change position in assets
        y = M.variable("y", n, Domain.binary())

        #  Maximize expected return
        M.objective('obj', ObjectiveSense.Maximize, Expr.dot(mu,x))

        # The amount invested  must be identical to initial wealth
        M.constraint('budget', Expr.sum(x), Domain.equalsTo(w+sum(x0)))

        # Imposes a bound on the risk
        M.constraint('risk', Expr.vstack( gamma,Expr.mul(GT,x)), Domain.inQCone())

        # z >= |x-x0| 
        M.constraint('buy', Expr.sub(z,Expr.sub(x,x0)),Domain.greaterThan(0.0))
        M.constraint('sell', Expr.sub(z,Expr.sub(x0,x)),Domain.greaterThan(0.0))

        # Constraints for turning y off and on. z-diag(u)*y<=0 i.e. z_j <= u_j*y_j
        M.constraint('y_on_off', Expr.sub(z,Expr.mulElm(u,y)), Domain.lessThan(0.0))

        # At most k assets change position
        M.constraint('cardinality', Expr.sum(y), Domain.lessThan(k))

        # Integer optimization problems can be very hard to solve so limiting the 
        # maximum amount of time is a valuable safe guard
        M.setSolverParam('mioMaxTime', 180.0) 
        M.solve()

        return x.level() 


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


    gamma = gammas[0]
    xsol = [ MarkowitzWithCardinality(n,mu,GT,x0,w,gamma,k) for k in range(1,n+1)]
    print("\n-----------------------------------------------------------------------------------");
    print('Markowitz portfolio optimization with cardinality constraints')
    print("-----------------------------------------------------------------------------------\n");
    for k in range(0,n):
        print('Bound: {0}   Expected return: {1:.4f}  Solution {2}'.format(k+1, np.dot(mu, xsol[k]), xsol[k]))