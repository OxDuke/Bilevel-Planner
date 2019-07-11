#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2018 Gao Tang <gt70@duke.edu>
#
# Distributed under terms of the MIT license.

"""
planarSolver.py

A solve that solves indoor navigation problem hopefully reliably.
"""
from __future__ import print_function, division
import sys, os, time
import itertools
import copy
import warnings

import numpy as np
from scipy.sparse import coo_matrix, csc_matrix
import matplotlib.pyplot as plt
from scipy.interpolate import BPoly

from mpl_toolkits.mplot3d.art3d import Poly3DCollection

import osqp
import mosek

#from pyLib.all import Mute

from libott import loadTGP, construct_P, construct_A, gradient_from_P, gradient_from_A, set_print_level
from libbezier import Bezier
from libsqopt import Sqopt

# from plotter import piecewisePolyDeriv, evaluatePiecewisePolyWhole, evaluatePiecewisePolyOne2, plotSamples
# from plot_3d_trajectory import plot3dTrajectory


def print_green(*args, **kwargs):
    print("\033[1;32m", end='')
    print(*args, **kwargs)
    print("\033[0m", end='')

def print_yellow(*args, **kwargs):
    print("\033[1;33m", end='')
    print(*args, **kwargs)
    print("\033[0m", end ='')

def print_purple(*args, **kwargs):
    print("\033[1;34m", end='')
    print(*args, **kwargs)
    print("\033[0m", end='')

def print_red(*args, **kwargs):
    print("\033[1;35m", end='')
    print(*args, **kwargs)
    print("\033[0m", end='')

def print_cyan(*args, **kwargs):
    print("\033[1;36m", end='')
    print(*args, **kwargs)
    print("\033[0m", end='')

def print_gray(*args, **kwargs):
    print("\033[1;37m", end='')
    print(*args, **kwargs)
    print("\033[0m", end='')


