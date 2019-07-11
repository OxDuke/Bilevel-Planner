#
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      gp1.R
#
#  Purpose:    Demonstrates how to solve a simple Geometric Program (GP)
#              cast into conic form with exponential cones and log-sum-exp.
#
#              Example from
#                https://gpkit.readthedocs.io/en/latest/examples.html//maximizing-the-volume-of-a-box
#

#
# maximize     h*w*d
# subjecto to  2*(h*w + h*d) <= Awall
#                       w*d <= Afloor
#              alpha <= h/w <= beta
#              gamma <= d/w <= delta
#
# Variable substitutions:  h = exp(x), w = exp(y), d = exp(z).
#
# maximize     x+y+z
# subject      log( exp(x+y+log(2/Awall)) + exp(x+z+log(2/Awall)) ) <= 0
#                              y+z <= log(Afloor)  
#              log( alpha ) <= x-y <= log( beta )
#              log( gamma ) <= z-y <= log( delta )
#
#
# log(sum(exp(A*x+b))) <= 0 is equivalent to sum(u) == 1, (ui,1,ai*x+bi) in Kexp,
# so we have two exp-cones and two auxilliary variables u1,u2.
#
# We order variables as (x,y,z,u1,u2),

library("Rmosek")

gp1 <- function()
{
    # Input data
    Awall  <- 200.0
    Afloor <- 50.0
    alpha  <- 2.0
    beta   <- 10
    gamma  <- 2
    delta  <- 10

    # Objective
    prob <- list(sense="max")
    prob$c <- c(1, 1, 1, 0, 0)

    # Linear constraints:
    # [ 0  0  0  1  1 ]                    == 1
    # [ 0  1  1  0  0 ]                    <= log(Afloor)     
    # [ 1 -1  0  0  0 ]                    in [log(alpha), log(beta)]
    # [ 0 -1  1  0  0 ]                    in [log(gamma), log(delta)]
    #
    prob$A <- rbind(c(0, 0, 0, 1, 1),
                    c(0, 1, 1, 0, 0),
                    c(1,-1, 0, 0, 0),
                    c(0,-1, 1, 0, 0))
               
    prob$bc <- rbind(c(1, -Inf,        log(alpha), log(gamma)),
                     c(1, log(Afloor), log(beta),  log(delta)))

    prob$bx <- rbind(c(-Inf, -Inf, -Inf, -Inf, -Inf),
                     c( Inf,  Inf,  Inf,  Inf,  Inf))

    # The conic part FX+g \in Kexp x Kexp
    #   x  y  z  u  v
    # [ 0  0  0  1  0 ]    0 
    # [ 0  0  0  0  0 ]    1               in Kexp
    # [ 1  1  0  0  0 ]    log(2/Awall)
    #
    # [ 0  0  0  0  1 ]    0
    # [ 0  0  0  0  0 ]    1               in Kexp
    # [ 1  0  1  0  0 ] +  log(2/Awall)
    #
    #         
    prob$F <- rbind(c(0, 0, 0, 1, 0), 
                    c(0, 0, 0, 0, 0),
                    c(1, 1, 0, 0, 0),
                    c(0, 0, 0, 0, 1),
                    c(0, 0, 0, 0, 0),
                    c(1, 0, 1, 0, 0))
                
    prob$g <- c(0, 1, log(2/Awall), 0, 1, log(2/Awall))

    prob$cones <- matrix(rep(list("PEXP", 3, NULL), 2), ncol=2)
    rownames(prob$cones) <- c("type","dim","conepar")

    # Optimize and print results
    r <- mosek(prob, list(soldetail=1))
    print(exp(r$sol$itr$xx[1:3]))
}

gp1()