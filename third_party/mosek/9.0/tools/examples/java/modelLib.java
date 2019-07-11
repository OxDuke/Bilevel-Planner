/*
  Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File :      modelLib.java

  Purpose: Library of simple building blocks in Mosek Optimizer API.
*/
package com.mosek.example;
import mosek.*;
import java.util.Arrays;

public class modelLib {

  public static double inf = 0.0;

  // Add new constraints and return the index of the first one
  public static int msk_newcon(Task task, int num) {
    int c = task.getnumcon();
    task.appendcons(num);
    return c;
  }

  // Add new variables and return the index of the first one
  public static int msk_newvar(Task task, int num) {  // free
    int v = task.getnumvar();
    task.appendvars(num);
    for(int i=0; i<num; i++)
      task.putvarbound(v+i, boundkey.fr, -inf, inf);
    return v;
  }
  public static int msk_newvar_fx(Task task, int num, double val) {  // fixed
    int v = task.getnumvar();
    task.appendvars(num);
    for(int i=0; i<num; i++)
      task.putvarbound(v+i, boundkey.fx, val, val);
    return v;
  }
  public static int msk_newvar_bin(Task task, int num) {  // binary
    int v = task.getnumvar();
    task.appendvars(num);
    for(int i=0; i<num; i++) {
      task.putvarbound(v+i, boundkey.ra, 0.0, 1.0);
      task.putvartype(v+i, variabletype.type_int);
    }
    return v;
  }

  // Declare variables are duplicates or duplicate a variable and return index of duplicate
  // x = y
  public static void msk_equal(Task task, int x, int y) {
    int c = msk_newcon(task, 1);
    task.putaij(c, x, 1.0);
    task.putaij(c, y, -1.0);
    task.putconbound(c, boundkey.fx, 0.0, 0.0);
  }
  public static int msk_dup(Task task, int x) {
    int y = msk_newvar(task, 1);
    msk_equal(task, x, y);
    return y;
  }

  // Absolute value
  // t >= |x|
  public static void msk_abs(Task task, int t, int x) {
    int c = msk_newcon(task, 2);
    task.putaij(c, t, 1.0);
    task.putaij(c, x, 1.0);
    task.putconbound(c, boundkey.lo, 0.0, inf);
    task.putaij(c+1, t, 1.0);
    task.putaij(c+1, x, -1.0);
    task.putconbound(c+1, boundkey.lo, 0.0, inf);
  }

  // 1-norm
  // t >= sum( |x_i| ), x is a list of variables
  public static void msk_norm1(Task task, int t, int[] x) {
    int n = x.length;
    int u = msk_newvar(task, n);
    for(int i=0; i<n; i++) msk_abs(task, u+i, x[i]);
    int c = msk_newcon(task, 1);
    for(int i=0; i<n; i++) task.putaij(c, u+i, -1.0);
    task.putaij(c, t, 1.0);
    task.putconbound(c, boundkey.lo, 0.0, inf);
  }

  // Square
  // t >= x^2
  public static void msk_sq(Task task, int t, int x) {
    task.appendcone(conetype.rquad, 0.0, new int[]{msk_newvar_fx(task, 1, 0.5), t, x});
  }

  // 2-norm
  // t >= sqrt(x_1^2 + ... + x_n^2) where x is a list of variables
  public static void msk_norm2(Task task, int t, int[] x) {
    int[] submem = new int[x.length+1];
    submem[0] = t;
    for(int i=0; i<x.length; i++) submem[i+1] = x[i];
    task.appendcone(conetype.quad, 0.0, submem);
  }

  // Power with exponent > 1
  // t >= |x|^p (where p>1)
  public static void msk_pow(Task task, int t, int x, double p) {
    task.appendcone(conetype.ppow, 1.0/p, new int[]{t, msk_newvar_fx(task, 1, 1.0), x});
  }

  // Inverse of power 
  // t >= 1/x^p, x>0 (where p>0)
  public static void msk_pow_inv(Task task, int t, int x, double p) {
    task.appendcone(conetype.ppow, 1.0/(1.0+p), new int[]{t, x, msk_newvar_fx(task, 1, 1.0)});
  }

  // p-norm, p>1
  // t >= \|x\|_p (where p>1), x is a list of variables
  public static void msk_pnorm(Task task, int t, int[] x, double p) {
    int n = x.length;
    int r = msk_newvar(task, n);
    for(int i=0; i<n; i++)
      task.appendcone(conetype.ppow, 1.0-1.0/p, new int[]{t, r+i, x[i]});
    int c = msk_newcon(task, 1);
    for(int i=0; i<n; i++)
      task.putaij(c, r+i, -1.0);
    task.putaij(c, t, 1.0);
    task.putconbound(c, boundkey.fx, 0.0, 0.0);
  }

