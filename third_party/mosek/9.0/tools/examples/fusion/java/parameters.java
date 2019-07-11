/*
  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File :      parameters.java

  Purpose :   Demonstrates a very simple example about how to set
              parameters and read information items
              with MOSEK Fusion
*/
package com.mosek.fusion.examples;
import mosek.*;
import mosek.fusion.*;

public class parameters {
    public static void main(String[] argv) {
        // Create the Model        
        Model M = new Model();
        System.out.println("Test MOSEK parameter get/set functions");
        
        // Set log level (integer parameter)
        M.setSolverParam("log", 1);
        // Select interior-point optimizer... (parameter with symbolic string values)
        M.setSolverParam("optimizer", "intpnt");
        // ... without basis identification (parameter with symbolic string values)
        M.setSolverParam("intpntBasis", "never");
        // Set relative gap tolerance (double parameter)
        M.setSolverParam("intpntCoTolRelGap", 1.0e-7);

        // The same in a different way
        M.setSolverParam("intpntCoTolRelGap", "1.0e-7");

        // Incorrect value
        try {
            M.setSolverParam("intpntCoTolRelGap", -1);
        }
        catch (mosek.fusion.ParameterError e) {
            System.out.println("Wrong parameter value");
        }


        // Define and solve an optimization problem here
        // M.solve()
        // After optimization: 

        System.out.println("Get MOSEK information items");

        double tm = M.getSolverDoubleInfo("optimizerTime");
        int it = M.getSolverIntInfo("intpntIter");

        System.out.println("Time: " + tm);
        System.out.println("Iterations: " + it);
    }
}