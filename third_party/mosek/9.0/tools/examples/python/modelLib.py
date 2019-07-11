##
#  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
#  File :      modelLib.py
#
#  Purpose: Library of simple building blocks in Mosek Optimizer API.
##
import mosek

inf = 0.0

# Add new constraints and return the index of the first one
def msk_newcon(task, num):
    c = task.getnumcon()
    task.appendcons(num)
    return c

# Add new variables and return the index of the first one
def msk_newvar(task, num):  # free
    v = task.getnumvar()
    task.appendvars(num)
    for i in range(num):
        task.putvarbound(v+i, mosek.boundkey.fr, -inf, inf)
    return v
def msk_newvar_fx(task, num, val):  # fixed
    v = task.getnumvar()
    task.appendvars(num)
    for i in range(num):
        task.putvarbound(v+i, mosek.boundkey.fx, val, val)
    return v
def msk_newvar_bin(task, num):  # binary
    v = task.getnumvar()
    task.appendvars(num)
    for i in range(num):
        task.putvarbound(v+i, mosek.boundkey.ra, 0.0, 1.0)
        task.putvartype(v+i, mosek.variabletype.type_int)
    return v

# Declare variables are duplicates or duplicate a variable and return index of duplicate
# x = y
def msk_equal(task, x, y):
    c = msk_newcon(task, 1)
    task.putaij(c, x, 1.0)
    task.putaij(c, y, -1.0)
    task.putconbound(c, mosek.boundkey.fx, 0.0, 0.0)
def msk_dup(task, x):
    y = msk_newvar(task, 1)
    msk_equal(task, x, y)
    return y

# Absolute value
# t >= |x|
def msk_abs(task, t, x):
    c = msk_newcon(task, 2)
    task.putaij(c, t, 1.0)
    task.putaij(c, x, 1.0)
    task.putconbound(c, mosek.boundkey.lo, 0.0, inf)
    task.putaij(c+1, t, 1.0)
    task.putaij(c+1, x, -1.0)
    task.putconbound(c+1, mosek.boundkey.lo, 0.0, inf)

# 1-norm
# t >= sum( |x_i| ), x is a list of variables
def msk_norm1(task, t, x):
    n = len(x)
    u = msk_newvar(task, n)
    for i in range(n): 
        msk_abs(task, u+i, x[i])
    c = msk_newcon(task, 1)
    task.putarow(c, range(u, u+n), [-1.0]*n)
    task.putaij(c, t, 1.0)
    task.putconbound(c, mosek.boundkey.lo, 0.0, inf)

# Square
# t >= x^2
def msk_sq(task, t, x):
    task.appendcone(mosek.conetype.rquad, 0.0, [msk_newvar_fx(task, 1, 0.5), t, x])

# 2-norm
# t >= sqrt(x_1^2 + ... + x_n^2) where x is a list of variables
def msk_norm2(task, t, x):
    task.appendcone(mosek.conetype.quad, 0.0, [t] + x)

# Power with exponent > 1
# t >= |x|^p (where p>1)
def msk_pow(task, t, x, p):
    task.appendcone(mosek.conetype.ppow, 1.0/p, [t, msk_newvar_fx(task, 1, 1.0), x])

# Inverse of power 
# t >= 1/x^p, x>0 (where p>0)
def msk_pow_inv(task, t, x, p):
    task.appendcone(mosek.conetype.ppow, 1.0/(1.0+p), [t, x, msk_newvar_fx(task, 1, 1.0)])

# p-norm, p>1
# t >= \|x\|_p (where p>1), x is a list of variables
def msk_pnorm(task, t, x, p):
    n = len(x)
    r = msk_newvar(task, n)
    for i in range(n):
        task.appendcone(mosek.conetype.ppow, 1.0-1.0/p, [t, r+i, x[i]])
    c = msk_newcon(task, 1)
    task.putarow(c, range(r, r+n), [-1.0]*n)
    task.putaij(c, t, 1.0)
    task.putconbound(c, mosek.boundkey.fx, 0.0, 0.0)

