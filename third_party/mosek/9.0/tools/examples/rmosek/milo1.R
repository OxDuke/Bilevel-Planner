##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      milo1.R
#
#  Purpose :   To demonstrate how to solve a small mixed integer linear
#              optimization problem using the Rmosek package.
##
library("Rmosek")

milo1 <- function()
{
    # Specify the continuous part of the problem.
    prob <- list(sense="max")
    prob$c  <- c(1, 0.64)
    prob$A  <- Matrix(rbind(c(50, 31),
                            c( 3, -2)), sparse=TRUE)
    prob$bc <- rbind(blc=c(-Inf,  -4),
                     buc=c( 250, Inf))
    prob$bx <- rbind(blx=c(  0,   0),
                     bux=c(Inf, Inf))

    # Specify the integer constraints
    prob$intsub <- c(1 ,2)

    # Solve the problem
    r <- mosek(prob)

    # Return the solution
    stopifnot(identical(r$response$code, 0))
    r$sol
}

milo1()