  // Geometric mean
  // |t| <= (x_1...x_n)^(1/n), x_i>=0, x is a list of variables of length >= 1
  public static void msk_geo_mean(Task task, int t, int[] x) {
    int n = x.length;
    if (n==1) msk_abs(task, x[0], t);
    else {
      int t2 = msk_newvar(task, 1);
      task.appendcone(conetype.ppow, 1.0-1.0/n, new int[]{t2, x[n-1], t});
      msk_geo_mean(task, msk_dup(task, t2), Arrays.copyOfRange(x, 0, n-1));
    }
  }

  // Logarithm
  // t <= log(x), x>=0
  public static void msk_log(Task task, int t, int x) {
    task.appendcone(conetype.pexp, 0.0, new int[]{x, msk_newvar_fx(task, 1, 1.0), t});
  }

  // Exponential
  // t >= exp(x)
  public static void msk_exp(Task task, int t, int x) {
    task.appendcone(conetype.pexp, 0.0, new int[]{t, msk_newvar_fx(task, 1, 1.0), x});
  }

  // Entropy
  // t >= x * log(x), x>=0
  public static void msk_ent(Task task, int t, int x) {
    int v = msk_newvar(task, 1);
    int c = msk_newcon(task, 1);
    task.putaij(c, v, 1.0);
    task.putaij(c, t, 1.0);
    task.putconbound(c, boundkey.fx, 0.0, 0.0);
    task.appendcone(conetype.pexp, 0.0, new int[]{msk_newvar_fx(task, 1, 1.0), x, v});
  }

  // Relative entropy
  // t >= x * log(x/y), x,y>=0
  public static void msk_relent(Task task, int t, int x, int y) {
    int v = msk_newvar(task, 1);
    int c = msk_newcon(task, 1);
    task.putaij(c, v, 1.0);
    task.putaij(c, t, 1.0);
    task.putconbound(c, boundkey.fx, 0.0, 0.0);
    task.appendcone(conetype.pexp, 0.0, new int[]{y, x, v});
  }

  // Log-sum-exp
  // log( sum_i(exp(x_i)) ) <= t, where x is a list of variables
  public static void msk_logsumexp(Task task, int t, int[] x) {
    int n = x.length;
    int u = msk_newvar(task, n);
    int z = msk_newvar(task, n);
    for(int i=0; i<n; i++) msk_exp(task, u+i, z+i);
    int c = msk_newcon(task, n);
    for(int i=0; i<n; i++) {
      task.putarow(c+i, new int[]{x[i], t, z+i}, new double[]{1.0, -1.0, -1.0});
      task.putconbound(c+i, boundkey.fx, 0.0, 0.0);
    }
    int s = msk_newcon(task, 1);
    for(int i=0; i<n; i++) task.putaij(s, u+i, 1.0);
    task.putconbound(s, boundkey.up, -inf, 1.0);
  }

  // Semicontinuous variable
  // x = 0 or a <= x <= b
  public static void msk_semicontinuous(Task task, int x, double a, double b) {
    int u = msk_newvar_bin(task, 1);
    int c = msk_newcon(task, 2);
    task.putarow(c, new int[]{x, u}, new double[]{1.0, -a});
    task.putconbound(c, boundkey.lo, 0.0, inf);
    task.putarow(c+1, new int[]{x, u}, new double[]{1.0, -b});
    task.putconbound(c+1, boundkey.up, -inf, 0.0);
  }

  // Indicator variable
  // x!=0 implies t=1. Assumes that |x|<=1 in advance.
  public static int msk_indicator(Task task, int x) {
    int t = msk_newvar_bin(task, 1);
    msk_abs(task, t, x);
    return t;
  }

  // Logical OR
  // x OR y, where x, y are binary
  public static void msk_logic_or(Task task, int x, int y) {
    int c = msk_newcon(task, 1);
    task.putarow(c, new int[]{x, y}, new double[]{1.0, 1.0});
    task.putconbound(c, boundkey.lo, 1.0, inf);
  }
  // x_1 OR ... OR x_n, where x is sequence of variables
  public static void msk_logic_or_vect(Task task, int[] x) {
    int c = msk_newcon(task, 1);
    for(int i=0; i<x.length; i++) task.putaij(c, x[i], 1.0);
    task.putconbound(c, boundkey.lo, 1.0, inf);
  }

