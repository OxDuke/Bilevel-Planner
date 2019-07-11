##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      portfolio_3_impact.R
#
#  Purpose :   Implements a basic portfolio optimization model
#              with x^(3/2) market impact costs.
##
library("Rmosek")

MarkowitzWithMarketImpact <- function(
    n,          # Number of assets
    mu,         # An n-dimmensional vector of expected returns
    GT,         # A matrix with n columns so (GT')*GT  = covariance matrix
    x0,         # Initial holdings 
    w,          # Initial cash holding
    gamma,      # Maximum risk (=std. dev) accepted
    m)          # Market impacts (we use m_j|x_j-x0_j|^3/2 for j'th asset)
{
    prob <- list(sense="max")
    prob$c <- c(mu, rep(0,n))
    prob$A <- cbind(Matrix(1.0,ncol=n), t(m))
    prob$bc <- rbind(blc=w+sum(x0),
                     buc=w+sum(x0))
    prob$bx <- rbind(blx=rep(0.0,2*n),
                     bux=rep(Inf,2*n))

    # Specify the affine conic constraints.
    # 1) Risk
    Fr <- rbind(
        Matrix(0.0,nrow=1,ncol=2*n), 
        cbind(GT, Matrix(0.0,nrow=n,ncol=n))
    )
    gr <- c(gamma,rep(0,n))
    Kr <- matrix(list("QUAD", 1+n, NULL), nrow=3, ncol=1)

    # 2) Market impact (t_j >= |x_j-x0_j|^3/2)
    # [    t_j     ]
    # [     1      ] \in PPOW(2,1)
    # [ x_j - x0_j ]
    Fm <- sparseMatrix(
                 i=c(seq(from=1,by=3,len=n), seq(from=3,by=3,len=n)),
                 j=c(seq(from=n+1,len=n),    seq(from=1,len=n)),
                 x=c(rep(1.0,n),             rep(1.0,n)),
                 dims=c(3*n, 2*n))
    gm <- rep(c(0,1,0), n)
    gm[seq(from=3,by=3,len=n)] <- -x0
    Km <- matrix(list("PPOW", 3, c(2,1)), nrow=3, ncol=n)
    
    prob$F <- rbind(Fr, Fm)
    prob$g <- c(gr, gm)
    prob$cones <- cbind(Kr, Km)
    rownames(prob$cones) <- c("type","dim","conepar")

    # Solve the problem
    r <- mosek(prob,list(verbose=1))
    stopifnot(identical(r$response$code, 0))

    # Return the solution
    x <- r$sol$itr$xx[1:n]
    tx <- r$sol$itr$xx[(n+1):(2*n)]
    list(expret=drop(mu %*% x), stddev=gamma, cost=drop(m %*% tx), x=x)
}

# Example of input
n      <- 3;
w      <- 1.0;   
mu     <- c(0.1073, 0.0737, 0.0627)
x0     <- c(0.0, 0.0, 0.0)
gamma  <- 0.035
m      <- c(0.01, 0.01, 0.01)
GT     <- rbind(c(0.1667, 0.0232, 0.0013 ),
                c(0.0000, 0.1033, -0.0022),
                c(0.0000, 0.0000, 0.0338 ))

r <- MarkowitzWithMarketImpact(n,mu,GT,x0,w,gamma,m)
print(sprintf('Expected return: %.4e   Std. deviation: %.4e  Market impact cost: %.4e', r$expret, r$stddev, r$cost))