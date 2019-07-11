##
# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      portfolio_4_transcost.py
#
#  Purpose :   Implements a basic portfolio optimization model
#              with fixed setup costs and transaction costs
#              as a mixed-integer problem.
##

from mosek.fusion import *
import numpy as np

"""
    Description:
        Extends the basic Markowitz model with a market cost term.

    Input:
        n: Number of assets
        mu: An n dimensional vector of expected returns
        GT: A matrix with n columns so (GT')*GT  = covariance matrix
        x0: Initial holdings 
        w: Initial cash holding
        gamma: Maximum risk (=std. dev) accepted
        f: If asset j is traded then a fixed cost f_j must be paid
        g: If asset j is traded then a cost g_j must be paid for each unit traded

    Output:
       Optimal expected return and the optimal portfolio     

"""
def MarkowitzWithTransactionsCost(n,mu,GT,x0,w,gamma,f,g):
    # Upper bound on the traded amount
    w0 = w+sum(x0)
    u = n*[w0]

    with Model("Markowitz portfolio with transaction costs") as M:
        #M.setLogHandler(sys.stdout)

        # Defines the variables. No shortselling is allowed.
        x = M.variable("x", n, Domain.greaterThan(0.0))

        # Additional "helper" variables 
        z = M.variable("z", n, Domain.unbounded())   
        # Binary variables
        y = M.variable("y", n, Domain.binary())

        #  Maximize expected return
        M.objective('obj', ObjectiveSense.Maximize, Expr.dot(mu,x))

        # Invest amount + transactions costs = initial wealth
        M.constraint('budget', Expr.add([ Expr.sum(x), Expr.dot(f,y),Expr.dot(g,z)] ), Domain.equalsTo(w0))

        # Imposes a bound on the risk
        M.constraint('risk', Expr.vstack( gamma,Expr.mul(GT,x)), Domain.inQCone())

        # z >= |x-x0| 
        M.constraint('buy', Expr.sub(z,Expr.sub(x,x0)),Domain.greaterThan(0.0))
        M.constraint('sell', Expr.sub(z,Expr.sub(x0,x)),Domain.greaterThan(0.0))
        # Alternatively, formulate the two constraints as
        #M.constraint('trade', Expr.hstack(z,Expr.sub(x,x0)), Domain.inQcone())

        # Constraints for turning y off and on. z-diag(u)*y<=0 i.e. z_j <= u_j*y_j
        M.constraint('y_on_off', Expr.sub(z,Expr.mulElm(u,y)), Domain.lessThan(0.0))

        # Integer optimization problems can be very hard to solve so limiting the 
        # maximum amount of time is a valuable safe guard
        M.setSolverParam('mioMaxTime', 180.0) 
        M.solve()

        return x.level(), y.level(), z.level() 

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


    f = n*[0.01]
    g = n*[0.001]
    gamma = gammas[0]
    xsol, ysol, zsol = MarkowitzWithTransactionsCost(n,mu,GT,x0,w,gamma,f,g)
    print("\n-----------------------------------------------------------------------------------");
    print('Markowitz portfolio optimization with transactions cost')
    print("-----------------------------------------------------------------------------------\n");
    print('Expected return: %.4e Std. deviation: %.4e Transactions cost: %.4e' % \
          (np.dot(mu,xsol),gamma, np.dot(f,ysol)+np.dot(g,zsol)))