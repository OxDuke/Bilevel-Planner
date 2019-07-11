##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      parameters.py
#
#  Purpose :   Demonstrates a very simple example about how to get/set
#              parameters with MOSEK Python API
##
import mosek

# Create the mosek environment.
with mosek.Env() as env:
    # Create the mosek environment.
    with env.Task() as task:
        print('Test MOSEK parameter get/set functions')

        # Set log level (integer parameter)
        task.putintparam(mosek.iparam.log, 1)
        # Select interior-point optimizer... (integer parameter)
        task.putintparam(mosek.iparam.optimizer, mosek.optimizertype.intpnt)
        # ... without basis identification (integer parameter)
        task.putintparam(mosek.iparam.intpnt_basis, mosek.basindtype.never)
        # Set relative gap tolerance (double parameter)
        task.putdouparam(mosek.dparam.intpnt_co_tol_rel_gap, 1.0e-7)

        # The same using explicit string names 
        task.putparam     ("MSK_DPAR_INTPNT_CO_TOL_REL_GAP", "1.0e-7")
        task.putnadouparam("MSK_DPAR_INTPNT_CO_TOL_REL_GAP",  1.0e-7 )

        # Incorrect value
        try:
            task.putdouparam(mosek.dparam.intpnt_co_tol_rel_gap, -1.0)
        except:
            print('Wrong parameter value') 


        param = task.getdouparam(mosek.dparam.intpnt_co_tol_rel_gap)
        print('Current value for parameter intpnt_co_tol_rel_gap = {}'.format(param))

        # Define and solve an optimization problem here
        # task.optimize() 
        # After optimization: 

        print('Get MOSEK information items')

        tm = task.getdouinf(mosek.dinfitem.optimizer_time)
        it = task.getintinf(mosek.iinfitem.intpnt_iter)   

        print('Time: {0}\nIterations: {1}'.format(tm,it))