  // SOS1 (NAND)
  // at most one of x_1,...,x_n, where x is a binary vector (SOS1 constraint)
  public static void msk_logic_sos1(Task task, int[] x) {
    int c = msk_newcon(task, 1);
    for(int i=0; i<x.length; i++) task.putaij(c, x[i], 1.0);
    task.putconbound(c, boundkey.up, -inf, 1.0);
  }
  // NOT(x AND y), where x, y are binary
  public static void msk_logic_nand(Task task, int x, int y) {
    int c = msk_newcon(task, 1);
    task.putarow(c, new int[]{x, y}, new double[]{1.0, 1.0});
    task.putconbound(c, mosek.boundkey.up, -inf, 1.0);
  }

  // Cardinality bound
  // At most k of entries in x are nonzero, assuming in advance that |x_i|<=1.
  public static void msk_card(Task task, int[] x, int k) {
    int n = x.length;
    int t = msk_newvar_bin(task, n);
    for(int i=0; i<n; i++) msk_abs(task, t+i, x[i]);
    int c = msk_newcon(task, 1);
    for(int i=0; i<n; i++) task.putaij(c, t+i, 1.0);
    task.putconbound(c,boundkey.up, -inf, k);
  }

  // This is just a syntactic test without much sense
  public static void testModels() {
    Env env = new Env();
    Task task = new Task(env);
    int x = msk_newvar(task, 1);
    int y = msk_newvar(task, 1);
    int t = msk_newvar(task, 1);
    int p = msk_newvar(task, 5);
    int a = msk_newvar(task, 15);
    int b = msk_newvar(task, 15);
    int e = msk_newvar_bin(task, 1);
    int f = msk_newvar_bin(task, 1);

    msk_log(task, t, x);
    msk_exp(task, msk_dup(task, t), msk_dup(task, x));
    msk_ent(task, msk_dup(task, t), msk_dup(task, x));
    msk_relent(task, msk_dup(task, t), msk_dup(task, x), msk_dup(task, y));
    msk_logsumexp(task, t, new int[]{p, p+1, p+2, p+3, p+4});
    msk_abs(task, msk_dup(task, x), b+3);
    msk_norm1(task, msk_newvar_fx(task, 1, 0.74), new int[]{a, a+9, a+13, a+4});
    msk_sq(task, msk_dup(task, t), msk_dup(task, x));
    msk_norm2(task, msk_dup(task, t), new int[]{b+1, b+3, b+4, b+5, b+6});
    msk_pow(task, msk_dup(task, t), msk_dup(task, x), 1.5);
    msk_pow_inv(task, msk_dup(task, t), msk_dup(task, x), 3.3);
    msk_geo_mean(task, msk_dup(task, t), new int[]{p, p+1, p+2, p+3, p+4});
    msk_semicontinuous(task, y, 1.1, 2.2);
    int i = msk_indicator(task, y);
    msk_logic_or(task, e, f);
    msk_logic_nand(task, e, f);
    msk_card(task, new int[]{b+1, b+3, b+4, b+5, b+6}, 2);
  }

  // A sample problem using functions from the library
  //
  // max -sqrt(x^2 + y^2) + log(y) - x^1.5
  //  st x >= y + 3
  //
  public static void testExample() {
    Env env = new Env();
    Task task = new Task(env);
    int x = msk_newvar(task, 1);
    int y = msk_newvar(task, 1);
    int t = msk_newvar(task, 3);

    int c = msk_newcon(task, 1);
    task.putarow(c, new int[]{x, y}, new double[]{1.0, -1.0});
    task.putconbound(c, boundkey.lo, 3.0, inf);

    msk_norm2(task, t+0, new int[]{x,y});
    msk_log  (task, t+1, msk_dup(task, y));
    msk_pow  (task, t+2, msk_dup(task, x), 1.5);

    task.putclist(new int[]{t, t+1, t+2}, new double[]{-1.0, 1.0, -1.0});
    task.putobjsense(objsense.maximize);

    task.set_Stream(
      mosek.streamtype.log,
      new mosek.Stream() { public void stream(String msg) { System.out.print(msg); }});

    task.optimize();
    task.solutionsummary(streamtype.log);

    double[] xx = new double[task.getnumvar()];
    task.getxx(soltype.itr, xx);
    System.out.format("x=%f y=%f obj=%f\n", xx[x], xx[y], task.getprimalobj(soltype.itr));
  }

  public static void main(String[] args) {
    testModels();
    testExample();
  }
}