# Geometric mean
# |t| <= (x_1...x_n)^(1/n), x_i>=0, x is a list of variables of length >= 1
def msk_geo_mean(task, t, x):
    n = len(x)
    if n==1:
        msk_abs(task, x[0], t)
    else:
        t2 = msk_newvar(task, 1)
        task.appendcone(mosek.conetype.ppow, 1.0-1.0/n, [t2, x[n-1], t])
        msk_geo_mean(task, msk_dup(task, t2), x[0:n-1])

# Logarithm
# t <= log(x), x>=0
def msk_log(task, t, x):
    task.appendcone(mosek.conetype.pexp, 0.0, [x, msk_newvar_fx(task, 1, 1.0), t])

# Exponential
# t >= exp(x)
def msk_exp(task, t, x):
    task.appendcone(mosek.conetype.pexp, 0.0, [t, msk_newvar_fx(task, 1, 1.0), x])

# Entropy
# t >= x * log(x), x>=0
def msk_ent(task, t, x):
    v = msk_newvar(task, 1)
    c = msk_newcon(task, 1)
    task.putaij(c, v, 1.0)
    task.putaij(c, t, 1.0)
    task.putconbound(c, mosek.boundkey.fx, 0.0, 0.0)
    task.appendcone(mosek.conetype.pexp, 0.0, [msk_newvar_fx(task, 1, 1.0), x, v])

# Relative entropy
# t >= x * log(x/y), x,y>=0
def msk_relent(task, t, x, y):
    v = msk_newvar(task, 1)
    c = msk_newcon(task, 1)
    task.putaij(c, v, 1.0)
    task.putaij(c, t, 1.0)
    task.putconbound(c, mosek.boundkey.fx, 0.0, 0.0)
    task.appendcone(mosek.conetype.pexp, 0.0, [y, x, v])

# Log-sum-exp
# log( sum_i(exp(x_i)) ) <= t, where x is a list of variables
def msk_logsumexp(task, t, x):
    n = len(x)
    u = msk_newvar(task, n)
    z = msk_newvar(task, n)
    for i in range(n): 
        msk_exp(task, u+i, z+i)
    c = msk_newcon(task, n)
    for i in range(n):
        task.putarow(c+i, [x[i], t, z+i], [1.0, -1.0, -1.0])
        task.putconbound(c+i, mosek.boundkey.fx, 0.0, 0.0)
    s = msk_newcon(task, 1)
    task.putarow(s, range(u, u+n), [1.0]*n)
    task.putconbound(s, mosek.boundkey.up, -inf, 1.0)

# Semicontinuous variable
# x = 0 or a <= x <= b
def msk_semicontinuous(task, x, a, b):
    u = msk_newvar_bin(task, 1)
    c = msk_newcon(task, 2)
    task.putarow(c, [x, u], [1.0, -a])
    task.putconbound(c, mosek.boundkey.lo, 0.0, inf)
    task.putarow(c+1, [x, u], [1.0, -b])
    task.putconbound(c+1, mosek.boundkey.up, -inf, 0.0)

# Indicator variable
# x!=0 implies t=1. Assumes that |x|<=1 in advance.
def msk_indicator(task, x):
    t = msk_newvar_bin(task, 1)
    msk_abs(task, t, x)
    return t

# Logical OR
# x OR y, where x, y are binary
def msk_logic_or(task, x, y):
    c = msk_newcon(task, 1)
    task.putarow(c, [x, y], [1.0, 1.0])
    task.putconbound(c, mosek.boundkey.lo, 1.0, inf)
# x_1 OR ... OR x_n, where x is sequence of variables
def msk_logic_or_vect(task, x):
    c = msk_newcon(task, 1)
    n = len(x)
    task.putarow(c, x, [1.0]*n)
    task.putconbound(c, mosek.boundkey.lo, 1.0, inf)

# SOS1 (NAND)
# at most one of x_1,...,x_n, where x is a binary vector (SOS1 constraint)
def msk_logic_sos1(task, x):
    c = msk_newcon(task, 1)
    n = len(x)
    task.putarow(c, x, [1.0]*n)
    task.putconbound(c, mosek.boundkey.up, -inf, 1.0)
