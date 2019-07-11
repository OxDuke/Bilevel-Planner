# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      baker.py
#
# Purpose: Demonstrates a small linear problem.
#
# Source: "Linaer Algebra" by Knut Sydsaeter and Bernt Oeksendal.
#
# The problem: A baker has 150 kg flour, 22 kg sugar, 25 kg butter and two
# recipes:
#   1) Cakes, requiring 3.0 kg flour, 1.0 kg sugar and 1.2 kg butter per dozen.
#   2) Breads, requiring 5.0 kg flour, 0.5 kg sugar and 0.5 kg butter per dozen.
# Let the revenue per dozen cakes be $4 and the revenue per dozen breads be $6.
#
# We now wish to compute the combination of cakes and breads that will optimize
# the total revenue.

from mosek.fusion import *

ingredientnames = ["Flour", "Sugar", "Butter"]
stock = [150.0, 22.0, 25.0]
recipe = [[3.0, 5.0],
          [1.0, 0.5],
          [1.2, 0.5]]
productnames = ["Cakes", "Breads"]
revenue = [4.0, 6.0]

def main(args):
    with Model("Recipe") as M:
        # "production" defines the amount of each product to bake.
        production = M.variable("production",
                                Set.make(productnames),
                                Domain.greaterThan(0.0))
        # The objective is to maximize the total revenue.
        M.objective("revenue",
                    ObjectiveSense.Maximize,
                    Expr.dot(revenue, production))

        # The production is constrained by stock:
        M.constraint(Expr.mul(recipe, production), Domain.lessThan(stock))
        # We set stdout as the loghandler
        M.setLogHandler(sys.stdout)
    # We solve and fetch the solution:
        M.solve()

        print("Solution:")
        res = production.level()
        for name, val in zip(productnames, res):
            print(" Number of %s : %.1f" % (name, val))
        print(" Revenue : %.2f" % (res[0] * revenue[0] + res[1] * revenue[1]))

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
    sys.exit(0)