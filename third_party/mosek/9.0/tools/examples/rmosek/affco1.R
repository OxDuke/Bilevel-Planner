##
#  File : affco1.R
#
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  Description :
#    Implements a basic tutorial example with affine conic constraints:
#
#    maximize x_1^(1/3) + (x_1+x_2+0.1)^(1/4)
#    st.      (x_1-0.5)^2 + (x_2-0.6)^2 <= 1
#             0 <= x_1 <= x_2 + 1
# 
##
library("Rmosek")

affco1 <- function()
{

    prob <- list(sense="max")

    # Variables [x1; x2; t1; t2]
    prob$c <- c(0, 0, 1, 1)

    # Linear inequality x_1 - x_2 <= 1
    prob$A <- Matrix(c(1, -1, 0, 0), nrow=1, sparse=TRUE)
    prob$bc <- rbind(blc=-Inf, buc=1)
    prob$bx <- rbind(blx=rep(-Inf,4), bux=rep(Inf,4))

    # The quadratic cone
    FQ <- rbind(c(0,0,0,0), c(1,0,0,0), c(0,1,0,0))
    gQ <- c(1, -0.5, -0.6)
    cQ <- matrix(list("QUAD", 3, NULL), nrow=3, ncol=1)

    # The power cone for (x_1, 1, t_1) \in POW3^(1/3,2/3)
    FP1 <- rbind(c(1,0,0,0), c(0,0,0,0), c(0,0,1,0))
    gP1 <- c(0, 1, 0)
    cP1 <- matrix(list("PPOW", 3, c(1/3, 2/3)), nrow=3, ncol=1)

    # The power cone for (x_1+x_2+0.1, 1, t_2) \in POW3^(1/4,3/4)
    FP2 <- rbind(c(1,1,0,0), c(0,0,0,0), c(0,0,0,1))
    gP2 <- c(0.1, 1, 0)
    cP2 <- matrix(list("PPOW", 3, c(1.0, 3.0)), nrow=3, ncol=1)

    # All cones
    prob$F <- rbind(FQ, FP1, FP2)
    prob$g <- cbind(gQ, gP1, gP2)
    prob$cones <- cbind(cQ, cP1, cP2)
    rownames(prob$cones) <- c("type","dim","conepar")

    r <- mosek(prob, list(soldetail=1))
    stopifnot(identical(r$response$code, 0))

    print(r$sol$itr$pobjval)
    print(r$sol$itr$xx[1:2])
}

affco1()