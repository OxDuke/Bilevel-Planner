////
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      logistic.c
//
// Purpose: Implements logistic regression with regulatization.
//
//          Demonstrates using the exponential cone and log-sum-exp in Optimizer API.

#include <stdio.h>
#include "mosek.h" /* Include the MOSEK definition file. */

#define MSKCALL(x) if (res==MSK_RES_OK) res = (x);

static void MSKAPI printstr(void *handle,
                            const char str[])
{
  printf("%s", str);
} /* printstr */

const double inf = 0.0;

// t >= log( 1 + exp(u) )
// t_i >= log( 1 + exp(u_i) ), i = 0..n-1
// Adds auxiliary variables and constraints
MSKrescodee softplus(MSKtask_t task, MSKint32t t, MSKint32t u, int n)
{
  MSKint32t nvar, ncon;
  MSKint32t z1, z2, v1, v2, q1, q2;
  MSKint32t zcon, v1con, v2con;
  MSKint32t     *subi = (MSKint32t*) calloc(7*n, sizeof(MSKint32t));
  MSKint32t     *subj = (MSKint32t*) calloc(7*n, sizeof(MSKint32t));
  MSKrealt      *aval = (MSKrealt*) calloc(7*n, sizeof(MSKrealt));
  int         k = 0, i = 0;
  MSKrescodee res = MSK_RES_OK;

  MSKCALL(MSK_getnumvar(task, &nvar));
  MSKCALL(MSK_getnumcon(task, &ncon));
  z1 = nvar, z2 = nvar+n, v1 = nvar+2*n, v2 = nvar+3*n, q1 = nvar+4*n, q2 = nvar+5*n;
  zcon = ncon, v1con = ncon+n, v2con=ncon+2*n;
  MSKCALL(MSK_appendvars(task, 6*n));
  MSKCALL(MSK_appendcons(task, 3*n));

  // Linear constraints
  for(i = 0; i < n; i++)
  {
    // z1 + z2 = 1
    subi[k] = zcon+i;  subj[k] = z1+i;  aval[k] = 1;  k++;
    subi[k] = zcon+i;  subj[k] = z2+i;  aval[k] = 1;  k++;
    // u - t - v1 = 0
    subi[k] = v1con+i; subj[k] = u+i;   aval[k] = 1;  k++;
    subi[k] = v1con+i; subj[k] = t+i;   aval[k] = -1; k++;
    subi[k] = v1con+i; subj[k] = v1+i;  aval[k] = -1; k++;
    // - t - v2 = 0
    subi[k] = v2con+i; subj[k] = t+i;   aval[k] = -1; k++;
    subi[k] = v2con+i; subj[k] = v2+i;  aval[k] = -1; k++;
  }
  MSKCALL(MSK_putaijlist(task, 7*n, subi, subj, aval));
  MSKCALL(MSK_putconboundsliceconst(task, ncon, ncon+n, MSK_BK_FX, 1, 1));
  MSKCALL(MSK_putconboundsliceconst(task, ncon+n, ncon+3*n, MSK_BK_FX, 0, 0));

  // Bounds for variables
  MSKCALL(MSK_putvarboundsliceconst(task, nvar, nvar+4*n, MSK_BK_FR, -inf, inf));
  MSKCALL(MSK_putvarboundsliceconst(task, nvar+4*n, nvar+6*n, MSK_BK_FX, 1, 1));

  // Cones
  for(i = 0; i < n && res == MSK_RES_OK; i++)
  {
    MSKint32t csub[3];
    csub[0] = z1+i; csub[1] = q1+i; csub[2] = v1+i;
    MSKCALL(MSK_appendcone(task, MSK_CT_PEXP, 0.0, 3, csub));
    csub[0] = z2+i; csub[1] = q2+i; csub[2] = v2+i;
    MSKCALL(MSK_appendcone(task, MSK_CT_PEXP, 0.0, 3, csub));
  }
  
  free(subi); free(subj); free(aval); 
  return res;
}

