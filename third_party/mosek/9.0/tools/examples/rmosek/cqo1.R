##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      cqo1.R
#
#  Purpose :   To demonstrate how to solve a small conic quadratic
#              optimization problem using the Rmosek package.
##
library("Rmosek")

cqo1 <- function()
{
    # Specify the non-conic part of the problem.
    prob <- list(sense="min")
    prob$c  <- c(0, 0, 0, 1, 1, 1)
    prob$A  <- Matrix(c(1, 1, 2, 0, 0, 0), nrow=1, sparse=TRUE)
    prob$bc <- rbind(blc=1, 
                     buc=1)
    prob$bx <- rbind(blx=c(rep(0,3), rep(-Inf,3)), 
                     bux=rep(Inf,6))
    
    # Specify the cones.
    NUMCONES <- 2
    prob$cones <- matrix(list(), nrow=2, ncol=NUMCONES)
    rownames(prob$cones) <- c("type","sub")

    prob$cones[,1] <- list("QUAD", c(4, 1, 2))
    prob$cones[,2] <- list("RQUAD", c(5, 6, 3))
    
    #
    # Use cbind to extend this chunk of cones if needed:
    #
    #    oldcones <- prob$cones
    #    prob$cones <- cbind(oldcones, newcones)
    #

    # Solve the problem
    r <- mosek(prob)

    # Return the solution
    stopifnot(identical(r$response$code, 0))
    r$sol
}

cqo1()