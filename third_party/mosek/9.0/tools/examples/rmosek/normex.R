##
#
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      normex.R
#
#  Purpose:   Demonstrates various norm minimization problems
#
#             * least squares regression
#             * ridge regularization
#             * lasso regularization
#             * p-norm minimization
#
##
library("Rmosek")

# This function runs a few examples
normex <- function()
{
    # Create some random data
    # We will attempt to fit various polynomials to
    # a noisy degree 3 polynomial p(t)
    N  <- 100
    x1 <- sort(runif(N))
    p  <- function(t) { 1.5*t^3-2*t^2+0.5*t^1-1 }
    y1 <- p(x1) + 0.01*runif(N);

    # Least squares regression
    deg<- 5
    xp <- outer(x1, 0:deg, '^')
    x <- norm_lse(xp, y1, matrix(0, 0, deg+1), numeric(0))
    print(sprintf("LSE: best fit %.2f x^%d + %.2f x^%d + ... + %.2f", x[deg+1], deg, x[deg], deg-1, x[1]))

    # With ridge regularization
    x <- norm_lse_reg(xp, y1, matrix(0, 0, deg+1), numeric(0), 0.2)
    print(sprintf("Ridge: best fit %.2f x^%d + %.2f x^%d + ... + %.2f", x[deg+1], deg, x[deg], deg-1, x[1]))

    # With squared version of ridge regularization
    x <- norm_lse_reg_quad(xp, y1, matrix(0, 0, deg+1), numeric(0), 0.2)
    print(sprintf("QRidge: best fit %.2f x^%d + %.2f x^%d + ... + %.2f", x[deg+1], deg, x[deg], deg-1, x[1]))

    # Completely random large data for lasso example
    N <- 10
    K <- 3000
    F <- matrix(rnorm(K*N), K, N)
    g <- runif(K)
    print(sprintf("Lasso regularization"))
    for (gamma in c(0.01, 0.1, 0.3, 0.6, 0.9, 1.3))
    {
        x <- norm_lse_lasso(F, g, matrix(0,0,N), numeric(0), gamma)
        z <- data.frame(x)
        print(sprintf('Gamma %.4f  density %.0f  |Fx-g|_2: %.4f', gamma, sum(abs(z)>1e-6)/N*100, norm(F%*%x-g, type="2")))
    }

    # Example with the p-norm cone for various p
    # We add a far outlier to the first example
    x12 <- c(x1, 0.73)
    y12 <- c(y1, -0.99)
    xp2 <- outer(x12, 0:deg, '^')
    for (p in c(1.1, 2.0, 3.0, 6.0))
    {
       x <- norm_p_norm(xp2, y12, matrix(0, 0, deg+1), numeric(0), p)
       print(sprintf("p = %.1f: best fit %.2f x^%d + %.2f x^%d + ... + %.2f", p, x[deg+1], deg, x[deg], deg-1, x[1]))
    }
}

# Least squares regression
# minimize \|Fx-g\|_2
norm_lse <- function(F,g,A,b)
{
    n <- dim(F)[2]
    k <- length(g)
    m <- dim(A)[1]

    # Linear constraints in [x; t]
    prob <- list(sense="min")
    prob$A <- cbind(A, rep(0, m))
    prob$bx <- rbind(rep(-Inf, n+1), rep(Inf, n+1))
    prob$bc <- rbind(b, b)
    prob$c <- c(rep(0, n), 1)

    # Affine conic constraint
    prob$F <- rbind( c(rep(0,n), 1),
                     cbind(F, rep(0, k)) )
    prob$g <- c(0, -g)
    prob$cones <- matrix(list("QUAD", k+1, NULL))
    rownames(prob$cones) <- c("type","dim","conepar")

    # Solve
    r <- mosek(prob, list(verbose=1))
    stopifnot(identical(r$response$code, 0))
    r$sol$itr$xx[1:n]
}

# Least squares regression with regularization
# minimize \|Fx-g\|_2 + gamma*\|x\|_2
norm_lse_reg <- function(F,g,A,b,gamma)
{
    n <- dim(F)[2]
    k <- length(g)
    m <- dim(A)[1]

    # Linear constraints in [x; t1; t2]
    prob <- list(sense="min")
    prob$A <- cbind(A, rep(0, m), rep(0, m))
    prob$bx <- rbind(rep(-Inf, n+2), rep(Inf, n+2))
    prob$bc <- rbind(b, b)
    prob$c <- c(rep(0, n), 1, gamma)

    # Affine conic constraint
    prob$F <- rbind(     c(rep(0,n), 1,         0),
                     cbind(F,        rep(0, k), rep(0, k)),
                         c(rep(0,n), 0,         1),
                     cbind(diag(n),  rep(0, n), rep(0, n)))
    prob$g <- c(0, -g, rep(0, n+1))
    prob$cones <- cbind(matrix(list("QUAD", k+1, NULL)), 
                        matrix(list("QUAD", n+1, NULL)))
    rownames(prob$cones) <- c("type","dim","conepar")

    # Solve
    r <- mosek(prob, list(verbose=1))
    stopifnot(identical(r$response$code, 0))
    r$sol$itr$xx[1:n]
}

