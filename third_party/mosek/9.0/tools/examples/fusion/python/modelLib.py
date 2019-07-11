##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      modelLib.py
#
#  Purpose: Library of simple building blocks in Mosek Fusion.
##
from mosek.fusion import *

# Duplicate variables
# x = y
def dup(M, x, y):
    M.constraint(Expr.sub(x,y), Domain.equalsTo(0.0))

# Absolute value
# t >= |x|, where t, x have the same shape
def abs(M, t, x):
    M.constraint(Expr.add(t,x), Domain.greaterThan(0.0))
    M.constraint(Expr.sub(t,x), Domain.greaterThan(0.0))

# 1-norm
# t >= sum( |x_i| ), x is a vector expression
def norm1(M, t, x):
    u = M.variable(x.getShape(), Domain.unbounded())
    abs(M, u, x)
    M.constraint(Expr.sub(t, Expr.sum(u)), Domain.greaterThan(0.0))

# Square
# t >= x^2
def sq(M, t, x):
    M.constraint(Expr.hstack(0.5, t, x), Domain.inRotatedQCone())

# 2-norm
# t >= sqrt(x_1^2 + ... + x_n^2) where x is a vector
def norm2(M, t, x):
    M.constraint(Expr.vstack(t, x), Domain.inQCone())

# Power with exponent > 1
# t >= |x|^p (where p>1)
def pow(M, t, x, p):
    M.constraint(Expr.hstack(t, 1, x), Domain.inPPowerCone(1.0/p))

# Inverse of power 
# t >= 1/|x|^p, x>0 (where p>0)
def pow_inv(M, t, x, p):
    M.constraint(Expr.hstack(t, x, 1), Domain.inPPowerCone(1.0/(1.0+p)))

# p-norm, p>1
# t >= \|x\|_p (where p>1), x is a vector expression
def pnorm(M, t, x, p):
    n = int(x.getSize())
    r = M.variable(n)
    M.constraint(Expr.sub(t, Expr.sum(r)), Domain.equalsTo(0.0))
    M.constraint(Expr.hstack(Var.repeat(t,n), r, x), Domain.inPPowerCone(1.0-1.0/p))

# Geometric mean
# |t| <= (x_1...x_n)^(1/n), x_i>=0, x is a vector expression of length >= 1
def geo_mean(M, t, x):
    n = int(x.getSize())
    if n==1:
        abs(M, x, t)
    else:
        t2 = M.variable()
        M.constraint(Expr.hstack(t2, x.index(n-1), t), Domain.inPPowerCone(1.0-1.0/n))
        geo_mean(M, t2, x.slice(0,n-1))

# Logarithm
# t <= log(x), x>=0
def log(M, t, x):
    M.constraint(Expr.hstack(x, 1, t), Domain.inPExpCone())

# Exponential
# t >= exp(x)
def exp(M, t, x):
    M.constraint(Expr.hstack(t, 1, x), Domain.inPExpCone())

# Entropy
# t >= x * log(x), x>=0
def ent(M, t, x):
    M.constraint(Expr.hstack(1, x, Expr.neg(t)), Domain.inPExpCone())

# Relative entropy
# t >= x * log(x/y), x,y>=0
def relent(M, t, x, y):
    M.constraint(Expr.hstack(y, x, Expr.neg(t)), Domain.inPExpCone())

# Log-sum-exp
# log( sum_i(exp(x_i)) ) <= t, where x is a vector
def logsumexp(M, t, x):
    n = int(x.getSize())
    u = M.variable(n)
    M.constraint(Expr.hstack(u, Expr.constTerm(n, 1.0), Expr.sub(x, Var.repeat(t, n))), Domain.inPExpCone())
    M.constraint(Expr.sum(u), Domain.lessThan(1.0))

# Semicontinuous variable
# x = 0 or a <= x <= b
def semicontinuous(M, x, a, b):
    u = M.variable(x.getShape(), Domain.binary())
    M.constraint(Expr.sub(x, Expr.mul(a, u)), Domain.greaterThan(0.0))
    M.constraint(Expr.sub(x, Expr.mul(b, u)), Domain.lessThan(0.0))

# Indicator variable
# x!=0 implies t=1. Assumes that |x|<=1 in advance.
def indicator(M, t, x):
    M.constraint(t, Domain.inRange(0,1))
    t.makeInteger()
    abs(M, t, x)

# Logical OR
# x OR y, where x, y are binary
def logic_or(M, x, y):
    M.constraint(Expr.add(x, y), Domain.greaterThan(1.0))
# x_1 OR ... OR x_n, where x is a binary vector
def logic_or_vect(M, x):
    M.constraint(Expr.sum(x), Domain.greaterThan(1.0))

# SOS1 (NAND)
# at most one of x_1,...,x_n, where x is a binary vector (SOS1 constraint)
def logic_sos1(M, x):
    M.constraint(Expr.sum(x), Domain.lessThan(1.0))
# NOT(x AND y), where x, y are binary
def logic_nand(M, x, y):
    M.constraint(Expr.add(x, y), Domain.lessThan(1.0))

# Cardinality bound
# At most k of entries in x are nonzero, assuming in advance |x_i|<=1.
def card(M, x, k):
    t = M.variable(x.getShape(), Domain.binary())
    abs(M, t, x)
    M.constraint(Expr.sum(t), Domain.lessThan(k))

# This is just a syntactic test without much sense
def testModels():
    M = Model()
    x = M.variable()
    y = M.variable()
    t = M.variable()
    p = M.variable(5)
    a = M.variable([10,2])
    b = M.variable([10,2])
    e = M.variable(Domain.binary())
    f = M.variable(Domain.binary())

    log(M, t, x)
    exp(M, t, x)
    ent(M, t, x)
    relent(M, t, x, y)
    logsumexp(M, t, p)
    abs(M, a, b)
    norm1(M, t, a)
    sq(M, t, x)
    norm2(M, t, p)
    pow(M, t, x, 1.5)
    pow_inv(M, t, x, 3.3)
    geo_mean(M, t, p)
    semicontinuous(M, y, 1.1, 2.2)
    indicator(M, e, y)
    logic_or(M, e, f)
    logic_nand(M, e, f)
    card(M, b, 5)

# A sample problem using functions from the library
#
# max -sqrt(x^2 + y^2) + log(y) - x^1.5
#  st x >= y + 3
#
def testExample():
    M = Model()
    x = M.variable()
    y = M.variable()
    t = M.variable(3)

    M.constraint(Expr.sub(x, y), Domain.greaterThan(3.0))
    norm2(M, t.index(0), Var.vstack(x,y))
    log  (M, t.index(1), y)
    pow  (M, t.index(2), x, 1.5)

    M.objective(ObjectiveSense.Maximize, Expr.dot(t, [-1,1,-1]))

    import sys, numpy
    M.setLogHandler(sys.stdout)
    M.solve()
    print("x={0} y={1} obj={2}".format(x.level()[0], y.level()[0], M.primalObjValue()))
    if numpy.abs(M.primalObjValue()+10.5033852146)>1e-4:
        print("Wrong answer")
        sys.exit(1)
    else:
        print("OK")

if __name__ == "__main__":
    testModels()
    testExample()