class IndoorOptProblem(object):
    """Convex optimization around corridor approach for drone trajectory optimization.

    Currently it support problem where each corridor has been specified a time, later we will let it be free.
    """
    def __init__(self, tgp, tfweight=0, connect_order=2, verbose=False):
        """Constructor for this problem class.

        Parameters
        ----------
        tgp: a PyTGProblem object.
        tfweight: float, the weight on transfer time
        connect_order: to which order of derivative at connection do we guarantee continuity.
        verbose: bool, if the solver is verbose
        """
        self.floor = tgp  # I shall still use the floor name for convenience
        self.x0_pack = [tgp.position[0], tgp.velocity[0], tgp.acceleration[0]]
        self.xf_pack = [tgp.position[1], tgp.velocity[1], tgp.acceleration[1]]
        self.tfweight = tfweight
        self.obj_order = tgp.minimizeOrder
        self.poly_order = tgp.trajectoryOrder
        self.connect_order = connect_order  # guarantee acceleration continuity
        self.verbose = verbose
        self.margin = tgp.margin
        self.is_limit_vel = tgp.doLimitVelocity
        self.is_limit_acc = tgp.doLimitAcceleration
        self.vel_limit = tgp.maxVelocity  # component wise limit on velocity
        self.acc_limit = tgp.maxAcceleration  # component wise limit on acceleration
        # get info on environment
        self.boxes = tgp.getCorridor()
        self.num_box = len(self.boxes)
        self.room_time = np.array([box.t for box in self.boxes])
        bz = Bezier(self.poly_order, self.poly_order, self.obj_order)
        self.bz = bz
        self.bzM = self.bz.M()[self.poly_order]  # use this for some output stuff
        self.MQM = self.bz.MQM()[self.poly_order]
        if verbose:
            print('has %d boxes' % self.num_box)
            print('init time ', self.room_time)
        # some default settings such as how close you can reach the corner
        self.abs_obj_tol = 1e-3  # this controls absolute objective tolerance and should not set small for problem with small obj
        self.rel_obj_tol = 1e-3  # this controls relative objective tolerance
        self.grad_tol = 1e-3  # this controls gradient tolerance

    def set_tfweight(self, weight):
        """Set weight on time"""
        self.tfweight = weight

    def construct_prob(self, x0_pack, xf_pack, poly_order, obj_order, connect_order):
        """Construct a problem."""
        pass

    def set_x0_pack_value(self, *args):
        """Set the contents of x0pack"""
        ff = zip(self.x0_pack, args)
        print(ff)
        for tmp, val in zip(self.x0_pack, args):
            tmp[:] = val[:]

    def set_xf_pack_value(self, *args):
        """Set the contents of xfpack"""
        for tmp, val in zip(self.xf_pack, args):
            tmp[:] = val[:]

    def solve_with_room_time(self, rm_time):
        """Specify room time and solve the problem"""
        raise NotImplementedError("Subclass should implement solve_with_room_time function")

    def get_output_coefficients(self, ):
        """
        Get the monomial coefficients representing a piece-wise polynomial trajectory
        return ndarray, (s, 1), time allocated for each segment 
        return ndarray, (s, o + 1, d), polynomial coefficients, where s is the number of segments, 
        o is the order of trajectory, d is the number of dimensions, which is 3
        """
        
        n_room = self.num_box
        
        poly_coef = np.zeros([n_room, self.poly_order + 1, 3])
        
        # coefficients in bezier and scaled form
        mat_x = np.reshape(self.sol, (n_room, 3, self.poly_order + 1))
        mat_x = np.transpose(mat_x, (0, 2, 1))

        # change coefficients to monomial and unscaled form
        for i in range(n_room):
            poly_coef[i, :, :] = self.bzM.dot(mat_x[i]) * self.room_time[i]

        return self.room_time.copy(), poly_coef

    def get_coef_matrix(self):
        """Return coefficients"""
        coef_mat = np.reshape(self.sol, (self.num_box, 3, self.poly_order + 1))
        coef_mat = np.transpose(coef_mat, (0, 2, 1))
        for i in range(self.num_box):
            coef_mat[i] *= self.room_time[i]
        return coef_mat

    def from_coef_matrix(self, mat_in):
        """Assign values to sol based on the input coefficient matrix."""
        self.sol = np.zeros(mat_in.size)
        coef_mat = np.reshape(self.sol, (self.num_box, 3, self.poly_order + 1))
        for i in range(self.num_box):
            coef_mat[i] = mat_in[i].T / self.room_time[i]
        with np.printoptions(precision=4, linewidth=10000):
            print(self.sol)

    def get_output_path(self, n):
        """Get a path for output that is linspace in time.

        :param n: int, the number of nodes for a path
        :return: float, the total time for this problem
        :return: ndarray, (n, 2) the optimal path
        """
        cum_sum_time = np.cumsum(self.room_time)
        output = np.zeros((n, 3))  # the output trajectory
        sample_time = np.linspace(0, cum_sum_time[-1], n)
        n_room = self.num_box

        # get all the coef of polynomials
        t, all_poly_coeffs = self.get_output_coefficients()
        for i in range(n_room):
            # poly_coef = self.bzM.dot(mat_x[i]) * self.room_time[i]
            poly_coef = all_poly_coeffs[i,:,:]
            if i == 0:
                t_mask = sample_time <= cum_sum_time[0]
                use_s = sample_time[t_mask] / cum_sum_time[0]
            else:
                t_mask = (sample_time > cum_sum_time[i - 1]) & (sample_time <= cum_sum_time[i])
                use_s = (sample_time[t_mask] - cum_sum_time[i - 1]) / self.room_time[i]
            output[t_mask, 0] = np.polyval(poly_coef[:, 0][::-1], use_s)
            output[t_mask, 1] = np.polyval(poly_coef[:, 1][::-1], use_s)
            output[t_mask, 2] = np.polyval(poly_coef[:, 2][::-1], use_s)
        return cum_sum_time[-1], output

    def get_gradient(self):
        raise NotImplementedError

    def get_gradient_fd(self, h=1e-6):
        """Use forward finite difference to approximate gradients."""
        grad = np.zeros(self.num_box)
        obj0 = self.obj
        origin_time = self.room_time.copy()
        for i in range(self.num_box):
            try_time = origin_time.copy()
            try_time[i] += h
            self.solve_with_room_time(try_time)
            grad[i] = (self.obj - obj0) / h
        grad += self.tfweight
        return grad

    def get_gradient_mellinger(self, h=1e-6):
        """
        Use finite difference described in:
        http://www-personal.acfr.usyd.edu.au/spns/cdm/papers/Mellinger.pdf
        to approximate gradients.
        """
        grad = np.zeros(self.num_box)
        obj0 = self.obj
        origin_time = self.room_time.copy()
        for i in range(self.num_box):
            m = self.num_box
            gi = -1/(m-1) * np.ones(m)
            gi[i] = 1
            try_time = origin_time.copy()
            try_time += h * gi
            # print("In gg Mellinger: gi", gi," try_time: ", try_time)
            self.solve_with_room_time(try_time)
            grad[i] = (self.obj - obj0) / h
        grad += self.tfweight
        return grad

    def refine_time_by_backtrack(self, alpha0=0.175, h=1e-5, c=0.2, tau=0.2, max_iter=50, j_iter=5, log=False, timeProfile=False, adaptiveLineSearch=False):
        """Use backtrack line search to refine time. We fix total time to make things easier.

        Parameters
        ----------
        alpha0: float, initial step length 0.175 and 0.375 are found to be very good.
        h: float, step size for finding gradient using forward differentiation
        c: float, the objective decrease parameter
        tau: float, the step length shrink parameter
        max_iter: int, maximum iteration for gradient descent
        j_iter: int, maximum iteration for finding alpha
        abs_tol: float, absolute objective tolerance
        rel_tol: float, Relative objective tolerance
        Returns
        -------
        is_okay: bool, indicates if some exception occurs
        converged: bool, indicates if the algorithm converges
        """

        if log == True:
            self.log = np.array([])
            self.log = np.append(self.log, [self.obj, 0])

        if timeProfile == True:
            self.timeProfile = np.array([])

        t0 = time.time()
        self.major_iteration = 0
        self.num_prob_solve = 0
        if self.num_box == 1 and self.tfweight == 0:
            self.major_iteration = 0
            self.num_prob_solve = 0
            self.time_cost = time.time() - t0
            self.converge_reason = 'No need to refine'
            return True, True
        n_room = self.num_box
        t_now = self.room_time.copy()

        converged = False
        converge_reason = 'Not converged'
        num_prob_solve = 0  # record number of problems being solved for a BTLS
        for i in range(max_iter):
            if self.verbose:
                print_green('Iteration %d' % i)
           
            obj0 = self.obj
            is_okay = True
            
            if timeProfile == True:
                tBeforeGrad = time.time()

            # choose a method to calculate gradient
            if self.grad_method == 'ours':
                grad = self.get_gradient()
            elif self.grad_method == 'fd':
                grad = self.get_gradient_fd(h=0.25*1e-6)
            elif self.grad_method == 'mel':
                grad = self.get_gradient_mellinger(h=0.25*1e-6)
            else:
                print_red("No this grad method!")
            
            if timeProfile == True:
                tAfterGrad = time.time()
                self.timeProfile = np.append(self.timeProfile, [tAfterGrad - tBeforeGrad])

            #print_red('grad_an ', grad, ' grad_fd ', grad_fd, 'grad_mel', grad_mel)
            if self.tfweight == 0:
                # get projected gradient, the linear manifold is \sum x_i = 0; if tfweight=0, we fix total time
                normal = np.ones(n_room) / np.sqrt(n_room)  # normal direction
                grad = grad - grad.dot(normal) * normal  # projected gradient descent direction
            # print_red('grad_an ', grad / np.linalg.norm(grad), ' grad_fd ', grad_fd / np.linalg.norm(grad_fd), 'grad_mel', grad_mel / np.linalg.norm(grad_mel))
            # print_yellow('sum_an ', np.sum(grad), 'sum_fd ', np.sum(grad_fd), 'sum_mel ', np.sum(grad_mel))
            if np.linalg.norm(grad) < self.grad_tol:  # break out if gradient is too small
                if self.verbose:
                    print('Gradient too small')
                converged = True
                converge_reason = 'Small gradient'
                break
            if self.verbose:
                print_green('At time ', t_now, ' grad is ', grad)
            m = -np.linalg.norm(grad)
            p = grad / m  # p is the descending direction
            # use a maximum alpha that makes sure time are always positive
            alpha_max = np.amax(-t_now / p) - 1e-6  # so I still get non-zero things
            if alpha_max > 0:
                alpha = min(alpha_max, alpha0)
            else:
                alpha = alpha0
            t = -c * m

            # find alpha
            alpha_found = False

            if timeProfile == True:
                tBeforeAlpha = time.time()

            for j in range(j_iter):
                if self.verbose:
                    print('Search alpha step %d, alpha = %f' % (j, alpha))
                candid_time = t_now + alpha * p

                # lower bound on the alpha
                if adaptiveLineSearch == True:
                    if alpha < 1e-4:
                        if self.verbose:
                            print_yellow('Stop line search because alpha is too small')
                        break
                
                # make sure that time will not go too small
                if np.any(candid_time < 1e-6):
                    alpha = tau * alpha
                    continue
                if self.verbose:
                    print('Try time: ', candid_time)
                self.solve_with_room_time(candid_time)
                num_prob_solve += 1

                # in case objective somehow falls below 0
                if self.obj < 0:
                    self.is_solved = False
                if not self.is_solved:
                    alpha = tau * alpha  # decrease step length
                    continue                
                objf = self.obj
                
                # in case objective somehow falls below 0
                if objf < 0:
                    print_yellow("Negative Objective!", self.obj)
                    print(self.solve_with_room_time(candid_time))
                    import pdb; pdb.set_trace()
                    alpha = tau * alpha
                    continue

                if self.verbose:
                    print('\talpha ', alpha, ' obj0 ', obj0, ' objf ', objf)
                if obj0 - objf >= alpha * t or obj0 - objf >= 0.1 * obj0:  # either backtrack or decrease sufficiently
                    alpha_found = True
                    # if use adaptive line search
                    if adaptiveLineSearch == True:
                        # adaptive line search, increase the initial alpha is alpha is good enough for the first ime
                        if j == 0:
                            alpha0 = 1.5 * alpha
                            if self.verbose:                    
                                 print('alpha growes from %.3f to %.3f' % (alpha, alpha0))
                        # set the initial alpha for the next iteration the same as this time's alpha 
                        else:
                            alpha0 = alpha
                            if self.verbose:
                                print('alpha set to %.3f' % alpha0)
                    break
                else:
                    alpha = tau * alpha  # decrease step length
                

            if timeProfile == True:
                tAfterAlpha = time.time()
                self.timeProfile = np.append(self.timeProfile, [tAfterAlpha - tBeforeAlpha])

            if self.verbose:
                if alpha_found:
                    print('We found alpha = %f' % alpha)
                else:
                    print('Fail to find alpha, use a conservative %f' % alpha)
            if not alpha_found:  # for case where alpha is not found
                converge_reason = 'Cannot find step size alpha'
                is_okay = True
                converged = False
                # roll back to t_now
                self.room_time = t_now
                self.obj = obj0
                if log == True:
                    duration = time.time() - t0
                    self.log = np.append(self.log, [obj0, duration])
                break

            # adaptive line search

            # ready to update time now and check convergence
            t_now = candid_time  # this is the alpha we desire
            if self.verbose:
                print('obj0 = ', obj0, 'objf = ', objf)
            if log == True:
                duration = time.time() - t0
                # we log the objective and duration for each major iteration
                if self.verbose == True:
                    print("Logging: obj: %f, T: %f" % (objf, duration))
                self.log = np.append(self.log, [objf, duration])
            if abs(objf - obj0) < self.abs_obj_tol:
                if self.verbose:
                    print('Absolute obj improvement too small')
                converged = True
                converge_reason = 'Absolute cost'
                break
            elif abs(objf - obj0) / abs(obj0) < self.rel_obj_tol:
                if self.verbose:
                    print('Relative obj improvement too small')
                converged = True
                converge_reason = 'Relative cost'
                break
        self.major_iteration = i
        self.num_prob_solve = num_prob_solve
        self.time_cost = time.time() - t0
        self.converge_reason = converge_reason
        return is_okay, converged

    def refine_time_by_BFGS(self, alpha0=0.1, h=1e-5, c=0.2, tau=0.2, max_iter=50, j_iter=5, log=False):
        """Use backtrack line search to refine time. We fix total time to make things easier.

        Parameters
        ----------
        alpha0: float, initial step length
        h: float, step size for finding gradient using forward differentiation
        c: float, the objective decrease parameter
        tau: float, the step length shrink parameter
        max_iter: int, maximum iteration for gradient descent
        j_iter: int, maximum iteration for finding alpha
        abs_tol: float, absolute objective tolerance
        rel_tol: float, Relative objective tolerance
        Returns
        -------
        is_okay: bool, indicates if some exception occurs
        converged: bool, indicates if the algorithm converges
        """

        if log == True:
            self.log = np.array([])

        t0 = time.time()
        self.major_iteration = 0
        self.num_prob_solve = 0
        if self.num_box == 1 and self.tfweight == 0:
            self.major_iteration = 0
            self.num_prob_solve = 0
            self.time_cost = time.time() - t0
            self.converge_reason = 'No need to refine'
            return True, True
        n_room = self.num_box
        t_now = self.room_time.copy()

        converged = False
        converge_reason = 'Not converged'
        num_prob_solve = 0  # record number of problems being solved for a BTLS
        for i in range(max_iter):
            if self.verbose:
                print_green('Iteration %d' % i)
           
            obj0 = self.obj
            is_okay = True

            # choose a method to calculate gradient
            if self.grad_method == 'ours':
                grad = self.get_gradient()
            elif self.grad_method == 'fd':
                grad = self.get_gradient_fd(h=0.25*1e-6)
            elif self.grad_method == 'mel':
                grad = self.get_gradient_mellinger(h=0.25*1e-6)
            else:
                print_red("No this grad method!")

            gfkp1 = grad # gradient of f at the (k+1)th step
            
            # update Hk matrix
            if i == 0:
                beta = 0.5
                # Hk is initialized to some scalar times I
                Hk = beta * np.eye(n_room)
            else:
                yk = gfkp1 - gfk
                rhok = 1.0 / yk.dot(sk)
                ml = np.eye(n_room) - rhok * np.outer(sk, yk)
                mr = np.eye(n_room) - rhok * np.outer(yk, sk)
                ma = rhok * np.outer(sk, sk)
                Hk = ml.dot(Hk.dot(mr)) + ma
             
            # change grad to the BFGS direction
            grad = Hk.dot(grad)
            #grad

            if self.verbose:
                print_yellow("BFGS Hk: ", Hk)
                print_cyan("new grad: ", grad)

            #print_red('grad_an ', grad, ' grad_fd ', grad_fd, 'grad_mel', grad_mel)
            if self.tfweight == 0:
                # get projected gradient, the linear manifold is \sum x_i = 0; if tfweight=0, we fix total time
                normal = np.ones(n_room) / np.sqrt(n_room)  # normal direction
                grad = grad - grad.dot(normal) * normal  # projected gradient descent direction
            # print_red('grad_an ', grad / np.linalg.norm(grad), ' grad_fd ', grad_fd / np.linalg.norm(grad_fd), 'grad_mel', grad_mel / np.linalg.norm(grad_mel))
            # print_yellow('sum_an ', np.sum(grad), 'sum_fd ', np.sum(grad_fd), 'sum_mel ', np.sum(grad_mel))
            if np.linalg.norm(grad) < self.grad_tol:  # break out if gradient is too small
                if self.verbose:
                    print('Gradient too small :', np.linalg.norm(grad))
                converged = True
                converge_reason = 'Small gradient'
                break
            if self.verbose:
                print_green('At time ', t_now, ' grad is ', grad)
            m = -np.linalg.norm(grad)
            p = grad / m  # p is the descending direction
            # use a maximum alpha that makes sure time are always positive
            alpha_max = np.amax(-t_now / p) - 1e-6  # so I still get non-zero things
            if alpha_max > 0:
                alpha = min(alpha_max, alpha0)
            else:
                alpha = alpha0
            t = -c * m

            # find alpha
            alpha_found = False
            for j in range(j_iter):
                if self.verbose:
                    print('Search alpha step %d, alpah = %f' % (j, alpha))
                candid_time = t_now + alpha * p
                self.solve_with_room_time(candid_time)
                num_prob_solve += 1
                if not self.is_solved:
                    alpha = tau * alpha  # decrease step length
                    continue                
                objf = self.obj
                if self.verbose:
                    print('\talpha ', alpha, ' obj0 ', obj0, ' objf ', objf)
                if obj0 - objf >= alpha * t or obj0 - objf >= 0.1 * obj0:  # either backtrack or decrease sufficiently
                    alpha_found = True
                    break
                else:
                    alpha = tau * alpha  # decrease step length
            if self.verbose:
                if alpha_found:
                    print('We found alpha = %f' % alpha)
                else:
                    print('Fail to find alpha, use a conservative %f' % alpha)
            if not alpha_found:  # for case where alpha is not found
                converge_reason = 'Cannot find step size alpha'
                is_okay = True
                converged = False
                # roll back to t_now
                self.room_time = t_now
                self.obj = obj0
                break

            # ready to update time now and check convergence
            t_now = candid_time  # this is the alpha we desire
            
            sk = alpha * p #sk in BFGS
            gfk = grad # gradient of f at kth step from BFGS
            
            if self.verbose:
                print('obj0 = ', obj0, 'objf = ', objf)
            # print_cyan("i: ", i, " objf: ", objf)
            if log == True:
                duration = time.time() - t0
                # we log the objective and duration for each major iteration
                self.log = np.append(self.log, [objf, duration])
            if abs(objf - obj0) < self.abs_obj_tol:
                if self.verbose:
                    print('Absolute obj improvement too small')
                converged = True
                converge_reason = 'Absolute cost'
                break
            elif abs(objf - obj0) / abs(obj0) < self.rel_obj_tol:
                if self.verbose:
                    print('Relative obj improvement too small')
                converged = True
                converge_reason = 'Relative cost'
                break
        self.major_iteration = i
        self.num_prob_solve = num_prob_solve
        self.time_cost = time.time() - t0
        self.converge_reason = converge_reason
        return is_okay, converged


