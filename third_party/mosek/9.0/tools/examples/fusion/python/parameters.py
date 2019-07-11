##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      parameters.py
#
#  Purpose :   Demonstrates a very simple example about how to set
#              parameters and read information items
#              with MOSEK Fusion
##
from mosek.fusion import *

# Create the Model
with Model() as M:
    print('Test MOSEK parameter get/set functions')

    # Set log level (integer parameter)
    M.setSolverParam("log", 1)
    # Select interior-point optimizer... (parameter with symbolic string values)
    M.setSolverParam("optimizer", "intpnt")
    # ... without basis identification (parameter with symbolic string values)
    M.setSolverParam("intpntBasis", "never")
    # Set relative gap tolerance (double parameter)
    M.setSolverParam("intpntCoTolRelGap", 1.0e-7)

    # The same in a different way
    M.setSolverParam("intpntCoTolRelGap", "1.0e-7")

    # Incorrect value
    try:
        M.setSolverParam("intpntCoTolRelGap", -1)
    except ParameterError as e:
        print('Wrong parameter value') 


    # Define and solve an optimization problem here
    # M.solve()
    # After optimization: 

    print('Get MOSEK information items')

    tm = M.getSolverDoubleInfo("optimizerTime")
    it = M.getSolverIntInfo("intpntIter")   

    print('Time: {0}\nIterations: {1}'.format(tm,it))