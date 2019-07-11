##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      qo1.R
#
#  Purpose :   To demonstrate how to solve a small quadratic
#              optimization problem using the Rmosek package.
##
library("Rmosek")

qo1 <- function()
{
    # Specify the non-quadratic part of the problem.
    prob <- list(sense="min")
    prob$c <- c(0, -1, 0)
    prob$A <- Matrix(c(1, 1, 1), nrow=1, sparse=TRUE)
    prob$bc <- rbind(blc=1, 
                     buc=Inf)
    prob$bx <- rbind(blx=rep(0,3), 
                     bux=rep(Inf,3))

    # Specify the quadratic objective matrix in triplet form.
    prob$qobj$i <- c(1,  3,   2,  3)
    prob$qobj$j <- c(1,  1,   2,  3)
    prob$qobj$v <- c(2, -1, 0.2,  2)

    # Solve the problem
    r <- mosek(prob)

    # Return the solution
    stopifnot(identical(r$response$code, 0))
    r$sol
}

qo1()