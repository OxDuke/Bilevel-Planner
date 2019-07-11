##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      parameters.R
#
#  Purpose :   To demonstrate how to set parameters 
#              using the Rmosek package.
##
library("Rmosek")

parameters <- function()
{
    # Specify the optimization problem
    prob <- list(sense="max")
    prob$c <- c(3, 1, 5, 1)
    prob$A <- sparseMatrix(i=c(1, 1, 1, 2, 2, 2, 2, 3, 3),
                           j=c(1, 2, 3, 1, 2, 3, 4, 2, 4),
                           x=c(3, 1, 2, 2, 1, 3, 1, 2, 3))
    prob$bc <- rbind(blc=c(30,  15, -Inf), 
                     buc=c(30, Inf,   25))
    prob$bx <- rbind(blx=rep(0,4), 
                     bux=c(Inf, 10, Inf, Inf))

    # Specify the parameters

    # Set log level (integer parameter)
    # and select interior-point optimizer... (integer parameter)    
    # ... without basis identification (integer parameter)    
    prob$iparam <- list(LOG=0, OPTIMIZER="OPTIMIZER_INTPNT", INTPNT_BASIS="BI_NEVER")

    # Set relative gap tolerance (double parameter)
    prob$dparam <- list(INTPNT_CO_TOL_REL_GAP=1.0e-7)    

    # Solve the problem
    r <- mosek(prob)

    # Demonstrate retrieving information items

    opts <- list(getinfo=TRUE)
    r <- mosek(prob, opts)
    
    print(r$dinfo$OPTIMIZER_TIME)
    print(r$iinfo$INTPNT_ITER)

    # Return the solution
    stopifnot(identical(r$response$code, 0))
    r$sol
}

parameters()