class IndoorQPProblem(IndoorOptProblem):
    """Formulate the indoor navigation problem explicitly as QP so we can either use mosek or osqp to solve it.

    I will manually maintain those matrices and hope it is more efficient.
    """
    def __init__(self, tgp, tfweight=0, connect_order=2, verbose=False):
        IndoorOptProblem.__init__(self, tgp, tfweight, connect_order, verbose)
        self.h_type = 'F'

    def update_prob(self):
        """Just update the problem since we are changing pretty fast.

        This function assume you might have change in room sequence so it reconstructs things. Take care with this.
        """
        self.construct_prob(self.x0_pack, self.xf_pack, self.poly_order, self.obj_order, self.connect_order)

    def construct_prob(self, x0_pack, xf_pack, poly_order, obj_order, connect_order):
        """Construct a problem."""
        # construct the problem using Fei's code
        self.construct_P()
        self.construct_A()

    def construct_P(self):
        # P is fixed and we do not alter it afterwards, so let's keep going
        pval, prow, pcol = construct_P(self.obj_order, self.num_box, self.poly_order, self.room_time, self.MQM, self.h_type)
        self.sp_P = coo_matrix((pval, (prow, pcol)))  # ugly hack since osqp only support upper triangular part or full
        self.n_var = self.sp_P.shape[0]
        # self.qp_P = spmatrix(sp_P.data, sp_P.row, sp_P.col)
        self.qp_q = np.zeros(self.n_var)

    def construct_A(self):
        lincon = construct_A(
                self.floor.getCorridor(),
                self.MQM,
                self.floor.position.copy(order='F'),
                self.floor.velocity.copy(order='F'),
                self.floor.acceleration.copy(order='F'),
                self.floor.maxVelocity,
                self.floor.maxAcceleration,
                self.floor.trajectoryOrder,
                self.floor.minimizeOrder,
                self.floor.margin,
                self.floor.doLimitVelocity,
                self.floor.doLimitAcceleration)
        self.xlb = lincon.xlb
        self.xub = lincon.xub
        self.clb = lincon.clb
        self.cub = lincon.cub
        # we need more
        self.sp_A = coo_matrix((lincon.aval, (lincon.arow, lincon.acol)))
        self.n_con = self.sp_A.shape[0]
        if self.verbose > 1:
            print('n_con', self.n_con)
            print('n_var', self.sp_A.shape[1])
            print("A has %d nnz" % lincon.aval.shape[0])

    def eval_cost_constr(self, mat_in):
        """Pass in a coefficient matrix, see results."""
        self.from_coef_matrix(mat_in)  # update self.sol
        self.update_prob()
        cost = 0.5 * self.sol.dot(self.sp_P.dot(self.sol))
        Ax = self.sp_A.dot(self.sol)
        # equality part
        error_clb = np.minimum(Ax - self.clb, 0)
        error_cub = np.minimum(-Ax + self.cub, 0)
        error_lb = np.minimum(self.sol - self.xlb, 0)
        error_ub = np.minimum(self.xub - self.sol, 0)
        with np.printoptions(precision=4, linewidth=10000):
            print('cost %f' % cost)
            print('error_eq', np.minimum(error_clb, error_cub))
            print('error_ieq', np.minimum(error_lb, error_ub))

    def get_gradient(self, sol, lmdy, lmdz):
        pgrad = gradient_from_P(self.obj_order, self.num_box, self.poly_order, self.room_time, self.MQM, sol)
        agrad = gradient_from_A(
                    self.floor.getCorridor(),
                    self.MQM,
                    self.floor.position.copy(order='F'),
                    self.floor.velocity.copy(order='F'),
                    self.floor.acceleration.copy(order='F'),
                    self.floor.maxVelocity,
                    self.floor.maxAcceleration,
                    self.floor.trajectoryOrder,
                    self.floor.minimizeOrder,
                    self.floor.margin,
                    self.floor.doLimitVelocity,
                    self.floor.doLimitAcceleration,
                    sol,
                    lmdy,
                    lmdz)
        if self.verbose > 1:
            print('pgrad', pgrad)
            print('agrad', agrad)
        return pgrad + agrad + self.tfweight

    def solve_once(self):
        """Solve the original problem once."""
        raise NotImplementedError

    def solve_with_room_time(self, rm_time):
        self.room_time[:] = rm_time
        self.floor.updateCorridorTime(self.room_time)
        return self.solve_once()


