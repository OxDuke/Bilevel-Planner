/*
  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:    solutionquality.c

  Purpose: To demonstrate how to examine the quality of a solution.
*/

#include <math.h>
#include "mosek.h"

static double dmin(double x,
                   double y)
{
  return (x <= y) ? (x) : (y);
} /* dmin */

static double dmax(double x,
                   double y)
{
  return (x >= y) ? (x) : (y);
} /* dmax */

static void MSKAPI printstr(void *handle,
                            const char str[])
{
  printf("%s", str);
} /* printstr */


int main(int argc, const char * argv[])
{
  double max_primal_viol, /* maximal primal violation */
         max_dual_viol,  /* maximal dual violation */
         abs_obj_gap,
         rel_obj_gap;

  MSKenv_t    env      = NULL;

  MSKint32t   numvar, j;

  MSKsolstae  solsta;
  MSKsoltypee whichsol = MSK_SOL_BAS;

  MSKrealt    primalobj, pviolcon, pviolvar, pviolbarvar, pviolcones, pviolitg,
              dualobj, dviolcon, dviolvar, dviolbarvar, dviolcones, xj;

  MSKrescodee r        = MSK_RES_OK;
  MSKrescodee trmcode;

  MSKtask_t   task     = NULL;

  int         accepted = 0;

  if (argc <= 1)
  {
    printf("Missing argument. The syntax is:\n");
    printf(" solutionquality inputfile\n");
  }
  else
  {

    r = MSK_makeenv(&env, NULL);

    if (r == MSK_RES_OK)
      r = MSK_makeemptytask(env, &task);

    if (r == MSK_RES_OK)
      MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

    /* We assume that a problem file was given as the first command
       line argument (received in `argv'). */
    if (r == MSK_RES_OK)
      r = MSK_readdata(task, argv[1]);

    /* Solve the problem */
    if (r == MSK_RES_OK)
    {
      r = MSK_optimizetrm(task, &trmcode);
    }

    /* Print a summary of the solution. */
    MSK_solutionsummary(task, MSK_STREAM_MSG);

    if (r == MSK_RES_OK)
    {

      MSK_getsolsta(task, whichsol, &solsta);

      r = MSK_getsolutioninfo(task,
                              whichsol,
                              &primalobj,
                              &pviolcon,
                              &pviolvar,
                              &pviolbarvar,
                              &pviolcones,
                              &pviolitg,
                              &dualobj,
                              &dviolcon,
                              &dviolvar,
                              &dviolbarvar,
                              &dviolcones);

      switch (solsta)
      {
        case MSK_SOL_STA_OPTIMAL:
          {

            abs_obj_gap     = fabs(dualobj - primalobj);
            rel_obj_gap     = abs_obj_gap / (1.0 + dmin(fabs(primalobj), fabs(dualobj)));
            max_primal_viol = dmax(pviolcon, pviolvar);
            max_primal_viol = dmax(max_primal_viol, pviolbarvar);
            max_primal_viol = dmax(max_primal_viol, pviolcones);

            max_dual_viol   = dmax(dviolcon, dviolvar);
            max_dual_viol   = dmax(max_dual_viol, dviolbarvar);
            max_dual_viol   = dmax(max_dual_viol, dviolcones);

            /* Assume the application needs the solution to be within
               1e-6 optimality in an absolute sense. Another approach
               would be looking at the relative objective gap */

            printf("\n\n");
            printf("Customized solution information.\n");
            printf("  Absolute objective gap: %e\n", abs_obj_gap);
            printf("  Relative objective gap: %e\n", rel_obj_gap);
            printf("  Max primal violation  : %e\n", max_primal_viol);
            printf("  Max dual violation    : %e\n", max_dual_viol);

            if (rel_obj_gap > 1e-6)
            {
              printf("Warning: The relative objective gap is LARGE.\n");
              accepted = 0;
            }

            /* We will accept a primal infeasibility of 1e-8 and
               dual infeasibility of 1e-6. These number should chosen problem
               dependent.
             */

            if (max_primal_viol > 1e-8)
            {
              printf("Warning: Primal violation is too LARGE.\n");
              accepted = 0;
            }

            if (max_dual_viol > 1e-6)
            {
              printf("Warning: Dual violation is too LARGE.\n");
              accepted = 0;
            }

            if (accepted)
            {

              if (MSK_RES_OK == MSK_getnumvar(task, &numvar))
              {
                printf("Optimal primal solution\n");
                for (j = 0; j < numvar && r == MSK_RES_OK; ++j)
                {
                  r = MSK_getxxslice(task, whichsol, j, j + 1, &xj);
                  if (r == MSK_RES_OK)
                    printf("x[%d]: %e\n", j, xj);
                }
              }
            }
            else if (r == MSK_RES_OK)
            {
              /* Print detailed information about the solution */
              r = MSK_analyzesolution(task, MSK_STREAM_LOG, whichsol);
            }
            break;
          }
        case MSK_SOL_STA_DUAL_INFEAS_CER:
        case MSK_SOL_STA_PRIM_INFEAS_CER:
          printf("Primal or dual infeasibility certificate found.\n");
          break;
        case MSK_SOL_STA_UNKNOWN:
          printf("The status of the solution is unknown. Termination code: %d.\n", trmcode);
          break;
        default:
          printf("Other solution status");
          break;
      }
    }

    MSK_deletetask(&task);
    MSK_deleteenv(&env);
  }
  return (r);
}