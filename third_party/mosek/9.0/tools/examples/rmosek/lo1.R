##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      lo1.R
#
#  Purpose :   To demonstrate how to solve a small linear
#              optimization problem using the Rmosek package.
##
library("Rmosek")

lo1 <- function()
{
    prob <- list()

    # Objective sense (maximize or minimize)
    prob$sense <- "max"

    # Objective coefficients
    prob$c <- c(3, 1, 5, 1)

    # Specify matrix 'A' in sparse format.
    asubi  <- c(1, 1, 1, 2, 2, 2, 2, 3, 3)
    asubj  <- c(1, 2, 3, 1, 2, 3, 4, 2, 4)
    aval   <- c(3, 1, 2, 2, 1, 3, 1, 2, 3)

    prob$A <- sparseMatrix(asubi,asubj,x=aval)

    # Bound values for constraints
    prob$bc <- rbind(blc=c(30,  15, -Inf), 
                     buc=c(30, Inf,   25))

    # Bound values for variables
    prob$bx <- rbind(blx=rep(0,4), 
                     bux=c(Inf, 10, Inf, Inf))

    # Solve the problem
    r <- mosek(prob)

    # Return the solution
    stopifnot(identical(r$response$code, 0))
    r$sol
}

lo1()