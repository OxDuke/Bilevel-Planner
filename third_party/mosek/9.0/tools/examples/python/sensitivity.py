##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      sensitivity.py
#
#  Purpose :   To demonstrate how to perform sensitivity
#              analysis from the API on a small problem:
#
#              minimize
#
#              obj: +1 x11 + 2 x12 + 5 x23 + 2 x24 + 1 x31 + 2 x33 + 1 x34
#              st
#              c1:   +  x11 +   x12                                           <= 400
#              c2:                  +   x23 +   x24                           <= 1200
#              c3:                                  +   x31 +   x33 +   x34   <= 1000
#              c4:   +  x11                         +   x31                   = 800
#              c5:          +   x12                                           = 100
#              c6:                  +   x23                 +   x33           = 500
#              c7:                          +   x24                 +   x34   = 500
#
#              The example uses basis type sensitivity analysis.
##

import sys
import mosek

# Since the actual value of Infinity is ignores, we define it solely
# for symbolic purposes:
inf = 0.0

# Define a stream printer to grab output from MOSEK
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()


def main():
    # Create a MOSEK environment
    with mosek.Env() as env:
        # Attach a printer to the environment
        env.set_Stream(mosek.streamtype.log, streamprinter)

        # Create a task
        with env.Task(0, 0) as task:
            # Attach a printer to the task
            task.set_Stream(mosek.streamtype.log, streamprinter)

            # Set up data
            bkc = [mosek.boundkey.up, mosek.boundkey.up,
                   mosek.boundkey.up, mosek.boundkey.fx,
                   mosek.boundkey.fx, mosek.boundkey.fx,
                   mosek.boundkey.fx]
            blc = [-inf, -inf, -inf, 800., 100., 500., 500.]
            buc = [400., 1200., 1000., 800., 100., 500., 500.]

            bkx = [mosek.boundkey.lo, mosek.boundkey.lo,
                   mosek.boundkey.lo, mosek.boundkey.lo,
                   mosek.boundkey.lo, mosek.boundkey.lo,
                   mosek.boundkey.lo]
            c = [1.0, 2.0, 5.0, 2.0, 1.0, 2.0, 1.0]
            blx = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
            bux = [inf, inf, inf, inf, inf, inf, inf]

            ptrb = [0, 2, 4, 6, 8, 10, 12]
            ptre = [2, 4, 6, 8, 10, 12, 14]
            sub = [0, 3, 0, 4, 1, 5, 1, 6, 2, 3, 2, 5, 2, 6]

            val = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                   1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]

            numcon = len(bkc)
            numvar = len(bkx)
            numanz = len(val)

            # Input linear data
            task.inputdata(numcon, numvar,
                           c, 0.0,
                           ptrb, ptre, sub, val,
                           bkc, blc, buc,
                           bkx, blx, bux)
            # Set objective sense
            task.putobjsense(mosek.objsense.minimize)

            # Optimize
            task.optimize()

            # Analyze upper bound on c1 and the equality constraint on c4
            subi = [0, 3]
            marki = [mosek.mark.up, mosek.mark.up]

            # Analyze lower bound on the variables x12 and x31
            subj = [1, 4]
            markj = [mosek.mark.lo, mosek.mark.lo]

            leftpricei = [0., 0.]
            rightpricei = [0., 0.]
            leftrangei = [0., 0.]
            rightrangei = [0., 0.]
            leftpricej = [0., 0.]
            rightpricej = [0., 0.]
            leftrangej = [0., 0.]
            rightrangej = [0., 0.]

            task.primalsensitivity(subi,
                                   marki,
                                   subj,
                                   markj,
                                   leftpricei,
                                   rightpricei,
                                   leftrangei,
                                   rightrangei,
                                   leftpricej,
                                   rightpricej,
                                   leftrangej,
                                   rightrangej)

            print('Results from sensitivity analysis on bounds:')
            print('\tleftprice  | rightprice | leftrange  | rightrange ')
            print('For constraints:')

            for i in range(2):
                print('\t%10f   %10f   %10f   %10f' % (leftpricei[i],
                                                       rightpricei[i],
                                                       leftrangei[i],
                                                       rightrangei[i]))

            print('For variables:')
            for i in range(2):
                print('\t%10f   %10f   %10f   %10f' % (leftpricej[i],
                                                       rightpricej[i],
                                                       leftrangej[i],
                                                       rightrangej[i]))

            leftprice = [0., 0.]
            rightprice = [0., 0.]
            leftrange = [0., 0.]
            rightrange = [0., 0.]
            subc = [2, 5]

            task.dualsensitivity(subc,
                                 leftprice,
                                 rightprice,
                                 leftrange,
                                 rightrange)

            print('Results from sensitivity analysis on objective coefficients:')

            for i in range(2):
                print('\t%10f   %10f   %10f   %10f' % (leftprice[i],
                                                       rightprice[i],
                                                       leftrange[i],
                                                       rightrange[i]))

    return None

# call the main function
try:
    main()
except mosek.MosekException as e:
    print("ERROR: %s" % str(e.errno))
    if e.msg is not None:
        print("\t%s" % e.msg)
    sys.exit(1)
except:
    import traceback
    traceback.print_exc()
    sys.exit(1)