// Model logistic regression (regularized with full 2-norm of theta)
// X - n x d matrix of data points
// y - length n vector classifying training points
// lamb - regularization parameter
MSKrescodee logisticRegression(MSKenv_t       env,
                               int            n,    // num samples
                               int            d,    // dimension
                               double        *X, 
                               int           *y,
                               double         lamb,
                               double        *thetaVal)   // result
{
  MSKrescodee res = MSK_RES_OK;
  MSKrescodee trm = MSK_RES_OK;
  MSKtask_t task = NULL;
  MSKint32t nvar = 1+d+2*n;
  MSKint32t r = 0, theta = 1, t = 1+d, u = 1+d+n;
  int i = 0;

  MSKCALL(MSK_maketask(env, 0, 0, &task));
  MSKCALL(MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr));

  // Variables [r; theta; t; u]
  MSKCALL(MSK_appendvars(task, nvar));
  MSKCALL(MSK_putvarboundsliceconst(task, 0, nvar, MSK_BK_FR, -inf, inf));
    
  // Constraints: theta'*X +/- u = 0
  MSKCALL(MSK_appendcons(task, n));
  MSKCALL(MSK_putconboundsliceconst(task, 0, n, MSK_BK_FX, 0, 0));
    
  // Objective lambda*r + sum(t)
  MSKCALL(MSK_putcj(task, r, lamb));
  for(i = 0; i < n && res == MSK_RES_OK; i++) 
    MSKCALL(MSK_putcj(task, t+i, 1.0));

  // The X block in theta'*X +/- u = 0
  {
    MSKint32t *subi = (MSKint32t*) calloc(d*n+n, sizeof(MSKint32t));
    MSKint32t *subj = (MSKint32t*) calloc(d*n+n, sizeof(MSKint32t));
    MSKrealt  *aval = (MSKrealt*) calloc(d*n+n, sizeof(MSKrealt));
    int  j, k;
    
    for(i = 0; i < n; i++)
    {
      for(j = 0; j < d; j++)
      {
        k = i * d + j;
        subi[k] = i; subj[k] = theta+j; aval[k] = X[k]; 
      }
      subi[d*n+i] = i; subj[d*n+i] = u+i;
      if (y[i]) aval[d*n+i] = 1; else aval[d*n+i] = -1;
    }

    MSKCALL(MSK_putaijlist(task, n*d+n, subi, subj, aval));

    free(subi); free(subj); free(aval);
  }

  // Softplus function constraints
  MSKCALL(softplus(task, t, u, n));

  // Regularization
  MSKCALL(MSK_appendconeseq(task, MSK_CT_QUAD, 0.0, 1+d, r));

  // Solution
  MSKCALL(MSK_optimizetrm(task, &trm));
  MSKCALL(MSK_solutionsummary(task, MSK_STREAM_MSG));

  MSKCALL(MSK_getxxslice(task, MSK_SOL_ITR, theta, theta+d, thetaVal));
  
  return res;
}

int main()
{
  MSKenv_t env;
  MSKrescodee res = MSK_RES_OK;

  MSKCALL(MSK_makeenv(&env, NULL));
  
  // Test: detect and approximate a circle using degree 2 polynomials
  {
    int     n = 30;
    double  X[6*30*30];
    int     Y[30*30];
    int     i,j;
    double  theta[6];
  
    for(i=0; i<n; i++) 
    for(j=0; j<n; j++)
    {
      int k = i*n+j;
      double x = -1 + 2.0*i/(n-1);
      double y = -1 + 2.0*j/(n-1);
      X[6*k+0] = 1.0; X[6*k+1] = x; X[6*k+2] = y; X[6*k+3] = x*y;
      X[6*k+4] = x*x; X[6*k+5] = y*y;
      Y[k] = (x*x+y*y>=0.69) ? 1 : 0;
    }

    MSKCALL(logisticRegression(env, n*n, 6, X, Y, 0.1, theta));

    if (res == MSK_RES_OK)
      for(i=0;i<6;i++) printf("%.4f\n", theta[i]);
  }

  return res;
}