class IndoorQPProblemOSQP(IndoorQPProblem):
    """Solve indoor problem using osqp with specific options."""
    def __init__(self, tgp, tfweight=0, connect_order=2, verbose=False):
        IndoorQPProblem.__init__(self, tgp, tfweight, connect_order, verbose)
        self.h_type = "U"
        self.osqp_verbose = False
        self.osqp_maxiter = 20000
        self.osqp_polish = 1
        self._check_solution_accuracy = False

    def solve_once(self):
        self.update_prob()
        use_val = np.concatenate((self.sp_A.data, np.ones(self.n_var)))
        use_row = np.concatenate((self.sp_A.row, self.n_con + np.arange(self.n_var)))
        use_col = np.concatenate((self.sp_A.col, np.arange(self.n_var)))
        sp_A = coo_matrix((use_val, (use_row, use_col)))
        qp_l = np.concatenate((self.clb, self.xlb))
        qp_u = np.concatenate((self.cub, self.xub))
        prob = osqp.OSQP()
        prob.setup(P=self.sp_P.tocsc(), q=self.qp_q, A=sp_A.tocsc(), l=qp_l, u=qp_u,
                    verbose=self.osqp_verbose, max_iter=self.osqp_maxiter, polish=self.osqp_polish)
        results = prob.solve()
        self.is_solved = results.info.status_val > 0
        if self.is_solved:
            self.sol = results.x
            self.lmd = results.y
            self.obj = results.info.obj_val + self.tfweight * np.sum(self.room_time)
        else:
            self.obj = np.inf
        if self.verbose:
            print('Solving status', self.is_solved, self.obj)

    def get_gradient(self):
        """Calculate the gradient based on dual variables."""
        lmdy = self.lmd[:self.n_con]
        lmdz = self.lmd[self.n_con:]
        return IndoorQPProblem.get_gradient(self, self.sol, lmdy, lmdz)


class IndoorQPProblemMOSEK(IndoorQPProblem):
    """Use mosek solver to solve this problem in cvxopt interface or not"""
    def __init__(self, tgp, tfweight=0, connect_order=2, verbose=False):
        IndoorQPProblem.__init__(self, tgp, tfweight, connect_order, verbose)
        self.h_type = "L"

    def solve_once(self):
        self.update_prob()
        # set up A
        A_sp = self.sp_A.tocsc()
        colptr, asub, acof = A_sp.indptr, A_sp.indices, A_sp.data
        aptrb, aptre = colptr[:-1], colptr[1:]
        # set up bounds on x
        bkx = self.n_var * [mosek.boundkey.ra]
        bkc = self.n_con * [mosek.boundkey.ra]
        with mosek.Env() as env:
            with env.Task(0, 1) as task:
                task.inputdata(self.n_con, self.n_var, self.qp_q.tolist(), 0.0,
                                list(aptrb), list(aptre), list(asub), list(acof),
                                bkc, self.clb.tolist(), self.cub.tolist(), 
                                bkx, self.xlb.tolist(), self.xub.tolist()
                                )
                # set up lower triangular part of P
                task.putqobj(self.sp_P.row.tolist(), self.sp_P.col.tolist(), self.sp_P.data.tolist())
                task.putobjsense(mosek.objsense.minimize)
                task.optimize()
                solsta = task.getsolsta(mosek.soltype.itr)
                x = self.n_var * [0.0]
                task.getsolutionslice(mosek.soltype.itr, mosek.solitem.xx, 0, self.n_var, x)
                x = np.array(x)
                # get dual variables on linear constraints
                zu, zl = self.n_con * [0.0], self.n_con * [0.0]
                task.getsolutionslice(mosek.soltype.itr, mosek.solitem.suc, 0, self.n_con, zu)
                task.getsolutionslice(mosek.soltype.itr, mosek.solitem.slc, 0, self.n_con, zl)
                z = np.array(zu) - np.array(zl)
                # get dual variables on variable bounds
                yu, yl = self.n_var * [0.0], self.n_var * [0.0]
                task.getsolutionslice(mosek.soltype.itr, mosek.solitem.sux, 0, self.n_var, yu)
                task.getsolutionslice(mosek.soltype.itr, mosek.solitem.slx, 0, self.n_var, yl)
                y = np.array(yu) - np.array(yl)
                if self.verbose:
                    print("Solving status", solsta)
                if solsta == mosek.solsta.optimal: #solsta == mosek.solsta.near_optimal: near_optimal is longer valid in Mosek 9.0
                    self.is_solved = True
                    self.obj = task.getprimalobj(mosek.soltype.itr) + self.tfweight * np.sum(self.room_time)
                    self.sol = x
                    self.lmdy = z
                    self.lmdz = y
                    return solsta, x, z, y
                else:
                    self.is_solved = False
                    self.obj = np.inf
                    #print("Mosek Failed, solsta: ", solsta)
                    return solsta, None, None, None

    def get_gradient(self):
        return IndoorQPProblem.get_gradient(self, self.sol, self.lmdy, self.lmdz)


class IndoorQPProblemSQOPT(IndoorQPProblem):
    """Use mosek solver to solve this problem in cvxopt interface or not"""
    def __init__(self, tgp, tfweight=0, connect_order=2, verbose=False):
        IndoorQPProblem.__init__(self, tgp, tfweight, connect_order, verbose)
        self.warm_flag = 0

    def solve_once(self):
        self.update_prob()
        # set up A
        A_sp = self.sp_A.tocsc()
        # set up H
        H_sp = self.sp_P.tocsc()
        # set up solver
        if hasattr(self, 'solver'):
            solver = self.solver
        else:
            solver = Sqopt(A_sp.shape[1], A_sp.shape[0])
        #print("here 8")
        solver.set_H(H_sp.data, H_sp.indices, H_sp.indptr)
        solver.set_q_zero()
        solver.set_A(A_sp.data, A_sp.indices, A_sp.indptr)
        solver.set_bound(self.xlb, self.xub, self.clb, self.cub)

        #with Mute():
        solver.solve(self.warm_flag)

        #while solver.get_obj() < 0:
        #    print_yellow('Negative objective detected:', solver.get_obj())
        #    print('Solve again with cold start!')
        #    solver.solve(0)
        #    print("Obj after cold start", solver.get_obj(), solver.get_info() == 1)


        # using warm start might give solutions even when the QP is acutually infeasible
        # we hereby add a checking to make sure that all the bounds are feasible when a warm start is used
        if self.warm_flag != 0:
            # check if the bounds are violated
            #print("slacks", A_sp.dot(solver.get_solution()))
            #print("lbA", self.clb)
            #print("ubA", self.cub)
            # print(np.any(A_sp.dot(solver.get_solution()) < self.clb - 1e-6), np.any(A_sp.dot(solver.get_solution()) > self.cub + 1e-6), np.any(solver.get_solution() < self.xlb - 1e-6), np.any(solver.get_solution() > self.xub + 1e-6))
            if(np.any(A_sp.dot(solver.get_solution()) < self.clb - 1e-6) or np.any(A_sp.dot(solver.get_solution()) > self.cub + 1e-6) or np.any(solver.get_solution() < self.xlb - 1e-6) or np.any(solver.get_solution() > self.xub + 1e-6)):
                print_red("Switch to cold start")
                self.warm_flag = 0
                solver.solve(self.warm_flag)
                # print(np.any(A_sp.dot(solver.get_solution()) < self.clb - 1e-6), np.any(A_sp.dot(solver.get_solution()) > self.cub + 1e-6), np.any(solver.get_solution() < self.xlb - 1e-6), np.any(solver.get_solution() > self.xub + 1e-6))


        self.warm_flag = 1  # set to 1 after every solve
        self.solver = solver
        self.is_solved = solver.get_info() == 1
        # print("is solved? ", self.is_solved, solver.get_info())
        if self.is_solved:
            self.obj = solver.get_obj() + self.tfweight*np.sum(self.room_time)
            self.sol = solver.get_solution()
            lmd = solver.get_lambda()
            #print("here 9")
            self.lmdz = -lmd[:self.n_var]
            self.lmdy = -lmd[self.n_var:]
        else:
            self.obj = np.inf
        # print("self.room_time", self.room_time)

    def get_gradient(self):
        return IndoorQPProblem.get_gradient(self, self.sol, self.lmdy, self.lmdz)


