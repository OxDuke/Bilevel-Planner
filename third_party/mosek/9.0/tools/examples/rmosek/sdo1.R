##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      sdo1.R
#
#  Purpose :   To solve the mixed semidefinite and conic quadratic optimization problem
#
#                 minimize    Tr [2, 1, 0; 1, 2, 1; 0, 1, 2]*X + x(1)
#
#                 subject to  Tr [1, 0, 0; 0, 1, 0; 0, 0, 1]*X + x(1)               = 1
#                             Tr [1, 1, 1; 1, 1, 1; 1, 1, 1]*X        + x(2) + x(3) = 0.5
#                             X>=0,  x(1) >= sqrt(x(2)^2 + x(3)^2)
##
library("Rmosek")

getbarvarMatrix <- function(barvar, bardim)
{
    N <- as.integer(bardim)
    new("dspMatrix", x=barvar, uplo="L", Dim=c(N,N))
}

sdo1 <- function()
{
    # Specify the non-matrix variable part of the problem.
    prob <- list(sense="min")
    prob$c     <- c(1, 0, 0)
    prob$A <- sparseMatrix(i=c(1, 2, 2),
                           j=c(1, 2, 3),
                           x=c(1, 1, 1), dims=c(2, 3))
    prob$bc    <- rbind(blc=c(1, 0.5), 
                        buc=c(1, 0.5))
    prob$bx    <- rbind(blx=rep(-Inf,3), 
                        bux=rep( Inf,3))
    prob$cones <- cbind(list("QUAD", c(1, 2, 3)))

    # Specify semidefinite matrix variables (one 3x3 block)
    prob$bardim <- c(3)

    # Block triplet format specifying the lower triangular part 
    # of the symmetric coefficient matrix 'barc':
    prob$barc$j <- c(1, 1, 1, 1, 1)
    prob$barc$k <- c(1, 2, 3, 2, 3)
    prob$barc$l <- c(1, 2, 3, 1, 2)
    prob$barc$v <- c(2, 2, 2, 1, 1)

    # Block triplet format specifying the lower triangular part 
    # of the symmetric coefficient matrix 'barA':
    prob$barA$i <- c(1, 1, 1, 2, 2, 2, 2, 2, 2)
    prob$barA$j <- c(1, 1, 1, 1, 1, 1, 1, 1, 1)
    prob$barA$k <- c(1, 2, 3, 1, 2, 3, 2, 3, 3)
    prob$barA$l <- c(1, 2, 3, 1, 2, 3, 1, 1, 2)
    prob$barA$v <- c(1, 1, 1, 1, 1, 1, 1, 1, 1)

    # Solve the problem
    r <- mosek(prob)

    # Print matrix variable and return the solution
    stopifnot(identical(r$response$code, 0))
    print( list(barx=getbarvarMatrix(r$sol$itr$barx[[1]], prob$bardim[1])) )
    r$sol
}

sdo1()