# Least squares regression with regularization
# The "classical" quadratic version
# minimize \|Fx-g\|_2^2 + gamma*\|x\|_2^2
norm_lse_reg_quad <- function(F,g,A,b,gamma)
{
    n <- dim(F)[2]
    k <- length(g)
    m <- dim(A)[1]

    # Linear constraints in [x; t1; t2]
    prob <- list(sense="min")
    prob$A <- cbind(A, rep(0, m), rep(0, m))
    prob$bx <- rbind(rep(-Inf, n+2), rep(Inf, n+2))
    prob$bc <- rbind(b, b)
    prob$c <- c(rep(0, n), 1, gamma)

    # Affine conic constraint
    prob$F <- rbind(     c(rep(0,n), 1,         0),
                         c(rep(0, n+2)),
                     cbind(F,        rep(0, k), rep(0, k)),
                         c(rep(0,n), 0,         1),
                         c(rep(0, n+2)),
                     cbind(diag(n),  rep(0, n), rep(0, n)))
    prob$g <- c(0, 0.5, -g, 0, 0.5, rep(0, n))
    prob$cones <- cbind(matrix(list("RQUAD", k+2, NULL)), 
                        matrix(list("RQUAD", n+2, NULL)))
    rownames(prob$cones) <- c("type","dim","conepar")

    # Solve
    r <- mosek(prob, list(verbose=1))
    stopifnot(identical(r$response$code, 0))
    r$sol$itr$xx[1:n]
}

# Least squares regression with lasso regularization
# minimize \|Fx-g\|_2 + gamma*\|x\|_1
norm_lse_lasso <- function(F,g,A,b,gamma)
{
    n <- dim(F)[2]
    k <- length(g)
    m <- dim(A)[1]

    # Linear constraints in [x; u; t1; t2]
    prob <- list(sense="min")
    prob$A <- rbind(cbind(A,    matrix(0, m, n+2)),
                    cbind(diag(n), diag(n), matrix(0, n, 2)),
                    cbind(-diag(n), diag(n), matrix(0, n, 2)),
                    c(rep(0, n), rep(-1, n), 0, 1))
    prob$bx <- rbind(rep(-Inf, 2*n+2), rep(Inf, 2*n+2))
    prob$bc <- rbind(c(b, rep(0, 2*n+1)), c(b, rep(Inf, 2*n+1)))
    prob$c <- c(rep(0, 2*n), 1, gamma)

    # Affine conic constraint
    prob$F <- rbind(c(rep(0, 2*n), 1, 0),
                    cbind(F, matrix(0, k, n+2)))
    prob$g <- c(0, -g)
    prob$cones <- matrix(list("QUAD", k+1, NULL))
    rownames(prob$cones) <- c("type","dim","conepar")

    # Solve
    r <- mosek(prob, list(verbose=1))
    stopifnot(identical(r$response$code, 0))
    r$sol$itr$xx[1:n]
}

# P-norm minimization
# minimize \|Fx-g\|_p
norm_p_norm <- function(F,g,A,b,p)
{
    n <- dim(F)[2]
    k <- length(g)
    m <- dim(A)[1]

    # Linear constraints in [x; r; t]
    prob <- list(sense="min")
    prob$A <- rbind(cbind(A,    matrix(0, m, k+1)),
                    c(rep(0, n), rep(1, k), -1))
    prob$bx <- rbind(rep(-Inf, n+k+1), rep(Inf, n+k+1))
    prob$bc <- rbind(c(b, 0), c(b, 0))
    prob$c <- c(rep(0, n+k), 1)

    # Permutation matrix which picks triples (r_i, t, F_ix-g_i)
    M <- cbind(sparseMatrix(seq(1,3*k,3), seq(1,k), x=rep(1,k), dims=c(3*k,k)),
               sparseMatrix(seq(2,3*k,3), seq(1,k), x=rep(1,k), dims=c(3*k,k)),
               sparseMatrix(seq(3,3*k,3), seq(1,k), x=rep(1,k), dims=c(3*k,k)))

    # Affine conic constraint
    prob$F <- M %*% rbind(cbind(matrix(0, k, n), diag(k), rep(0, k)),
                          cbind(matrix(0, k, n+k), rep(1, k)),
                          cbind(F, matrix(0, k, k+1)))
    prob$g <- as.numeric(M %*% c(rep(0, 2*k), -g))
    prob$cones <- matrix(list("PPOW", 3, c(1, p-1)), nrow=3, ncol=k)
    rownames(prob$cones) <- c("type","dim","conepar")

    # Solve
    r <- mosek(prob, list(verbose=1))
    stopifnot(identical(r$sol$itr$prosta, "PRIMAL_AND_DUAL_FEASIBLE"))
    r$sol$itr$xx[1:n]
}

normex()