def testSQOPTSolver():
    prob = 20
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6
    solver = IndoorQPProblemSQOPT(tgp, verbose=True)
    t0 = time.time()
    solver.solve_once()
    tf = time.time()
    print('flag %d cost %f' % (solver.is_solved, solver.obj))
    print('grad ', solver.get_gradient())
    print('sol ', solver.sol)
    print("Time consumption: ", tf - t0)
    
    poly_coef = solver.get_coef_matrix()
    
    np.savez('data/prob%d.npz' % prob, coef=poly_coef, time=solver.room_time)


def testIPBackTrack():
    """Test the backtrack line search with IP solver."""
    prob = 11
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])

    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6
    print("\033[1;91mTesting on problem: %d \033[0m" % prob)
    print("initial room time", [round(box.t,2) for box in tgp.getCorridor()])


    solver = IndoorQPProblemSQOPT(tgp, verbose=False)
    ts1 = time.time()
    solver.solve_once()
    tf1 = time.time()
    
    print_green("Use Sqopt + Adaptive LS")
    initialCost = solver.obj
    solver.grad_method = 'ours'
    
    solver.verbose = False
    ts2 = time.time()
    is_okay, converged = solver.refine_time_by_backtrack(max_iter=100, log=True, adaptiveLineSearch=True)
    tf2 = time.time()
    print("initial plan cost", initialCost)
    print('Sqopt refine ', is_okay, converged, solver.converge_reason, solver.obj, 
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    with np.printoptions(precision=2, suppress=True):
        print('Sqopt final time', solver.room_time, np.sum(solver.room_time))
    sqopt_time = tf2 - ts2 + tf1 - ts1
    sqopt_once_time = tf1 - ts1
    print('Sqopt first computation time', sqopt_once_time, 'Sqopt total computation time:', sqopt_time)
    sqopt_adaptive_convergence = solver.log
    sqopt_adaptive_convergence[1::2] += sqopt_once_time
    

    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)  # every time you have to reload from hard disk since it is modified before
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6
    print_green("Use Sqopt")
    
    solver = IndoorQPProblemSQOPT(tgp, verbose=False)
    ts1 = time.time()
    solver.solve_once()
    tf1 = time.time()
    
    
    initialCost = solver.obj
    solver.grad_method = 'ours'
    
    #solver.verbose = True
    ts2 = time.time()
    is_okay, converged = solver.refine_time_by_backtrack(max_iter=100, log=True, timeProfile=False, adaptiveLineSearch=False)
    tf2 = time.time()
    print("initial plan cost", initialCost)
    print('Sqopt refine ', is_okay, converged, solver.converge_reason, solver.obj, 
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    with np.printoptions(precision=2, suppress=True):
        print('Sqopt final time', solver.room_time, np.sum(solver.room_time))
    sqopt_time = tf2 - ts2 + tf1 - ts1
    sqopt_once_time = tf1 - ts1
    print('Sqopt first computation time', sqopt_once_time, 'Sqopt total computation time:', sqopt_time)
    sqopt_convergence = solver.log
    sqopt_convergence[1::2] += sqopt_once_time
    # print(solver.log)
    #print('Time profile: -Grad', solver.timeProfile[::2], 'Percentage: ', 100*np.sum(solver.timeProfile[::2])/sqopt_time)
    #print('Time profile: -Alpha', solver.timeProfile[1::2], 'Percentage: ', 100*np.sum(solver.timeProfile[1::2])/sqopt_time)

    
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)  # every time you have to reload from hard disk since it is modified before
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6
    print_green("Use OSQP")
    
    solver = IndoorQPProblemOSQP(tgp, verbose=False)
    ts1 = time.time()
    solver.solve_once()
    tf1 = time.time()

    initialCost = solver.obj
    print("initial plan cost", initialCost)
    print("osqp solved?", solver.is_solved)
    solver.grad_method = 'ours'

    ts2 = time.time()
    is_okay, converged = solver.refine_time_by_backtrack(max_iter=100, log=True, adaptiveLineSearch=False)
    tf2 = time.time()

    print('osqp refine ', is_okay, converged, solver.converge_reason, solver.obj,
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    #with np.printoptions(precision=2, suppress=True):
    #    print('osqp final time', solver.room_time, np.sum(solver.room_time))
    osqp_time = tf2 - ts2 + tf1 - ts1
    osqp_once_time = tf1 - ts1
    print('osqp first computation time', osqp_once_time, 'osqp computation time: ', osqp_time)
    osqp_convergence = solver.log
    osqp_convergence[1::2] += osqp_once_time
    

    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)  # every time you have to reload from hard disk since it is modified before
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6
    print_green("Use Mosek + Adaptive LS")
    
    solver = IndoorQPProblemMOSEK(tgp, verbose=False)
    ts1 = time.time()
    solver.solve_once()
    tf1 = time.time()

    print("initial plan cost", solver.obj)
    print('Mosek inital time', solver.room_time)
    t_before_opt, coeff_before_opt = solver.get_output_coefficients()
    solver.grad_method = 'ours'
    #solver.verbose = True
    
    ts2 = time.time()
    is_okay, converged = solver.refine_time_by_backtrack(max_iter=100, log=True, adaptiveLineSearch=True)
    tf2 = time.time()
    print('mosek refine ', is_okay, converged, solver.converge_reason, solver.obj, 
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    #with np.printoptions(precision=2, suppress=True):
    #    print('mosek final time', solver.room_time, np.sum(solver.room_time))
    mosek_time = tf2 - ts2 + tf1 - ts1
    mosek_once_time = tf1 - ts1

    print('mosek first computation time', mosek_once_time, 'mosek computation time:', mosek_time)
    mosek_convergence = solver.log
    mosek_convergence[1::2] += mosek_once_time

    t_after_opt, coeff_after_opt = solver.get_output_coefficients()

    
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)  # every time you have to reload from hard disk since it is modified before
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6
    print_green("Use Mosek + Mellinger")
   
    solver = IndoorQPProblemMOSEK(tgp, verbose=False)
    ts1 = time.time()
    solver.solve_once()
    tf1 = time.time()
    
    print("initial plan cost", solver.obj)
    t_before_opt, coeff_before_opt = solver.get_output_coefficients()
    solver.grad_method = 'mel'
    ts2 = time.time()
    is_okay, converged = solver.refine_time_by_backtrack(max_iter=100, log=True, adaptiveLineSearch=True)
    tf2 = time.time()
    print('mosek refine ', is_okay, converged, solver.converge_reason, solver.obj, 
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    #with np.printoptions(precision=2, suppress=True):
    #    print('mosek final time', solver.room_time, np.sum(solver.room_time))
    
    mosek_mel_time = tf2 - ts2 + tf1 - ts1
    mosek_mel_once_time = tf1 - ts1

    print('mosek first computation time', mosek_mel_once_time, 'mosek computation time:', mosek_mel_time)
    mosek_mel_convergence = solver.log
    mosek_mel_convergence[1::2] += mosek_mel_once_time

    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)  # every time you have to reload from hard disk since it is modified before
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6
    print_green("Use Sqopt + Mellinger")
    
    solver = IndoorQPProblemSQOPT(tgp, verbose=False)
    ts1 = time.time()
    solver.solve_once()
    tf1 = time.time()
    
    print("initial plan cost", solver.obj)
    t_before_opt, coeff_before_opt = solver.get_output_coefficients()
    solver.grad_method = 'mel'

    ts2 = time.time()
    is_okay, converged = solver.refine_time_by_backtrack(max_iter=100, log=True, timeProfile=False, adaptiveLineSearch=False)
    tf2 = time.time()
    print('sqopt refine ', is_okay, converged, solver.converge_reason, solver.obj, 
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    #with np.printoptions(precision=2, suppress=True):
    #    print('sqopt final time', solver.room_time, np.sum(solver.room_time))   
    sqopt_mel_time = tf2 - ts2 + tf1 - ts1
    sqopt_mel_once_time = tf1 - ts1

    print('sqopt first computation time', sqopt_mel_once_time, 'sqopt computation time:', sqopt_mel_time)
    sqopt_mel_convergence = solver.log
    sqopt_mel_convergence[1::2] += sqopt_mel_once_time
    #print('Time profile: -Grad', solver.timeProfile[::2], 'Percentage: ', 100*np.sum(solver.timeProfile[::2])/sqopt_mel_time)
    #print('Time profile: -Alpha', solver.timeProfile[1::2], 'Percentage: ', 100*np.sum(solver.timeProfile[1::2])/sqopt_mel_time)
    
    
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)  # every time you have to reload from hard disk since it is modified before
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6
    print_green("Use Sqopt + Mellinger + Adaptive LS")
    
    solver = IndoorQPProblemSQOPT(tgp, verbose=False)
    ts1 = time.time()
    solver.solve_once()
    tf1 = time.time()
    
    print("initial plan cost", solver.obj)
    t_before_opt, coeff_before_opt = solver.get_output_coefficients()
    solver.grad_method = 'mel'

    ts2 = time.time()
    is_okay, converged = solver.refine_time_by_backtrack(max_iter=100, log=True, timeProfile=False, adaptiveLineSearch=True)
    tf2 = time.time()
    print('sqopt refine ', is_okay, converged, solver.converge_reason, solver.obj, 
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    #with np.printoptions(precision=2, suppress=True):
    #    print('sqopt final time', solver.room_time, np.sum(solver.room_time))   
    sqopt_mel_time = tf2 - ts2 + tf1 - ts1
    sqopt_mel_once_time = tf1 - ts1

    print('sqopt first computation time', sqopt_mel_once_time, 'sqopt computation time:', sqopt_mel_time)
    sqopt_mel_adaptive_convergence = solver.log
    sqopt_mel_adaptive_convergence[1::2] += sqopt_mel_once_time


    np.savez('data/prob_coeffs%d.npz' % prob, 
        t_before_opt=t_before_opt, coeff_before_opt=coeff_before_opt,
        t_after_opt=t_after_opt, coeff_after_opt=coeff_after_opt,
        sqopt_convergence=sqopt_convergence, 
        osqp_convergence=osqp_convergence, 
        mosek_convergence=mosek_convergence,
        mosek_mel_convergence=mosek_mel_convergence,
        sqopt_mel_convergence=sqopt_mel_convergence,
        sqopt_adaptive_convergence=sqopt_adaptive_convergence,
        sqopt_mel_adaptive_convergence=sqopt_mel_adaptive_convergence)

def testBFGS():
    prob = 0
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])

    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    print("\033[1;92mTesting BFGS on problem: %d \033[0m" % prob)
    print("initial room time", [round(box.t,2) for box in tgp.getCorridor()])
    
    
    t0 = time.time()
    solver = IndoorQPProblemSQOPT(tgp, verbose=False)
    
    solver.solve_once()
    tf1 = time.time()
    initialCost = solver.obj
    t_before_opt, coeff_before_opt = solver.get_output_coefficients()
    
    solver.grad_method = 'ours'   
    solver.verbose = True  
    is_okay, converged = solver.refine_time_by_BFGS(max_iter=100, log=True)
    tf = time.time()
    t_after_opt, coeff_after_opt = solver.get_output_coefficients()
    
    print("initial plan cost", initialCost)
    print('Sqopt refine ', is_okay, converged, solver.converge_reason, solver.obj, 
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    with np.printoptions(precision=2, suppress=True):
        print('Sqopt final time', solver.room_time)
    
    sqopt_time = tf - t0
    sqopt_once_time = tf1 - t0
    print('Sqopt first computation time', sqopt_once_time, 'Sqopt total computation time:', sqopt_time)
    sqopt_bfgs_convergence = solver.log

    np.savez('data/prob_bfgs_coeffs%d.npz' % prob, 
        t_before_opt=t_before_opt, coeff_before_opt=coeff_before_opt,
        t_after_opt=t_after_opt, coeff_after_opt=coeff_after_opt,
        sqopt_bfgs_convergence=sqopt_bfgs_convergence)


def check_gradient():
    prob = 0
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])
    print("Test on problem %d" % prob)
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)
    print("initial room time", [round(box.t, 2) for box in tgp.getCorridor()])

    print("SQOPT")
    solver = IndoorQPProblemSQOPT(tgp, verbose=False)
    solver.solve_once()
    print('Sqopt gradient', solver.get_gradient())
    print("MOSEK")
    solver2 = IndoorQPProblemMOSEK(tgp, verbose=False)
    solver2.solve_once()
    print('mosek gradient', solver2.get_gradient())
    print('obj: %f %f' % (solver.obj, solver2.obj))
    print('sol diff %f' % np.linalg.norm(solver.sol - solver2.sol))
    print('lmdy diff %f' % np.linalg.norm(solver.lmdy - solver2.lmdy))
    print('lmdz diff %f' % np.linalg.norm(solver.lmdz - solver2.lmdz))
    print('lmdy ', solver.lmdy[np.abs(solver.lmdy) > 1e-3], solver2.lmdy[np.abs(solver2.lmdy) > 1e-3])
    print('lmdz ', solver.lmdz[np.abs(solver.lmdz) > 1e-3], solver2.lmdz[np.abs(solver2.lmdz) > 1e-3])


