/*
Copyright : Copyright (c) MOSEK ApS, Denmark. All rights reserved.

File :      modelLib.c

Purpose: Library of simple building blocks in Mosek Optimizer API.
*/
#include<mosek.h>
#include<stdio.h>

const double inf = 0.0;

// Add new constraints and return the index of the first one
int msk_newcon(MSKtask_t task, int num) {
  int c;
  MSK_getnumcon(task, &c);
  MSK_appendcons(task, num);
  return c;
}

// Add new variables and return the index of the first one
int msk_newvar(MSKtask_t task, int num) {  // free
  int i, v;
  MSK_getnumvar(task, &v);
  MSK_appendvars(task, num);
  for(i=0; i<num; i++)
    MSK_putvarbound(task, v+i, MSK_BK_FR, -inf, inf);
  return v;
}
int msk_newvar_fx(MSKtask_t task, int num, double val) {  // fixed
  int i, v;
  MSK_getnumvar(task, &v);
  MSK_appendvars(task, num);
  for(i=0; i<num; i++)
    MSK_putvarbound(task, v+i, MSK_BK_FX, val, val);
  return v;
}
int msk_newvar_bin(MSKtask_t task, int num) {  // binary
  int i, v;
  MSK_getnumvar(task, &v);
  MSK_appendvars(task, num);
  for(i=0; i<num; i++) {
    MSK_putvarbound(task, v+i, MSK_BK_RA, 0.0, 1.0);
    MSK_putvartype(task, v+i, MSK_VAR_TYPE_INT);
  }
  return v;
}

// Declare variables are duplicates or duplicate a variable and return index of duplicate
// x = y
void msk_equal(MSKtask_t task, int x, int y) {
  int c;
  c = msk_newcon(task, 1);
  MSK_putaij(task, c, x, 1.0);
  MSK_putaij(task, c, y, -1.0);
  MSK_putconbound(task, c, MSK_BK_FX, 0.0, 0.0);
}
int msk_dup(MSKtask_t task, int x) {
  int y;
  y = msk_newvar(task, 1);
  msk_equal(task, x, y);
  return y;
}

// Absolute value
// t >= |x|
void msk_abs(MSKtask_t task, int t, int x) {
  int c;
  c = msk_newcon(task, 2);
  MSK_putaij(task, c, t, 1.0);
  MSK_putaij(task, c, x, 1.0);
  MSK_putconbound(task, c, MSK_BK_LO, 0.0, inf);
  MSK_putaij(task, c+1, t, 1.0);
  MSK_putaij(task, c+1, x, -1.0);
  MSK_putconbound(task, c+1, MSK_BK_LO, 0.0, inf);
}

// 1-norm
// t >= sum( |x_i| ), x is a list of variables
void msk_norm1(MSKtask_t task, int t, int n, int* x) {
  int i, c, u;
  u = msk_newvar(task, n);
  for(i=0; i<n; i++) msk_abs(task, u+i, x[i]);
  c = msk_newcon(task, 1);
  for(i=0; i<n; i++) MSK_putaij(task, c, u+i, -1.0);
  MSK_putaij(task, c, t, 1.0);
  MSK_putconbound(task, c, MSK_BK_LO, 0.0, inf);
}

// Square
// t >= x^2
void msk_sq(MSKtask_t task, int t, int x) {
  int submem[3];
  submem[0] = msk_newvar_fx(task, 1, 0.5), submem[1] = t, submem[2] = x;
  MSK_appendcone(task, MSK_CT_RQUAD, 0.0, 3, submem);
}

// 2-norm
// t >= sqrt(x_1^2 + ... + x_n^2) where x is a list of variables
void msk_norm2(MSKtask_t task, int t, int n, int* x) {
  int* submem;
  int i;
  submem = (int*) MSK_calloctask(task, n+1, sizeof(int));
  submem[0] = t;
  for(i=0; i<n; i++) submem[i+1] = x[i];
  MSK_appendcone(task, MSK_CT_QUAD, 0.0, n+1, submem);
  MSK_freetask(task, (void*)submem);
}

