##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      portfolio_5_card.R
#
#  Description :  Implements a basic portfolio optimization model
#                 with cardinality constraints on number of assets traded.
##
library("Rmosek")

MarkowitzWithCardinality <- function(
    n,          # Number of assets
    mu,         # An n-dimmensional vector of expected returns
    GT,         # A matrix with n columns so (GT')*GT  = covariance matrix
    x0,         # Initial holdings 
    w,          # Initial cash holding
    gamma,      # Maximum risk (=std. dev) accepted
    k)          # Cardinality bound
{

    # Upper bound on the traded amount
    u <- w+sum(x0)

    prob <- list(sense="max")
    prob$c <- c(mu, rep(0,2*n))

    # Specify linear constraints
    # [ e'  0   0  ]           =   w + e'*x0
    # [ I  -I   0  ]   [ x ]  <=  x0
    # [ I   I   0  ] * [ z ]  >=  x0
    # [ 0   I  -U  ]   [ y ]  <=  0
    # [ 0   0   e' ]          <=  k
    prob$A <- rbind(cbind(Matrix(1.0,ncol=n), Matrix(0.0,ncol=2*n)),
                    cbind(Diagonal(n, 1.0),   -Diagonal(n, 1.0), Matrix(0,n,n)),
                    cbind(Diagonal(n, 1.0),   Diagonal(n, 1.0),  Matrix(0,n,n)),
                    cbind(Matrix(0,n,n),      Diagonal(n, 1.0),  Diagonal(n, -u)),
                    cbind(Matrix(0.0,ncol=2*n), Matrix(1.0,ncol=n)))
    prob$bc <- rbind(blc=c(w+sum(x0), rep(-Inf,n), x0, rep(-Inf,n), 0.0),
                     buc=c(w+sum(x0), x0, rep(Inf,n), rep(0.0,n), k))
    # No shortselling and the linear bound 0 <= y <= 1     
    prob$bx <- rbind(blx=c(rep(0.0,n), rep(-Inf,n), rep(0.0,n)),
                     bux=c(rep(Inf,n), rep(Inf, n), rep(1.0,n)))

    # Specify the affine conic constraints for risk
    prob$F <- rbind(
        Matrix(0.0,nrow=1,ncol=3*n), 
        cbind(GT, Matrix(0.0,nrow=n,ncol=2*n))
    )
    prob$g <- c(gamma,rep(0,n))
    prob$cones <- matrix(list("QUAD", 1+n, NULL), nrow=3, ncol=1)
    rownames(prob$cones) <- c("type","dim","conepar")

    # Demand y to be integer (hence binary)
    prob$intsub <- (2*n+1):(3*n);

    # Solve the problem
    r <- mosek(prob,list(verbose=1))
    stopifnot(identical(r$response$code, 0))

    # Return the solution
    x <- r$sol$int$xx[1:n]
    list(card=k, expret=drop(mu %*% x), x=x)
}

# Example of input
n      <- 3;
w      <- 1.0;   
mu     <- c(0.1073, 0.0737, 0.0627)
x0     <- c(0.0, 0.0, 0.0)
gamma  <- 0.035
GT     <- rbind(c(0.1667, 0.0232, 0.0013 ),
                c(0.0000, 0.1033, -0.0022),
                c(0.0000, 0.0000, 0.0338 ))

print(sprintf("Markowitz portfolio optimization with cardinality constraints"))
for (k in 1:n) {
    r <- MarkowitzWithCardinality(n,mu,GT,x0,w,gamma,k)
    print(sprintf("Bound: %d   Expected return: %.4e  Solution:", r$card, r$expret))
    print(r$x)
}