def test_osqp_solver():
    prob = 0
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)
    print(tgp.position)
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.maxVelocity = 100
    solver = IndoorQPProblemOSQP(tgp, verbose=False)
    solver.solve_once()
    n_var_per_room = 3 * (solver.poly_order + 1)
    sol = np.concatenate([solver.sol[i*n_var_per_room: (i+1)*n_var_per_room] * solver.room_time[i] for i in range(solver.num_box)])
    print(solver.is_solved, solver.obj, sol)


def test_mosek_solver():
    prob = 20
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)
    print(tgp.position)
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.maxVelocity = 100
    solver = IndoorQPProblemMOSEK(tgp, verbose=True)
    solver.solve_once()
    n_var_per_room = 3 * (solver.poly_order + 1)
    sol = np.concatenate([solver.sol[i*n_var_per_room: (i+1)*n_var_per_room] * solver.room_time[i] for i in range(solver.num_box)])
    print(solver.is_solved, solver.obj, sol)


def test_mosek_naive_case():
    prob = 0
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)
    pos = tgp.get_position()
    box = tgp.getCorridor()[0]
    print(box.getBox())
    pos[0] = [-20, -20, 1]
    pos[1] = [-19, -20, 1]
    print(tgp.position)
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 5
    tgp.maxVelocity = 100
    tgp.updateCorridorTime([2])  # let the corridor be 2 seconds
    print('time', tgp.getCorridor()[0].t)
    solver = IndoorQPProblemMOSEK(tgp, verbose=False)
    solver.solve_once()
    print('mqm', solver.MQM)
    print("P ", solver.sp_P.toarray()[:6, :6])
    n_var_per_room = 3 * (solver.poly_order + 1)
    sol = np.concatenate([solver.sol[i*n_var_per_room: (i+1)*n_var_per_room] * solver.room_time[i] for i in range(solver.num_box)])
    print(solver.is_solved, solver.obj, sol)

def test_mosek_case():
    tgp = loadTGP('dataset/tgp_%d.tgp' % 27)
    print("\033[1;92mTesting on problem: %d \033[0m" % 27)
    print("initial room time", [round(box.t,2) for box in tgp.getCorridor()])

    t0 = time.time()
    solver = IndoorQPProblemMOSEK(tgp, verbose=True)
    candy = np.array([ 0.8936, 1.1442, 0.8448, 1.1877, 0.7909, 0.0221, 3.4663, 0.1477, 0.4696, 0.7575, 1.1961, 12.0162])
    candy2 = np.array([8.9560e-01, 1.1462e+00, 8.4679e-01, 1.1897e+00, 7.9294e-01, 9.5743e-07, 3.4683e+00, 1.4965e-01, 4.7158e-01, 7.5952e-01, 1.1981e+00, 1.2018e+01])
    solver.solve_with_room_time(candy2)
    print(solver.is_solved, solver.obj)