// Power with exponent > 1
// t >= |x|^p (where p>1)
void msk_pow(MSKtask_t task, int t, int x, double p) {
  int submem[3];
  submem[0] = t, submem[1] = msk_newvar_fx(task, 1, 1.0), submem[2] = x;
  MSK_appendcone(task, MSK_CT_PPOW, 1.0/p, 3, submem);
}

// Inverse of power 
// t >= 1/x^p, x>0 (where p>0)
void msk_pow_inv(MSKtask_t task, int t, int x, double p) {
  int submem[3];
  submem[0] = t, submem[1] = x, submem[2] = msk_newvar_fx(task, 1, 1.0);
  MSK_appendcone(task, MSK_CT_PPOW, 1.0/(1.0+p), 3, submem);
}

// p-norm, p>1
// t >= \|x\|_p (where p>1), x is a list of variables
void msk_pnorm(MSKtask_t task, int t, int n, int* x, double p) {
  int i, r, c, submem[3];
  r = msk_newvar(task, n);
  for(i=0; i<n; i++) {
    submem[0] = t, submem[1] = r+i, submem[2] = x[i];
    MSK_appendcone(task, MSK_CT_PPOW, 1.0-1.0/p, 3, submem);
  }
  c = msk_newcon(task, 1);
  for(i=0; i<n; i++)
    MSK_putaij(task, c, r+i, -1.0);
  MSK_putaij(task, c, t, 1.0);
  MSK_putconbound(task, c, MSK_BK_FX, 0.0, 0.0);
}

// Geometric mean
// |t| <= (x_1...x_n)^(1/n), x_i>=0, x is a list of variables of Length >= 1
void msk_geo_mean(MSKtask_t task, int t, int n, int* x) {
  if (n==1) msk_abs(task, x[0], t);
  else {
    int t2, submem[3];
    t2 = msk_newvar(task, 1);
    submem[0] = t2, submem[1] = x[n-1], submem[2] = t;
    MSK_appendcone(task, MSK_CT_PPOW, 1.0-1.0/n, 3, submem);
    msk_geo_mean(task, msk_dup(task, t2), n-1, x);
  }
}

// Logarithm
// t <= log(x), x>=0
void msk_log(MSKtask_t task, int t, int x) {
  int submem[3];
  submem[0] = x, submem[1] = msk_newvar_fx(task, 1, 1.0), submem[2] = t;
  MSK_appendcone(task, MSK_CT_PEXP, 0.0, 3, submem);
}

// Exponential
// t >= exp(x)
void msk_exp(MSKtask_t task, int t, int x) {
  int submem[3];
  submem[0] = t, submem[1] = msk_newvar_fx(task, 1, 1.0), submem[2] = x;
  MSK_appendcone(task, MSK_CT_PEXP, 0.0, 3, submem);
}

// Entropy
// t >= x * log(x), x>=0
void msk_ent(MSKtask_t task, int t, int x) {
  int v, c, submem[3];
  v = msk_newvar(task, 1);
  c = msk_newcon(task, 1);
  MSK_putaij(task, c, v, 1.0);
  MSK_putaij(task, c, t, 1.0);
  MSK_putconbound(task, c, MSK_BK_FX, 0.0, 0.0);
  submem[0] = msk_newvar_fx(task, 1, 1.0), submem[1] = x, submem[2] = v;
  MSK_appendcone(task, MSK_CT_PEXP, 0.0, 3, submem);
}

// Relative entropy
// t >= x * log(x/y), x,y>=0
void msk_relent(MSKtask_t task, int t, int x, int y) {
  int v, c, submem[3];
  v = msk_newvar(task, 1);
  c = msk_newcon(task, 1);
  MSK_putaij(task, c, v, 1.0);
  MSK_putaij(task, c, t, 1.0);
  MSK_putconbound(task, c, MSK_BK_FX, 0.0, 0.0);
  submem[0] = y, submem[1] = x, submem[2] = v;
  MSK_appendcone(task, MSK_CT_PEXP, 0.0, 3, submem);
}

