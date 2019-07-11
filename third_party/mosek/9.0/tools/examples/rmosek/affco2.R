##
#  File : affco2.R
#
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  Description :
#    Implements a basic tutorial example with affine conic constraints:
#
#    minimize t
#    st.      (d, z1*y1,... zn*yn) \in Q^{n+1}
#             (yi, 1, ai*t)        \in EXP, i=1,\ldots,n
#
#    with input ai<0, zi, d.
#
#    See also https://docs.mosek.com/modeling-cookbook/expo.html#hitting-time-of-a-linear-system
# 
##
library("Rmosek")

firstHittingTime <- function(n, z, a, d)
{
    prob <- list(sense="min")
    # Variables [t, y1, ..., yn]
    prob$A <- Matrix(nrow=0, ncol=n+1)
    prob$bx<- rbind(rep(-Inf,n+1), rep(Inf,n+1))
    prob$c <- c(1, rep(0, n))

    # Quadratic cone
    FQ <- Diagonal(n+1, c(0, z))
    gQ <- c(d, rep(0, n))

    # All exponential cones
    FE <- sparseMatrix(  c(seq(1,3*n,by=3), seq(3,3*n,by=3)),
                         c(2:(n+1),         rep(1,n)),
                       x=c(rep(1,n),        t(a)))
    gE <- rep(c(0, 1, 0), n)

    # Assemble input data
    prob$F <- rbind(FQ, FE)
    prob$g <- c(gQ, gE)
    prob$cones <- cbind(matrix(list("QUAD", 1+n, NULL), nrow=3, ncol=1),
                        matrix(list("PEXP", 3, NULL), nrow=3, ncol=n))
    rownames(prob$cones) <- c("type","dim","conepar")

    # Solve
    r <- mosek(prob, list(soldetail=1))
    stopifnot(identical(r$response$code, 0))

    r$sol$itr$xx[1]
}

n = 3
z = c(3.3, 2.2, 1.3)
a = c(-0.08, -0.3, -0.06)
d = 0.5

t <- firstHittingTime(n, z, a, d)
print(sprintf("t = %.4e", t))