def test_mosek_active_constraints():
    prob = 0
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)
    print("\033[1;92mTesting on problem: %d \033[0m" % prob)
    print("initial room time", [round(box.t,2) for box in tgp.getCorridor()])

    #t0 = time.time()
    solver = IndoorQPProblemMOSEK(tgp, verbose=True)
    #candy = np.array([ 0.8936, 1.1442, 0.8448, 1.1877, 0.7909, 0.0221, 3.4663, 0.1477, 0.4696, 0.7575, 1.1961, 12.0162])
    #candy2 = np.array([8.9560e-01, 1.1462e+00, 8.4679e-01, 1.1897e+00, 7.9294e-01, 9.5743e-07, 3.4683e+00, 1.4965e-01, 4.7158e-01, 7.5952e-01, 1.1981e+00, 1.2018e+01])
    solver.solve_once()
    slacks = solver.sp_A.dot(solver.sol)
    variableMask = np.where((solver.sol <= solver.xlb + 1e-7) | (solver.sol >= solver.xub - 1e-7))[0]
    slacksMask = np.where((slacks <= solver.clb + 1e-7) | (slacks >= solver.cub - 1e-7))[0]
    print('Variable Mask', variableMask)
    print('slacks Mask', slacksMask)
    #solver.solve_with_room_time(candy2)
    #print(solver.is_solved, solver.obj)

def check_prob_two():
    tgp = loadTGP('dataset/tgp_2.tgp')
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    solver = IndoorQPProblemMOSEK(tgp, verbose=2)
    solver.solve_once()
    print(solver.is_solved, solver.obj)


def test_grad_active_constr():
    """Test gradient the same issue"""
    prob = 0
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    solver = IndoorQPProblemMOSEK(tgp, verbose=True)
    solver.solve_once()
    grad = solver.get_gradient()
    print('grad is ', grad)
    n_var_per_room = 3 * (solver.poly_order + 1)
    active_idx = np.where(np.abs(solver.lmdz) > 1e-6)[0]
    room_id = active_idx // n_var_per_room
    print('active room ', room_id)

def compareDifferentTimes():
    prob = 400
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])

    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)  # every time you have to reload from hard disk since it is modified before
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6
    #print("unrefined room time", [round(box.t,2) for box in tgp.getCorridor()])
    #import pdb
    #pdb.set_trace()
    #print_green("Use Sqopt")
    
    solver = IndoorQPProblemSQOPT(tgp, verbose=False)
    #solver = IndoorQPProblemMOSEK(tgp, verbose=False)
    #solver.room_time = 1/0.82*solver.room_time
    
    ts1 = time.time()
    solver.solve_once()
    tf1 = time.time()
    #solver.solve_with_room_time(1/0.82*solver.room_time)

    t1, p1 = solver.get_output_coefficients()
    
    initialCost = solver.obj
    solver.grad_method = 'ours'
    
    #solver.verbose = True
    ts2 = time.time()
    is_okay, converged = solver.refine_time_by_backtrack(max_iter=100, log=True, timeProfile=False, adaptiveLineSearch=False)
    tf2 = time.time()
    print("initial plan cost", initialCost)
    print('Sqopt refine ', is_okay, converged, solver.converge_reason, solver.obj, 
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    #with np.printoptions(precision=2, suppress=True):
    #    print('Sqopt final time', solver.room_time)
    sqopt_time = tf2 - ts2 + tf1 - ts1
    sqopt_once_time = tf1 - ts1
    print('Sqopt first computation time', sqopt_once_time, 'Sqopt total computation time:', sqopt_time)
    sqopt_convergence = solver.log
    sqopt_convergence[1::2] += sqopt_once_time
    print("refined room time", [round(box.t,2) for box in tgp.getCorridor()])

    t2, p2 = solveScaled(prob, 80.0)

    v1 = piecewisePolyDeriv(p1, t1, orderOfDerivative = 1)
    v2 = piecewisePolyDeriv(p2, t2, orderOfDerivative = 1)
    
    a1 = piecewisePolyDeriv(p1, t1, orderOfDerivative = 2)
    a2 = piecewisePolyDeriv(p2, t2, orderOfDerivative = 2)
    
    j1 = piecewisePolyDeriv(p1, t1, orderOfDerivative = 3)
    j2 = piecewisePolyDeriv(p2, t2, orderOfDerivative = 3)
    
    s1 = piecewisePolyDeriv(p1, t1, orderOfDerivative = 4)
    s2 = piecewisePolyDeriv(p2, t2, orderOfDerivative = 4)
    
    sample_time_1, samples_1 = evaluatePiecewisePolyWhole(p1, t1)
    sample_time_2, samples_2 = evaluatePiecewisePolyWhole(p2, t2)
    
    sample_time_vt1, samples_v1 = evaluatePiecewisePolyWhole(v1, t1)
    sample_time_vt2, samples_v2 = evaluatePiecewisePolyWhole(v2, t2)
    
    sample_time_at1, samples_a1 = evaluatePiecewisePolyWhole(a1, t1)
    sample_time_at2, samples_a2 = evaluatePiecewisePolyWhole(a2, t2)
    
    sample_time_jt1, samples_j1 = evaluatePiecewisePolyWhole(j1, t1)
    sample_time_jt2, samples_j2 = evaluatePiecewisePolyWhole(j2, t2)

    sample_time_st1, samples_s1 = evaluatePiecewisePolyWhole(s1, t1)
    sample_time_st2, samples_s2 = evaluatePiecewisePolyWhole(s2, t2)
    
    connection_t1, connection_p1 = evaluatePiecewisePolyOne2(p1, t1, np.cumsum(t1)[:-1])
    connection_t2, connection_p2 = evaluatePiecewisePolyOne2(p2, t2, np.cumsum(t2)[:-1])
    
    connection_t1, connection_v1 = evaluatePiecewisePolyOne2(v1, t1, np.cumsum(t1)[:-1])
    connection_t2, connection_v2 = evaluatePiecewisePolyOne2(v2, t2, np.cumsum(t2)[:-1])
    
    connection_t1, connection_a1 = evaluatePiecewisePolyOne2(a1, t1, np.cumsum(t1)[:-1])
    connection_t2, connection_a2 = evaluatePiecewisePolyOne2(a2, t2, np.cumsum(t2)[:-1])
    
    connection_t1, connection_j1 = evaluatePiecewisePolyOne2(j1, t1, np.cumsum(t1)[:-1])
    connection_t2, connection_j2 = evaluatePiecewisePolyOne2(j2, t2, np.cumsum(t2)[:-1])

    connection_t1, connection_s1 = evaluatePiecewisePolyOne2(s1, t1, np.cumsum(t1)[:-1])
    connection_t2, connection_s2 = evaluatePiecewisePolyOne2(s2, t2, np.cumsum(t2)[:-1])

    saveToSeparatePdf = False
    
    fig_pos = plotSamples(sample_time_1, samples_1, connection_t1, connection_p1, sample_time_2, samples_2, connection_t2, connection_p2, "Position", r"Position [$m$]", heading=True)
    if saveToSeparatePdf is True:
        plt.savefig('trj_pos.pdf')
    fig_vel = plotSamples(sample_time_vt1, samples_v1, connection_t1, connection_v1, sample_time_vt2, samples_v2, connection_t2, connection_v2, "Velocity", r"Velocity [$m/s$]", plotConnection=True)
    if saveToSeparatePdf is True:
        plt.savefig('trj_vel.pdf')
    fig_acc = plotSamples(sample_time_at1, samples_a1, connection_t1, connection_a1, sample_time_at2, samples_a2, connection_t2, connection_a2, "Acceleration", r"Acceleration [$m/s^{2}$]", plotConnection=True)
    if saveToSeparatePdf is True:
        plt.savefig('trj_acc.pdf')
    fig_jerk = plotSamples(sample_time_jt1, samples_j1, connection_t1, connection_j1, sample_time_jt2, samples_j2, connection_t2, connection_j2, "Jerk", r"Jerk [$m/s^{3}$]", plotConnection=False)
    if saveToSeparatePdf is True:
        plt.savefig('trj_jerk.pdf')
    fig_snap = plotSamples(sample_time_st1, samples_s1, connection_t1, connection_s1, sample_time_st2, samples_s2, connection_t2, connection_s2, "Snap", r"Snap [$m/s^{3}$]")
    if saveToSeparatePdf is True:
        plt.savefig('trj_snap.pdf')



    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    #ax.set_aspect('equal')

    boxes = tgp.getCorridor()

    from mpl_toolkits.mplot3d.art3d import Poly3DCollection

    for i in range(len(boxes)):
        vertices = boxes[i].vertex
        ax.scatter(vertices[:, 0], vertices[:, 1], vertices[:, 2], s=1)
        Z = vertices
        
        verts = [[Z[0],Z[1],Z[2],Z[3]],
                 [Z[4],Z[5],Z[6],Z[7]], 
                 [Z[0],Z[1],Z[5],Z[4]], 
                 [Z[2],Z[3],Z[7],Z[6]], 
                 [Z[1],Z[2],Z[6],Z[5]],
                 [Z[4],Z[7],Z[3],Z[0]], 
                 [Z[2],Z[3],Z[7],Z[6]]]

        # plot sides
        pc = Poly3DCollection(verts, alpha = 0.02, facecolor='gray', linewidths=0.1, edgecolors='red')

        ax.add_collection3d(pc)

    plot3dTrajectory(ax, samples_1, samples_2, connection_t1,connection_p1, connection_t2,connection_p2, plotConnection=False, plotTiming=False)

    set_axes_equal(ax)
    ax.set_axis_off()

    plt.show()

def set_axes_equal(ax):
    '''Make axes of 3D plot have equal scale so that spheres appear as spheres,
    cubes as cubes, etc..  This is one possible solution to Matplotlib's
    ax.set_aspect('equal') and ax.axis('equal') not working for 3D.

    Input
      ax: a matplotlib axis, e.g., as output from plt.gca().
    '''

    x_limits = ax.get_xlim3d()
    y_limits = ax.get_ylim3d()
    z_limits = ax.get_zlim3d()

    x_range = abs(x_limits[1] - x_limits[0])
    x_middle = np.mean(x_limits)
    y_range = abs(y_limits[1] - y_limits[0])
    y_middle = np.mean(y_limits)
    z_range = abs(z_limits[1] - z_limits[0])
    z_middle = np.mean(z_limits)

    # The plot bounding box is a sphere in the sense of the infinity
    # norm, hence I call half the max range the plot radius.
    plot_radius = 0.5*max([x_range, y_range, z_range])

    ax.set_xlim3d([x_middle - plot_radius, x_middle + plot_radius])
    ax.set_ylim3d([y_middle - plot_radius, y_middle + plot_radius])
    ax.set_zlim3d([z_middle - plot_radius, z_middle + plot_radius])

def solveScaled(prob, scale=1.0):
    print_purple("In Scaled")

    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)  # every time you have to reload from hard disk since it is modified before
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6

    solver = IndoorQPProblemSQOPT(tgp, tfweight=scale, verbose=False)

    print("unrefined room time", solver.room_time, np.sum(solver.room_time))

    #solver.room_time *= scale
    #solver.room_time = 1.0*np.sum(solver.room_time)/len(solver.room_time)*np.ones(len(solver.room_time))

    #print("unrefined scaled room time", solver.room_time, np.sum(solver.room_time))


    ts1 = time.time()
    solver.solve_once()
    tf1 = time.time()

    initialCost = solver.obj
    solver.grad_method = 'ours'
    
    #solver.verbose = True
    ts2 = time.time()
    is_okay, converged = solver.refine_time_by_backtrack(max_iter=100, log=True, timeProfile=False, adaptiveLineSearch=False)
    tf2 = time.time()
    print("initial plan cost", initialCost)
    print('Sqopt refine ', is_okay, converged, solver.converge_reason, solver.obj, 
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    #with np.printoptions(precision=2, suppress=True):
    #    print('Sqopt final time', solver.room_time)
    sqopt_time = tf2 - ts2 + tf1 - ts1
    sqopt_once_time = tf1 - ts1
    print('Sqopt first computation time', sqopt_once_time, 'Sqopt total computation time:', sqopt_time)
    sqopt_convergence = solver.log
    sqopt_convergence[1::2] += sqopt_once_time

    refined_time = np.array([round(box.t,2) for box in tgp.getCorridor()])
    print("refined room time", refined_time, np.sum(refined_time))
    print("cost - time: ", solver.obj - scale * np.sum(refined_time))

    return solver.get_output_coefficients()
    
    
def testChopping():
    prob = 0
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])
    tgp = loadTGP('dataset/tgp_%d.tgp' % prob)

    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6

    #solver = IndoorQPProblemSQOPT(tgp, verbose=True)
    solver = IndoorQPProblemOSQP(tgp, verbose=True)
    solver.solve_once()

    t, poly_coeffs = solver.get_output_coefficients()
    print(t)
    print(poly_coeffs[:,:,0])
    print(poly_coeffs[:,:,1])



    boxes = tgp.getCorridor()

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    for i in range(len(boxes)):
        vertices = boxes[i].vertex
        ax.scatter(vertices[:, 0], vertices[:, 1], vertices[:, 2], s=1)
        Z = vertices
        
        verts = [[Z[0],Z[1],Z[2],Z[3]],
                 [Z[4],Z[5],Z[6],Z[7]], 
                 [Z[0],Z[1],Z[5],Z[4]], 
                 [Z[2],Z[3],Z[7],Z[6]], 
                 [Z[1],Z[2],Z[6],Z[5]],
                 [Z[4],Z[7],Z[3],Z[0]], 
                 [Z[2],Z[3],Z[7],Z[6]]]

        # plot sides
        pc = Poly3DCollection(verts, alpha = 0.02, facecolor='gray', linewidths=0.1, edgecolors='red')
        #pc.set_alpha(0.1)
        #pc.set_facecolor('cyan')
        
        #face_color = [0.5, 0.5, 1] # alternative: matplotlib.colors.rgb2hex([0.5, 0.5, 1])
        #pc.set_facecolor('none')

        ax.add_collection3d(pc)

    ax.axis('off')

    plt.show()


