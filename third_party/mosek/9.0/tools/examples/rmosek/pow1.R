##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      pow1.R
#
#  Purpose :   To demonstrate how to solve a small power cone
#              optimization problem using the Rmosek package.
##
library("Rmosek")

pow1 <- function()
{
    # Specify the non-conic part of the problem.
    prob <- list(sense="max")
    prob$c  <- c(-1, 0, 0, 1, 1, 0)
    prob$A  <- Matrix(c(1, 1, 0.5, 0, 0, 0), nrow=1, sparse=TRUE)
    prob$bc <- rbind(blc=2, 
                     buc=2)
    prob$bx <- rbind(blx=c(rep(-Inf,5), 1), 
                     bux=c(rep( Inf,5), 1))
    
    # Specify the cones.
    NUMCONES <- 2
    prob$cones <- matrix(list(), nrow=3, ncol=NUMCONES)
    rownames(prob$cones) <- c("type","sub","conepar")

    prob$cones[,1] <- list("PPOW", c(1, 2, 4), c(0.2, 0.8))
    prob$cones[,2] <- list("PPOW", c(3, 6, 5), c(0.4, 0.6))

    #
    # Non-parametric cones (such as "QUAD") are ignorant to the existence of 
    # the third row "conepar" and can be assigned any value (such as NaN).
    #

    # Solve the problem
    r <- mosek(prob)

    # Return the solution
    stopifnot(identical(r$response$code, 0))
    r$sol
}

pow1()