# NOT(x AND y), where x, y are binary
def msk_logic_nand(task, x, y):
    c = msk_newcon(task, 1)
    task.putarow(c, [x, y], [1.0, 1.0])
    task.putconbound(c, mosek.boundkey.up, -inf, 1.0)

# Cardinality bound
# At most k of entries in x are nonzero, assuming in advance that |x_i|<=1.
def msk_card(task, x, k):
    n = len(x)
    t = msk_newvar_bin(task, n)
    for i in range(n):
        msk_abs(task, t+i, x[i])
    c = msk_newcon(task, 1)
    task.putarow(c, range(t, t+n), [1.0]*n)
    task.putconbound(c, mosek.boundkey.up, -inf, k)

# This is just a syntactic test without much sense
def testModels():
    env = mosek.Env()
    task = env.Task()
    x = msk_newvar(task, 1)
    y = msk_newvar(task, 1)
    t = msk_newvar(task, 1)
    p = msk_newvar(task, 5)
    a = msk_newvar(task, 15)
    b = msk_newvar(task, 15)
    e = msk_newvar_bin(task, 1)
    f = msk_newvar_bin(task, 1)

    msk_log(task, t, x)
    msk_exp(task, msk_dup(task, t), msk_dup(task, x))
    msk_ent(task, msk_dup(task, t), msk_dup(task, x))
    msk_relent(task, msk_dup(task, t), msk_dup(task, x), msk_dup(task, y))
    msk_logsumexp(task, t, range(p, p+5))
    msk_abs(task, msk_dup(task, x), b+3)
    msk_norm1(task, msk_newvar_fx(task, 1, 0.74), range(a, a+15))
    msk_sq(task, msk_dup(task, t), msk_dup(task, x))
    msk_norm2(task, msk_dup(task, t), [a+1,a+2,b+3,b+4])
    msk_pow(task, msk_dup(task, t), msk_dup(task, x), 1.5)
    msk_pow_inv(task, msk_dup(task, t), msk_dup(task, x), 3.3)
    msk_geo_mean(task, msk_dup(task, t), range(p, p+5))
    msk_semicontinuous(task, y, 1.1, 2.2)
    i = msk_indicator(task, y)
    msk_logic_or(task, e, f)
    msk_logic_nand(task, e, f)
    msk_card(task, range(b, b+5), 2)

# A sample problem using functions from the library
#
# max -sqrt(x^2 + y^2) + log(y) - x^1.5
#  st x >= y + 3
#
def testExample():
    env = mosek.Env()
    task = env.Task()
    x = msk_newvar(task, 1)
    y = msk_newvar(task, 1)
    t = msk_newvar(task, 3)

    c = msk_newcon(task, 1)
    task.putarow(c, [x, y], [1.0, -1.0])
    task.putconbound(c, mosek.boundkey.lo, 3.0, inf)

    msk_norm2(task, t+0, [x,y])
    msk_log  (task, t+1, msk_dup(task, y))
    msk_pow  (task, t+2, msk_dup(task, x), 1.5)

    task.putclist(range(t, t+3), [-1.0, 1.0, -1.0])
    task.putobjsense(mosek.objsense.maximize)

    import sys
    task.set_Stream(mosek.streamtype.log, lambda s: sys.stdout.write('{0}'.format(s)))
    task.optimize()
    task.solutionsummary(mosek.streamtype.log)

    xx = [0.0] * task.getnumvar()
    task.getxx(mosek.soltype.itr, xx)
    print("x={0} y={1} obj={2}".format(xx[x], xx[y], task.getprimalobj(mosek.soltype.itr)))
    
    import numpy
    if numpy.abs(task.getprimalobj(mosek.soltype.itr)+10.5033852146)>1e-4:
        print("Wrong answer")
        sys.exit(1)
    else:
        print("OK")

if __name__ == "__main__":
    testModels()
    testExample()