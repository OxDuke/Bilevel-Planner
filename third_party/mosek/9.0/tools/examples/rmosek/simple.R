##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      simple.R
#
#  Purpose :   To demonstrate a very simple example using the Rmosek package
#              to read a problem file, solve the optimization problem and
#              write the solution.
##
library("Rmosek")

simple <- function(filename)
{
    # Read problem
    r <- mosek_read(filename, list(verbose=1, usesol=FALSE, useparam=TRUE))
    stopifnot(identical(r$response$code, 0))
    prob <- r$prob

    # Solve problem
    r <- mosek(prob, list(verbose=1))
    stopifnot(identical(r$response$code, 0))
    sol <- r$sol

    # Print solution
    print(sol)
}

# Run only if called directly from the command-line
if( sys.nframe() == 0L )
{
    args <- commandArgs(trailingOnly=TRUE)
    if( !file.exists(args[1]) ) {
        stop("Expected problem file as input argument.")
    }

    simple(args[1])
}