// Log-sum-exp
// log( sum_i(exp(x_i)) ) <= t, where x is a list of variables
void msk_logsumexp(MSKtask_t task, int t, int n, int* x) {
  int z, u, c, s, i;
  u = msk_newvar(task, n);
  z = msk_newvar(task, n);
  for(i=0; i<n; i++) msk_exp(task, u+i, z+i);
  c = msk_newcon(task, n);
  for(i=0; i<n; i++) {
    MSK_putaij(task, c+i, x[i], 1.0);
    MSK_putaij(task, c+i, t,   -1.0);
    MSK_putaij(task, c+i, z+i, -1.0);
    MSK_putconbound(task, c+i, MSK_BK_FX, 0.0, 0.0);
  }
  s = msk_newcon(task, 1);
  for(i=0; i<n; i++) MSK_putaij(task, s, u+i, 1.0);
  MSK_putconbound(task, s, MSK_BK_UP, -inf, 1.0);
}

// Semicontinuous variable
// x = 0 or a <= x <= b
void msk_semicontinuous(MSKtask_t task, int x, double a, double b) {
  int u, c;
  u = msk_newvar_bin(task, 1);
  c = msk_newcon(task, 2);
  MSK_putaij(task, c, x, 1.0);
  MSK_putaij(task, c, u, -a);
  MSK_putconbound(task, c, MSK_BK_LO, 0.0, inf);
  MSK_putaij(task, c+1, x, 1.0);
  MSK_putaij(task, c+1, u, -b);
  MSK_putconbound(task, c+1, MSK_BK_UP, -inf, 0.0);
}

// Indicator variable
// x!=0 implies t=1. Assumes that |x|<=1 in advance.
int msk_indicator(MSKtask_t task, int x) {
  int t;
  t = msk_newvar_bin(task, 1);
  msk_abs(task, t, x);
  return t;
}

// Logical OR
// x OR y, where x, y are binary
void msk_logic_or(MSKtask_t task, int x, int y) {
  int c;
  c = msk_newcon(task, 1);
  MSK_putaij(task, c, x, 1.0);
  MSK_putaij(task, c, y, 1.0);
  MSK_putconbound(task, c, MSK_BK_LO, 1.0, inf);
}
// x_1 OR ... OR x_n, where x is sequence of variables
void msk_logic_or_vect(MSKtask_t task, int n, int* x) {
  int c, i;
  c = msk_newcon(task, 1);
  for(i=0; i<n; i++) MSK_putaij(task, c, x[i], 1.0);
  MSK_putconbound(task, c, MSK_BK_LO, 1.0, inf);
}

// SOS1 (NAND)
// at most one of x_1,...,x_n, where x is a binary vector (SOS1 constraint)
void msk_logic_sos1(MSKtask_t task, int n, int* x) {
  int c, i;
  c = msk_newcon(task, 1);
  for(i=0; i<n; i++) MSK_putaij(task, c, x[i], 1.0);
  MSK_putconbound(task, c, MSK_BK_UP, -inf, 1.0);
}
// NOT(x AND y), where x, y are binary
void msk_logic_nand(MSKtask_t task, int x, int y) {
  int c;
  c = msk_newcon(task, 1);
  MSK_putaij(task, c, x, 1.0);
  MSK_putaij(task, c, y, 1.0);
  MSK_putconbound(task, c, MSK_BK_UP, -inf, 1.0);
}

