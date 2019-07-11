/**
  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File :      reoptimization.java

  Purpose:   Demonstrates how to solve a  linear
             optimization problem using the MOSEK API
             and modify and re-optimize the problem.
*/
package com.mosek.fusion.examples;
import mosek.fusion.*;

public class reoptimization {
  public static void printsol(double[] x) {
    System.out.print("x = ");
    for(double val : x)
        System.out.printf("%.2f  ", val);
    System.out.println();   
  }

  public static void main(String[] args)
  throws SolutionError {
    double[]   c = new double[] { 1.5, 2.5, 3.0 };
    double[][] A = new double[][] { {2, 4, 3},
                                    {3, 2, 3},
                                    {2, 3, 2} };
    double[]   b = new double[] { 100000.0, 50000.0, 60000.0 };
    int   numvar = c.length;
    int   numcon = b.length;

    // Create a model and input data
    Model M = new Model();
    Variable x = M.variable(numvar, Domain.greaterThan(0.0));
    Constraint con = M.constraint(Expr.mul(A, x), Domain.lessThan(b));
    M.objective(ObjectiveSense.Maximize, Expr.dot(c, x));
    // Solve the problem
    M.solve();
    printsol(x.level());

    /************** Change an element of the A matrix ****************/
    con.index(0).update(Expr.mul(3.0, x.index(0)), x.index(0));
    M.solve();
    printsol(x.level());

    /*************** Add a new variable ******************************/
    // Create a variable and a compound view of all variables
    Variable x3 = M.variable(Domain.greaterThan(0.0));
    Variable xNew = Var.vstack(x, x3);
    // Add to the exising constraint
    con.update(Expr.mul(x3, new double[]{4, 0, 1}),x3);
    // Change the objective to include x3
    M.objective(ObjectiveSense.Maximize, Expr.dot(new double[]{1.5,2.5,3.0,1.0}, xNew));
    M.solve();
    printsol(xNew.level());

    /**************** Add a new constraint *****************************/
    Constraint con2 = M.constraint(Expr.dot(xNew, new double[]{1, 2, 1, 1}), Domain.lessThan(30000.0));
    M.solve();
    printsol(xNew.level());

    /**************** Change constraint bounds *****************************/
    // Assemble all constraints we previously defined into one
    Constraint cAll = Constraint.vstack(con, con2);
    // Change bounds by effectively updating fixed terms with the difference
    cAll.update(new double[]{20000, 10000, 10000, 8000});
    M.solve();
    printsol(xNew.level());
  }
}