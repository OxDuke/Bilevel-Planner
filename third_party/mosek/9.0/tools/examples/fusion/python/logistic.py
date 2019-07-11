# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      logistic.py
#
# Purpose: Implements logistic regression with regulatization.
#
#          Demonstrates using the exponential cone and log-sum-exp in Fusion.
#
#          Plots an example for 2D datasets.
from mosek.fusion import *
import numpy as np 
import sys, itertools

# t >= log( 1 + exp(u) ) coordinatewise
def softplus(M, t, u):
    n = t.getShape()[0]
    z1 = M.variable(n)
    z2 = M.variable(n)
    M.constraint(Expr.add(z1, z2), Domain.equalsTo(1))
    M.constraint(Expr.hstack(z1, Expr.constTerm(n, 1.0), Expr.sub(u,t)), Domain.inPExpCone())
    M.constraint(Expr.hstack(z2, Expr.constTerm(n, 1.0), Expr.neg(t)), Domain.inPExpCone())

# Model logistic regression (regularized with full 2-norm of theta)
# X - n x d matrix of data points
# y - length n vector classifying training points
# lamb - regularization parameter
def logisticRegression(X, y, lamb=1.0):
    n, d = int(X.shape[0]), int(X.shape[1])         # num samples, dimension
    M = Model()
    theta = M.variable(d)
    t     = M.variable(n)
    reg   = M.variable()

    M.objective(ObjectiveSense.Minimize, Expr.add(Expr.sum(t), Expr.mul(lamb,reg)))
    M.constraint(Var.vstack(reg, theta), Domain.inQCone())

    signs = list(map(lambda y: -1.0 if y==1 else 1.0, y))
    softplus(M, t, Expr.mulElm(Expr.mul(X, theta), signs))

    return M, theta

# Map the 2d data through all monomials of degree <= d
def mapFeature(p,d):
    return np.array([(p[0]**a)*(p[1]**b) for a,b in itertools.product(range(d+1), range(d+1)) if a+b<=d])
def mapFeatures(x,d):
    return np.array([mapFeature(p,d) for p in x])

# Load the file and map using degree d monomials
# The file format is
# x_1, x_2, y
# for all training examples
def loaddata(filename):
    # Read coordinates and y values
    x, y = [], []
    with open(filename, "r") as f:
        for l in f.readlines():
            num = l.split(',')
            x.append([float(num[0]), float(num[1])])
            y.append(int(num[2]))
    return np.array(x), np.array(y)

# Plot some 2d results
def plot2d(x, y, d, theta):
    import matplotlib
    import matplotlib.pyplot as plt

    pos = np.where(y==1)
    neg = np.where(y==0)
    plt.scatter(x[pos,0], x[pos,1], marker='o', color='b')
    plt.scatter(x[neg,0], x[neg,1], marker='x', color='r')

    u = np.linspace(-1, 1, 50)
    v = np.linspace(-1, 1, 50)
    z = np.zeros(shape=(len(u), len(v)))
    for i in range(len(u)):
        for j in range(len(v)):
            z[i,j] = np.dot(mapFeature([u[i],v[j]], d), theta)
    plt.contour(u, v, z.T, [0])

    plt.show()

###############################################################################

# Example from documentation is contained in
# https://datascienceplus.com/wp-content/uploads/2017/02/ex2data2.txt
'''
for lamb in [1e-0,1e-2,1e-4]:
    x, y = loaddata("ex2data2.txt")
    X = mapFeatures(x, d=6)
    M, theta = logisticRegression(X, y, lamb)
    M.solve()    
    plot2d(x, y, 6, theta.level())
'''

# Example 2: discover a circle
x, y = [], []
for x1,x2 in itertools.product(np.linspace(-1, 1, 30),np.linspace(-1, 1, 30)):
    x.append([x1,x2])
    y.append(0 if x1**2+x2**2<0.69 else 1)
x, y = np.array(x), np.array(y)
X = mapFeatures(x, d=2)
M, theta = logisticRegression(X, y, 0.1)
M.setLogHandler(sys.stdout)

M.solve()
try:
    print(theta.level())
except:
    print("Could not get solution")
    sys.exit(1)