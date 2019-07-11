##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      mioinitsol.R
#
#  Purpose :   To demonstrate how to solve a linear mixed-integer
#              problem with a start guess.
##
library("Rmosek")

mioinitsol <- function()
{
    # Specify the problem.
    prob <- list(sense="max")
    prob$c  <- c(7, 10, 1, 5)
    prob$A  <- Matrix(c(1, 1, 1, 1), nrow=1, sparse=TRUE)
    prob$bc <- rbind(blc=-Inf,
                     buc=2.5)
    prob$bx <- rbind(blx=rep(0,4),
                     bux=rep(Inf,4))
    prob$intsub <- c(1 ,2, 3)

    # Specify start guess for the integer variables.
    prob$sol$int$xx <- c(1, 1, 0, NaN)

    # Solve the problem
    r <- mosek(prob, list(getinfo=TRUE))

    # The solution
    stopifnot(identical(r$response$code, 0))
    print(r$sol$int$xx)

    # Was the initial solution used ?
    print(r$iinfo$MIO_CONSTRUCT_SOLUTION)
    print(r$dinfo$MIO_CONSTRUCT_SOLUTION_OBJ)
}

mioinitsol()