def use_mosek_bezier_coef():
    prob = 11
    if len(sys.argv) > 1:
        prob = int(sys.argv[1])

    tgp = loadTGP('ral_tgp/tgp_%d.tgp' % prob)  # every time you have to reload from hard disk since it is modified before
    tgp.doLimitVelocity = False
    tgp.minimizeOrder = 3
    tgp.trajectoryOrder = 6
    #import pdb
    #pdb.set_trace()
    #tgp.velocity = 0.0
    #tgp.acceleration = 0.0
    print(tgp.position)
    print(tgp.velocity)
    print(tgp.acceleration)
    print_red("Bezier coefs for RA-L:", end=" ")
    print_cyan("No.%d" % prob, end=" ")
    print_green("Mosek + Adaptive LS")
    
    solver = IndoorQPProblemMOSEK(tgp, verbose=False)
    ts1 = time.time()
    solver.solve_once()
    tf1 = time.time()

    print("initial plan cost", solver.obj)
    print('Mosek inital time', solver.room_time, "Total", np.sum(solver.room_time))
    t_before_opt, coeff_before_opt = solver.get_output_coefficients()
    solver.grad_method = 'ours'
    #solver.verbose = True
    
    ts2 = time.time()
    is_okay, converged = solver.refine_time_by_backtrack(max_iter=100, log=True, adaptiveLineSearch=True)
    tf2 = time.time()
    print('mosek refine ', is_okay, converged, solver.converge_reason, solver.obj, 
                'iter', solver.major_iteration, 'fval', solver.num_prob_solve)
    #with np.printoptions(precision=2, suppress=True):
    #    print('mosek final time', solver.room_time, np.sum(solver.room_time))
    mosek_time = tf2 - ts2 + tf1 - ts1
    mosek_once_time = tf1 - ts1

    print('mosek first computation time', mosek_once_time, 'mosek computation time:', mosek_time)
    mosek_convergence = solver.log
    mosek_convergence[1::2] += mosek_once_time

    t_after_opt, coeff_after_opt = solver.get_output_coefficients()
    poly_coef = solver.get_coef_matrix()
    poly_coef = poly_coef.transpose((1,0,2))
    
    if False:
        break_points = np.insert(np.cumsum(t_after_opt), 0, 0.0)
        break_points2 = np.arange(break_points.size)
        
        bp = BPoly(poly_coef, break_points)
        bp2 = BPoly(poly_coef, break_points2)

        #import pdb
        #pdb.set_trace()

        kkt = np.linspace(0.0, break_points[-1], 100)
        kkt2 = np.linspace(0.0, break_points2[-1], 100)
        
        plt.plot(bp(kkt)[:,0], bp(kkt)[:,1], '-..', label='bp: tro t')
        plt.plot(bp2(kkt2)[:,0], bp2(kkt2)[:,1], '.', label='bp2: uniform t')
        plt.legend()
        plt.show()
    
    if True:
        np.savez('ral_init/prob%d.npz' % prob, poly_coef=poly_coef, t_after_opt=t_after_opt)


def main():
    np.set_printoptions(precision=4, linewidth=10000)
    #test_grad_active_constr()  # this functions checks relation between # unique gradients and # active inequality constraints

    # check_prob_two()
    # test_mosek_case()
    
    testIPBackTrack()

    # compareDifferentTimes()
    
    # test_osqp_solver()
    # test_mosek_solver()
    # test_mosek_naive_case()
    # check_gradient()
    # testSQOPTSolver()
    # print("hi missy")
    # test_mosek_active_constraints()
    # testBFGS()
    
    # testChopping()
    
    # use_mosek_bezier_coef()


if __name__ == '__main__':
    # print("hi missy")
    main()
    # print("Hi again")
