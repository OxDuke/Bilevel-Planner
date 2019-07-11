##
#  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File:      sospoly.py
#
#  Purpose: 
#  Models the cone of nonnegative polynomials and nonnegative trigonometric
#  polynomials using Nesterov's framework  [1].
#
#  Given a set of coefficients (x0, x1, ..., xn), the functions model the 
#  cone of nonnegative polynomials 
#
#  P_m = { x \in R^{n+1} | x0 + x1*t + ... xn*t^n >= 0, forall t \in I }
#
#  where I can be the entire real axis, the semi-infinite interval [0,inf), or
#  a finite interval I = [a, b], respectively.
#
#  References:
#
#  [1] "Squared Functional Systems and Optimization Problems",  
#      Y. Nesterov, in High Performance Optimization,
#      Kluwer Academic Publishers, 2000.
##

import mosek
from   mosek.fusion import *

def Hankel(n, i, a=1.0):
    '''Creates a Hankel matrix of dimension n+1, where 
       H_lk = a if l+k=i, and 0 otherwise.'''

    if i < n+1:
        return Matrix.sparse(n+1, n+1, range(i,-1,-1), range(i+1), (i+1)*[a])

    return Matrix.sparse(n+1, n+1, range(n, i-n-1, -1), range(i-n, n+1), (2*n+1-i)*[a])
    
def nn_inf(M, x):
    '''Models the cone of nonnegative polynomials on the real axis'''

    m = int(x.getSize() - 1)
    n = int(m/2) # degree of polynomial is 2*n

    assert(m == 2*n)    

    # Setup variables
    X = M.variable(Domain.inPSDCone(n+1))

    # x_i = Tr H(n, i) * X  i=0,...,m
    for i in range(m+1):
        M.constraint( Expr.sub(x.index(i), Expr.dot(Hankel(n,i),X)), Domain.equalsTo(0.0))

def nn_semiinf(M, x):
    '''Models the cone of nonnegative polynomials on the semi-infinite interval [0,inf)'''

    n = int(x.getSize()-1)
    n1, n2 = int(n/2), int((n-1)/2)
    
    # Setup variables
    X1 = M.variable(Domain.inPSDCone(n1+1))
    X2 = M.variable(Domain.inPSDCone(n2+1))

    # x_i = Tr H(n1, i) * X1 + Tr H(n2,i-1) * X2, i=0,...,n
    for i in range(n+1):       
        e1 = Expr.dot(Hankel(n1,i),  X1)
        e2 = Expr.dot(Hankel(n2,i-1),X2)        
        M.constraint( Expr.sub(x.index(i), Expr.add(e1, e2)), Domain.equalsTo(0.0) )
                 
def nn_finite(M, x, a, b):
    '''Models the cone of nonnegative polynomials on the finite interval [a,b]'''

    assert(a < b)
    m = int(x.getSize()-1)
    n = int(m/2)

    X1 = M.variable(Domain.inPSDCone(n+1))

    if (m == 2*n):
        X2 = M.variable(Domain.inPSDCone(n))

        # x_i = Tr H(n,i)*X1 + (a+b)*Tr H(n-1,i-1) * X2 - a*b*Tr H(n-1,i)*X2 - Tr H(n-1,i-2)*X2, i=0,...,m
        for i in range(m+1):        
            e1 = Expr.sub(Expr.dot(Hankel(n  , i       ), X1), Expr.dot(Hankel(n-1, i, a*b), X2))
            e2 = Expr.sub(Expr.dot(Hankel(n-1, i-1, a+b), X2), Expr.dot(Hankel(n-1, i-2   ), X2))
            M.constraint( Expr.sub(x.index(i), Expr.add(e1, e2)), Domain.equalsTo(0.0) )
    else:
        X2 = M.variable(Domain.inPSDCone(n+1))

        # x_i = Tr H(n,i-1)*X1 - a*Tr H(n,i)*X1 + b*Tr H(n,i)*X2 - Tr H(n,i-1)*X2, i=0,...,m
        for i in range(m+1):
            e1 = Expr.sub(Expr.dot(Hankel(n, i-1 ), X1), Expr.dot(Hankel(n, i, a), X1))
            e2 = Expr.sub(Expr.dot(Hankel(n, i, b), X2), Expr.dot(Hankel(n, i-1 ), X2))
            M.constraint( Expr.sub(x.index(i), Expr.add(e1, e2)), Domain.equalsTo(0.0) )

