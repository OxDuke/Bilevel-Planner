##
# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      portfolio_3_impact.py
#
#  Purpose :   Implements a basic portfolio optimization model
#              with x^(3/2) market impact costs.
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
        m: It is assumed that  market impact cost for the j'th asset is
           m_j|x_j-x0_j|^3/2

    Output:
       Optimal expected return and the optimal portfolio     

"""
def MarkowitzWithMarketImpact(n,mu,GT,x0,w,gamma,m):
    with  Model("Markowitz portfolio with market impact") as M:

        #M.setLogHandler(sys.stdout) 
    
        # Defines the variables. No shortselling is allowed.
        x = M.variable("x", n, Domain.greaterThan(0.0))
        
        # Variables computing market impact 
        t = M.variable("t", n, Domain.unbounded())

        #  Maximize expected return
        M.objective('obj', ObjectiveSense.Maximize, Expr.dot(mu,x))

        # Invested amount + slippage cost = initial wealth
        M.constraint('budget', Expr.add(Expr.sum(x),Expr.dot(m,t)), Domain.equalsTo(w+sum(x0)))

        # Imposes a bound on the risk
        M.constraint('risk', Expr.vstack(gamma,Expr.mul(GT,x)), Domain.inQCone())

        # t >= |x-x0|^1.5 using a power cone
        M.constraint('tz', Expr.hstack(t, Expr.constTerm(n, 1.0), Expr.sub(x,x0)), Domain.inPPowerCone(2.0/3.0))

        M.solve()

        return x.level(), t.level()


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
                  
    # Somewhat arbitrary choice of m
    gamma = gammas[0]
    m = n*[1.0e-2]
    xsol, tsol = MarkowitzWithMarketImpact(n,mu,GT,x0,w,gamma,m)
    print("\n-----------------------------------------------------------------------------------");
    print('Markowitz portfolio optimization with market impact cost')
    print("-----------------------------------------------------------------------------------\n");
    print('Expected return: %.4e Std. deviation: %.4e Market impact cost: %.4e' % \
          (np.dot(mu,xsol),gamma,np.dot(m,tsol)))