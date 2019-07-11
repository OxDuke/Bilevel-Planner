# Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
#
# File:      logistic.py
#
# Purpose: Implements logistic regression with regulatization.
#
#          Demonstrates using the exponential cone and log-sum-exp in Optimizer API.
#
#          Plots an example for 2D datasets.
from mosek import *
import numpy as np 
import sys, itertools

inf = 0.0

# t_i >= log( 1 + exp(u_i) ), i = 0..n-1
# Adds auxiliary variables and constraints
def softplus(task, t, u, n):
    nvar = task.getnumvar()
    ncon = task.getnumcon()
    task.appendvars(6*n)
    task.appendcons(3*n)
    z1, z2, v1, v2, q1, q2 = nvar, nvar+n, nvar+2*n, nvar+3*n, nvar+4*n, nvar+5*n
    zcon, v1con, v2con = ncon, ncon+n, ncon+2*n

    # z1 + z2 = 1
    task.putaijlist(range(zcon, zcon+n), range(z1, z1+n), [1]*n)
    task.putaijlist(range(zcon, zcon+n), range(z2, z2+n), [1]*n)
    # u - t - v1 = 0
    task.putaijlist(range(v1con, v1con+n), range(u, u+n), [1]*n)
    task.putaijlist(range(v1con, v1con+n), range(t, t+n), [-1]*n)
    task.putaijlist(range(v1con, v1con+n), range(v1, v1+n), [-1]*n)
    # - t - v2 = 0
    task.putaijlist(range(v2con, v2con+n), range(t, t+n), [-1]*n)
    task.putaijlist(range(v2con, v2con+n), range(v2, v2+n), [-1]*n)
    # Bounds for all constraints
    task.putconboundslice(ncon, ncon+3*n, [boundkey.fx]*(3*n), [1]*n+[0]*(2*n), [1]*n+[0]*(2*n))
    # Bounds for variables
    task.putvarboundsliceconst(nvar, nvar+4*n, boundkey.fr, -inf, inf)
    task.putvarboundsliceconst(nvar+4*n, nvar+6*n, boundkey.fx, 1, 1)

    # Cones
    for i in range(n):
        task.appendcone(conetype.pexp, 0.0, [z1+i, q1+i, v1+i])
        task.appendcone(conetype.pexp, 0.0, [z2+i, q2+i, v2+i])


# Model logistic regression (regularized with full 2-norm of theta)
# X - n x d matrix of data points
# y - length n vector classifying training points
# lamb - regularization parameter
def logisticRegression(env, X, y, lamb=1.0):
    n, d = int(X.shape[0]), int(X.shape[1])         # num samples, dimension
    
    with env.Task() as task:
        # Variables [r; theta; t; u]
        nvar = 1+d+2*n
        task.appendvars(nvar)
        task.putvarboundsliceconst(0, nvar, boundkey.fr, -inf, inf)
        r, theta, t, u, = 0, 1, 1+d, 1+d+n
        
        # Constraints: theta'*X +/- u = 0
        task.appendcons(n)
        task.putconboundsliceconst(0, n, boundkey.fx, 0.0, 0.0)            

        # Objective lambda*r + sum(t)
        task.putcj(r, lamb)
        task.putclist(range(t, t+n), [1.0]*n)

        # The X block in theta'*X +/- u = 0
        uCoeff = []
        for i in range(n):
            task.putaijlist([i]*d, range(theta, theta+d), X[i])
            uCoeff.append(1 if y[i] == 1 else -1)
        # +/- coefficients in u depending on y
        task.putaijlist(range(n), range(u, u+n), uCoeff)

        # Softplus function constraints
        softplus(task, t, u, n)

        # Regularization
        task.appendconeseq(conetype.quad, 0.0, 1+d, r)

        # Solution
        task.optimize()
        xx = [0.0]*d
        task.getxxslice(soltype.itr, theta, theta+d, xx)

        return xx

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

env = Env()

# Example from documentation is contained in
# https://datascienceplus.com/wp-content/uploads/2017/02/ex2data2.txt
'''
for lamb in [1e-0,1e-2,1e-4]:
    x, y = loaddata("ex2data2.txt")
    X = mapFeatures(x, d=6)
    theta = logisticRegression(env, X, y, lamb)
    plot2d(x, y, 6, theta)
'''

# Example 2: discover a circle
x, y = [], []
for x1,x2 in itertools.product(np.linspace(-1, 1, 30),np.linspace(-1, 1, 30)):
    x.append([x1,x2])
    y.append(0 if x1**2+x2**2<0.69 else 1)
x, y = np.array(x), np.array(y)
X = mapFeatures(x, d=2)
theta = logisticRegression(env, X, y, 0.1)
print(theta)