def diff(M, x):
    '''returns variables u representing the derivative of
    x(0) + x(1)*t + ... + x(n)*t^n,
    with u(0) = x(1), u(1) = 2*x(2), ..., u(n-1) = n*x(n).'''

    n = int(x.getSize()-1)
    u = M.variable(n, Domain.unbounded())

    mask = Matrix.dense(n,1,[float(i) for i in range(1,n+1)])

    M.constraint(Expr.sub(u, Expr.mulElm(mask, x.slice(1,n+1))), 
                 Domain.equalsTo(0.0))
    return u

def fitpoly(data, n):

    with Model('smooth poly') as M:

        m = len(data)
        A = [ [ data[j][0]**i for i in range(n+1) ] for j in range(m)]
        b = [ data[j][1] for j in range(m) ]
            
        x  = M.variable('x', n+1, Domain.unbounded())
        z  = M.variable('z', 1, Domain.unbounded())
        dx = diff(M, x)
                       
        M.constraint(Expr.mul( A, x), Domain.equalsTo(b))
                        
        # z - f'(t) >= 0, for all t \in [a, b]
        ub = M.variable(n, Domain.unbounded())

        dx0= dx.index(0)
        dx1n= dx.slice(1,n)

        M.constraint(Expr.sub(ub, Expr.vstack(Expr.sub(z,dx0), Expr.neg(dx1n) ) ), 
                     Domain.equalsTo(0.0))

        nn_finite(M, ub, data[0][0], data[-1][0])
            
        # f'(t) + z >= 0, for all t \in [a, b]
        lb = M.variable(n, Domain.unbounded())
        M.constraint(Expr.sub(lb, Expr.vstack(Expr.add(z, dx0), dx1n)), 
                     Domain.equalsTo(0.0))

        nn_finite(M, lb, data[0][0], data[-1][0])

        M.objective(ObjectiveSense.Minimize, z)
        M.solve()
        
        return x.level()

if __name__ == '__main__':

    data = [ [-1.0, 1.0], 
             [ 0.0, 0.0],
             [ 1.0, 1.0] ]
       
    x2 = fitpoly(data, 2)
    x4 = fitpoly(data, 4)
    x8 = fitpoly(data, 8)

    try:
        from pyx import *            
        from pyx.graph.axis import painter, tick

        text.set(mode="latex")

        p = painter.regular(basepathattrs=[deco.earrow.normal],
                            titlepos=0.95, 
                            titledist=-0.3,
                            titledirection=None,
                            outerticklength=graph.axis.painter.ticklength.normal,
                            )

        g = graph.graphxy(width=8, xaxisat=0, yaxisat=0,
                          x=graph.axis.linear(title="$t$", min=-2.9, max=2.9,
                                              painter=p),
                          y=graph.axis.linear(min=-0.05, max=1.9,
                                              manualticks=[tick.tick(0, None, None),
                                                           tick.tick(0.5, label=r"$\frac{1}{2}$", ticklevel=0),
                                                           tick.tick(1.5, label=r"$\frac{3}{2}$", ticklevel=0),
                                                           ],
                                              painter=p))
    
        def f(t,x): return t, sum([ x[i]*(t**i) for i in range(len(x)) ]) 

        g.plot(graph.data.paramfunction("t", -3, 3, "x, y = f(t,x)", points=500, context={"f": f, "x":x2}), 
               [graph.style.line([style.linewidth.Thin, style.linestyle.solid])])
        g.plot(graph.data.paramfunction("t", -3, 3, "x, y = f(t,x)", points=500, context={"f": f, "x":x4}),
               [graph.style.line([style.linewidth.Thin, style.linestyle.solid])])
        g.plot(graph.data.paramfunction("t", -3, 3, "x, y = f(t,x)", points=500, context={"f": f, "x":x8}),
               [graph.style.line([style.linewidth.Thin, style.linestyle.solid])])

        px, py = g.pos(1.3, f(1.3, x2)[1])
        g.text(px - 0.1, py, "$f_2(t)$", [text.halign.right, text.valign.top])
        
        px, py = g.pos(1.6, f(1.6, x4)[1])
        g.text(px + 0.1, py, "$f_4(t)$", [text.halign.left, text.valign.top])

        px, py = g.pos(1.31, f(1.31, x8)[1])
        g.text(px - 0.1, py, "$f_8(t)$", [text.halign.right, text.valign.top])

        g.writeEPSfile("sospoly")
        g.writePDFfile("sospoly")
        print("generated sospoly.eps")
    except ImportError:
        print("No PyX available")
        # No pyx available
        pass