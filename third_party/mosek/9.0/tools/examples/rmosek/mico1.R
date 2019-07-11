##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      mico1.R
#
#  Purpose :   To demonstrate how to solve a small mixed integer conic
#              optimization problem using the Rmosek package.
#
#              minimize    x^2 + y^2
#              subject to  x >= e^y + 3.8
#                          x, y - integer
##
library("Rmosek")

mico1 <- function()
{
    # Specify the continuous part of the problem.
    # Variables are [t; x; y]
    prob <- list(sense="min")
    prob$c <- c(1, 0, 0)
    prob$A <- Matrix(nrow=0, ncol=3)   # 0 constraints, 3 variables
    prob$bx <- rbind(blx=rep(-Inf,3), bux=rep(Inf,3))

    # Conic part of the problem
    prob$F <- rbind(diag(3), c(0,1,0), c(0,0,0), c(0,0,1))
    prob$g <- c(0, 0, 0, -3.8, 1, 0)
    prob$cones <- cbind(matrix(list("QUAD", 3, NULL), nrow=3, ncol=1),
                        matrix(list("PEXP", 3, NULL), nrow=3, ncol=1))
    rownames(prob$cones) <- c("type","dim","conepar")

    # Specify the integer constraints
    prob$intsub <- c(2 ,3)

    # Solve the problem
    r <- mosek(prob)

    # Return the solution
    stopifnot(identical(r$response$code, 0))
    print(r$sol$int$xx[2:3])
}

mico1()