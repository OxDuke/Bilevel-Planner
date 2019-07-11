#
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      reoptimization.R
#
#  Purpose : Demonstrates how to modify and re-optimize a linear problem
#
library("Rmosek")

reoptimization <- function()
{
    # Specify the c vector.
    prob   <- list(sense="max")
    prob$c <- c(1.5, 2.5, 3.0)

    # Specify a in sparse format.
    subi   <- c(1, 1, 1, 2, 2, 2, 3, 3, 3)
    subj   <- c(1, 2, 3, 1, 2, 3, 1, 2, 3)
    valij  <- c(2, 4, 3, 3, 2, 3, 2, 3, 2)

    prob$A <- sparseMatrix(subi,subj,x=valij);

    # Specify bounds of the constraints.
    prob$bc<- rbind(blc=rep(-Inf, 3),
                    buc=c(100000, 50000, 60000))

    # Specify bounds of the variables.
    prob$bx<- rbind(blx=rep(0,3),
                    bux=rep(Inf,3))

    # Perform the optimization.
    prob$iparam <- list(OPTIMIZER="OPTIMIZER_DUAL_SIMPLEX")
    r <- mosek(prob)

    # Show the optimal x solution.
    print(r$sol$bas$xx)

    #####################################################
    # Change a coefficient
    prob$A[1,1] <- 3.0

    # Reoptimize with changed coefficient
    # Use previous solution to perform very simple hot-start.
    # This part can be skipped, but then the optimizer will start
    # from scratch on the new problem, i.e. without any hot-start.
    prob$sol <- list(bas=r$sol$bas)     
    r <- mosek(prob)
    print(r$sol$bas$xx)

    #####################################################
    # Add a variable
    prob$c       <- c(prob$c, 1.0)
    prob$A       <- cbind(prob$A, c(4.0, 0.0, 1.0))
    prob$bx      <- cbind(prob$bx, c(0.0,Inf))

    # Reoptimize with a new variable and hot-start
    # All parts of the solution must be extended to the new dimensions.
    prob$sol <- list(bas=r$sol$bas)     
    prob$sol$bas$xx  <- c(prob$sol$bas$xx, 0.0)
    prob$sol$bas$slx <- c(prob$sol$bas$slx, 0.0)
    prob$sol$bas$sux <- c(prob$sol$bas$sux, 0.0)
    prob$sol$bas$skx <- c(prob$sol$bas$skx, "UN")
    r <- mosek(prob)
    print(r$sol$bas$xx)

    #####################################################
    # Add a constraint
    prob$A       <- rbind(prob$A, c(1.0, 2.0, 1.0, 1.0)) 
    prob$bc      <- cbind(prob$bc, c(-Inf, 30000.0)) 

    # Reoptimization with a new constraint
    # All parts of the solution must be extended to the new dimensions.
    prob$sol <- list(bas=r$sol$bas)     
    prob$sol$bas$xc  <- c(prob$sol$bas$xc, 0.0)
    prob$sol$bas$slc <- c(prob$sol$bas$slc, 0.0)
    prob$sol$bas$suc <- c(prob$sol$bas$suc, 0.0)
    prob$sol$bas$skc <- c(prob$sol$bas$skc, "UN")
    r <- mosek(prob) 
    print(r$sol$bas$xx)

    #####################################################
    # Change constraint bounds
    prob$bc<- rbind(blc=rep(-Inf, 4),
                    buc=c(80000, 40000, 50000, 22000))
    r <- mosek(prob) 
    print(r$sol$bas$xx)

}

reoptimization()