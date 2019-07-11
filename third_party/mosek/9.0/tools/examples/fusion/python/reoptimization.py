##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      reoptimization.py
#
#  Purpose:   Demonstrates how to solve a  linear
#             optimization problem using the MOSEK API
#             and modify and re-optimize the problem.
from mosek.fusion import *

# Problem data
c = [ 1.5, 2.5, 3.0 ]
A = [ [2, 4, 3],
          [3, 2, 3],
          [2, 3, 2] ]
b = [ 100000.0, 50000.0, 60000.0 ]
numvar = len(c)
numcon = len(b)

# Create a model and input data
with Model() as M:
        x = M.variable("x",numvar, Domain.greaterThan(0.0))
        con = M.constraint(Expr.mul(A, x), Domain.lessThan(b))
        M.objective(ObjectiveSense.Maximize, Expr.dot(c, x))
        # Solve the problem
        M.solve()
        print("x = {}".format(x.level()))

        ############# Replace an element  of the A matrix ###############
        x0 = x.index(0)
        con.index(0).update(Expr.mul(3.0, x0), x0)
        M.solve()
        print("x = {}".format(x.level()))

        ############## Add a new variable ################
        # Create a variable and a compound view of all variables
        x3 = M.variable("y",Domain.greaterThan(0.0))
        xNew = Var.vstack(x, x3)
        # Add to the exising constraint
        con.update(Expr.mul(x3, [4, 0, 1]), x3)
        # Change the objective to include x3
        M.objective(ObjectiveSense.Maximize, Expr.dot(c+[1.0], xNew))
        M.solve()
        print("x = {}".format(xNew.level()))

        ############## Add a new constraint ################
        con2 = M.constraint(Expr.dot(xNew, [1, 2, 1, 1]), Domain.lessThan(30000.0))
        M.solve()
        print("x = {}".format(xNew.level()))

        ############## Change constraint bounds ################
        cAll = Constraint.vstack(con, con2)
        # Change bounds by effectively updating fixed terms with the difference
        cAll.update([20000, 10000, 10000, 8000])
        M.solve()
        print("x = {}".format(xNew.level()))