##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      portfolio_1_basic.R
#
#  Purpose :   To implement a basic Markowitz optimization model computing 
#              the optimal expected return and portfolio for a given risk.
##
library("Rmosek")

BasicMarkowitz <- function(
    n,          # Number of assets
    mu,         # An n-dimmensional vector of expected returns
    GT,         # A matrix with n columns so (GT')*GT  = covariance matrix
    x0,         # Initial holdings 
    w,          # Initial cash holding
    gamma)      # Maximum risk (=std. dev) accepted
{
    prob <- list(sense="max")
    prob$c <- mu
    prob$A <- Matrix(1.0, ncol=n)
    prob$bc <- rbind(blc=w+sum(x0), 
                     buc=w+sum(x0))
    prob$bx <- rbind(blx=rep(0.0,n),
                     bux=rep(Inf,n))

    # Specify the affine conic constraints.
    NUMCONES <- 1
    prob$F <- rbind(
        Matrix(0.0,ncol=n), 
        GT
    )
    prob$g <- c(gamma,rep(0,n))
    prob$cones <- matrix(list(), nrow=3, ncol=NUMCONES)
    rownames(prob$cones) <- c("type","dim","conepar")

    prob$cones[-3,1] <- list("QUAD", n+1)

    # Solve the problem
    r <- mosek(prob,list(verbose=1))
    stopifnot(identical(r$response$code, 0))

    # Return the solution
    x <- r$sol$itr$xx
    list(expret=drop(mu %*% x), stddev=gamma, x=x)
}

# Example of input
n      <- 3;
w      <- 1.0;   
mu     <- c(0.1073, 0.0737, 0.0627)
x0     <- c(0.0, 0.0, 0.0)
gammas <- c(0.035, 0.040, 0.050, 0.060, 0.070, 0.080, 0.090)
GT     <- rbind(c(0.1667, 0.0232, 0.0013 ),
                c(0.0000, 0.1033, -0.0022),
                c(0.0000, 0.0000, 0.0338 ))

for (gamma in gammas) {
    r <- BasicMarkowitz(n,mu,GT,x0,w,gamma)
    print(sprintf('Expected return: %.4e   Std. deviation: %.4e', r$expret, r$stddev))
}