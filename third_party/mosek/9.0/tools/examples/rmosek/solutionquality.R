###
##  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
##
##  File :      solutionquality.R
##
##  Purpose :   To demonstrate how to examine the quality of a solution. 
###
library("Rmosek")

solutionquality <- function()
{
    # Specify problem.
    prob <- list(sense="max")
    prob$c <- c(3, 1, 5, 1)
    prob$A <- sparseMatrix(i=c(1, 1, 1, 2, 2, 2, 2, 3, 3),
                           j=c(1, 2, 3, 1, 2, 3, 4, 2, 4),
                           x=c(3, 1, 2, 2, 1, 3, 1, 2, 3))
    prob$bc <- rbind(blc=c(30,  15, -Inf), 
                     buc=c(30, Inf,   25))
    prob$bx <- rbind(blx=rep(0,4), 
                     bux=c(Inf, 10, Inf, Inf))

    # Solve the problem.
    r <- mosek(prob, list(verbose=1, soldetail=2))
    stopifnot(identical(r$response$code, 0))

    # Examine interior-point solution quality.
    isol <- r$sol$itr
    if( is.element(isol$solsta, list("OPTIMAL")) )
    {
      pobj <- isol$pobjval
      dobj <- isol$dobjval

      abs_obj_gap <- abs(dobj-pobj)
      rel_obj_gap <- abs_obj_gap/(1.0 + min(abs(pobj),abs(dobj)))

      max_primal_viol <- max( isol$maxinfeas$pcon, 
                              isol$maxinfeas$pbound,
                              isol$maxinfeas$pbarvar,
                              isol$maxinfeas$pcone )

      max_dual_viol <- max( isol$maxinfeas$dcon,
                            isol$maxinfeas$dbound,
                            isol$maxinfeas$dbarvar,
                            isol$maxinfeas$dcone )

      ## Assume the application needs the solution to be within
      ## 1e-6 of optimality in an absolute sense. Another approach
      ## would be looking at the relative objective gap 

      cat("Customized solution information.\n")
      cat("  Absolute objective gap: ",abs_obj_gap,"\n")
      cat("  Relative objective gap: ",rel_obj_gap,"\n")
      cat("  Max primal violation  : ",max_primal_viol,"\n")
      cat("  Max dual violation    : ",max_dual_viol,"\n")

      accepted <- TRUE

      if( rel_obj_gap > 1e-6 ) {
        print("Warning: The relative objective gap is LARGE.")
        accepted <- FALSE
      }

      ## We will accept a primal and dual infeasibility of 1e-6. 
      ## These numbers should chosen problem dependent.

      if( max_primal_viol > 1e-6 ) {
        print("Warning: Primal violation is too LARGE")
        accepted <- FALSE
      }

      if( max_dual_viol > 1e-6 ) {
        print("Warning: Dual violation is too LARGE.")
        accepted <- FALSE
      }

      if( accepted ) {
        print("Optimal primal interior-point solution")
        print(isol$xx)
      }

    } else if( is.element(isol$solsta, list("DUAL_INFEAS_CER","PRIM_INFEAS_CER")) ) {
        cat("Primal or dual infeasibility certificate found.\n")

    } else {
        cat("Other solution status.\n")
    }
}

solutionquality()