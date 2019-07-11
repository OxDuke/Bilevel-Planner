##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      solvebasis.py
#
#  Purpose :   To demonstrate the usage of
#              MSK_solvewithbasis on the problem:
#
#              maximize  x0 + x1
#              st.
#                      x0 + 2.0 x1 <= 2
#                      x0  +    x1 <= 6
#                      x0 >= 0, x1>= 0
#
#               The problem has the slack variables
#               xc0, xc1 on the constraints
#               and the variabels x0 and x1.
#
#               maximize  x0 + x1
#               st.
#                  x0 + 2.0 x1 -xc1       = 2
#                  x0  +    x1       -xc2 = 6
#                  x0 >= 0, x1>= 0,
#                  xc1 <=  0 , xc2 <= 0
##
import mosek

def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()

def main():
    numcon = 2
    numvar = 2

    # Since the value infinity is never used, we define
    # 'infinity' symbolic purposes only
    infinity = 0

    c = [1.0, 1.0]
    ptrb = [0, 2]
    ptre = [2, 3]
    asub = [0, 1,
            0, 1]
    aval = [1.0, 1.0,
            2.0, 1.0]
    bkc = [mosek.boundkey.up,
           mosek.boundkey.up]

    blc = [-infinity,
           -infinity]
    buc = [2.0,
           6.0]

    bkx = [mosek.boundkey.lo,
           mosek.boundkey.lo]
    blx = [0.0,
           0.0]

    bux = [+infinity,
           +infinity]
    w1 = [2.0, 6.0]
    w2 = [1.0, 0.0]

    try:
        with mosek.Env() as env:
            with env.Task(0, 0) as task:
                task.set_Stream(mosek.streamtype.log, streamprinter)
                task.inputdata(numcon, numvar,
                               c,
                               0.0,
                               ptrb,
                               ptre,
                               asub,
                               aval,
                               bkc,
                               blc,
                               buc,
                               bkx,
                               blx,
                               bux)
                task.putobjsense(mosek.objsense.maximize)
                r = task.optimize()
                if r != mosek.rescode.ok:
                    print("Mosek warning:", r)

                basis = [0] * numcon
                task.initbasissolve(basis)

                #List basis variables corresponding to columns of B
                varsub = [0, 1]

                for i in range(numcon):
                    if basis[varsub[i]] < numcon:
                        print("Basis variable no %d is xc%d" % (i, basis[i]))
                    else:
                        print("Basis variable no %d is x%d" %
                              (i, basis[i] - numcon))

                # solve Bx = w1
                # varsub contains index of non-zeros in b.
                #  On return b contains the solution x and
                # varsub the index of the non-zeros in x.
                nz = 2

                nz = task.solvewithbasis(0, nz, varsub, w1)
                print("nz = %s" % nz)
                print("Solution to Bx = w1:")

                for i in range(nz):
                    if basis[varsub[i]] < numcon:
                        print("xc %s = %s" % (basis[varsub[i]], w1[varsub[i]]))
                    else:
                        print("x%s = %s" %
                              (basis[varsub[i]] - numcon, w1[varsub[i]]))

                # Solve B^Tx = w2
                nz = 1
                varsub[0] = 0

                nz = task.solvewithbasis(1, nz, varsub, w2)

                print("Solution to B^Tx = w2:")

                for i in range(nz):
                    if basis[varsub[i]] < numcon:
                        print("xc %s = %s" % (basis[varsub[i]], w2[varsub[i]]))
                    else:
                        print("x %s = %s" %
                              (basis[varsub[i]] - numcon, w2[varsub[i]]))
    except Exception as e:
        print(e)

if __name__ == '__main__':
    main()