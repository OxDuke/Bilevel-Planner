##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      portfolio_2_frontier.R
#
#  Purpose :   To implements a basic portfolio optimization model.
#              Computes points on the efficient frontier.
##
library("Rmosek")

EfficientFrontier <- function(
    n,          # Number of assets
    mu,         # An n-dimmensional vector of expected returns
    GT,         # A matrix with n columns so (GT')*GT  = covariance matrix
    x0,         # Initial holdings 
    w,          # Initial cash holding
    alphas)     # List of risk penalties (we maximize expected return - alpha * variance)
{
    prob <- list(sense="max")
    prob$A <- cbind(Matrix(1.0, ncol=n), 0.0)
    prob$bc <- rbind(blc=w+sum(x0), 
                     buc=w+sum(x0))
    prob$bx <- rbind(blx=c(rep(0.0,n), -Inf),
                     bux=rep(Inf,n+1))

    # Specify the affine conic constraints.
    NUMCONES <- 1
    prob$F <- rbind(
        cbind(Matrix(0.0,ncol=n), 1.0),
        rep(0, n+1),
        cbind(GT                , 0.0)
    )
    prob$g <- c(0, 0.5, rep(0, n))
    prob$cones <- matrix(list(), nrow=3, ncol=NUMCONES)
    rownames(prob$cones) <- c("type","dim","conepar")

    prob$cones[-3,1] <- list("RQUAD", n+2)

    frontier <- matrix(NaN, ncol=3, nrow=length(alphas))
    colnames(frontier) <- c("alpha","exp.ret.","variance")

    for (i in seq_along(alphas))
    {
        prob$c <- c(mu, -alphas[i])

        r <- mosek(prob, list(verbose=1))
        stopifnot(identical(r$response$code, 0))

        x     <- r$sol$itr$xx[1:n]
        gamma <- r$sol$itr$xx[n+1]
        
        frontier[i,] <- c(alphas[i], drop(mu%*%x), gamma)
    }

    frontier
}

# Example of input
n      <- 3;
w      <- 1.0;   
mu     <- c(0.1073, 0.0737, 0.0627)
x0     <- c(0.0, 0.0, 0.0)
alphas <- c(0.0, 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5)
GT     <- rbind(c(0.1667, 0.0232, 0.0013 ),
                c(0.0000, 0.1033, -0.0022),
                c(0.0000, 0.0000, 0.0338 ))

frontier <- EfficientFrontier(n,mu,GT,x0,w,alphas)
print(frontier)