// Cardinality bound
// At most k of entries in x are nonzero, assuming in advance that |x_i|<=1.
void msk_card(MSKtask_t task, int n, int* x, int k) {
  int t, i, c;
  t = msk_newvar_bin(task, n);
  for(i=0; i<n; i++) msk_abs(task, t+i, x[i]);
  c = msk_newcon(task, 1);
  for(i=0; i<n; i++) MSK_putaij(task, c, t+i, 1.0);
  MSK_putconbound(task, c, MSK_BK_UP, -inf, k);
}

// This is just a syntactic test without much sense
void testModels() {
  MSKenv_t env; 
  MSKtask_t task;
  int x, y, t, p, a, b, e, f, i;
  int sub[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30};

  MSK_makeenv(&env, NULL);
  MSK_makeemptytask(env, &task);

  x = msk_newvar(task, 1);
  y = msk_newvar(task, 1);
  t = msk_newvar(task, 1);
  p = msk_newvar(task, 5);
  a = msk_newvar(task, 5);
  b = msk_newvar(task, 15);
  e = msk_newvar_bin(task, 1);
  f = msk_newvar_bin(task, 1);

  msk_log(task, t, x);
  msk_exp(task, msk_dup(task, t), msk_dup(task, x));
  msk_ent(task, msk_dup(task, t), msk_dup(task, x));
  msk_relent(task, msk_dup(task, t), msk_dup(task, x), msk_dup(task, y));
  msk_logsumexp(task, t, 4, &(sub[p]));
  msk_abs(task, msk_dup(task, x), b+3);
  msk_norm1(task, msk_newvar_fx(task, 1, 0.74), 4, &(sub[a]));
  msk_sq(task, msk_dup(task, t), msk_dup(task, x));
  msk_norm2(task, msk_dup(task, t), 5, &(sub[b]));
  msk_pow(task, msk_dup(task, t), msk_dup(task, x), 1.5);
  msk_pow_inv(task, msk_dup(task, t), msk_dup(task, x), 3.3);
  msk_geo_mean(task, msk_dup(task, t), 5, &(sub[p]));
  msk_semicontinuous(task, y, 1.1, 2.2);
  i = msk_indicator(task, y);
  msk_logic_or(task, e, f);
  msk_logic_nand(task, e, f);
  msk_card(task, 5, &(sub[b]), 2);
}

// A sample problem using functions from the library
//
// max -sqrt(x^2 + y^2) + log(y) - x^1.5
//  st x >= y + 3
//
void testExample() {
  MSKenv_t env; 
  MSKtask_t task;
  int x, y, t, c, sub[3];
  double cj[3];

  MSK_makeenv(&env, NULL);
  MSK_makeemptytask(env, &task);

  x = msk_newvar(task, 1);
  y = msk_newvar(task, 1);
  t = msk_newvar(task, 3);

  c = msk_newcon(task, 1);
  MSK_putaij(task, c, x, 1.0);
  MSK_putaij(task, c, y, -1.0);
  MSK_putconbound(task, c, MSK_BK_LO, 3.0, inf);

  sub[0] = x, sub[1] = y;
  msk_norm2(task, t+0, 2, sub);
  msk_log  (task, t+1, msk_dup(task, y));
  msk_pow  (task, t+2, msk_dup(task, x), 1.5);

  sub[0] = t, sub[1] = t+1, sub[2] = t+2;
  cj[0] = -1.0, cj[1] = 1.0, cj[2] = -1.0;
  MSK_putclist(task, 3, sub, cj);
  MSK_putobjsense(task, MSK_OBJECTIVE_SENSE_MAXIMIZE);

  MSK_optimizetrm(task, NULL);

  {
    double *xx;
    double obj;
    int numvar;

    MSK_getnumvar(task, &numvar);
    xx = (double*) calloc(numvar, sizeof(double));
    MSK_getxx(task, MSK_SOL_ITR, xx);
    MSK_getprimalobj(task, MSK_SOL_ITR, &obj);
    printf("x=%f, y=%f, val=%f\n", xx[x], xx[y], obj);
    free(xx);
  }
}

int main() {
  testModels();
  testExample();
  return 0;
}