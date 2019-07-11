//
//   Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//   File:      gp1.c
//
//   Purpose:   Demonstrates how to solve a simple Geometric Program (GP)
//              cast into conic form with exponential cones and log-sum-exp.
//
//              Example from
//                https://gpkit.readthedocs.io/en/latest/examples.html//maximizing-the-volume-of-a-box
//
#include <stdio.h>
#include <math.h>
#include "mosek.h"

/* This function prints log output from MOSEK to the terminal. */
static void MSKAPI printstr(void       *handle,
                            const char str[])
{
  printf("%s", str);
} /* printstr */

// maximize     h*w*d
// subjecto to  2*(h*w + h*d) <= Awall
//              w*d <= Afloor
//              alpha <= h/w <= beta
//              gamma <= d/w <= delta
//
// Variable substitutions:  h = exp(x), w = exp(y), d = exp(z).
//
// maximize     x+y+z
// subject      log( exp(x+y+log(2/Awall)) + exp(x+z+log(2/Awall)) ) <= 0
//                              y+z <= log(Afloor)
//              log( alpha ) <= x-y <= log( beta )
//              log( gamma ) <= z-y <= log( delta )
int max_volume_box(double Aw, double Af, 
                   double alpha, double beta, double gamma, double delta,
                   double hwd[])
{
  // Basic dimensions of our problem
  const int numvar    = 3;  // Variables in original problem
  const int numLinCon = 3;  // Linear constraints in original problem
  const int numExp    = 2;  // Number of exp-terms in the log-sum-exp constraint

  // Linear part of the problem
  const double       cval[]  = {1, 1, 1};
  const int          asubi[] = {0, 0, 1, 1, 2, 2};
  const int          asubj[] = {1, 2, 0, 1, 2, 1};
  const int          alen    = 6;
  const double       aval[]  = {1.0, 1.0, 1.0, -1.0, 1.0, -1.0};
  const MSKboundkeye bkc[]   = {MSK_BK_UP, MSK_BK_RA, MSK_BK_RA};
  const double       blc[]   = {-MSK_INFINITY, log(alpha), log(gamma)};
  const double       buc[]   = {log(Af), log(beta), log(delta)};

  // Linear part setting up slack variables
  // for the linear expressions appearing inside exps
  // x_5 - x - y = log(2/Awall)
  // x_8 - x - z = log(2/Awall)
  // The slack indexes are convenient for defining exponential cones, see later
  const int          a2subi[] = {3, 3, 3, 4, 4, 4};
  const int          a2subj[] = {5, 0, 1, 8, 0, 2};
  const int          a2len    = 6;
  const double       a2val[]  = {1.0, -1.0, -1.0, 1.0, -1.0, -1.0};
  const MSKboundkeye b2kc[]   = {MSK_BK_FX, MSK_BK_FX};
  const double       b2luc[]  = {log(2/Aw), log(2/Aw)};

  MSKenv_t           env  = NULL;
  MSKtask_t          task = NULL;
  MSKrescodee        r, trmcode;
  MSKsolstae         solsta;
  MSKint32t          i;
  double             *xyz = (double*) calloc(numvar, sizeof(double));

  r = MSK_makeenv(&env, NULL);

  if (r == MSK_RES_OK)
    r = MSK_maketask(env, 0, 0, &task);

  if (r == MSK_RES_OK)
    r = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

  if (r == MSK_RES_OK)
    r = MSK_appendvars(task, numvar + 3*numExp);

  if (r == MSK_RES_OK)
    r = MSK_appendcons(task, numLinCon + numExp + 1);

  // Objective is the sum of three first variables
  if (r == MSK_RES_OK)
    r = MSK_putobjsense(task, MSK_OBJECTIVE_SENSE_MAXIMIZE);
  if (r == MSK_RES_OK)
    r = MSK_putcslice(task, 0, numvar, cval);
  if (r == MSK_RES_OK)
    r = MSK_putvarboundsliceconst(task, 0, numvar, MSK_BK_FR, -MSK_INFINITY, +MSK_INFINITY);

  // Add the three linear constraints
  if (r == MSK_RES_OK)
    r = MSK_putaijlist(task, alen, asubi, asubj, aval);
  if (r == MSK_RES_OK)  
    r = MSK_putconboundslice(task, 0, numvar, bkc, blc, buc);

  // Add linear constraints for the expressions appearing in exp(...)
  if (r == MSK_RES_OK)
    r = MSK_putaijlist(task, a2len, a2subi, a2subj, a2val);
  if (r == MSK_RES_OK)
    r = MSK_putconboundslice(task, numLinCon, numLinCon+numExp, b2kc, b2luc, b2luc);

  {
    int c = numLinCon + numExp;
    int expStart = numvar;
    // Add a single log-sum-exp constraint sum(log(exp(z_i))) <= 0
    // Assume numExp variable triples are ordered as (u0,t0,z0,u1,t1,z1...)
    // starting from variable with index expStart
    double       *val = (double*) calloc(numExp, sizeof(double));
    int          *sub = (int*) calloc(numExp, sizeof(int));
    MSKconetypee  *ct = (MSKconetypee*) calloc(numExp, sizeof(MSKconetypee));
    int          *len = (int*) calloc(numExp, sizeof(int));

    // sum(u_i) = 1 as constraint number c, u_i unbounded
    for(i = 0; i < numExp; i++)
      { sub[i] = expStart + 3*i; val[i] = 1.0; }
    if (r == MSK_RES_OK)
      r = MSK_putarow(task, c, numExp, sub, val);
    if (r == MSK_RES_OK)
      r = MSK_putconbound(task, c, MSK_BK_FX, 1.0, 1.0);
    if (r == MSK_RES_OK)
      r = MSK_putvarboundlistconst(task, numExp, sub, MSK_BK_FR, -MSK_INFINITY, MSK_INFINITY);

    // z_i unbounded
    for(i = 0; i < numExp; i++) sub[i] = expStart + 3*i + 2;
    if (r == MSK_RES_OK)
      r = MSK_putvarboundlistconst(task, numExp, sub, MSK_BK_FR, -MSK_INFINITY, MSK_INFINITY);

    // t_i = 1
    for(i = 0; i < numExp; i++) sub[i] = expStart + 3*i + 1;
    if (r == MSK_RES_OK)
      r = MSK_putvarboundlistconst(task, numExp, sub, MSK_BK_FX, 1, 1);

    // Every triple is in an exponential cone
    for(i = 0; i < numExp; i++) 
      { ct[i] = MSK_CT_PEXP; val[i] = 0.0; len[i] = 3; }
    if (r == MSK_RES_OK)
      r = MSK_appendconesseq(task, numExp, ct, val, len, expStart);

    free(val); free(sub); free(ct); free(len);
  }

  // Solve and map to original h, w, d
  if (r == MSK_RES_OK)
    r = MSK_optimizetrm(task, &trmcode);

  if (r == MSK_RES_OK)
    MSK_getsolsta(task, MSK_SOL_ITR, &solsta);

  if (solsta == MSK_SOL_STA_OPTIMAL)
  {
    if (r == MSK_RES_OK)
      r = MSK_getxxslice(task, MSK_SOL_ITR, 0, numvar, xyz);
    for(i = 0; i < numvar; i++) hwd[i] = exp(xyz[i]);
  }
  else
  {
    printf("Solution not optimal, termination code %d.\n", trmcode);
    r = trmcode;
  }

  free(xyz);
  return r;
}
    
int main()
{
  const double Aw    = 200.0;
  const double Af    = 50.0;
  const double alpha = 2.0;
  const double beta  = 10.0;
  const double gamma = 2.0;
  const double delta = 10.0;
  MSKrescodee  r;
  double       hwd[3];

  r = max_volume_box(Aw, Af, alpha, beta, gamma, delta, hwd);

  printf("Response code: %d\n", r);
  if (r == MSK_RES_OK)
    printf("Solution h=%.4f w=%.4f d=%.4f\n", hwd[0], hwd[1], hwd[2]);

  return r;
}