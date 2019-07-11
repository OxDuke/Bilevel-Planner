#ifndef MOSEK_H
#define MOSEK_H

/******************************************************************************
 ** Module : mosek.h
 **
 ** Generated 2019
 **
 ** Copyright (c) MOSEK ApS, Denmark.
 **
 ** All rights reserved
 **
 ******************************************************************************/
/* 
 The content of this file is subject to copyright. However, it may free
 of charge be redistributed in identical form --- i.e. with no changes of
 the wording --- for any legitimate purpose.
*/ 


#include <stdlib.h>
#include <stdio.h>

#define MSK_VERSION_MAJOR    9
#define MSK_VERSION_MINOR    0
#define MSK_VERSION_REVISION 97
#define MSK_VERSION_STATE    ""

#define MSK_INFINITY 1.0e30

/* BEGIN PLATFORM SPECIFIC DEFINITIONS (linux64x86) */
#define MSKAPI   
#define MSKAPIVA 
/* END   PLATFORM SPECIFIC DEFINITIONS (linux64x86) */


/* Enums and constants */
/* namespace mosek { */
enum MSKsolveform_enum {
  MSK_SOLVE_BEGIN  = 0,
  MSK_SOLVE_END    = 3,

  MSK_SOLVE_FREE   = 0,
  MSK_SOLVE_PRIMAL = 1,
  MSK_SOLVE_DUAL   = 2
};

enum MSKpurify_enum {
  MSK_PURIFY_BEGIN       = 0,
  MSK_PURIFY_END         = 5,

  MSK_PURIFY_NONE        = 0,
  MSK_PURIFY_PRIMAL      = 1,
  MSK_PURIFY_DUAL        = 2,
  MSK_PURIFY_PRIMAL_DUAL = 3,
  MSK_PURIFY_AUTO        = 4
};

enum MSKsensitivitytype_enum {
  MSK_SENSITIVITY_TYPE_BEGIN = 0,
  MSK_SENSITIVITY_TYPE_END   = 1,

  MSK_SENSITIVITY_TYPE_BASIS = 0
};

enum MSKsimreform_enum {
  MSK_SIM_REFORMULATION_BEGIN      = 0,
  MSK_SIM_REFORMULATION_END        = 4,

  MSK_SIM_REFORMULATION_OFF        = 0,
  MSK_SIM_REFORMULATION_ON         = 1,
  MSK_SIM_REFORMULATION_FREE       = 2,
  MSK_SIM_REFORMULATION_AGGRESSIVE = 3
};

enum MSKsimhotstart_enum {
  MSK_SIM_HOTSTART_BEGIN       = 0,
  MSK_SIM_HOTSTART_END         = 3,

  MSK_SIM_HOTSTART_NONE        = 0,
  MSK_SIM_HOTSTART_FREE        = 1,
  MSK_SIM_HOTSTART_STATUS_KEYS = 2
};

enum MSKstreamtype_enum {
  MSK_STREAM_BEGIN = 0,
  MSK_STREAM_END = 4,

  MSK_STREAM_LOG = 0,
  MSK_STREAM_MSG = 1,
  MSK_STREAM_ERR = 2,
  MSK_STREAM_WRN = 3
};

enum MSKsymmattype_enum {
  MSK_SYMMAT_TYPE_BEGIN  = 0,
  MSK_SYMMAT_TYPE_END    = 1,

  MSK_SYMMAT_TYPE_SPARSE = 0
};

enum MSKiinfitem_enum {
  MSK_IINF_BEGIN                        = 0,
  MSK_IINF_END                          = 101,

  MSK_IINF_ANA_PRO_NUM_CON              = 0,
  MSK_IINF_ANA_PRO_NUM_CON_EQ           = 1,
  MSK_IINF_ANA_PRO_NUM_CON_FR           = 2,
  MSK_IINF_ANA_PRO_NUM_CON_LO           = 3,
  MSK_IINF_ANA_PRO_NUM_CON_RA           = 4,
  MSK_IINF_ANA_PRO_NUM_CON_UP           = 5,
  MSK_IINF_ANA_PRO_NUM_VAR              = 6,
  MSK_IINF_ANA_PRO_NUM_VAR_BIN          = 7,
  MSK_IINF_ANA_PRO_NUM_VAR_CONT         = 8,
  MSK_IINF_ANA_PRO_NUM_VAR_EQ           = 9,
  MSK_IINF_ANA_PRO_NUM_VAR_FR           = 10,
  MSK_IINF_ANA_PRO_NUM_VAR_INT          = 11,
  MSK_IINF_ANA_PRO_NUM_VAR_LO           = 12,
  MSK_IINF_ANA_PRO_NUM_VAR_RA           = 13,
  MSK_IINF_ANA_PRO_NUM_VAR_UP           = 14,
  MSK_IINF_INTPNT_FACTOR_DIM_DENSE      = 15,
  MSK_IINF_INTPNT_ITER                  = 16,
  MSK_IINF_INTPNT_NUM_THREADS           = 17,
  MSK_IINF_INTPNT_SOLVE_DUAL            = 18,
  MSK_IINF_MIO_ABSGAP_SATISFIED         = 19,
  MSK_IINF_MIO_CLIQUE_TABLE_SIZE        = 20,
  MSK_IINF_MIO_CONSTRUCT_SOLUTION       = 21,
  MSK_IINF_MIO_NODE_DEPTH               = 22,
  MSK_IINF_MIO_NUM_ACTIVE_NODES         = 23,
  MSK_IINF_MIO_NUM_BRANCH               = 24,
  MSK_IINF_MIO_NUM_CLIQUE_CUTS          = 25,
  MSK_IINF_MIO_NUM_CMIR_CUTS            = 26,
  MSK_IINF_MIO_NUM_GOMORY_CUTS          = 27,
  MSK_IINF_MIO_NUM_IMPLIED_BOUND_CUTS   = 28,
  MSK_IINF_MIO_NUM_INT_SOLUTIONS        = 29,
  MSK_IINF_MIO_NUM_KNAPSACK_COVER_CUTS  = 30,
  MSK_IINF_MIO_NUM_RELAX                = 31,
  MSK_IINF_MIO_NUM_REPEATED_PRESOLVE    = 32,
  MSK_IINF_MIO_NUMBIN                   = 33,
  MSK_IINF_MIO_NUMBINCONEVAR            = 34,
  MSK_IINF_MIO_NUMCON                   = 35,
  MSK_IINF_MIO_NUMCONE                  = 36,
  MSK_IINF_MIO_NUMCONEVAR               = 37,
  MSK_IINF_MIO_NUMCONT                  = 38,
  MSK_IINF_MIO_NUMCONTCONEVAR           = 39,
  MSK_IINF_MIO_NUMDEXPCONES             = 40,
  MSK_IINF_MIO_NUMDPOWCONES             = 41,
  MSK_IINF_MIO_NUMINT                   = 42,
  MSK_IINF_MIO_NUMINTCONEVAR            = 43,
  MSK_IINF_MIO_NUMPEXPCONES             = 44,
  MSK_IINF_MIO_NUMPPOWCONES             = 45,
  MSK_IINF_MIO_NUMQCONES                = 46,
  MSK_IINF_MIO_NUMRQCONES               = 47,
  MSK_IINF_MIO_NUMVAR                   = 48,
  MSK_IINF_MIO_OBJ_BOUND_DEFINED        = 49,
  MSK_IINF_MIO_PRESOLVED_NUMBIN         = 50,
  MSK_IINF_MIO_PRESOLVED_NUMBINCONEVAR  = 51,
  MSK_IINF_MIO_PRESOLVED_NUMCON         = 52,
  MSK_IINF_MIO_PRESOLVED_NUMCONE        = 53,
  MSK_IINF_MIO_PRESOLVED_NUMCONEVAR     = 54,
  MSK_IINF_MIO_PRESOLVED_NUMCONT        = 55,
  MSK_IINF_MIO_PRESOLVED_NUMCONTCONEVAR = 56,
  MSK_IINF_MIO_PRESOLVED_NUMDEXPCONES   = 57,
  MSK_IINF_MIO_PRESOLVED_NUMDPOWCONES   = 58,
  MSK_IINF_MIO_PRESOLVED_NUMINT         = 59,
  MSK_IINF_MIO_PRESOLVED_NUMINTCONEVAR  = 60,
  MSK_IINF_MIO_PRESOLVED_NUMPEXPCONES   = 61,
  MSK_IINF_MIO_PRESOLVED_NUMPPOWCONES   = 62,
  MSK_IINF_MIO_PRESOLVED_NUMQCONES      = 63,
  MSK_IINF_MIO_PRESOLVED_NUMRQCONES     = 64,
  MSK_IINF_MIO_PRESOLVED_NUMVAR         = 65,
  MSK_IINF_MIO_RELGAP_SATISFIED         = 66,
  MSK_IINF_MIO_TOTAL_NUM_CUTS           = 67,
  MSK_IINF_MIO_USER_OBJ_CUT             = 68,
  MSK_IINF_OPT_NUMCON                   = 69,
  MSK_IINF_OPT_NUMVAR                   = 70,
  MSK_IINF_OPTIMIZE_RESPONSE            = 71,
  MSK_IINF_PURIFY_DUAL_SUCCESS          = 72,
  MSK_IINF_PURIFY_PRIMAL_SUCCESS        = 73,
  MSK_IINF_RD_NUMBARVAR                 = 74,
  MSK_IINF_RD_NUMCON                    = 75,
  MSK_IINF_RD_NUMCONE                   = 76,
  MSK_IINF_RD_NUMINTVAR                 = 77,
  MSK_IINF_RD_NUMQ                      = 78,
  MSK_IINF_RD_NUMVAR                    = 79,
  MSK_IINF_RD_PROTYPE                   = 80,
  MSK_IINF_SIM_DUAL_DEG_ITER            = 81,
  MSK_IINF_SIM_DUAL_HOTSTART            = 82,
  MSK_IINF_SIM_DUAL_HOTSTART_LU         = 83,
  MSK_IINF_SIM_DUAL_INF_ITER            = 84,
  MSK_IINF_SIM_DUAL_ITER                = 85,
  MSK_IINF_SIM_NUMCON                   = 86,
  MSK_IINF_SIM_NUMVAR                   = 87,
  MSK_IINF_SIM_PRIMAL_DEG_ITER          = 88,
  MSK_IINF_SIM_PRIMAL_HOTSTART          = 89,
  MSK_IINF_SIM_PRIMAL_HOTSTART_LU       = 90,
  MSK_IINF_SIM_PRIMAL_INF_ITER          = 91,
  MSK_IINF_SIM_PRIMAL_ITER              = 92,
  MSK_IINF_SIM_SOLVE_DUAL               = 93,
  MSK_IINF_SOL_BAS_PROSTA               = 94,
  MSK_IINF_SOL_BAS_SOLSTA               = 95,
  MSK_IINF_SOL_ITG_PROSTA               = 96,
  MSK_IINF_SOL_ITG_SOLSTA               = 97,
  MSK_IINF_SOL_ITR_PROSTA               = 98,
  MSK_IINF_SOL_ITR_SOLSTA               = 99,
  MSK_IINF_STO_NUM_A_REALLOC            = 100
};

enum MSKdataformat_enum {
  MSK_DATA_FORMAT_BEGIN     = 0,
  MSK_DATA_FORMAT_END       = 9,

  MSK_DATA_FORMAT_EXTENSION = 0,
  MSK_DATA_FORMAT_MPS       = 1,
  MSK_DATA_FORMAT_LP        = 2,
  MSK_DATA_FORMAT_OP        = 3,
  MSK_DATA_FORMAT_FREE_MPS  = 4,
  MSK_DATA_FORMAT_TASK      = 5,
  MSK_DATA_FORMAT_PTF       = 6,
  MSK_DATA_FORMAT_CB        = 7,
  MSK_DATA_FORMAT_JSON_TASK = 8
};

enum MSKprosta_enum {
  MSK_PRO_STA_BEGIN                    = 0,
  MSK_PRO_STA_END                      = 9,

  MSK_PRO_STA_UNKNOWN                  = 0,
  MSK_PRO_STA_PRIM_AND_DUAL_FEAS       = 1,
  MSK_PRO_STA_PRIM_FEAS                = 2,
  MSK_PRO_STA_DUAL_FEAS                = 3,
  MSK_PRO_STA_PRIM_INFEAS              = 4,
  MSK_PRO_STA_DUAL_INFEAS              = 5,
  MSK_PRO_STA_PRIM_AND_DUAL_INFEAS     = 6,
  MSK_PRO_STA_ILL_POSED                = 7,
  MSK_PRO_STA_PRIM_INFEAS_OR_UNBOUNDED = 8
};

enum MSKboundkey_enum {
  MSK_BK_BEGIN = 0,
  MSK_BK_END = 5,

  MSK_BK_LO = 0,
  MSK_BK_UP = 1,
  MSK_BK_FX = 2,
  MSK_BK_FR = 3,
  MSK_BK_RA = 4
};

enum MSKliinfitem_enum {
  MSK_LIINF_BEGIN                    = 0,
  MSK_LIINF_END                      = 13,

  MSK_LIINF_BI_CLEAN_DUAL_DEG_ITER   = 0,
  MSK_LIINF_BI_CLEAN_DUAL_ITER       = 1,
  MSK_LIINF_BI_CLEAN_PRIMAL_DEG_ITER = 2,
  MSK_LIINF_BI_CLEAN_PRIMAL_ITER     = 3,
  MSK_LIINF_BI_DUAL_ITER             = 4,
  MSK_LIINF_BI_PRIMAL_ITER           = 5,
  MSK_LIINF_INTPNT_FACTOR_NUM_NZ     = 6,
  MSK_LIINF_MIO_ANZ                  = 7,
  MSK_LIINF_MIO_INTPNT_ITER          = 8,
  MSK_LIINF_MIO_PRESOLVED_ANZ        = 9,
  MSK_LIINF_MIO_SIMPLEX_ITER         = 10,
  MSK_LIINF_RD_NUMANZ                = 11,
  MSK_LIINF_RD_NUMQNZ                = 12
};

enum MSKinftype_enum {
  MSK_INF_BEGIN     = 0,
  MSK_INF_END       = 3,

  MSK_INF_DOU_TYPE  = 0,
  MSK_INF_INT_TYPE  = 1,
  MSK_INF_LINT_TYPE = 2
};

enum MSKorderingtype_enum {
  MSK_ORDER_METHOD_BEGIN          = 0,
  MSK_ORDER_METHOD_END            = 6,

  MSK_ORDER_METHOD_FREE           = 0,
  MSK_ORDER_METHOD_APPMINLOC      = 1,
  MSK_ORDER_METHOD_EXPERIMENTAL   = 2,
  MSK_ORDER_METHOD_TRY_GRAPHPAR   = 3,
  MSK_ORDER_METHOD_FORCE_GRAPHPAR = 4,
  MSK_ORDER_METHOD_NONE           = 5
};

enum MSKbasindtype_enum {
  MSK_BI_BEGIN       = 0,
  MSK_BI_END         = 5,

  MSK_BI_NEVER       = 0,
  MSK_BI_ALWAYS      = 1,
  MSK_BI_NO_ERROR    = 2,
  MSK_BI_IF_FEASIBLE = 3,
  MSK_BI_RESERVERED  = 4
};

enum MSKparametertype_enum {
  MSK_PAR_BEGIN        = 0,
  MSK_PAR_END          = 4,

  MSK_PAR_INVALID_TYPE = 0,
  MSK_PAR_DOU_TYPE     = 1,
  MSK_PAR_INT_TYPE     = 2,
  MSK_PAR_STR_TYPE     = 3
};

enum MSKdinfitem_enum {
  MSK_DINF_BEGIN                                          = 0,
  MSK_DINF_END                                            = 90,

  MSK_DINF_BI_CLEAN_DUAL_TIME                             = 0,
  MSK_DINF_BI_CLEAN_PRIMAL_TIME                           = 1,
  MSK_DINF_BI_CLEAN_TIME                                  = 2,
  MSK_DINF_BI_DUAL_TIME                                   = 3,
  MSK_DINF_BI_PRIMAL_TIME                                 = 4,
  MSK_DINF_BI_TIME                                        = 5,
  MSK_DINF_INTPNT_DUAL_FEAS                               = 6,
  MSK_DINF_INTPNT_DUAL_OBJ                                = 7,
  MSK_DINF_INTPNT_FACTOR_NUM_FLOPS                        = 8,
  MSK_DINF_INTPNT_OPT_STATUS                              = 9,
  MSK_DINF_INTPNT_ORDER_TIME                              = 10,
  MSK_DINF_INTPNT_PRIMAL_FEAS                             = 11,
  MSK_DINF_INTPNT_PRIMAL_OBJ                              = 12,
  MSK_DINF_INTPNT_TIME                                    = 13,
  MSK_DINF_MIO_CLIQUE_SEPARATION_TIME                     = 14,
  MSK_DINF_MIO_CMIR_SEPARATION_TIME                       = 15,
  MSK_DINF_MIO_CONSTRUCT_SOLUTION_OBJ                     = 16,
  MSK_DINF_MIO_DUAL_BOUND_AFTER_PRESOLVE                  = 17,
  MSK_DINF_MIO_GMI_SEPARATION_TIME                        = 18,
  MSK_DINF_MIO_IMPLIED_BOUND_TIME                         = 19,
  MSK_DINF_MIO_KNAPSACK_COVER_SEPARATION_TIME             = 20,
  MSK_DINF_MIO_OBJ_ABS_GAP                                = 21,
  MSK_DINF_MIO_OBJ_BOUND                                  = 22,
  MSK_DINF_MIO_OBJ_INT                                    = 23,
  MSK_DINF_MIO_OBJ_REL_GAP                                = 24,
  MSK_DINF_MIO_PROBING_TIME                               = 25,
  MSK_DINF_MIO_ROOT_CUTGEN_TIME                           = 26,
  MSK_DINF_MIO_ROOT_OPTIMIZER_TIME                        = 27,
  MSK_DINF_MIO_ROOT_PRESOLVE_TIME                         = 28,
  MSK_DINF_MIO_TIME                                       = 29,
  MSK_DINF_MIO_USER_OBJ_CUT                               = 30,
  MSK_DINF_OPTIMIZER_TIME                                 = 31,
  MSK_DINF_PRESOLVE_ELI_TIME                              = 32,
  MSK_DINF_PRESOLVE_LINDEP_TIME                           = 33,
  MSK_DINF_PRESOLVE_TIME                                  = 34,
  MSK_DINF_PRIMAL_REPAIR_PENALTY_OBJ                      = 35,
  MSK_DINF_QCQO_REFORMULATE_MAX_PERTURBATION              = 36,
  MSK_DINF_QCQO_REFORMULATE_TIME                          = 37,
  MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_COLUMN_SCALING = 38,
  MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_DIAG_SCALING   = 39,
  MSK_DINF_RD_TIME                                        = 40,
  MSK_DINF_SIM_DUAL_TIME                                  = 41,
  MSK_DINF_SIM_FEAS                                       = 42,
  MSK_DINF_SIM_OBJ                                        = 43,
  MSK_DINF_SIM_PRIMAL_TIME                                = 44,
  MSK_DINF_SIM_TIME                                       = 45,
  MSK_DINF_SOL_BAS_DUAL_OBJ                               = 46,
  MSK_DINF_SOL_BAS_DVIOLCON                               = 47,
  MSK_DINF_SOL_BAS_DVIOLVAR                               = 48,
  MSK_DINF_SOL_BAS_NRM_BARX                               = 49,
  MSK_DINF_SOL_BAS_NRM_SLC                                = 50,
  MSK_DINF_SOL_BAS_NRM_SLX                                = 51,
  MSK_DINF_SOL_BAS_NRM_SUC                                = 52,
  MSK_DINF_SOL_BAS_NRM_SUX                                = 53,
  MSK_DINF_SOL_BAS_NRM_XC                                 = 54,
  MSK_DINF_SOL_BAS_NRM_XX                                 = 55,
  MSK_DINF_SOL_BAS_NRM_Y                                  = 56,
  MSK_DINF_SOL_BAS_PRIMAL_OBJ                             = 57,
  MSK_DINF_SOL_BAS_PVIOLCON                               = 58,
  MSK_DINF_SOL_BAS_PVIOLVAR                               = 59,
  MSK_DINF_SOL_ITG_NRM_BARX                               = 60,
  MSK_DINF_SOL_ITG_NRM_XC                                 = 61,
  MSK_DINF_SOL_ITG_NRM_XX                                 = 62,
  MSK_DINF_SOL_ITG_PRIMAL_OBJ                             = 63,
  MSK_DINF_SOL_ITG_PVIOLBARVAR                            = 64,
  MSK_DINF_SOL_ITG_PVIOLCON                               = 65,
  MSK_DINF_SOL_ITG_PVIOLCONES                             = 66,
  MSK_DINF_SOL_ITG_PVIOLITG                               = 67,
  MSK_DINF_SOL_ITG_PVIOLVAR                               = 68,
  MSK_DINF_SOL_ITR_DUAL_OBJ                               = 69,
  MSK_DINF_SOL_ITR_DVIOLBARVAR                            = 70,
  MSK_DINF_SOL_ITR_DVIOLCON                               = 71,
  MSK_DINF_SOL_ITR_DVIOLCONES                             = 72,
  MSK_DINF_SOL_ITR_DVIOLVAR                               = 73,
  MSK_DINF_SOL_ITR_NRM_BARS                               = 74,
  MSK_DINF_SOL_ITR_NRM_BARX                               = 75,
  MSK_DINF_SOL_ITR_NRM_SLC                                = 76,
  MSK_DINF_SOL_ITR_NRM_SLX                                = 77,
  MSK_DINF_SOL_ITR_NRM_SNX                                = 78,
  MSK_DINF_SOL_ITR_NRM_SUC                                = 79,
  MSK_DINF_SOL_ITR_NRM_SUX                                = 80,
  MSK_DINF_SOL_ITR_NRM_XC                                 = 81,
  MSK_DINF_SOL_ITR_NRM_XX                                 = 82,
  MSK_DINF_SOL_ITR_NRM_Y                                  = 83,
  MSK_DINF_SOL_ITR_PRIMAL_OBJ                             = 84,
  MSK_DINF_SOL_ITR_PVIOLBARVAR                            = 85,
  MSK_DINF_SOL_ITR_PVIOLCON                               = 86,
  MSK_DINF_SOL_ITR_PVIOLCONES                             = 87,
  MSK_DINF_SOL_ITR_PVIOLVAR                               = 88,
  MSK_DINF_TO_CONIC_TIME                                  = 89
};

enum MSKmiomode_enum {
  MSK_MIO_MODE_BEGIN     = 0,
  MSK_MIO_MODE_END       = 2,

  MSK_MIO_MODE_IGNORED   = 0,
  MSK_MIO_MODE_SATISFIED = 1
};

enum MSKmiocontsoltype_enum {
  MSK_MIO_CONT_SOL_BEGIN   = 0,
  MSK_MIO_CONT_SOL_END     = 4,

  MSK_MIO_CONT_SOL_NONE    = 0,
  MSK_MIO_CONT_SOL_ROOT    = 1,
  MSK_MIO_CONT_SOL_ITG     = 2,
  MSK_MIO_CONT_SOL_ITG_REL = 3
};

enum MSKscalingmethod_enum {
  MSK_SCALING_METHOD_BEGIN = 0,
  MSK_SCALING_METHOD_END  = 2,

  MSK_SCALING_METHOD_POW2 = 0,
  MSK_SCALING_METHOD_FREE = 1
};

enum MSKbranchdir_enum {
  MSK_BRANCH_DIR_BEGIN      = 0,
  MSK_BRANCH_DIR_END        = 8,

  MSK_BRANCH_DIR_FREE       = 0,
  MSK_BRANCH_DIR_UP         = 1,
  MSK_BRANCH_DIR_DOWN       = 2,
  MSK_BRANCH_DIR_NEAR       = 3,
  MSK_BRANCH_DIR_FAR        = 4,
  MSK_BRANCH_DIR_ROOT_LP    = 5,
  MSK_BRANCH_DIR_GUIDED     = 6,
  MSK_BRANCH_DIR_PSEUDOCOST = 7
};

enum MSKsolitem_enum {
  MSK_SOL_ITEM_BEGIN = 0,
  MSK_SOL_ITEM_END = 8,

  MSK_SOL_ITEM_XC  = 0,
  MSK_SOL_ITEM_XX  = 1,
  MSK_SOL_ITEM_Y   = 2,
  MSK_SOL_ITEM_SLC = 3,
  MSK_SOL_ITEM_SUC = 4,
  MSK_SOL_ITEM_SLX = 5,
  MSK_SOL_ITEM_SUX = 6,
  MSK_SOL_ITEM_SNX = 7
};

enum MSKcheckconvexitytype_enum {
  MSK_CHECK_CONVEXITY_BEGIN  = 0,
  MSK_CHECK_CONVEXITY_END    = 3,

  MSK_CHECK_CONVEXITY_NONE   = 0,
  MSK_CHECK_CONVEXITY_SIMPLE = 1,
  MSK_CHECK_CONVEXITY_FULL   = 2
};

enum MSKintpnthotstart_enum {
  MSK_INTPNT_HOTSTART_BEGIN       = 0,
  MSK_INTPNT_HOTSTART_END         = 4,

  MSK_INTPNT_HOTSTART_NONE        = 0,
  MSK_INTPNT_HOTSTART_PRIMAL      = 1,
  MSK_INTPNT_HOTSTART_DUAL        = 2,
  MSK_INTPNT_HOTSTART_PRIMAL_DUAL = 3
};

enum MSKmionodeseltype_enum {
  MSK_MIO_NODE_SELECTION_BEGIN  = 0,
  MSK_MIO_NODE_SELECTION_END    = 4,

  MSK_MIO_NODE_SELECTION_FREE   = 0,
  MSK_MIO_NODE_SELECTION_FIRST  = 1,
  MSK_MIO_NODE_SELECTION_BEST   = 2,
  MSK_MIO_NODE_SELECTION_PSEUDO = 3
};

enum MSKscalingtype_enum {
  MSK_SCALING_BEGIN      = 0,
  MSK_SCALING_END        = 4,

  MSK_SCALING_FREE       = 0,
  MSK_SCALING_NONE       = 1,
  MSK_SCALING_MODERATE   = 2,
  MSK_SCALING_AGGRESSIVE = 3
};

enum MSKproblemtype_enum {
  MSK_PROBTYPE_BEGIN = 0,
  MSK_PROBTYPE_END   = 5,

  MSK_PROBTYPE_LO    = 0,
  MSK_PROBTYPE_QO    = 1,
  MSK_PROBTYPE_QCQO  = 2,
  MSK_PROBTYPE_CONIC = 3,
  MSK_PROBTYPE_MIXED = 4
};

enum MSKmark_enum {
  MSK_MARK_BEGIN = 0,
  MSK_MARK_END = 2,

  MSK_MARK_LO = 0,
  MSK_MARK_UP = 1
};

enum MSKstakey_enum {
  MSK_SK_BEGIN  = 0,
  MSK_SK_END    = 7,

  MSK_SK_UNK    = 0,
  MSK_SK_BAS    = 1,
  MSK_SK_SUPBAS = 2,
  MSK_SK_LOW    = 3,
  MSK_SK_UPR    = 4,
  MSK_SK_FIX    = 5,
  MSK_SK_INF    = 6
};

enum MSKcallbackcode_enum {
  MSK_CALLBACK_BEGIN                      = 0,
  MSK_CALLBACK_END                        = 93,

  MSK_CALLBACK_BEGIN_BI                   = 0,
  MSK_CALLBACK_BEGIN_CONIC                = 1,
  MSK_CALLBACK_BEGIN_DUAL_BI              = 2,
  MSK_CALLBACK_BEGIN_DUAL_SENSITIVITY     = 3,
  MSK_CALLBACK_BEGIN_DUAL_SETUP_BI        = 4,
  MSK_CALLBACK_BEGIN_DUAL_SIMPLEX         = 5,
  MSK_CALLBACK_BEGIN_DUAL_SIMPLEX_BI      = 6,
  MSK_CALLBACK_BEGIN_FULL_CONVEXITY_CHECK = 7,
  MSK_CALLBACK_BEGIN_INFEAS_ANA           = 8,
  MSK_CALLBACK_BEGIN_INTPNT               = 9,
  MSK_CALLBACK_BEGIN_LICENSE_WAIT         = 10,
  MSK_CALLBACK_BEGIN_MIO                  = 11,
  MSK_CALLBACK_BEGIN_OPTIMIZER            = 12,
  MSK_CALLBACK_BEGIN_PRESOLVE             = 13,
  MSK_CALLBACK_BEGIN_PRIMAL_BI            = 14,
  MSK_CALLBACK_BEGIN_PRIMAL_REPAIR        = 15,
  MSK_CALLBACK_BEGIN_PRIMAL_SENSITIVITY   = 16,
  MSK_CALLBACK_BEGIN_PRIMAL_SETUP_BI      = 17,
  MSK_CALLBACK_BEGIN_PRIMAL_SIMPLEX       = 18,
  MSK_CALLBACK_BEGIN_PRIMAL_SIMPLEX_BI    = 19,
  MSK_CALLBACK_BEGIN_QCQO_REFORMULATE     = 20,
  MSK_CALLBACK_BEGIN_READ                 = 21,
  MSK_CALLBACK_BEGIN_ROOT_CUTGEN          = 22,
  MSK_CALLBACK_BEGIN_SIMPLEX              = 23,
  MSK_CALLBACK_BEGIN_SIMPLEX_BI           = 24,
  MSK_CALLBACK_BEGIN_TO_CONIC             = 25,
  MSK_CALLBACK_BEGIN_WRITE                = 26,
  MSK_CALLBACK_CONIC                      = 27,
  MSK_CALLBACK_DUAL_SIMPLEX               = 28,
  MSK_CALLBACK_END_BI                     = 29,
  MSK_CALLBACK_END_CONIC                  = 30,
  MSK_CALLBACK_END_DUAL_BI                = 31,
  MSK_CALLBACK_END_DUAL_SENSITIVITY       = 32,
  MSK_CALLBACK_END_DUAL_SETUP_BI          = 33,
  MSK_CALLBACK_END_DUAL_SIMPLEX           = 34,
  MSK_CALLBACK_END_DUAL_SIMPLEX_BI        = 35,
  MSK_CALLBACK_END_FULL_CONVEXITY_CHECK   = 36,
  MSK_CALLBACK_END_INFEAS_ANA             = 37,
  MSK_CALLBACK_END_INTPNT                 = 38,
  MSK_CALLBACK_END_LICENSE_WAIT           = 39,
  MSK_CALLBACK_END_MIO                    = 40,
  MSK_CALLBACK_END_OPTIMIZER              = 41,
  MSK_CALLBACK_END_PRESOLVE               = 42,
  MSK_CALLBACK_END_PRIMAL_BI              = 43,
  MSK_CALLBACK_END_PRIMAL_REPAIR          = 44,
  MSK_CALLBACK_END_PRIMAL_SENSITIVITY     = 45,
  MSK_CALLBACK_END_PRIMAL_SETUP_BI        = 46,
  MSK_CALLBACK_END_PRIMAL_SIMPLEX         = 47,
  MSK_CALLBACK_END_PRIMAL_SIMPLEX_BI      = 48,
  MSK_CALLBACK_END_QCQO_REFORMULATE       = 49,
  MSK_CALLBACK_END_READ                   = 50,
  MSK_CALLBACK_END_ROOT_CUTGEN            = 51,
  MSK_CALLBACK_END_SIMPLEX                = 52,
  MSK_CALLBACK_END_SIMPLEX_BI             = 53,
  MSK_CALLBACK_END_TO_CONIC               = 54,
  MSK_CALLBACK_END_WRITE                  = 55,
  MSK_CALLBACK_IM_BI                      = 56,
  MSK_CALLBACK_IM_CONIC                   = 57,
  MSK_CALLBACK_IM_DUAL_BI                 = 58,
  MSK_CALLBACK_IM_DUAL_SENSIVITY          = 59,
  MSK_CALLBACK_IM_DUAL_SIMPLEX            = 60,
  MSK_CALLBACK_IM_FULL_CONVEXITY_CHECK    = 61,
  MSK_CALLBACK_IM_INTPNT                  = 62,
  MSK_CALLBACK_IM_LICENSE_WAIT            = 63,
  MSK_CALLBACK_IM_LU                      = 64,
  MSK_CALLBACK_IM_MIO                     = 65,
  MSK_CALLBACK_IM_MIO_DUAL_SIMPLEX        = 66,
  MSK_CALLBACK_IM_MIO_INTPNT              = 67,
  MSK_CALLBACK_IM_MIO_PRIMAL_SIMPLEX      = 68,
  MSK_CALLBACK_IM_ORDER                   = 69,
  MSK_CALLBACK_IM_PRESOLVE                = 70,
  MSK_CALLBACK_IM_PRIMAL_BI               = 71,
  MSK_CALLBACK_IM_PRIMAL_SENSIVITY        = 72,
  MSK_CALLBACK_IM_PRIMAL_SIMPLEX          = 73,
  MSK_CALLBACK_IM_QO_REFORMULATE          = 74,
  MSK_CALLBACK_IM_READ                    = 75,
  MSK_CALLBACK_IM_ROOT_CUTGEN             = 76,
  MSK_CALLBACK_IM_SIMPLEX                 = 77,
  MSK_CALLBACK_IM_SIMPLEX_BI              = 78,
  MSK_CALLBACK_INTPNT                     = 79,
  MSK_CALLBACK_NEW_INT_MIO                = 80,
  MSK_CALLBACK_PRIMAL_SIMPLEX             = 81,
  MSK_CALLBACK_READ_OPF                   = 82,
  MSK_CALLBACK_READ_OPF_SECTION           = 83,
  MSK_CALLBACK_SOLVING_REMOTE             = 84,
  MSK_CALLBACK_UPDATE_DUAL_BI             = 85,
  MSK_CALLBACK_UPDATE_DUAL_SIMPLEX        = 86,
  MSK_CALLBACK_UPDATE_DUAL_SIMPLEX_BI     = 87,
  MSK_CALLBACK_UPDATE_PRESOLVE            = 88,
  MSK_CALLBACK_UPDATE_PRIMAL_BI           = 89,
  MSK_CALLBACK_UPDATE_PRIMAL_SIMPLEX      = 90,
  MSK_CALLBACK_UPDATE_PRIMAL_SIMPLEX_BI   = 91,
  MSK_CALLBACK_WRITE_OPF                  = 92
};

enum MSKsoltype_enum {
  MSK_SOL_BEGIN = 0,
  MSK_SOL_END = 3,

  MSK_SOL_ITR = 0,
  MSK_SOL_BAS = 1,
  MSK_SOL_ITG = 2
};

enum MSKfeature_enum {
  MSK_FEATURE_BEGIN = 0,
  MSK_FEATURE_END  = 2,

  MSK_FEATURE_PTS  = 0,
  MSK_FEATURE_PTON = 1
};

enum MSKuplo_enum {
  MSK_UPLO_BEGIN = 0,
  MSK_UPLO_END = 2,

  MSK_UPLO_LO = 0,
  MSK_UPLO_UP = 1
};

enum MSKvalue_enum {
  MSK_LICENSE_BUFFER_LENGTH = 21,
  MSK_MAX_STR_LEN           = 1024
};

enum MSKscopr_enum {
  MSK_OPR_BEGIN = 0,
  MSK_OPR_END  = 5,

  MSK_OPR_ENT  = 0,
  MSK_OPR_EXP  = 1,
  MSK_OPR_LOG  = 2,
  MSK_OPR_POW  = 3,
  MSK_OPR_SQRT = 4
};

enum MSKproblemitem_enum {
  MSK_PI_BEGIN = 0,
  MSK_PI_END  = 3,

  MSK_PI_VAR  = 0,
  MSK_PI_CON  = 1,
  MSK_PI_CONE = 2
};

enum MSKtranspose_enum {
  MSK_TRANSPOSE_BEGIN = 0,
  MSK_TRANSPOSE_END = 2,

  MSK_TRANSPOSE_NO  = 0,
  MSK_TRANSPOSE_YES = 1
};

enum MSKsimseltype_enum {
  MSK_SIM_SELECTION_BEGIN   = 0,
  MSK_SIM_SELECTION_END     = 6,

  MSK_SIM_SELECTION_FREE    = 0,
  MSK_SIM_SELECTION_FULL    = 1,
  MSK_SIM_SELECTION_ASE     = 2,
  MSK_SIM_SELECTION_DEVEX   = 3,
  MSK_SIM_SELECTION_SE      = 4,
  MSK_SIM_SELECTION_PARTIAL = 5
};

enum MSKxmlwriteroutputtype_enum {
  MSK_WRITE_XML_MODE_BEGIN = 0,
  MSK_WRITE_XML_MODE_END = 2,

  MSK_WRITE_XML_MODE_ROW = 0,
  MSK_WRITE_XML_MODE_COL = 1
};

enum MSKobjsense_enum {
  MSK_OBJECTIVE_SENSE_BEGIN    = 0,
  MSK_OBJECTIVE_SENSE_END      = 2,

  MSK_OBJECTIVE_SENSE_MINIMIZE = 0,
  MSK_OBJECTIVE_SENSE_MAXIMIZE = 1
};

enum MSKrescodetype_enum {
  MSK_RESPONSE_BEGIN = 0,
  MSK_RESPONSE_END = 5,

  MSK_RESPONSE_OK  = 0,
  MSK_RESPONSE_WRN = 1,
  MSK_RESPONSE_TRM = 2,
  MSK_RESPONSE_ERR = 3,
  MSK_RESPONSE_UNK = 4
};

enum MSKsolsta_enum {
  MSK_SOL_STA_BEGIN              = 0,
  MSK_SOL_STA_END                = 10,

  MSK_SOL_STA_UNKNOWN            = 0,
  MSK_SOL_STA_OPTIMAL            = 1,
  MSK_SOL_STA_PRIM_FEAS          = 2,
  MSK_SOL_STA_DUAL_FEAS          = 3,
  MSK_SOL_STA_PRIM_AND_DUAL_FEAS = 4,
  MSK_SOL_STA_PRIM_INFEAS_CER    = 5,
  MSK_SOL_STA_DUAL_INFEAS_CER    = 6,
  MSK_SOL_STA_PRIM_ILLPOSED_CER  = 7,
  MSK_SOL_STA_DUAL_ILLPOSED_CER  = 8,
  MSK_SOL_STA_INTEGER_OPTIMAL    = 9
};

enum MSKconetype_enum {
  MSK_CT_BEGIN = 0,
  MSK_CT_END   = 7,

  MSK_CT_QUAD  = 0,
  MSK_CT_RQUAD = 1,
  MSK_CT_PEXP  = 2,
  MSK_CT_DEXP  = 3,
  MSK_CT_PPOW  = 4,
  MSK_CT_DPOW  = 5,
  MSK_CT_ZERO  = 6
};

enum MSKonoffkey_enum {
  MSK_BEGIN = 0,
  MSK_END = 2,

  MSK_OFF = 0,
  MSK_ON  = 1
};

enum MSKsparam_enum {
  MSK_SPAR_BEGIN                     = 0,
  MSK_SPAR_END                       = 24,

  MSK_SPAR_BAS_SOL_FILE_NAME         = 0,
  MSK_SPAR_DATA_FILE_NAME            = 1,
  MSK_SPAR_DEBUG_FILE_NAME           = 2,
  MSK_SPAR_INT_SOL_FILE_NAME         = 3,
  MSK_SPAR_ITR_SOL_FILE_NAME         = 4,
  MSK_SPAR_MIO_DEBUG_STRING          = 5,
  MSK_SPAR_PARAM_COMMENT_SIGN        = 6,
  MSK_SPAR_PARAM_READ_FILE_NAME      = 7,
  MSK_SPAR_PARAM_WRITE_FILE_NAME     = 8,
  MSK_SPAR_READ_MPS_BOU_NAME         = 9,
  MSK_SPAR_READ_MPS_OBJ_NAME         = 10,
  MSK_SPAR_READ_MPS_RAN_NAME         = 11,
  MSK_SPAR_READ_MPS_RHS_NAME         = 12,
  MSK_SPAR_REMOTE_ACCESS_TOKEN       = 13,
  MSK_SPAR_SENSITIVITY_FILE_NAME     = 14,
  MSK_SPAR_SENSITIVITY_RES_FILE_NAME = 15,
  MSK_SPAR_SOL_FILTER_XC_LOW         = 16,
  MSK_SPAR_SOL_FILTER_XC_UPR         = 17,
  MSK_SPAR_SOL_FILTER_XX_LOW         = 18,
  MSK_SPAR_SOL_FILTER_XX_UPR         = 19,
  MSK_SPAR_STAT_FILE_NAME            = 20,
  MSK_SPAR_STAT_KEY                  = 21,
  MSK_SPAR_STAT_NAME                 = 22,
  MSK_SPAR_WRITE_LP_GEN_VAR_NAME     = 23
};

enum MSKcompresstype_enum {
  MSK_COMPRESS_BEGIN = 0,
  MSK_COMPRESS_END  = 4,

  MSK_COMPRESS_NONE = 0,
  MSK_COMPRESS_FREE = 1,
  MSK_COMPRESS_GZIP = 2,
  MSK_COMPRESS_ZSTD = 3
};

enum MSKstartpointtype_enum {
  MSK_STARTING_POINT_BEGIN          = 0,
  MSK_STARTING_POINT_END            = 4,

  MSK_STARTING_POINT_FREE           = 0,
  MSK_STARTING_POINT_GUESS          = 1,
  MSK_STARTING_POINT_CONSTANT       = 2,
  MSK_STARTING_POINT_SATISFY_BOUNDS = 3
};

enum MSKsimdupvec_enum {
  MSK_SIM_EXPLOIT_DUPVEC_BEGIN = 0,
  MSK_SIM_EXPLOIT_DUPVEC_END  = 3,

  MSK_SIM_EXPLOIT_DUPVEC_OFF  = 0,
  MSK_SIM_EXPLOIT_DUPVEC_ON   = 1,
  MSK_SIM_EXPLOIT_DUPVEC_FREE = 2
};

enum MSKsimdegen_enum {
  MSK_SIM_DEGEN_BEGIN      = 0,
  MSK_SIM_DEGEN_END        = 5,

  MSK_SIM_DEGEN_NONE       = 0,
  MSK_SIM_DEGEN_FREE       = 1,
  MSK_SIM_DEGEN_AGGRESSIVE = 2,
  MSK_SIM_DEGEN_MODERATE   = 3,
  MSK_SIM_DEGEN_MINIMUM    = 4
};

enum MSKmpsformat_enum {
  MSK_MPS_FORMAT_BEGIN   = 0,
  MSK_MPS_FORMAT_END     = 4,

  MSK_MPS_FORMAT_STRICT  = 0,
  MSK_MPS_FORMAT_RELAXED = 1,
  MSK_MPS_FORMAT_FREE    = 2,
  MSK_MPS_FORMAT_CPLEX   = 3
};

enum MSKdparam_enum {
  MSK_DPAR_BEGIN                              = 0,
  MSK_DPAR_END                                = 59,

  MSK_DPAR_ANA_SOL_INFEAS_TOL                 = 0,
  MSK_DPAR_BASIS_REL_TOL_S                    = 1,
  MSK_DPAR_BASIS_TOL_S                        = 2,
  MSK_DPAR_BASIS_TOL_X                        = 3,
  MSK_DPAR_CHECK_CONVEXITY_REL_TOL            = 4,
  MSK_DPAR_DATA_SYM_MAT_TOL                   = 5,
  MSK_DPAR_DATA_SYM_MAT_TOL_HUGE              = 6,
  MSK_DPAR_DATA_SYM_MAT_TOL_LARGE             = 7,
  MSK_DPAR_DATA_TOL_AIJ_HUGE                  = 8,
  MSK_DPAR_DATA_TOL_AIJ_LARGE                 = 9,
  MSK_DPAR_DATA_TOL_BOUND_INF                 = 10,
  MSK_DPAR_DATA_TOL_BOUND_WRN                 = 11,
  MSK_DPAR_DATA_TOL_C_HUGE                    = 12,
  MSK_DPAR_DATA_TOL_CJ_LARGE                  = 13,
  MSK_DPAR_DATA_TOL_QIJ                       = 14,
  MSK_DPAR_DATA_TOL_X                         = 15,
  MSK_DPAR_INTPNT_CO_TOL_DFEAS                = 16,
  MSK_DPAR_INTPNT_CO_TOL_INFEAS               = 17,
  MSK_DPAR_INTPNT_CO_TOL_MU_RED               = 18,
  MSK_DPAR_INTPNT_CO_TOL_NEAR_REL             = 19,
  MSK_DPAR_INTPNT_CO_TOL_PFEAS                = 20,
  MSK_DPAR_INTPNT_CO_TOL_REL_GAP              = 21,
  MSK_DPAR_INTPNT_QO_TOL_DFEAS                = 22,
  MSK_DPAR_INTPNT_QO_TOL_INFEAS               = 23,
  MSK_DPAR_INTPNT_QO_TOL_MU_RED               = 24,
  MSK_DPAR_INTPNT_QO_TOL_NEAR_REL             = 25,
  MSK_DPAR_INTPNT_QO_TOL_PFEAS                = 26,
  MSK_DPAR_INTPNT_QO_TOL_REL_GAP              = 27,
  MSK_DPAR_INTPNT_TOL_DFEAS                   = 28,
  MSK_DPAR_INTPNT_TOL_DSAFE                   = 29,
  MSK_DPAR_INTPNT_TOL_INFEAS                  = 30,
  MSK_DPAR_INTPNT_TOL_MU_RED                  = 31,
  MSK_DPAR_INTPNT_TOL_PATH                    = 32,
  MSK_DPAR_INTPNT_TOL_PFEAS                   = 33,
  MSK_DPAR_INTPNT_TOL_PSAFE                   = 34,
  MSK_DPAR_INTPNT_TOL_REL_GAP                 = 35,
  MSK_DPAR_INTPNT_TOL_REL_STEP                = 36,
  MSK_DPAR_INTPNT_TOL_STEP_SIZE               = 37,
  MSK_DPAR_LOWER_OBJ_CUT                      = 38,
  MSK_DPAR_LOWER_OBJ_CUT_FINITE_TRH           = 39,
  MSK_DPAR_MIO_MAX_TIME                       = 40,
  MSK_DPAR_MIO_REL_GAP_CONST                  = 41,
  MSK_DPAR_MIO_TOL_ABS_GAP                    = 42,
  MSK_DPAR_MIO_TOL_ABS_RELAX_INT              = 43,
  MSK_DPAR_MIO_TOL_FEAS                       = 44,
  MSK_DPAR_MIO_TOL_REL_DUAL_BOUND_IMPROVEMENT = 45,
  MSK_DPAR_MIO_TOL_REL_GAP                    = 46,
  MSK_DPAR_OPTIMIZER_MAX_TIME                 = 47,
  MSK_DPAR_PRESOLVE_TOL_ABS_LINDEP            = 48,
  MSK_DPAR_PRESOLVE_TOL_AIJ                   = 49,
  MSK_DPAR_PRESOLVE_TOL_REL_LINDEP            = 50,
  MSK_DPAR_PRESOLVE_TOL_S                     = 51,
  MSK_DPAR_PRESOLVE_TOL_X                     = 52,
  MSK_DPAR_QCQO_REFORMULATE_REL_DROP_TOL      = 53,
  MSK_DPAR_SEMIDEFINITE_TOL_APPROX            = 54,
  MSK_DPAR_SIM_LU_TOL_REL_PIV                 = 55,
  MSK_DPAR_SIMPLEX_ABS_TOL_PIV                = 56,
  MSK_DPAR_UPPER_OBJ_CUT                      = 57,
  MSK_DPAR_UPPER_OBJ_CUT_FINITE_TRH           = 58
};

enum MSKiparam_enum {
  MSK_IPAR_BEGIN                              = 0,
  MSK_IPAR_END                                = 180,

  MSK_IPAR_ANA_SOL_BASIS                      = 0,
  MSK_IPAR_ANA_SOL_PRINT_VIOLATED             = 1,
  MSK_IPAR_AUTO_SORT_A_BEFORE_OPT             = 2,
  MSK_IPAR_AUTO_UPDATE_SOL_INFO               = 3,
  MSK_IPAR_BASIS_SOLVE_USE_PLUS_ONE           = 4,
  MSK_IPAR_BI_CLEAN_OPTIMIZER                 = 5,
  MSK_IPAR_BI_IGNORE_MAX_ITER                 = 6,
  MSK_IPAR_BI_IGNORE_NUM_ERROR                = 7,
  MSK_IPAR_BI_MAX_ITERATIONS                  = 8,
  MSK_IPAR_CACHE_LICENSE                      = 9,
  MSK_IPAR_CHECK_CONVEXITY                    = 10,
  MSK_IPAR_COMPRESS_STATFILE                  = 11,
  MSK_IPAR_INFEAS_GENERIC_NAMES               = 12,
  MSK_IPAR_INFEAS_PREFER_PRIMAL               = 13,
  MSK_IPAR_INFEAS_REPORT_AUTO                 = 14,
  MSK_IPAR_INFEAS_REPORT_LEVEL                = 15,
  MSK_IPAR_INTPNT_BASIS                       = 16,
  MSK_IPAR_INTPNT_DIFF_STEP                   = 17,
  MSK_IPAR_INTPNT_HOTSTART                    = 18,
  MSK_IPAR_INTPNT_MAX_ITERATIONS              = 19,
  MSK_IPAR_INTPNT_MAX_NUM_COR                 = 20,
  MSK_IPAR_INTPNT_MAX_NUM_REFINEMENT_STEPS    = 21,
  MSK_IPAR_INTPNT_MULTI_THREAD                = 22,
  MSK_IPAR_INTPNT_OFF_COL_TRH                 = 23,
  MSK_IPAR_INTPNT_ORDER_GP_NUM_SEEDS          = 24,
  MSK_IPAR_INTPNT_ORDER_METHOD                = 25,
  MSK_IPAR_INTPNT_PURIFY                      = 26,
  MSK_IPAR_INTPNT_REGULARIZATION_USE          = 27,
  MSK_IPAR_INTPNT_SCALING                     = 28,
  MSK_IPAR_INTPNT_SOLVE_FORM                  = 29,
  MSK_IPAR_INTPNT_STARTING_POINT              = 30,
  MSK_IPAR_LICENSE_DEBUG                      = 31,
  MSK_IPAR_LICENSE_PAUSE_TIME                 = 32,
  MSK_IPAR_LICENSE_SUPPRESS_EXPIRE_WRNS       = 33,
  MSK_IPAR_LICENSE_TRH_EXPIRY_WRN             = 34,
  MSK_IPAR_LICENSE_WAIT                       = 35,
  MSK_IPAR_LOG                                = 36,
  MSK_IPAR_LOG_ANA_PRO                        = 37,
  MSK_IPAR_LOG_BI                             = 38,
  MSK_IPAR_LOG_BI_FREQ                        = 39,
  MSK_IPAR_LOG_CHECK_CONVEXITY                = 40,
  MSK_IPAR_LOG_CUT_SECOND_OPT                 = 41,
  MSK_IPAR_LOG_EXPAND                         = 42,
  MSK_IPAR_LOG_FEAS_REPAIR                    = 43,
  MSK_IPAR_LOG_FILE                           = 44,
  MSK_IPAR_LOG_INCLUDE_SUMMARY                = 45,
  MSK_IPAR_LOG_INFEAS_ANA                     = 46,
  MSK_IPAR_LOG_INTPNT                         = 47,
  MSK_IPAR_LOG_LOCAL_INFO                     = 48,
  MSK_IPAR_LOG_MIO                            = 49,
  MSK_IPAR_LOG_MIO_FREQ                       = 50,
  MSK_IPAR_LOG_ORDER                          = 51,
  MSK_IPAR_LOG_PRESOLVE                       = 52,
  MSK_IPAR_LOG_RESPONSE                       = 53,
  MSK_IPAR_LOG_SENSITIVITY                    = 54,
  MSK_IPAR_LOG_SENSITIVITY_OPT                = 55,
  MSK_IPAR_LOG_SIM                            = 56,
  MSK_IPAR_LOG_SIM_FREQ                       = 57,
  MSK_IPAR_LOG_SIM_MINOR                      = 58,
  MSK_IPAR_LOG_STORAGE                        = 59,
  MSK_IPAR_MAX_NUM_WARNINGS                   = 60,
  MSK_IPAR_MIO_BRANCH_DIR                     = 61,
  MSK_IPAR_MIO_CONIC_OUTER_APPROXIMATION      = 62,
  MSK_IPAR_MIO_CUT_CLIQUE                     = 63,
  MSK_IPAR_MIO_CUT_CMIR                       = 64,
  MSK_IPAR_MIO_CUT_GMI                        = 65,
  MSK_IPAR_MIO_CUT_IMPLIED_BOUND              = 66,
  MSK_IPAR_MIO_CUT_KNAPSACK_COVER             = 67,
  MSK_IPAR_MIO_CUT_SELECTION_LEVEL            = 68,
  MSK_IPAR_MIO_FEASPUMP_LEVEL                 = 69,
  MSK_IPAR_MIO_HEURISTIC_LEVEL                = 70,
  MSK_IPAR_MIO_MAX_NUM_BRANCHES               = 71,
  MSK_IPAR_MIO_MAX_NUM_RELAXS                 = 72,
  MSK_IPAR_MIO_MAX_NUM_ROOT_CUT_ROUNDS        = 73,
  MSK_IPAR_MIO_MAX_NUM_SOLUTIONS              = 74,
  MSK_IPAR_MIO_MODE                           = 75,
  MSK_IPAR_MIO_NODE_OPTIMIZER                 = 76,
  MSK_IPAR_MIO_NODE_SELECTION                 = 77,
  MSK_IPAR_MIO_PERSPECTIVE_REFORMULATE        = 78,
  MSK_IPAR_MIO_PROBING_LEVEL                  = 79,
  MSK_IPAR_MIO_PROPAGATE_OBJECTIVE_CONSTRAINT = 80,
  MSK_IPAR_MIO_RINS_MAX_NODES                 = 81,
  MSK_IPAR_MIO_ROOT_OPTIMIZER                 = 82,
  MSK_IPAR_MIO_ROOT_REPEAT_PRESOLVE_LEVEL     = 83,
  MSK_IPAR_MIO_SEED                           = 84,
  MSK_IPAR_MIO_VB_DETECTION_LEVEL             = 85,
  MSK_IPAR_MT_SPINCOUNT                       = 86,
  MSK_IPAR_NUM_THREADS                        = 87,
  MSK_IPAR_OPF_WRITE_HEADER                   = 88,
  MSK_IPAR_OPF_WRITE_HINTS                    = 89,
  MSK_IPAR_OPF_WRITE_LINE_LENGTH              = 90,
  MSK_IPAR_OPF_WRITE_PARAMETERS               = 91,
  MSK_IPAR_OPF_WRITE_PROBLEM                  = 92,
  MSK_IPAR_OPF_WRITE_SOL_BAS                  = 93,
  MSK_IPAR_OPF_WRITE_SOL_ITG                  = 94,
  MSK_IPAR_OPF_WRITE_SOL_ITR                  = 95,
  MSK_IPAR_OPF_WRITE_SOLUTIONS                = 96,
  MSK_IPAR_OPTIMIZER                          = 97,
  MSK_IPAR_PARAM_READ_CASE_NAME               = 98,
  MSK_IPAR_PARAM_READ_IGN_ERROR               = 99,
  MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_FILL       = 100,
  MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_NUM_TRIES  = 101,
  MSK_IPAR_PRESOLVE_LEVEL                     = 102,
  MSK_IPAR_PRESOLVE_LINDEP_ABS_WORK_TRH       = 103,
  MSK_IPAR_PRESOLVE_LINDEP_REL_WORK_TRH       = 104,
  MSK_IPAR_PRESOLVE_LINDEP_USE                = 105,
  MSK_IPAR_PRESOLVE_MAX_NUM_PASS              = 106,
  MSK_IPAR_PRESOLVE_MAX_NUM_REDUCTIONS        = 107,
  MSK_IPAR_PRESOLVE_USE                       = 108,
  MSK_IPAR_PRIMAL_REPAIR_OPTIMIZER            = 109,
  MSK_IPAR_PTF_WRITE_TRANSFORM                = 110,
  MSK_IPAR_READ_DEBUG                         = 111,
  MSK_IPAR_READ_KEEP_FREE_CON                 = 112,
  MSK_IPAR_READ_LP_DROP_NEW_VARS_IN_BOU       = 113,
  MSK_IPAR_READ_LP_QUOTED_NAMES               = 114,
  MSK_IPAR_READ_MPS_FORMAT                    = 115,
  MSK_IPAR_READ_MPS_WIDTH                     = 116,
  MSK_IPAR_READ_TASK_IGNORE_PARAM             = 117,
  MSK_IPAR_REMOVE_UNUSED_SOLUTIONS            = 118,
  MSK_IPAR_SENSITIVITY_ALL                    = 119,
  MSK_IPAR_SENSITIVITY_OPTIMIZER              = 120,
  MSK_IPAR_SENSITIVITY_TYPE                   = 121,
  MSK_IPAR_SIM_BASIS_FACTOR_USE               = 122,
  MSK_IPAR_SIM_DEGEN                          = 123,
  MSK_IPAR_SIM_DUAL_CRASH                     = 124,
  MSK_IPAR_SIM_DUAL_PHASEONE_METHOD           = 125,
  MSK_IPAR_SIM_DUAL_RESTRICT_SELECTION        = 126,
  MSK_IPAR_SIM_DUAL_SELECTION                 = 127,
  MSK_IPAR_SIM_EXPLOIT_DUPVEC                 = 128,
  MSK_IPAR_SIM_HOTSTART                       = 129,
  MSK_IPAR_SIM_HOTSTART_LU                    = 130,
  MSK_IPAR_SIM_MAX_ITERATIONS                 = 131,
  MSK_IPAR_SIM_MAX_NUM_SETBACKS               = 132,
  MSK_IPAR_SIM_NON_SINGULAR                   = 133,
  MSK_IPAR_SIM_PRIMAL_CRASH                   = 134,
  MSK_IPAR_SIM_PRIMAL_PHASEONE_METHOD         = 135,
  MSK_IPAR_SIM_PRIMAL_RESTRICT_SELECTION      = 136,
  MSK_IPAR_SIM_PRIMAL_SELECTION               = 137,
  MSK_IPAR_SIM_REFACTOR_FREQ                  = 138,
  MSK_IPAR_SIM_REFORMULATION                  = 139,
  MSK_IPAR_SIM_SAVE_LU                        = 140,
  MSK_IPAR_SIM_SCALING                        = 141,
  MSK_IPAR_SIM_SCALING_METHOD                 = 142,
  MSK_IPAR_SIM_SEED                           = 143,
  MSK_IPAR_SIM_SOLVE_FORM                     = 144,
  MSK_IPAR_SIM_STABILITY_PRIORITY             = 145,
  MSK_IPAR_SIM_SWITCH_OPTIMIZER               = 146,
  MSK_IPAR_SOL_FILTER_KEEP_BASIC              = 147,
  MSK_IPAR_SOL_FILTER_KEEP_RANGED             = 148,
  MSK_IPAR_SOL_READ_NAME_WIDTH                = 149,
  MSK_IPAR_SOL_READ_WIDTH                     = 150,
  MSK_IPAR_SOLUTION_CALLBACK                  = 151,
  MSK_IPAR_TIMING_LEVEL                       = 152,
  MSK_IPAR_WRITE_BAS_CONSTRAINTS              = 153,
  MSK_IPAR_WRITE_BAS_HEAD                     = 154,
  MSK_IPAR_WRITE_BAS_VARIABLES                = 155,
  MSK_IPAR_WRITE_COMPRESSION                  = 156,
  MSK_IPAR_WRITE_DATA_PARAM                   = 157,
  MSK_IPAR_WRITE_FREE_CON                     = 158,
  MSK_IPAR_WRITE_GENERIC_NAMES                = 159,
  MSK_IPAR_WRITE_GENERIC_NAMES_IO             = 160,
  MSK_IPAR_WRITE_IGNORE_INCOMPATIBLE_ITEMS    = 161,
  MSK_IPAR_WRITE_INT_CONSTRAINTS              = 162,
  MSK_IPAR_WRITE_INT_HEAD                     = 163,
  MSK_IPAR_WRITE_INT_VARIABLES                = 164,
  MSK_IPAR_WRITE_LP_FULL_OBJ                  = 165,
  MSK_IPAR_WRITE_LP_LINE_WIDTH                = 166,
  MSK_IPAR_WRITE_LP_QUOTED_NAMES              = 167,
  MSK_IPAR_WRITE_LP_STRICT_FORMAT             = 168,
  MSK_IPAR_WRITE_LP_TERMS_PER_LINE            = 169,
  MSK_IPAR_WRITE_MPS_FORMAT                   = 170,
  MSK_IPAR_WRITE_MPS_INT                      = 171,
  MSK_IPAR_WRITE_PRECISION                    = 172,
  MSK_IPAR_WRITE_SOL_BARVARIABLES             = 173,
  MSK_IPAR_WRITE_SOL_CONSTRAINTS              = 174,
  MSK_IPAR_WRITE_SOL_HEAD                     = 175,
  MSK_IPAR_WRITE_SOL_IGNORE_INVALID_NAMES     = 176,
  MSK_IPAR_WRITE_SOL_VARIABLES                = 177,
  MSK_IPAR_WRITE_TASK_INC_SOL                 = 178,
  MSK_IPAR_WRITE_XML_MODE                     = 179
};

enum MSKoptimizertype_enum {
  MSK_OPTIMIZER_BEGIN          = 0,
  MSK_OPTIMIZER_END            = 7,

  MSK_OPTIMIZER_CONIC          = 0,
  MSK_OPTIMIZER_DUAL_SIMPLEX   = 1,
  MSK_OPTIMIZER_FREE           = 2,
  MSK_OPTIMIZER_FREE_SIMPLEX   = 3,
  MSK_OPTIMIZER_INTPNT         = 4,
  MSK_OPTIMIZER_MIXED_INT      = 5,
  MSK_OPTIMIZER_PRIMAL_SIMPLEX = 6
};

enum MSKrescode_enum {
  MSK_RES_OK                                             = 0,
  MSK_RES_WRN_OPEN_PARAM_FILE                            = 50,
  MSK_RES_WRN_LARGE_BOUND                                = 51,
  MSK_RES_WRN_LARGE_LO_BOUND                             = 52,
  MSK_RES_WRN_LARGE_UP_BOUND                             = 53,
  MSK_RES_WRN_LARGE_CON_FX                               = 54,
  MSK_RES_WRN_LARGE_CJ                                   = 57,
  MSK_RES_WRN_LARGE_AIJ                                  = 62,
  MSK_RES_WRN_ZERO_AIJ                                   = 63,
  MSK_RES_WRN_NAME_MAX_LEN                               = 65,
  MSK_RES_WRN_SPAR_MAX_LEN                               = 66,
  MSK_RES_WRN_MPS_SPLIT_RHS_VECTOR                       = 70,
  MSK_RES_WRN_MPS_SPLIT_RAN_VECTOR                       = 71,
  MSK_RES_WRN_MPS_SPLIT_BOU_VECTOR                       = 72,
  MSK_RES_WRN_LP_OLD_QUAD_FORMAT                         = 80,
  MSK_RES_WRN_LP_DROP_VARIABLE                           = 85,
  MSK_RES_WRN_NZ_IN_UPR_TRI                              = 200,
  MSK_RES_WRN_DROPPED_NZ_QOBJ                            = 201,
  MSK_RES_WRN_IGNORE_INTEGER                             = 250,
  MSK_RES_WRN_NO_GLOBAL_OPTIMIZER                        = 251,
  MSK_RES_WRN_MIO_INFEASIBLE_FINAL                       = 270,
  MSK_RES_WRN_SOL_FILTER                                 = 300,
  MSK_RES_WRN_UNDEF_SOL_FILE_NAME                        = 350,
  MSK_RES_WRN_SOL_FILE_IGNORED_CON                       = 351,
  MSK_RES_WRN_SOL_FILE_IGNORED_VAR                       = 352,
  MSK_RES_WRN_TOO_FEW_BASIS_VARS                         = 400,
  MSK_RES_WRN_TOO_MANY_BASIS_VARS                        = 405,
  MSK_RES_WRN_LICENSE_EXPIRE                             = 500,
  MSK_RES_WRN_LICENSE_SERVER                             = 501,
  MSK_RES_WRN_EMPTY_NAME                                 = 502,
  MSK_RES_WRN_USING_GENERIC_NAMES                        = 503,
  MSK_RES_WRN_LICENSE_FEATURE_EXPIRE                     = 505,
  MSK_RES_WRN_PARAM_NAME_DOU                             = 510,
  MSK_RES_WRN_PARAM_NAME_INT                             = 511,
  MSK_RES_WRN_PARAM_NAME_STR                             = 512,
  MSK_RES_WRN_PARAM_STR_VALUE                            = 515,
  MSK_RES_WRN_PARAM_IGNORED_CMIO                         = 516,
  MSK_RES_WRN_ZEROS_IN_SPARSE_ROW                        = 705,
  MSK_RES_WRN_ZEROS_IN_SPARSE_COL                        = 710,
  MSK_RES_WRN_INCOMPLETE_LINEAR_DEPENDENCY_CHECK         = 800,
  MSK_RES_WRN_ELIMINATOR_SPACE                           = 801,
  MSK_RES_WRN_PRESOLVE_OUTOFSPACE                        = 802,
  MSK_RES_WRN_WRITE_CHANGED_NAMES                        = 803,
  MSK_RES_WRN_WRITE_DISCARDED_CFIX                       = 804,
  MSK_RES_WRN_DUPLICATE_CONSTRAINT_NAMES                 = 850,
  MSK_RES_WRN_DUPLICATE_VARIABLE_NAMES                   = 851,
  MSK_RES_WRN_DUPLICATE_BARVARIABLE_NAMES                = 852,
  MSK_RES_WRN_DUPLICATE_CONE_NAMES                       = 853,
  MSK_RES_WRN_ANA_LARGE_BOUNDS                           = 900,
  MSK_RES_WRN_ANA_C_ZERO                                 = 901,
  MSK_RES_WRN_ANA_EMPTY_COLS                             = 902,
  MSK_RES_WRN_ANA_CLOSE_BOUNDS                           = 903,
  MSK_RES_WRN_ANA_ALMOST_INT_BOUNDS                      = 904,
  MSK_RES_WRN_QUAD_CONES_WITH_ROOT_FIXED_AT_ZERO         = 930,
  MSK_RES_WRN_RQUAD_CONES_WITH_ROOT_FIXED_AT_ZERO        = 931,
  MSK_RES_WRN_EXP_CONES_WITH_VARIABLES_FIXED_AT_ZERO     = 932,
  MSK_RES_WRN_POW_CONES_WITH_ROOT_FIXED_AT_ZERO          = 933,
  MSK_RES_WRN_NO_DUALIZER                                = 950,
  MSK_RES_WRN_SYM_MAT_LARGE                              = 960,
  MSK_RES_ERR_LICENSE                                    = 1000,
  MSK_RES_ERR_LICENSE_EXPIRED                            = 1001,
  MSK_RES_ERR_LICENSE_VERSION                            = 1002,
  MSK_RES_ERR_SIZE_LICENSE                               = 1005,
  MSK_RES_ERR_PROB_LICENSE                               = 1006,
  MSK_RES_ERR_FILE_LICENSE                               = 1007,
  MSK_RES_ERR_MISSING_LICENSE_FILE                       = 1008,
  MSK_RES_ERR_SIZE_LICENSE_CON                           = 1010,
  MSK_RES_ERR_SIZE_LICENSE_VAR                           = 1011,
  MSK_RES_ERR_SIZE_LICENSE_INTVAR                        = 1012,
  MSK_RES_ERR_OPTIMIZER_LICENSE                          = 1013,
  MSK_RES_ERR_FLEXLM                                     = 1014,
  MSK_RES_ERR_LICENSE_SERVER                             = 1015,
  MSK_RES_ERR_LICENSE_MAX                                = 1016,
  MSK_RES_ERR_LICENSE_MOSEKLM_DAEMON                     = 1017,
  MSK_RES_ERR_LICENSE_FEATURE                            = 1018,
  MSK_RES_ERR_PLATFORM_NOT_LICENSED                      = 1019,
  MSK_RES_ERR_LICENSE_CANNOT_ALLOCATE                    = 1020,
  MSK_RES_ERR_LICENSE_CANNOT_CONNECT                     = 1021,
  MSK_RES_ERR_LICENSE_INVALID_HOSTID                     = 1025,
  MSK_RES_ERR_LICENSE_SERVER_VERSION                     = 1026,
  MSK_RES_ERR_LICENSE_NO_SERVER_SUPPORT                  = 1027,
  MSK_RES_ERR_LICENSE_NO_SERVER_LINE                     = 1028,
  MSK_RES_ERR_OLDER_DLL                                  = 1035,
  MSK_RES_ERR_NEWER_DLL                                  = 1036,
  MSK_RES_ERR_LINK_FILE_DLL                              = 1040,
  MSK_RES_ERR_THREAD_MUTEX_INIT                          = 1045,
  MSK_RES_ERR_THREAD_MUTEX_LOCK                          = 1046,
  MSK_RES_ERR_THREAD_MUTEX_UNLOCK                        = 1047,
  MSK_RES_ERR_THREAD_CREATE                              = 1048,
  MSK_RES_ERR_THREAD_COND_INIT                           = 1049,
  MSK_RES_ERR_UNKNOWN                                    = 1050,
  MSK_RES_ERR_SPACE                                      = 1051,
  MSK_RES_ERR_FILE_OPEN                                  = 1052,
  MSK_RES_ERR_FILE_READ                                  = 1053,
  MSK_RES_ERR_FILE_WRITE                                 = 1054,
  MSK_RES_ERR_DATA_FILE_EXT                              = 1055,
  MSK_RES_ERR_INVALID_FILE_NAME                          = 1056,
  MSK_RES_ERR_INVALID_SOL_FILE_NAME                      = 1057,
  MSK_RES_ERR_END_OF_FILE                                = 1059,
  MSK_RES_ERR_NULL_ENV                                   = 1060,
  MSK_RES_ERR_NULL_TASK                                  = 1061,
  MSK_RES_ERR_INVALID_STREAM                             = 1062,
  MSK_RES_ERR_NO_INIT_ENV                                = 1063,
  MSK_RES_ERR_INVALID_TASK                               = 1064,
  MSK_RES_ERR_NULL_POINTER                               = 1065,
  MSK_RES_ERR_LIVING_TASKS                               = 1066,
  MSK_RES_ERR_BLANK_NAME                                 = 1070,
  MSK_RES_ERR_DUP_NAME                                   = 1071,
  MSK_RES_ERR_FORMAT_STRING                              = 1072,
  MSK_RES_ERR_INVALID_OBJ_NAME                           = 1075,
  MSK_RES_ERR_INVALID_CON_NAME                           = 1076,
  MSK_RES_ERR_INVALID_VAR_NAME                           = 1077,
  MSK_RES_ERR_INVALID_CONE_NAME                          = 1078,
  MSK_RES_ERR_INVALID_BARVAR_NAME                        = 1079,
  MSK_RES_ERR_SPACE_LEAKING                              = 1080,
  MSK_RES_ERR_SPACE_NO_INFO                              = 1081,
  MSK_RES_ERR_READ_FORMAT                                = 1090,
  MSK_RES_ERR_MPS_FILE                                   = 1100,
  MSK_RES_ERR_MPS_INV_FIELD                              = 1101,
  MSK_RES_ERR_MPS_INV_MARKER                             = 1102,
  MSK_RES_ERR_MPS_NULL_CON_NAME                          = 1103,
  MSK_RES_ERR_MPS_NULL_VAR_NAME                          = 1104,
  MSK_RES_ERR_MPS_UNDEF_CON_NAME                         = 1105,
  MSK_RES_ERR_MPS_UNDEF_VAR_NAME                         = 1106,
  MSK_RES_ERR_MPS_INV_CON_KEY                            = 1107,
  MSK_RES_ERR_MPS_INV_BOUND_KEY                          = 1108,
  MSK_RES_ERR_MPS_INV_SEC_NAME                           = 1109,
  MSK_RES_ERR_MPS_NO_OBJECTIVE                           = 1110,
  MSK_RES_ERR_MPS_SPLITTED_VAR                           = 1111,
  MSK_RES_ERR_MPS_MUL_CON_NAME                           = 1112,
  MSK_RES_ERR_MPS_MUL_QSEC                               = 1113,
  MSK_RES_ERR_MPS_MUL_QOBJ                               = 1114,
  MSK_RES_ERR_MPS_INV_SEC_ORDER                          = 1115,
  MSK_RES_ERR_MPS_MUL_CSEC                               = 1116,
  MSK_RES_ERR_MPS_CONE_TYPE                              = 1117,
  MSK_RES_ERR_MPS_CONE_OVERLAP                           = 1118,
  MSK_RES_ERR_MPS_CONE_REPEAT                            = 1119,
  MSK_RES_ERR_MPS_NON_SYMMETRIC_Q                        = 1120,
  MSK_RES_ERR_MPS_DUPLICATE_Q_ELEMENT                    = 1121,
  MSK_RES_ERR_MPS_INVALID_OBJSENSE                       = 1122,
  MSK_RES_ERR_MPS_TAB_IN_FIELD2                          = 1125,
  MSK_RES_ERR_MPS_TAB_IN_FIELD3                          = 1126,
  MSK_RES_ERR_MPS_TAB_IN_FIELD5                          = 1127,
  MSK_RES_ERR_MPS_INVALID_OBJ_NAME                       = 1128,
  MSK_RES_ERR_LP_INCOMPATIBLE                            = 1150,
  MSK_RES_ERR_LP_EMPTY                                   = 1151,
  MSK_RES_ERR_LP_DUP_SLACK_NAME                          = 1152,
  MSK_RES_ERR_WRITE_MPS_INVALID_NAME                     = 1153,
  MSK_RES_ERR_LP_INVALID_VAR_NAME                        = 1154,
  MSK_RES_ERR_LP_FREE_CONSTRAINT                         = 1155,
  MSK_RES_ERR_WRITE_OPF_INVALID_VAR_NAME                 = 1156,
  MSK_RES_ERR_LP_FILE_FORMAT                             = 1157,
  MSK_RES_ERR_WRITE_LP_FORMAT                            = 1158,
  MSK_RES_ERR_READ_LP_MISSING_END_TAG                    = 1159,
  MSK_RES_ERR_LP_FORMAT                                  = 1160,
  MSK_RES_ERR_WRITE_LP_NON_UNIQUE_NAME                   = 1161,
  MSK_RES_ERR_READ_LP_NONEXISTING_NAME                   = 1162,
  MSK_RES_ERR_LP_WRITE_CONIC_PROBLEM                     = 1163,
  MSK_RES_ERR_LP_WRITE_GECO_PROBLEM                      = 1164,
  MSK_RES_ERR_WRITING_FILE                               = 1166,
  MSK_RES_ERR_PTF_FORMAT                                 = 1167,
  MSK_RES_ERR_OPF_FORMAT                                 = 1168,
  MSK_RES_ERR_OPF_NEW_VARIABLE                           = 1169,
  MSK_RES_ERR_INVALID_NAME_IN_SOL_FILE                   = 1170,
  MSK_RES_ERR_LP_INVALID_CON_NAME                        = 1171,
  MSK_RES_ERR_OPF_PREMATURE_EOF                          = 1172,
  MSK_RES_ERR_JSON_SYNTAX                                = 1175,
  MSK_RES_ERR_JSON_STRING                                = 1176,
  MSK_RES_ERR_JSON_NUMBER_OVERFLOW                       = 1177,
  MSK_RES_ERR_JSON_FORMAT                                = 1178,
  MSK_RES_ERR_JSON_DATA                                  = 1179,
  MSK_RES_ERR_JSON_MISSING_DATA                          = 1180,
  MSK_RES_ERR_ARGUMENT_LENNEQ                            = 1197,
  MSK_RES_ERR_ARGUMENT_TYPE                              = 1198,
  MSK_RES_ERR_NUM_ARGUMENTS                              = 1199,
  MSK_RES_ERR_IN_ARGUMENT                                = 1200,
  MSK_RES_ERR_ARGUMENT_DIMENSION                         = 1201,
  MSK_RES_ERR_SHAPE_IS_TOO_LARGE                         = 1202,
  MSK_RES_ERR_INDEX_IS_TOO_SMALL                         = 1203,
  MSK_RES_ERR_INDEX_IS_TOO_LARGE                         = 1204,
  MSK_RES_ERR_PARAM_NAME                                 = 1205,
  MSK_RES_ERR_PARAM_NAME_DOU                             = 1206,
  MSK_RES_ERR_PARAM_NAME_INT                             = 1207,
  MSK_RES_ERR_PARAM_NAME_STR                             = 1208,
  MSK_RES_ERR_PARAM_INDEX                                = 1210,
  MSK_RES_ERR_PARAM_IS_TOO_LARGE                         = 1215,
  MSK_RES_ERR_PARAM_IS_TOO_SMALL                         = 1216,
  MSK_RES_ERR_PARAM_VALUE_STR                            = 1217,
  MSK_RES_ERR_PARAM_TYPE                                 = 1218,
  MSK_RES_ERR_INF_DOU_INDEX                              = 1219,
  MSK_RES_ERR_INF_INT_INDEX                              = 1220,
  MSK_RES_ERR_INDEX_ARR_IS_TOO_SMALL                     = 1221,
  MSK_RES_ERR_INDEX_ARR_IS_TOO_LARGE                     = 1222,
  MSK_RES_ERR_INF_LINT_INDEX                             = 1225,
  MSK_RES_ERR_ARG_IS_TOO_SMALL                           = 1226,
  MSK_RES_ERR_ARG_IS_TOO_LARGE                           = 1227,
  MSK_RES_ERR_INVALID_WHICHSOL                           = 1228,
  MSK_RES_ERR_INF_DOU_NAME                               = 1230,
  MSK_RES_ERR_INF_INT_NAME                               = 1231,
  MSK_RES_ERR_INF_TYPE                                   = 1232,
  MSK_RES_ERR_INF_LINT_NAME                              = 1234,
  MSK_RES_ERR_INDEX                                      = 1235,
  MSK_RES_ERR_WHICHSOL                                   = 1236,
  MSK_RES_ERR_SOLITEM                                    = 1237,
  MSK_RES_ERR_WHICHITEM_NOT_ALLOWED                      = 1238,
  MSK_RES_ERR_MAXNUMCON                                  = 1240,
  MSK_RES_ERR_MAXNUMVAR                                  = 1241,
  MSK_RES_ERR_MAXNUMBARVAR                               = 1242,
  MSK_RES_ERR_MAXNUMQNZ                                  = 1243,
  MSK_RES_ERR_TOO_SMALL_MAX_NUM_NZ                       = 1245,
  MSK_RES_ERR_INVALID_IDX                                = 1246,
  MSK_RES_ERR_INVALID_MAX_NUM                            = 1247,
  MSK_RES_ERR_NUMCONLIM                                  = 1250,
  MSK_RES_ERR_NUMVARLIM                                  = 1251,
  MSK_RES_ERR_TOO_SMALL_MAXNUMANZ                        = 1252,
  MSK_RES_ERR_INV_APTRE                                  = 1253,
  MSK_RES_ERR_MUL_A_ELEMENT                              = 1254,
  MSK_RES_ERR_INV_BK                                     = 1255,
  MSK_RES_ERR_INV_BKC                                    = 1256,
  MSK_RES_ERR_INV_BKX                                    = 1257,
  MSK_RES_ERR_INV_VAR_TYPE                               = 1258,
  MSK_RES_ERR_SOLVER_PROBTYPE                            = 1259,
  MSK_RES_ERR_OBJECTIVE_RANGE                            = 1260,
  MSK_RES_ERR_UNDEF_SOLUTION                             = 1265,
  MSK_RES_ERR_BASIS                                      = 1266,
  MSK_RES_ERR_INV_SKC                                    = 1267,
  MSK_RES_ERR_INV_SKX                                    = 1268,
  MSK_RES_ERR_INV_SK_STR                                 = 1269,
  MSK_RES_ERR_INV_SK                                     = 1270,
  MSK_RES_ERR_INV_CONE_TYPE_STR                          = 1271,
  MSK_RES_ERR_INV_CONE_TYPE                              = 1272,
  MSK_RES_ERR_INV_SKN                                    = 1274,
  MSK_RES_ERR_INVALID_SURPLUS                            = 1275,
  MSK_RES_ERR_INV_NAME_ITEM                              = 1280,
  MSK_RES_ERR_PRO_ITEM                                   = 1281,
  MSK_RES_ERR_INVALID_FORMAT_TYPE                        = 1283,
  MSK_RES_ERR_FIRSTI                                     = 1285,
  MSK_RES_ERR_LASTI                                      = 1286,
  MSK_RES_ERR_FIRSTJ                                     = 1287,
  MSK_RES_ERR_LASTJ                                      = 1288,
  MSK_RES_ERR_MAX_LEN_IS_TOO_SMALL                       = 1289,
  MSK_RES_ERR_NONLINEAR_EQUALITY                         = 1290,
  MSK_RES_ERR_NONCONVEX                                  = 1291,
  MSK_RES_ERR_NONLINEAR_RANGED                           = 1292,
  MSK_RES_ERR_CON_Q_NOT_PSD                              = 1293,
  MSK_RES_ERR_CON_Q_NOT_NSD                              = 1294,
  MSK_RES_ERR_OBJ_Q_NOT_PSD                              = 1295,
  MSK_RES_ERR_OBJ_Q_NOT_NSD                              = 1296,
  MSK_RES_ERR_ARGUMENT_PERM_ARRAY                        = 1299,
  MSK_RES_ERR_CONE_INDEX                                 = 1300,
  MSK_RES_ERR_CONE_SIZE                                  = 1301,
  MSK_RES_ERR_CONE_OVERLAP                               = 1302,
  MSK_RES_ERR_CONE_REP_VAR                               = 1303,
  MSK_RES_ERR_MAXNUMCONE                                 = 1304,
  MSK_RES_ERR_CONE_TYPE                                  = 1305,
  MSK_RES_ERR_CONE_TYPE_STR                              = 1306,
  MSK_RES_ERR_CONE_OVERLAP_APPEND                        = 1307,
  MSK_RES_ERR_REMOVE_CONE_VARIABLE                       = 1310,
  MSK_RES_ERR_APPENDING_TOO_BIG_CONE                     = 1311,
  MSK_RES_ERR_CONE_PARAMETER                             = 1320,
  MSK_RES_ERR_SOL_FILE_INVALID_NUMBER                    = 1350,
  MSK_RES_ERR_HUGE_C                                     = 1375,
  MSK_RES_ERR_HUGE_AIJ                                   = 1380,
  MSK_RES_ERR_DUPLICATE_AIJ                              = 1385,
  MSK_RES_ERR_LOWER_BOUND_IS_A_NAN                       = 1390,
  MSK_RES_ERR_UPPER_BOUND_IS_A_NAN                       = 1391,
  MSK_RES_ERR_INFINITE_BOUND                             = 1400,
  MSK_RES_ERR_INV_QOBJ_SUBI                              = 1401,
  MSK_RES_ERR_INV_QOBJ_SUBJ                              = 1402,
  MSK_RES_ERR_INV_QOBJ_VAL                               = 1403,
  MSK_RES_ERR_INV_QCON_SUBK                              = 1404,
  MSK_RES_ERR_INV_QCON_SUBI                              = 1405,
  MSK_RES_ERR_INV_QCON_SUBJ                              = 1406,
  MSK_RES_ERR_INV_QCON_VAL                               = 1407,
  MSK_RES_ERR_QCON_SUBI_TOO_SMALL                        = 1408,
  MSK_RES_ERR_QCON_SUBI_TOO_LARGE                        = 1409,
  MSK_RES_ERR_QOBJ_UPPER_TRIANGLE                        = 1415,
  MSK_RES_ERR_QCON_UPPER_TRIANGLE                        = 1417,
  MSK_RES_ERR_FIXED_BOUND_VALUES                         = 1420,
  MSK_RES_ERR_TOO_SMALL_A_TRUNCATION_VALUE               = 1421,
  MSK_RES_ERR_INVALID_OBJECTIVE_SENSE                    = 1445,
  MSK_RES_ERR_UNDEFINED_OBJECTIVE_SENSE                  = 1446,
  MSK_RES_ERR_Y_IS_UNDEFINED                             = 1449,
  MSK_RES_ERR_NAN_IN_DOUBLE_DATA                         = 1450,
  MSK_RES_ERR_NAN_IN_BLC                                 = 1461,
  MSK_RES_ERR_NAN_IN_BUC                                 = 1462,
  MSK_RES_ERR_NAN_IN_C                                   = 1470,
  MSK_RES_ERR_NAN_IN_BLX                                 = 1471,
  MSK_RES_ERR_NAN_IN_BUX                                 = 1472,
  MSK_RES_ERR_INVALID_AIJ                                = 1473,
  MSK_RES_ERR_SYM_MAT_INVALID                            = 1480,
  MSK_RES_ERR_SYM_MAT_HUGE                               = 1482,
  MSK_RES_ERR_INV_PROBLEM                                = 1500,
  MSK_RES_ERR_MIXED_CONIC_AND_NL                         = 1501,
  MSK_RES_ERR_GLOBAL_INV_CONIC_PROBLEM                   = 1503,
  MSK_RES_ERR_INV_OPTIMIZER                              = 1550,
  MSK_RES_ERR_MIO_NO_OPTIMIZER                           = 1551,
  MSK_RES_ERR_NO_OPTIMIZER_VAR_TYPE                      = 1552,
  MSK_RES_ERR_FINAL_SOLUTION                             = 1560,
  MSK_RES_ERR_FIRST                                      = 1570,
  MSK_RES_ERR_LAST                                       = 1571,
  MSK_RES_ERR_SLICE_SIZE                                 = 1572,
  MSK_RES_ERR_NEGATIVE_SURPLUS                           = 1573,
  MSK_RES_ERR_NEGATIVE_APPEND                            = 1578,
  MSK_RES_ERR_POSTSOLVE                                  = 1580,
  MSK_RES_ERR_OVERFLOW                                   = 1590,
  MSK_RES_ERR_NO_BASIS_SOL                               = 1600,
  MSK_RES_ERR_BASIS_FACTOR                               = 1610,
  MSK_RES_ERR_BASIS_SINGULAR                             = 1615,
  MSK_RES_ERR_FACTOR                                     = 1650,
  MSK_RES_ERR_FEASREPAIR_CANNOT_RELAX                    = 1700,
  MSK_RES_ERR_FEASREPAIR_SOLVING_RELAXED                 = 1701,
  MSK_RES_ERR_FEASREPAIR_INCONSISTENT_BOUND              = 1702,
  MSK_RES_ERR_REPAIR_INVALID_PROBLEM                     = 1710,
  MSK_RES_ERR_REPAIR_OPTIMIZATION_FAILED                 = 1711,
  MSK_RES_ERR_NAME_MAX_LEN                               = 1750,
  MSK_RES_ERR_NAME_IS_NULL                               = 1760,
  MSK_RES_ERR_INVALID_COMPRESSION                        = 1800,
  MSK_RES_ERR_INVALID_IOMODE                             = 1801,
  MSK_RES_ERR_NO_PRIMAL_INFEAS_CER                       = 2000,
  MSK_RES_ERR_NO_DUAL_INFEAS_CER                         = 2001,
  MSK_RES_ERR_NO_SOLUTION_IN_CALLBACK                    = 2500,
  MSK_RES_ERR_INV_MARKI                                  = 2501,
  MSK_RES_ERR_INV_MARKJ                                  = 2502,
  MSK_RES_ERR_INV_NUMI                                   = 2503,
  MSK_RES_ERR_INV_NUMJ                                   = 2504,
  MSK_RES_ERR_TASK_INCOMPATIBLE                          = 2560,
  MSK_RES_ERR_TASK_INVALID                               = 2561,
  MSK_RES_ERR_TASK_WRITE                                 = 2562,
  MSK_RES_ERR_LU_MAX_NUM_TRIES                           = 2800,
  MSK_RES_ERR_INVALID_UTF8                               = 2900,
  MSK_RES_ERR_INVALID_WCHAR                              = 2901,
  MSK_RES_ERR_NO_DUAL_FOR_ITG_SOL                        = 2950,
  MSK_RES_ERR_NO_SNX_FOR_BAS_SOL                         = 2953,
  MSK_RES_ERR_INTERNAL                                   = 3000,
  MSK_RES_ERR_API_ARRAY_TOO_SMALL                        = 3001,
  MSK_RES_ERR_API_CB_CONNECT                             = 3002,
  MSK_RES_ERR_API_FATAL_ERROR                            = 3005,
  MSK_RES_ERR_SEN_FORMAT                                 = 3050,
  MSK_RES_ERR_SEN_UNDEF_NAME                             = 3051,
  MSK_RES_ERR_SEN_INDEX_RANGE                            = 3052,
  MSK_RES_ERR_SEN_BOUND_INVALID_UP                       = 3053,
  MSK_RES_ERR_SEN_BOUND_INVALID_LO                       = 3054,
  MSK_RES_ERR_SEN_INDEX_INVALID                          = 3055,
  MSK_RES_ERR_SEN_INVALID_REGEXP                         = 3056,
  MSK_RES_ERR_SEN_SOLUTION_STATUS                        = 3057,
  MSK_RES_ERR_SEN_NUMERICAL                              = 3058,
  MSK_RES_ERR_SEN_UNHANDLED_PROBLEM_TYPE                 = 3080,
  MSK_RES_ERR_UNB_STEP_SIZE                              = 3100,
  MSK_RES_ERR_IDENTICAL_TASKS                            = 3101,
  MSK_RES_ERR_AD_INVALID_CODELIST                        = 3102,
  MSK_RES_ERR_INTERNAL_TEST_FAILED                       = 3500,
  MSK_RES_ERR_XML_INVALID_PROBLEM_TYPE                   = 3600,
  MSK_RES_ERR_INVALID_AMPL_STUB                          = 3700,
  MSK_RES_ERR_INT64_TO_INT32_CAST                        = 3800,
  MSK_RES_ERR_SIZE_LICENSE_NUMCORES                      = 3900,
  MSK_RES_ERR_INFEAS_UNDEFINED                           = 3910,
  MSK_RES_ERR_NO_BARX_FOR_SOLUTION                       = 3915,
  MSK_RES_ERR_NO_BARS_FOR_SOLUTION                       = 3916,
  MSK_RES_ERR_BAR_VAR_DIM                                = 3920,
  MSK_RES_ERR_SYM_MAT_INVALID_ROW_INDEX                  = 3940,
  MSK_RES_ERR_SYM_MAT_INVALID_COL_INDEX                  = 3941,
  MSK_RES_ERR_SYM_MAT_NOT_LOWER_TRINGULAR                = 3942,
  MSK_RES_ERR_SYM_MAT_INVALID_VALUE                      = 3943,
  MSK_RES_ERR_SYM_MAT_DUPLICATE                          = 3944,
  MSK_RES_ERR_INVALID_SYM_MAT_DIM                        = 3950,
  MSK_RES_ERR_API_INTERNAL                               = 3999,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_SYM_MAT            = 4000,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_CFIX               = 4001,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_RANGED_CONSTRAINTS = 4002,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_FREE_CONSTRAINTS   = 4003,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_CONES              = 4005,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_NONLINEAR          = 4010,
  MSK_RES_ERR_DUPLICATE_CONSTRAINT_NAMES                 = 4500,
  MSK_RES_ERR_DUPLICATE_VARIABLE_NAMES                   = 4501,
  MSK_RES_ERR_DUPLICATE_BARVARIABLE_NAMES                = 4502,
  MSK_RES_ERR_DUPLICATE_CONE_NAMES                       = 4503,
  MSK_RES_ERR_NON_UNIQUE_ARRAY                           = 5000,
  MSK_RES_ERR_ARGUMENT_IS_TOO_LARGE                      = 5005,
  MSK_RES_ERR_MIO_INTERNAL                               = 5010,
  MSK_RES_ERR_INVALID_PROBLEM_TYPE                       = 6000,
  MSK_RES_ERR_UNHANDLED_SOLUTION_STATUS                  = 6010,
  MSK_RES_ERR_UPPER_TRIANGLE                             = 6020,
  MSK_RES_ERR_LAU_SINGULAR_MATRIX                        = 7000,
  MSK_RES_ERR_LAU_NOT_POSITIVE_DEFINITE                  = 7001,
  MSK_RES_ERR_LAU_INVALID_LOWER_TRIANGULAR_MATRIX        = 7002,
  MSK_RES_ERR_LAU_UNKNOWN                                = 7005,
  MSK_RES_ERR_LAU_ARG_M                                  = 7010,
  MSK_RES_ERR_LAU_ARG_N                                  = 7011,
  MSK_RES_ERR_LAU_ARG_K                                  = 7012,
  MSK_RES_ERR_LAU_ARG_TRANSA                             = 7015,
  MSK_RES_ERR_LAU_ARG_TRANSB                             = 7016,
  MSK_RES_ERR_LAU_ARG_UPLO                               = 7017,
  MSK_RES_ERR_LAU_ARG_TRANS                              = 7018,
  MSK_RES_ERR_LAU_INVALID_SPARSE_SYMMETRIC_MATRIX        = 7019,
  MSK_RES_ERR_CBF_PARSE                                  = 7100,
  MSK_RES_ERR_CBF_OBJ_SENSE                              = 7101,
  MSK_RES_ERR_CBF_NO_VARIABLES                           = 7102,
  MSK_RES_ERR_CBF_TOO_MANY_CONSTRAINTS                   = 7103,
  MSK_RES_ERR_CBF_TOO_MANY_VARIABLES                     = 7104,
  MSK_RES_ERR_CBF_NO_VERSION_SPECIFIED                   = 7105,
  MSK_RES_ERR_CBF_SYNTAX                                 = 7106,
  MSK_RES_ERR_CBF_DUPLICATE_OBJ                          = 7107,
  MSK_RES_ERR_CBF_DUPLICATE_CON                          = 7108,
  MSK_RES_ERR_CBF_DUPLICATE_VAR                          = 7109,
  MSK_RES_ERR_CBF_DUPLICATE_INT                          = 7110,
  MSK_RES_ERR_CBF_INVALID_VAR_TYPE                       = 7111,
  MSK_RES_ERR_CBF_INVALID_CON_TYPE                       = 7112,
  MSK_RES_ERR_CBF_INVALID_DOMAIN_DIMENSION               = 7113,
  MSK_RES_ERR_CBF_DUPLICATE_OBJACOORD                    = 7114,
  MSK_RES_ERR_CBF_DUPLICATE_BCOORD                       = 7115,
  MSK_RES_ERR_CBF_DUPLICATE_ACOORD                       = 7116,
  MSK_RES_ERR_CBF_TOO_FEW_VARIABLES                      = 7117,
  MSK_RES_ERR_CBF_TOO_FEW_CONSTRAINTS                    = 7118,
  MSK_RES_ERR_CBF_TOO_FEW_INTS                           = 7119,
  MSK_RES_ERR_CBF_TOO_MANY_INTS                          = 7120,
  MSK_RES_ERR_CBF_INVALID_INT_INDEX                      = 7121,
  MSK_RES_ERR_CBF_UNSUPPORTED                            = 7122,
  MSK_RES_ERR_CBF_DUPLICATE_PSDVAR                       = 7123,
  MSK_RES_ERR_CBF_INVALID_PSDVAR_DIMENSION               = 7124,
  MSK_RES_ERR_CBF_TOO_FEW_PSDVAR                         = 7125,
  MSK_RES_ERR_CBF_INVALID_EXP_DIMENSION                  = 7126,
  MSK_RES_ERR_CBF_DUPLICATE_POW_CONES                    = 7130,
  MSK_RES_ERR_CBF_DUPLICATE_POW_STAR_CONES               = 7131,
  MSK_RES_ERR_CBF_INVALID_POWER                          = 7132,
  MSK_RES_ERR_CBF_POWER_CONE_IS_TOO_LONG                 = 7133,
  MSK_RES_ERR_CBF_INVALID_POWER_CONE_INDEX               = 7134,
  MSK_RES_ERR_CBF_INVALID_POWER_STAR_CONE_INDEX          = 7135,
  MSK_RES_ERR_CBF_UNHANDLED_POWER_CONE_TYPE              = 7136,
  MSK_RES_ERR_CBF_UNHANDLED_POWER_STAR_CONE_TYPE         = 7137,
  MSK_RES_ERR_CBF_POWER_CONE_MISMATCH                    = 7138,
  MSK_RES_ERR_CBF_POWER_STAR_CONE_MISMATCH               = 7139,
  MSK_RES_ERR_MIO_INVALID_ROOT_OPTIMIZER                 = 7700,
  MSK_RES_ERR_MIO_INVALID_NODE_OPTIMIZER                 = 7701,
  MSK_RES_ERR_CBF_INVALID_NUMBER_OF_CONES                = 7740,
  MSK_RES_ERR_CBF_INVALID_DIMENSION_OF_CONES             = 7741,
  MSK_RES_ERR_TOCONIC_CONSTR_Q_NOT_PSD                   = 7800,
  MSK_RES_ERR_TOCONIC_CONSTRAINT_FX                      = 7801,
  MSK_RES_ERR_TOCONIC_CONSTRAINT_RA                      = 7802,
  MSK_RES_ERR_TOCONIC_CONSTR_NOT_CONIC                   = 7803,
  MSK_RES_ERR_TOCONIC_OBJECTIVE_NOT_PSD                  = 7804,
  MSK_RES_ERR_SERVER_CONNECT                             = 8000,
  MSK_RES_ERR_SERVER_PROTOCOL                            = 8001,
  MSK_RES_ERR_SERVER_STATUS                              = 8002,
  MSK_RES_ERR_SERVER_TOKEN                               = 8003,
  MSK_RES_TRM_MAX_ITERATIONS                             = 10000,
  MSK_RES_TRM_MAX_TIME                                   = 10001,
  MSK_RES_TRM_OBJECTIVE_RANGE                            = 10002,
  MSK_RES_TRM_STALL                                      = 10006,
  MSK_RES_TRM_USER_CALLBACK                              = 10007,
  MSK_RES_TRM_MIO_NUM_RELAXS                             = 10008,
  MSK_RES_TRM_MIO_NUM_BRANCHES                           = 10009,
  MSK_RES_TRM_NUM_MAX_NUM_INT_SOLUTIONS                  = 10015,
  MSK_RES_TRM_MAX_NUM_SETBACKS                           = 10020,
  MSK_RES_TRM_NUMERICAL_PROBLEM                          = 10025,
  MSK_RES_TRM_INTERNAL                                   = 10030,
  MSK_RES_TRM_INTERNAL_STOP                              = 10031
};

enum MSKpresolvemode_enum {
  MSK_PRESOLVE_MODE_BEGIN = 0,
  MSK_PRESOLVE_MODE_END  = 3,

  MSK_PRESOLVE_MODE_OFF  = 0,
  MSK_PRESOLVE_MODE_ON   = 1,
  MSK_PRESOLVE_MODE_FREE = 2
};

enum MSKiomode_enum {
  MSK_IOMODE_BEGIN     = 0,
  MSK_IOMODE_END       = 3,

  MSK_IOMODE_READ      = 0,
  MSK_IOMODE_WRITE     = 1,
  MSK_IOMODE_READWRITE = 2
};

enum MSKnametype_enum {
  MSK_NAME_TYPE_BEGIN = 0,
  MSK_NAME_TYPE_END = 3,

  MSK_NAME_TYPE_GEN = 0,
  MSK_NAME_TYPE_MPS = 1,
  MSK_NAME_TYPE_LP  = 2
};

enum MSKvariabletype_enum {
  MSK_VAR_BEGIN     = 0,
  MSK_VAR_END       = 2,

  MSK_VAR_TYPE_CONT = 0,
  MSK_VAR_TYPE_INT  = 1
};

/* } namespace mosek; */
/**************************************************/
#define MSK_FIRST_ERR_CODE 1000 
#define MSK_LAST_ERR_CODE  9999 
/**************************************************/



#define MSK_SPAR_BAS_SOL_FILE_NAME_                         "MSK_SPAR_BAS_SOL_FILE_NAME"
#define MSK_SPAR_DATA_FILE_NAME_                            "MSK_SPAR_DATA_FILE_NAME"
#define MSK_SPAR_DEBUG_FILE_NAME_                           "MSK_SPAR_DEBUG_FILE_NAME"
#define MSK_SPAR_INT_SOL_FILE_NAME_                         "MSK_SPAR_INT_SOL_FILE_NAME"
#define MSK_SPAR_ITR_SOL_FILE_NAME_                         "MSK_SPAR_ITR_SOL_FILE_NAME"
#define MSK_SPAR_MIO_DEBUG_STRING_                          "MSK_SPAR_MIO_DEBUG_STRING"
#define MSK_SPAR_PARAM_COMMENT_SIGN_                        "MSK_SPAR_PARAM_COMMENT_SIGN"
#define MSK_SPAR_PARAM_READ_FILE_NAME_                      "MSK_SPAR_PARAM_READ_FILE_NAME"
#define MSK_SPAR_PARAM_WRITE_FILE_NAME_                     "MSK_SPAR_PARAM_WRITE_FILE_NAME"
#define MSK_SPAR_READ_MPS_BOU_NAME_                         "MSK_SPAR_READ_MPS_BOU_NAME"
#define MSK_SPAR_READ_MPS_OBJ_NAME_                         "MSK_SPAR_READ_MPS_OBJ_NAME"
#define MSK_SPAR_READ_MPS_RAN_NAME_                         "MSK_SPAR_READ_MPS_RAN_NAME"
#define MSK_SPAR_READ_MPS_RHS_NAME_                         "MSK_SPAR_READ_MPS_RHS_NAME"
#define MSK_SPAR_REMOTE_ACCESS_TOKEN_                       "MSK_SPAR_REMOTE_ACCESS_TOKEN"
#define MSK_SPAR_SENSITIVITY_FILE_NAME_                     "MSK_SPAR_SENSITIVITY_FILE_NAME"
#define MSK_SPAR_SENSITIVITY_RES_FILE_NAME_                 "MSK_SPAR_SENSITIVITY_RES_FILE_NAME"
#define MSK_SPAR_SOL_FILTER_XC_LOW_                         "MSK_SPAR_SOL_FILTER_XC_LOW"
#define MSK_SPAR_SOL_FILTER_XC_UPR_                         "MSK_SPAR_SOL_FILTER_XC_UPR"
#define MSK_SPAR_SOL_FILTER_XX_LOW_                         "MSK_SPAR_SOL_FILTER_XX_LOW"
#define MSK_SPAR_SOL_FILTER_XX_UPR_                         "MSK_SPAR_SOL_FILTER_XX_UPR"
#define MSK_SPAR_STAT_FILE_NAME_                            "MSK_SPAR_STAT_FILE_NAME"
#define MSK_SPAR_STAT_KEY_                                  "MSK_SPAR_STAT_KEY"
#define MSK_SPAR_STAT_NAME_                                 "MSK_SPAR_STAT_NAME"
#define MSK_SPAR_WRITE_LP_GEN_VAR_NAME_                     "MSK_SPAR_WRITE_LP_GEN_VAR_NAME"

#define MSK_DPAR_ANA_SOL_INFEAS_TOL_                        "MSK_DPAR_ANA_SOL_INFEAS_TOL"
#define MSK_DPAR_BASIS_REL_TOL_S_                           "MSK_DPAR_BASIS_REL_TOL_S"
#define MSK_DPAR_BASIS_TOL_S_                               "MSK_DPAR_BASIS_TOL_S"
#define MSK_DPAR_BASIS_TOL_X_                               "MSK_DPAR_BASIS_TOL_X"
#define MSK_DPAR_CHECK_CONVEXITY_REL_TOL_                   "MSK_DPAR_CHECK_CONVEXITY_REL_TOL"
#define MSK_DPAR_DATA_SYM_MAT_TOL_                          "MSK_DPAR_DATA_SYM_MAT_TOL"
#define MSK_DPAR_DATA_SYM_MAT_TOL_HUGE_                     "MSK_DPAR_DATA_SYM_MAT_TOL_HUGE"
#define MSK_DPAR_DATA_SYM_MAT_TOL_LARGE_                    "MSK_DPAR_DATA_SYM_MAT_TOL_LARGE"
#define MSK_DPAR_DATA_TOL_AIJ_HUGE_                         "MSK_DPAR_DATA_TOL_AIJ_HUGE"
#define MSK_DPAR_DATA_TOL_AIJ_LARGE_                        "MSK_DPAR_DATA_TOL_AIJ_LARGE"
#define MSK_DPAR_DATA_TOL_BOUND_INF_                        "MSK_DPAR_DATA_TOL_BOUND_INF"
#define MSK_DPAR_DATA_TOL_BOUND_WRN_                        "MSK_DPAR_DATA_TOL_BOUND_WRN"
#define MSK_DPAR_DATA_TOL_C_HUGE_                           "MSK_DPAR_DATA_TOL_C_HUGE"
#define MSK_DPAR_DATA_TOL_CJ_LARGE_                         "MSK_DPAR_DATA_TOL_CJ_LARGE"
#define MSK_DPAR_DATA_TOL_QIJ_                              "MSK_DPAR_DATA_TOL_QIJ"
#define MSK_DPAR_DATA_TOL_X_                                "MSK_DPAR_DATA_TOL_X"
#define MSK_DPAR_INTPNT_CO_TOL_DFEAS_                       "MSK_DPAR_INTPNT_CO_TOL_DFEAS"
#define MSK_DPAR_INTPNT_CO_TOL_INFEAS_                      "MSK_DPAR_INTPNT_CO_TOL_INFEAS"
#define MSK_DPAR_INTPNT_CO_TOL_MU_RED_                      "MSK_DPAR_INTPNT_CO_TOL_MU_RED"
#define MSK_DPAR_INTPNT_CO_TOL_NEAR_REL_                    "MSK_DPAR_INTPNT_CO_TOL_NEAR_REL"
#define MSK_DPAR_INTPNT_CO_TOL_PFEAS_                       "MSK_DPAR_INTPNT_CO_TOL_PFEAS"
#define MSK_DPAR_INTPNT_CO_TOL_REL_GAP_                     "MSK_DPAR_INTPNT_CO_TOL_REL_GAP"
#define MSK_DPAR_INTPNT_QO_TOL_DFEAS_                       "MSK_DPAR_INTPNT_QO_TOL_DFEAS"
#define MSK_DPAR_INTPNT_QO_TOL_INFEAS_                      "MSK_DPAR_INTPNT_QO_TOL_INFEAS"
#define MSK_DPAR_INTPNT_QO_TOL_MU_RED_                      "MSK_DPAR_INTPNT_QO_TOL_MU_RED"
#define MSK_DPAR_INTPNT_QO_TOL_NEAR_REL_                    "MSK_DPAR_INTPNT_QO_TOL_NEAR_REL"
#define MSK_DPAR_INTPNT_QO_TOL_PFEAS_                       "MSK_DPAR_INTPNT_QO_TOL_PFEAS"
#define MSK_DPAR_INTPNT_QO_TOL_REL_GAP_                     "MSK_DPAR_INTPNT_QO_TOL_REL_GAP"
#define MSK_DPAR_INTPNT_TOL_DFEAS_                          "MSK_DPAR_INTPNT_TOL_DFEAS"
#define MSK_DPAR_INTPNT_TOL_DSAFE_                          "MSK_DPAR_INTPNT_TOL_DSAFE"
#define MSK_DPAR_INTPNT_TOL_INFEAS_                         "MSK_DPAR_INTPNT_TOL_INFEAS"
#define MSK_DPAR_INTPNT_TOL_MU_RED_                         "MSK_DPAR_INTPNT_TOL_MU_RED"
#define MSK_DPAR_INTPNT_TOL_PATH_                           "MSK_DPAR_INTPNT_TOL_PATH"
#define MSK_DPAR_INTPNT_TOL_PFEAS_                          "MSK_DPAR_INTPNT_TOL_PFEAS"
#define MSK_DPAR_INTPNT_TOL_PSAFE_                          "MSK_DPAR_INTPNT_TOL_PSAFE"
#define MSK_DPAR_INTPNT_TOL_REL_GAP_                        "MSK_DPAR_INTPNT_TOL_REL_GAP"
#define MSK_DPAR_INTPNT_TOL_REL_STEP_                       "MSK_DPAR_INTPNT_TOL_REL_STEP"
#define MSK_DPAR_INTPNT_TOL_STEP_SIZE_                      "MSK_DPAR_INTPNT_TOL_STEP_SIZE"
#define MSK_DPAR_LOWER_OBJ_CUT_                             "MSK_DPAR_LOWER_OBJ_CUT"
#define MSK_DPAR_LOWER_OBJ_CUT_FINITE_TRH_                  "MSK_DPAR_LOWER_OBJ_CUT_FINITE_TRH"
#define MSK_DPAR_MIO_MAX_TIME_                              "MSK_DPAR_MIO_MAX_TIME"
#define MSK_DPAR_MIO_REL_GAP_CONST_                         "MSK_DPAR_MIO_REL_GAP_CONST"
#define MSK_DPAR_MIO_TOL_ABS_GAP_                           "MSK_DPAR_MIO_TOL_ABS_GAP"
#define MSK_DPAR_MIO_TOL_ABS_RELAX_INT_                     "MSK_DPAR_MIO_TOL_ABS_RELAX_INT"
#define MSK_DPAR_MIO_TOL_FEAS_                              "MSK_DPAR_MIO_TOL_FEAS"
#define MSK_DPAR_MIO_TOL_REL_DUAL_BOUND_IMPROVEMENT_        "MSK_DPAR_MIO_TOL_REL_DUAL_BOUND_IMPROVEMENT"
#define MSK_DPAR_MIO_TOL_REL_GAP_                           "MSK_DPAR_MIO_TOL_REL_GAP"
#define MSK_DPAR_OPTIMIZER_MAX_TIME_                        "MSK_DPAR_OPTIMIZER_MAX_TIME"
#define MSK_DPAR_PRESOLVE_TOL_ABS_LINDEP_                   "MSK_DPAR_PRESOLVE_TOL_ABS_LINDEP"
#define MSK_DPAR_PRESOLVE_TOL_AIJ_                          "MSK_DPAR_PRESOLVE_TOL_AIJ"
#define MSK_DPAR_PRESOLVE_TOL_REL_LINDEP_                   "MSK_DPAR_PRESOLVE_TOL_REL_LINDEP"
#define MSK_DPAR_PRESOLVE_TOL_S_                            "MSK_DPAR_PRESOLVE_TOL_S"
#define MSK_DPAR_PRESOLVE_TOL_X_                            "MSK_DPAR_PRESOLVE_TOL_X"
#define MSK_DPAR_QCQO_REFORMULATE_REL_DROP_TOL_             "MSK_DPAR_QCQO_REFORMULATE_REL_DROP_TOL"
#define MSK_DPAR_SEMIDEFINITE_TOL_APPROX_                   "MSK_DPAR_SEMIDEFINITE_TOL_APPROX"
#define MSK_DPAR_SIM_LU_TOL_REL_PIV_                        "MSK_DPAR_SIM_LU_TOL_REL_PIV"
#define MSK_DPAR_SIMPLEX_ABS_TOL_PIV_                       "MSK_DPAR_SIMPLEX_ABS_TOL_PIV"
#define MSK_DPAR_UPPER_OBJ_CUT_                             "MSK_DPAR_UPPER_OBJ_CUT"
#define MSK_DPAR_UPPER_OBJ_CUT_FINITE_TRH_                  "MSK_DPAR_UPPER_OBJ_CUT_FINITE_TRH"

#define MSK_IPAR_ANA_SOL_BASIS_                             "MSK_IPAR_ANA_SOL_BASIS"
#define MSK_IPAR_ANA_SOL_PRINT_VIOLATED_                    "MSK_IPAR_ANA_SOL_PRINT_VIOLATED"
#define MSK_IPAR_AUTO_SORT_A_BEFORE_OPT_                    "MSK_IPAR_AUTO_SORT_A_BEFORE_OPT"
#define MSK_IPAR_AUTO_UPDATE_SOL_INFO_                      "MSK_IPAR_AUTO_UPDATE_SOL_INFO"
#define MSK_IPAR_BASIS_SOLVE_USE_PLUS_ONE_                  "MSK_IPAR_BASIS_SOLVE_USE_PLUS_ONE"
#define MSK_IPAR_BI_CLEAN_OPTIMIZER_                        "MSK_IPAR_BI_CLEAN_OPTIMIZER"
#define MSK_IPAR_BI_IGNORE_MAX_ITER_                        "MSK_IPAR_BI_IGNORE_MAX_ITER"
#define MSK_IPAR_BI_IGNORE_NUM_ERROR_                       "MSK_IPAR_BI_IGNORE_NUM_ERROR"
#define MSK_IPAR_BI_MAX_ITERATIONS_                         "MSK_IPAR_BI_MAX_ITERATIONS"
#define MSK_IPAR_CACHE_LICENSE_                             "MSK_IPAR_CACHE_LICENSE"
#define MSK_IPAR_CHECK_CONVEXITY_                           "MSK_IPAR_CHECK_CONVEXITY"
#define MSK_IPAR_COMPRESS_STATFILE_                         "MSK_IPAR_COMPRESS_STATFILE"
#define MSK_IPAR_INFEAS_GENERIC_NAMES_                      "MSK_IPAR_INFEAS_GENERIC_NAMES"
#define MSK_IPAR_INFEAS_PREFER_PRIMAL_                      "MSK_IPAR_INFEAS_PREFER_PRIMAL"
#define MSK_IPAR_INFEAS_REPORT_AUTO_                        "MSK_IPAR_INFEAS_REPORT_AUTO"
#define MSK_IPAR_INFEAS_REPORT_LEVEL_                       "MSK_IPAR_INFEAS_REPORT_LEVEL"
#define MSK_IPAR_INTPNT_BASIS_                              "MSK_IPAR_INTPNT_BASIS"
#define MSK_IPAR_INTPNT_DIFF_STEP_                          "MSK_IPAR_INTPNT_DIFF_STEP"
#define MSK_IPAR_INTPNT_HOTSTART_                           "MSK_IPAR_INTPNT_HOTSTART"
#define MSK_IPAR_INTPNT_MAX_ITERATIONS_                     "MSK_IPAR_INTPNT_MAX_ITERATIONS"
#define MSK_IPAR_INTPNT_MAX_NUM_COR_                        "MSK_IPAR_INTPNT_MAX_NUM_COR"
#define MSK_IPAR_INTPNT_MAX_NUM_REFINEMENT_STEPS_           "MSK_IPAR_INTPNT_MAX_NUM_REFINEMENT_STEPS"
#define MSK_IPAR_INTPNT_MULTI_THREAD_                       "MSK_IPAR_INTPNT_MULTI_THREAD"
#define MSK_IPAR_INTPNT_OFF_COL_TRH_                        "MSK_IPAR_INTPNT_OFF_COL_TRH"
#define MSK_IPAR_INTPNT_ORDER_GP_NUM_SEEDS_                 "MSK_IPAR_INTPNT_ORDER_GP_NUM_SEEDS"
#define MSK_IPAR_INTPNT_ORDER_METHOD_                       "MSK_IPAR_INTPNT_ORDER_METHOD"
#define MSK_IPAR_INTPNT_PURIFY_                             "MSK_IPAR_INTPNT_PURIFY"
#define MSK_IPAR_INTPNT_REGULARIZATION_USE_                 "MSK_IPAR_INTPNT_REGULARIZATION_USE"
#define MSK_IPAR_INTPNT_SCALING_                            "MSK_IPAR_INTPNT_SCALING"
#define MSK_IPAR_INTPNT_SOLVE_FORM_                         "MSK_IPAR_INTPNT_SOLVE_FORM"
#define MSK_IPAR_INTPNT_STARTING_POINT_                     "MSK_IPAR_INTPNT_STARTING_POINT"
#define MSK_IPAR_LICENSE_DEBUG_                             "MSK_IPAR_LICENSE_DEBUG"
#define MSK_IPAR_LICENSE_PAUSE_TIME_                        "MSK_IPAR_LICENSE_PAUSE_TIME"
#define MSK_IPAR_LICENSE_SUPPRESS_EXPIRE_WRNS_              "MSK_IPAR_LICENSE_SUPPRESS_EXPIRE_WRNS"
#define MSK_IPAR_LICENSE_TRH_EXPIRY_WRN_                    "MSK_IPAR_LICENSE_TRH_EXPIRY_WRN"
#define MSK_IPAR_LICENSE_WAIT_                              "MSK_IPAR_LICENSE_WAIT"
#define MSK_IPAR_LOG_                                       "MSK_IPAR_LOG"
#define MSK_IPAR_LOG_ANA_PRO_                               "MSK_IPAR_LOG_ANA_PRO"
#define MSK_IPAR_LOG_BI_                                    "MSK_IPAR_LOG_BI"
#define MSK_IPAR_LOG_BI_FREQ_                               "MSK_IPAR_LOG_BI_FREQ"
#define MSK_IPAR_LOG_CHECK_CONVEXITY_                       "MSK_IPAR_LOG_CHECK_CONVEXITY"
#define MSK_IPAR_LOG_CUT_SECOND_OPT_                        "MSK_IPAR_LOG_CUT_SECOND_OPT"
#define MSK_IPAR_LOG_EXPAND_                                "MSK_IPAR_LOG_EXPAND"
#define MSK_IPAR_LOG_FEAS_REPAIR_                           "MSK_IPAR_LOG_FEAS_REPAIR"
#define MSK_IPAR_LOG_FILE_                                  "MSK_IPAR_LOG_FILE"
#define MSK_IPAR_LOG_INCLUDE_SUMMARY_                       "MSK_IPAR_LOG_INCLUDE_SUMMARY"
#define MSK_IPAR_LOG_INFEAS_ANA_                            "MSK_IPAR_LOG_INFEAS_ANA"
#define MSK_IPAR_LOG_INTPNT_                                "MSK_IPAR_LOG_INTPNT"
#define MSK_IPAR_LOG_LOCAL_INFO_                            "MSK_IPAR_LOG_LOCAL_INFO"
#define MSK_IPAR_LOG_MIO_                                   "MSK_IPAR_LOG_MIO"
#define MSK_IPAR_LOG_MIO_FREQ_                              "MSK_IPAR_LOG_MIO_FREQ"
#define MSK_IPAR_LOG_ORDER_                                 "MSK_IPAR_LOG_ORDER"
#define MSK_IPAR_LOG_PRESOLVE_                              "MSK_IPAR_LOG_PRESOLVE"
#define MSK_IPAR_LOG_RESPONSE_                              "MSK_IPAR_LOG_RESPONSE"
#define MSK_IPAR_LOG_SENSITIVITY_                           "MSK_IPAR_LOG_SENSITIVITY"
#define MSK_IPAR_LOG_SENSITIVITY_OPT_                       "MSK_IPAR_LOG_SENSITIVITY_OPT"
#define MSK_IPAR_LOG_SIM_                                   "MSK_IPAR_LOG_SIM"
#define MSK_IPAR_LOG_SIM_FREQ_                              "MSK_IPAR_LOG_SIM_FREQ"
#define MSK_IPAR_LOG_SIM_MINOR_                             "MSK_IPAR_LOG_SIM_MINOR"
#define MSK_IPAR_LOG_STORAGE_                               "MSK_IPAR_LOG_STORAGE"
#define MSK_IPAR_MAX_NUM_WARNINGS_                          "MSK_IPAR_MAX_NUM_WARNINGS"
#define MSK_IPAR_MIO_BRANCH_DIR_                            "MSK_IPAR_MIO_BRANCH_DIR"
#define MSK_IPAR_MIO_CONIC_OUTER_APPROXIMATION_             "MSK_IPAR_MIO_CONIC_OUTER_APPROXIMATION"
#define MSK_IPAR_MIO_CUT_CLIQUE_                            "MSK_IPAR_MIO_CUT_CLIQUE"
#define MSK_IPAR_MIO_CUT_CMIR_                              "MSK_IPAR_MIO_CUT_CMIR"
#define MSK_IPAR_MIO_CUT_GMI_                               "MSK_IPAR_MIO_CUT_GMI"
#define MSK_IPAR_MIO_CUT_IMPLIED_BOUND_                     "MSK_IPAR_MIO_CUT_IMPLIED_BOUND"
#define MSK_IPAR_MIO_CUT_KNAPSACK_COVER_                    "MSK_IPAR_MIO_CUT_KNAPSACK_COVER"
#define MSK_IPAR_MIO_CUT_SELECTION_LEVEL_                   "MSK_IPAR_MIO_CUT_SELECTION_LEVEL"
#define MSK_IPAR_MIO_FEASPUMP_LEVEL_                        "MSK_IPAR_MIO_FEASPUMP_LEVEL"
#define MSK_IPAR_MIO_HEURISTIC_LEVEL_                       "MSK_IPAR_MIO_HEURISTIC_LEVEL"
#define MSK_IPAR_MIO_MAX_NUM_BRANCHES_                      "MSK_IPAR_MIO_MAX_NUM_BRANCHES"
#define MSK_IPAR_MIO_MAX_NUM_RELAXS_                        "MSK_IPAR_MIO_MAX_NUM_RELAXS"
#define MSK_IPAR_MIO_MAX_NUM_ROOT_CUT_ROUNDS_               "MSK_IPAR_MIO_MAX_NUM_ROOT_CUT_ROUNDS"
#define MSK_IPAR_MIO_MAX_NUM_SOLUTIONS_                     "MSK_IPAR_MIO_MAX_NUM_SOLUTIONS"
#define MSK_IPAR_MIO_MODE_                                  "MSK_IPAR_MIO_MODE"
#define MSK_IPAR_MIO_NODE_OPTIMIZER_                        "MSK_IPAR_MIO_NODE_OPTIMIZER"
#define MSK_IPAR_MIO_NODE_SELECTION_                        "MSK_IPAR_MIO_NODE_SELECTION"
#define MSK_IPAR_MIO_PERSPECTIVE_REFORMULATE_               "MSK_IPAR_MIO_PERSPECTIVE_REFORMULATE"
#define MSK_IPAR_MIO_PROBING_LEVEL_                         "MSK_IPAR_MIO_PROBING_LEVEL"
#define MSK_IPAR_MIO_PROPAGATE_OBJECTIVE_CONSTRAINT_        "MSK_IPAR_MIO_PROPAGATE_OBJECTIVE_CONSTRAINT"
#define MSK_IPAR_MIO_RINS_MAX_NODES_                        "MSK_IPAR_MIO_RINS_MAX_NODES"
#define MSK_IPAR_MIO_ROOT_OPTIMIZER_                        "MSK_IPAR_MIO_ROOT_OPTIMIZER"
#define MSK_IPAR_MIO_ROOT_REPEAT_PRESOLVE_LEVEL_            "MSK_IPAR_MIO_ROOT_REPEAT_PRESOLVE_LEVEL"
#define MSK_IPAR_MIO_SEED_                                  "MSK_IPAR_MIO_SEED"
#define MSK_IPAR_MIO_VB_DETECTION_LEVEL_                    "MSK_IPAR_MIO_VB_DETECTION_LEVEL"
#define MSK_IPAR_MT_SPINCOUNT_                              "MSK_IPAR_MT_SPINCOUNT"
#define MSK_IPAR_NUM_THREADS_                               "MSK_IPAR_NUM_THREADS"
#define MSK_IPAR_OPF_WRITE_HEADER_                          "MSK_IPAR_OPF_WRITE_HEADER"
#define MSK_IPAR_OPF_WRITE_HINTS_                           "MSK_IPAR_OPF_WRITE_HINTS"
#define MSK_IPAR_OPF_WRITE_LINE_LENGTH_                     "MSK_IPAR_OPF_WRITE_LINE_LENGTH"
#define MSK_IPAR_OPF_WRITE_PARAMETERS_                      "MSK_IPAR_OPF_WRITE_PARAMETERS"
#define MSK_IPAR_OPF_WRITE_PROBLEM_                         "MSK_IPAR_OPF_WRITE_PROBLEM"
#define MSK_IPAR_OPF_WRITE_SOL_BAS_                         "MSK_IPAR_OPF_WRITE_SOL_BAS"
#define MSK_IPAR_OPF_WRITE_SOL_ITG_                         "MSK_IPAR_OPF_WRITE_SOL_ITG"
#define MSK_IPAR_OPF_WRITE_SOL_ITR_                         "MSK_IPAR_OPF_WRITE_SOL_ITR"
#define MSK_IPAR_OPF_WRITE_SOLUTIONS_                       "MSK_IPAR_OPF_WRITE_SOLUTIONS"
#define MSK_IPAR_OPTIMIZER_                                 "MSK_IPAR_OPTIMIZER"
#define MSK_IPAR_PARAM_READ_CASE_NAME_                      "MSK_IPAR_PARAM_READ_CASE_NAME"
#define MSK_IPAR_PARAM_READ_IGN_ERROR_                      "MSK_IPAR_PARAM_READ_IGN_ERROR"
#define MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_FILL_              "MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_FILL"
#define MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_NUM_TRIES_         "MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_NUM_TRIES"
#define MSK_IPAR_PRESOLVE_LEVEL_                            "MSK_IPAR_PRESOLVE_LEVEL"
#define MSK_IPAR_PRESOLVE_LINDEP_ABS_WORK_TRH_              "MSK_IPAR_PRESOLVE_LINDEP_ABS_WORK_TRH"
#define MSK_IPAR_PRESOLVE_LINDEP_REL_WORK_TRH_              "MSK_IPAR_PRESOLVE_LINDEP_REL_WORK_TRH"
#define MSK_IPAR_PRESOLVE_LINDEP_USE_                       "MSK_IPAR_PRESOLVE_LINDEP_USE"
#define MSK_IPAR_PRESOLVE_MAX_NUM_PASS_                     "MSK_IPAR_PRESOLVE_MAX_NUM_PASS"
#define MSK_IPAR_PRESOLVE_MAX_NUM_REDUCTIONS_               "MSK_IPAR_PRESOLVE_MAX_NUM_REDUCTIONS"
#define MSK_IPAR_PRESOLVE_USE_                              "MSK_IPAR_PRESOLVE_USE"
#define MSK_IPAR_PRIMAL_REPAIR_OPTIMIZER_                   "MSK_IPAR_PRIMAL_REPAIR_OPTIMIZER"
#define MSK_IPAR_PTF_WRITE_TRANSFORM_                       "MSK_IPAR_PTF_WRITE_TRANSFORM"
#define MSK_IPAR_READ_DEBUG_                                "MSK_IPAR_READ_DEBUG"
#define MSK_IPAR_READ_KEEP_FREE_CON_                        "MSK_IPAR_READ_KEEP_FREE_CON"
#define MSK_IPAR_READ_LP_DROP_NEW_VARS_IN_BOU_              "MSK_IPAR_READ_LP_DROP_NEW_VARS_IN_BOU"
#define MSK_IPAR_READ_LP_QUOTED_NAMES_                      "MSK_IPAR_READ_LP_QUOTED_NAMES"
#define MSK_IPAR_READ_MPS_FORMAT_                           "MSK_IPAR_READ_MPS_FORMAT"
#define MSK_IPAR_READ_MPS_WIDTH_                            "MSK_IPAR_READ_MPS_WIDTH"
#define MSK_IPAR_READ_TASK_IGNORE_PARAM_                    "MSK_IPAR_READ_TASK_IGNORE_PARAM"
#define MSK_IPAR_REMOVE_UNUSED_SOLUTIONS_                   "MSK_IPAR_REMOVE_UNUSED_SOLUTIONS"
#define MSK_IPAR_SENSITIVITY_ALL_                           "MSK_IPAR_SENSITIVITY_ALL"
#define MSK_IPAR_SENSITIVITY_OPTIMIZER_                     "MSK_IPAR_SENSITIVITY_OPTIMIZER"
#define MSK_IPAR_SENSITIVITY_TYPE_                          "MSK_IPAR_SENSITIVITY_TYPE"
#define MSK_IPAR_SIM_BASIS_FACTOR_USE_                      "MSK_IPAR_SIM_BASIS_FACTOR_USE"
#define MSK_IPAR_SIM_DEGEN_                                 "MSK_IPAR_SIM_DEGEN"
#define MSK_IPAR_SIM_DUAL_CRASH_                            "MSK_IPAR_SIM_DUAL_CRASH"
#define MSK_IPAR_SIM_DUAL_PHASEONE_METHOD_                  "MSK_IPAR_SIM_DUAL_PHASEONE_METHOD"
#define MSK_IPAR_SIM_DUAL_RESTRICT_SELECTION_               "MSK_IPAR_SIM_DUAL_RESTRICT_SELECTION"
#define MSK_IPAR_SIM_DUAL_SELECTION_                        "MSK_IPAR_SIM_DUAL_SELECTION"
#define MSK_IPAR_SIM_EXPLOIT_DUPVEC_                        "MSK_IPAR_SIM_EXPLOIT_DUPVEC"
#define MSK_IPAR_SIM_HOTSTART_                              "MSK_IPAR_SIM_HOTSTART"
#define MSK_IPAR_SIM_HOTSTART_LU_                           "MSK_IPAR_SIM_HOTSTART_LU"
#define MSK_IPAR_SIM_MAX_ITERATIONS_                        "MSK_IPAR_SIM_MAX_ITERATIONS"
#define MSK_IPAR_SIM_MAX_NUM_SETBACKS_                      "MSK_IPAR_SIM_MAX_NUM_SETBACKS"
#define MSK_IPAR_SIM_NON_SINGULAR_                          "MSK_IPAR_SIM_NON_SINGULAR"
#define MSK_IPAR_SIM_PRIMAL_CRASH_                          "MSK_IPAR_SIM_PRIMAL_CRASH"
#define MSK_IPAR_SIM_PRIMAL_PHASEONE_METHOD_                "MSK_IPAR_SIM_PRIMAL_PHASEONE_METHOD"
#define MSK_IPAR_SIM_PRIMAL_RESTRICT_SELECTION_             "MSK_IPAR_SIM_PRIMAL_RESTRICT_SELECTION"
#define MSK_IPAR_SIM_PRIMAL_SELECTION_                      "MSK_IPAR_SIM_PRIMAL_SELECTION"
#define MSK_IPAR_SIM_REFACTOR_FREQ_                         "MSK_IPAR_SIM_REFACTOR_FREQ"
#define MSK_IPAR_SIM_REFORMULATION_                         "MSK_IPAR_SIM_REFORMULATION"
#define MSK_IPAR_SIM_SAVE_LU_                               "MSK_IPAR_SIM_SAVE_LU"
#define MSK_IPAR_SIM_SCALING_                               "MSK_IPAR_SIM_SCALING"
#define MSK_IPAR_SIM_SCALING_METHOD_                        "MSK_IPAR_SIM_SCALING_METHOD"
#define MSK_IPAR_SIM_SEED_                                  "MSK_IPAR_SIM_SEED"
#define MSK_IPAR_SIM_SOLVE_FORM_                            "MSK_IPAR_SIM_SOLVE_FORM"
#define MSK_IPAR_SIM_STABILITY_PRIORITY_                    "MSK_IPAR_SIM_STABILITY_PRIORITY"
#define MSK_IPAR_SIM_SWITCH_OPTIMIZER_                      "MSK_IPAR_SIM_SWITCH_OPTIMIZER"
#define MSK_IPAR_SOL_FILTER_KEEP_BASIC_                     "MSK_IPAR_SOL_FILTER_KEEP_BASIC"
#define MSK_IPAR_SOL_FILTER_KEEP_RANGED_                    "MSK_IPAR_SOL_FILTER_KEEP_RANGED"
#define MSK_IPAR_SOL_READ_NAME_WIDTH_                       "MSK_IPAR_SOL_READ_NAME_WIDTH"
#define MSK_IPAR_SOL_READ_WIDTH_                            "MSK_IPAR_SOL_READ_WIDTH"
#define MSK_IPAR_SOLUTION_CALLBACK_                         "MSK_IPAR_SOLUTION_CALLBACK"
#define MSK_IPAR_TIMING_LEVEL_                              "MSK_IPAR_TIMING_LEVEL"
#define MSK_IPAR_WRITE_BAS_CONSTRAINTS_                     "MSK_IPAR_WRITE_BAS_CONSTRAINTS"
#define MSK_IPAR_WRITE_BAS_HEAD_                            "MSK_IPAR_WRITE_BAS_HEAD"
#define MSK_IPAR_WRITE_BAS_VARIABLES_                       "MSK_IPAR_WRITE_BAS_VARIABLES"
#define MSK_IPAR_WRITE_COMPRESSION_                         "MSK_IPAR_WRITE_COMPRESSION"
#define MSK_IPAR_WRITE_DATA_PARAM_                          "MSK_IPAR_WRITE_DATA_PARAM"
#define MSK_IPAR_WRITE_FREE_CON_                            "MSK_IPAR_WRITE_FREE_CON"
#define MSK_IPAR_WRITE_GENERIC_NAMES_                       "MSK_IPAR_WRITE_GENERIC_NAMES"
#define MSK_IPAR_WRITE_GENERIC_NAMES_IO_                    "MSK_IPAR_WRITE_GENERIC_NAMES_IO"
#define MSK_IPAR_WRITE_IGNORE_INCOMPATIBLE_ITEMS_           "MSK_IPAR_WRITE_IGNORE_INCOMPATIBLE_ITEMS"
#define MSK_IPAR_WRITE_INT_CONSTRAINTS_                     "MSK_IPAR_WRITE_INT_CONSTRAINTS"
#define MSK_IPAR_WRITE_INT_HEAD_                            "MSK_IPAR_WRITE_INT_HEAD"
#define MSK_IPAR_WRITE_INT_VARIABLES_                       "MSK_IPAR_WRITE_INT_VARIABLES"
#define MSK_IPAR_WRITE_LP_FULL_OBJ_                         "MSK_IPAR_WRITE_LP_FULL_OBJ"
#define MSK_IPAR_WRITE_LP_LINE_WIDTH_                       "MSK_IPAR_WRITE_LP_LINE_WIDTH"
#define MSK_IPAR_WRITE_LP_QUOTED_NAMES_                     "MSK_IPAR_WRITE_LP_QUOTED_NAMES"
#define MSK_IPAR_WRITE_LP_STRICT_FORMAT_                    "MSK_IPAR_WRITE_LP_STRICT_FORMAT"
#define MSK_IPAR_WRITE_LP_TERMS_PER_LINE_                   "MSK_IPAR_WRITE_LP_TERMS_PER_LINE"
#define MSK_IPAR_WRITE_MPS_FORMAT_                          "MSK_IPAR_WRITE_MPS_FORMAT"
#define MSK_IPAR_WRITE_MPS_INT_                             "MSK_IPAR_WRITE_MPS_INT"
#define MSK_IPAR_WRITE_PRECISION_                           "MSK_IPAR_WRITE_PRECISION"
#define MSK_IPAR_WRITE_SOL_BARVARIABLES_                    "MSK_IPAR_WRITE_SOL_BARVARIABLES"
#define MSK_IPAR_WRITE_SOL_CONSTRAINTS_                     "MSK_IPAR_WRITE_SOL_CONSTRAINTS"
#define MSK_IPAR_WRITE_SOL_HEAD_                            "MSK_IPAR_WRITE_SOL_HEAD"
#define MSK_IPAR_WRITE_SOL_IGNORE_INVALID_NAMES_            "MSK_IPAR_WRITE_SOL_IGNORE_INVALID_NAMES"
#define MSK_IPAR_WRITE_SOL_VARIABLES_                       "MSK_IPAR_WRITE_SOL_VARIABLES"
#define MSK_IPAR_WRITE_TASK_INC_SOL_                        "MSK_IPAR_WRITE_TASK_INC_SOL"
#define MSK_IPAR_WRITE_XML_MODE_                            "MSK_IPAR_WRITE_XML_MODE"

#define MSK_IINF_ANA_PRO_NUM_CON_                           "MSK_IINF_ANA_PRO_NUM_CON"
#define MSK_IINF_ANA_PRO_NUM_CON_EQ_                        "MSK_IINF_ANA_PRO_NUM_CON_EQ"
#define MSK_IINF_ANA_PRO_NUM_CON_FR_                        "MSK_IINF_ANA_PRO_NUM_CON_FR"
#define MSK_IINF_ANA_PRO_NUM_CON_LO_                        "MSK_IINF_ANA_PRO_NUM_CON_LO"
#define MSK_IINF_ANA_PRO_NUM_CON_RA_                        "MSK_IINF_ANA_PRO_NUM_CON_RA"
#define MSK_IINF_ANA_PRO_NUM_CON_UP_                        "MSK_IINF_ANA_PRO_NUM_CON_UP"
#define MSK_IINF_ANA_PRO_NUM_VAR_                           "MSK_IINF_ANA_PRO_NUM_VAR"
#define MSK_IINF_ANA_PRO_NUM_VAR_BIN_                       "MSK_IINF_ANA_PRO_NUM_VAR_BIN"
#define MSK_IINF_ANA_PRO_NUM_VAR_CONT_                      "MSK_IINF_ANA_PRO_NUM_VAR_CONT"
#define MSK_IINF_ANA_PRO_NUM_VAR_EQ_                        "MSK_IINF_ANA_PRO_NUM_VAR_EQ"
#define MSK_IINF_ANA_PRO_NUM_VAR_FR_                        "MSK_IINF_ANA_PRO_NUM_VAR_FR"
#define MSK_IINF_ANA_PRO_NUM_VAR_INT_                       "MSK_IINF_ANA_PRO_NUM_VAR_INT"
#define MSK_IINF_ANA_PRO_NUM_VAR_LO_                        "MSK_IINF_ANA_PRO_NUM_VAR_LO"
#define MSK_IINF_ANA_PRO_NUM_VAR_RA_                        "MSK_IINF_ANA_PRO_NUM_VAR_RA"
#define MSK_IINF_ANA_PRO_NUM_VAR_UP_                        "MSK_IINF_ANA_PRO_NUM_VAR_UP"
#define MSK_IINF_INTPNT_FACTOR_DIM_DENSE_                   "MSK_IINF_INTPNT_FACTOR_DIM_DENSE"
#define MSK_IINF_INTPNT_ITER_                               "MSK_IINF_INTPNT_ITER"
#define MSK_IINF_INTPNT_NUM_THREADS_                        "MSK_IINF_INTPNT_NUM_THREADS"
#define MSK_IINF_INTPNT_SOLVE_DUAL_                         "MSK_IINF_INTPNT_SOLVE_DUAL"
#define MSK_IINF_MIO_ABSGAP_SATISFIED_                      "MSK_IINF_MIO_ABSGAP_SATISFIED"
#define MSK_IINF_MIO_CLIQUE_TABLE_SIZE_                     "MSK_IINF_MIO_CLIQUE_TABLE_SIZE"
#define MSK_IINF_MIO_CONSTRUCT_SOLUTION_                    "MSK_IINF_MIO_CONSTRUCT_SOLUTION"
#define MSK_IINF_MIO_NODE_DEPTH_                            "MSK_IINF_MIO_NODE_DEPTH"
#define MSK_IINF_MIO_NUM_ACTIVE_NODES_                      "MSK_IINF_MIO_NUM_ACTIVE_NODES"
#define MSK_IINF_MIO_NUM_BRANCH_                            "MSK_IINF_MIO_NUM_BRANCH"
#define MSK_IINF_MIO_NUM_CLIQUE_CUTS_                       "MSK_IINF_MIO_NUM_CLIQUE_CUTS"
#define MSK_IINF_MIO_NUM_CMIR_CUTS_                         "MSK_IINF_MIO_NUM_CMIR_CUTS"
#define MSK_IINF_MIO_NUM_GOMORY_CUTS_                       "MSK_IINF_MIO_NUM_GOMORY_CUTS"
#define MSK_IINF_MIO_NUM_IMPLIED_BOUND_CUTS_                "MSK_IINF_MIO_NUM_IMPLIED_BOUND_CUTS"
#define MSK_IINF_MIO_NUM_INT_SOLUTIONS_                     "MSK_IINF_MIO_NUM_INT_SOLUTIONS"
#define MSK_IINF_MIO_NUM_KNAPSACK_COVER_CUTS_               "MSK_IINF_MIO_NUM_KNAPSACK_COVER_CUTS"
#define MSK_IINF_MIO_NUM_RELAX_                             "MSK_IINF_MIO_NUM_RELAX"
#define MSK_IINF_MIO_NUM_REPEATED_PRESOLVE_                 "MSK_IINF_MIO_NUM_REPEATED_PRESOLVE"
#define MSK_IINF_MIO_NUMBIN_                                "MSK_IINF_MIO_NUMBIN"
#define MSK_IINF_MIO_NUMBINCONEVAR_                         "MSK_IINF_MIO_NUMBINCONEVAR"
#define MSK_IINF_MIO_NUMCON_                                "MSK_IINF_MIO_NUMCON"
#define MSK_IINF_MIO_NUMCONE_                               "MSK_IINF_MIO_NUMCONE"
#define MSK_IINF_MIO_NUMCONEVAR_                            "MSK_IINF_MIO_NUMCONEVAR"
#define MSK_IINF_MIO_NUMCONT_                               "MSK_IINF_MIO_NUMCONT"
#define MSK_IINF_MIO_NUMCONTCONEVAR_                        "MSK_IINF_MIO_NUMCONTCONEVAR"
#define MSK_IINF_MIO_NUMDEXPCONES_                          "MSK_IINF_MIO_NUMDEXPCONES"
#define MSK_IINF_MIO_NUMDPOWCONES_                          "MSK_IINF_MIO_NUMDPOWCONES"
#define MSK_IINF_MIO_NUMINT_                                "MSK_IINF_MIO_NUMINT"
#define MSK_IINF_MIO_NUMINTCONEVAR_                         "MSK_IINF_MIO_NUMINTCONEVAR"
#define MSK_IINF_MIO_NUMPEXPCONES_                          "MSK_IINF_MIO_NUMPEXPCONES"
#define MSK_IINF_MIO_NUMPPOWCONES_                          "MSK_IINF_MIO_NUMPPOWCONES"
#define MSK_IINF_MIO_NUMQCONES_                             "MSK_IINF_MIO_NUMQCONES"
#define MSK_IINF_MIO_NUMRQCONES_                            "MSK_IINF_MIO_NUMRQCONES"
#define MSK_IINF_MIO_NUMVAR_                                "MSK_IINF_MIO_NUMVAR"
#define MSK_IINF_MIO_OBJ_BOUND_DEFINED_                     "MSK_IINF_MIO_OBJ_BOUND_DEFINED"
#define MSK_IINF_MIO_PRESOLVED_NUMBIN_                      "MSK_IINF_MIO_PRESOLVED_NUMBIN"
#define MSK_IINF_MIO_PRESOLVED_NUMBINCONEVAR_               "MSK_IINF_MIO_PRESOLVED_NUMBINCONEVAR"
#define MSK_IINF_MIO_PRESOLVED_NUMCON_                      "MSK_IINF_MIO_PRESOLVED_NUMCON"
#define MSK_IINF_MIO_PRESOLVED_NUMCONE_                     "MSK_IINF_MIO_PRESOLVED_NUMCONE"
#define MSK_IINF_MIO_PRESOLVED_NUMCONEVAR_                  "MSK_IINF_MIO_PRESOLVED_NUMCONEVAR"
#define MSK_IINF_MIO_PRESOLVED_NUMCONT_                     "MSK_IINF_MIO_PRESOLVED_NUMCONT"
#define MSK_IINF_MIO_PRESOLVED_NUMCONTCONEVAR_              "MSK_IINF_MIO_PRESOLVED_NUMCONTCONEVAR"
#define MSK_IINF_MIO_PRESOLVED_NUMDEXPCONES_                "MSK_IINF_MIO_PRESOLVED_NUMDEXPCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMDPOWCONES_                "MSK_IINF_MIO_PRESOLVED_NUMDPOWCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMINT_                      "MSK_IINF_MIO_PRESOLVED_NUMINT"
#define MSK_IINF_MIO_PRESOLVED_NUMINTCONEVAR_               "MSK_IINF_MIO_PRESOLVED_NUMINTCONEVAR"
#define MSK_IINF_MIO_PRESOLVED_NUMPEXPCONES_                "MSK_IINF_MIO_PRESOLVED_NUMPEXPCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMPPOWCONES_                "MSK_IINF_MIO_PRESOLVED_NUMPPOWCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMQCONES_                   "MSK_IINF_MIO_PRESOLVED_NUMQCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMRQCONES_                  "MSK_IINF_MIO_PRESOLVED_NUMRQCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMVAR_                      "MSK_IINF_MIO_PRESOLVED_NUMVAR"
#define MSK_IINF_MIO_RELGAP_SATISFIED_                      "MSK_IINF_MIO_RELGAP_SATISFIED"
#define MSK_IINF_MIO_TOTAL_NUM_CUTS_                        "MSK_IINF_MIO_TOTAL_NUM_CUTS"
#define MSK_IINF_MIO_USER_OBJ_CUT_                          "MSK_IINF_MIO_USER_OBJ_CUT"
#define MSK_IINF_OPT_NUMCON_                                "MSK_IINF_OPT_NUMCON"
#define MSK_IINF_OPT_NUMVAR_                                "MSK_IINF_OPT_NUMVAR"
#define MSK_IINF_OPTIMIZE_RESPONSE_                         "MSK_IINF_OPTIMIZE_RESPONSE"
#define MSK_IINF_PURIFY_DUAL_SUCCESS_                       "MSK_IINF_PURIFY_DUAL_SUCCESS"
#define MSK_IINF_PURIFY_PRIMAL_SUCCESS_                     "MSK_IINF_PURIFY_PRIMAL_SUCCESS"
#define MSK_IINF_RD_NUMBARVAR_                              "MSK_IINF_RD_NUMBARVAR"
#define MSK_IINF_RD_NUMCON_                                 "MSK_IINF_RD_NUMCON"
#define MSK_IINF_RD_NUMCONE_                                "MSK_IINF_RD_NUMCONE"
#define MSK_IINF_RD_NUMINTVAR_                              "MSK_IINF_RD_NUMINTVAR"
#define MSK_IINF_RD_NUMQ_                                   "MSK_IINF_RD_NUMQ"
#define MSK_IINF_RD_NUMVAR_                                 "MSK_IINF_RD_NUMVAR"
#define MSK_IINF_RD_PROTYPE_                                "MSK_IINF_RD_PROTYPE"
#define MSK_IINF_SIM_DUAL_DEG_ITER_                         "MSK_IINF_SIM_DUAL_DEG_ITER"
#define MSK_IINF_SIM_DUAL_HOTSTART_                         "MSK_IINF_SIM_DUAL_HOTSTART"
#define MSK_IINF_SIM_DUAL_HOTSTART_LU_                      "MSK_IINF_SIM_DUAL_HOTSTART_LU"
#define MSK_IINF_SIM_DUAL_INF_ITER_                         "MSK_IINF_SIM_DUAL_INF_ITER"
#define MSK_IINF_SIM_DUAL_ITER_                             "MSK_IINF_SIM_DUAL_ITER"
#define MSK_IINF_SIM_NUMCON_                                "MSK_IINF_SIM_NUMCON"
#define MSK_IINF_SIM_NUMVAR_                                "MSK_IINF_SIM_NUMVAR"
#define MSK_IINF_SIM_PRIMAL_DEG_ITER_                       "MSK_IINF_SIM_PRIMAL_DEG_ITER"
#define MSK_IINF_SIM_PRIMAL_HOTSTART_                       "MSK_IINF_SIM_PRIMAL_HOTSTART"
#define MSK_IINF_SIM_PRIMAL_HOTSTART_LU_                    "MSK_IINF_SIM_PRIMAL_HOTSTART_LU"
#define MSK_IINF_SIM_PRIMAL_INF_ITER_                       "MSK_IINF_SIM_PRIMAL_INF_ITER"
#define MSK_IINF_SIM_PRIMAL_ITER_                           "MSK_IINF_SIM_PRIMAL_ITER"
#define MSK_IINF_SIM_SOLVE_DUAL_                            "MSK_IINF_SIM_SOLVE_DUAL"
#define MSK_IINF_SOL_BAS_PROSTA_                            "MSK_IINF_SOL_BAS_PROSTA"
#define MSK_IINF_SOL_BAS_SOLSTA_                            "MSK_IINF_SOL_BAS_SOLSTA"
#define MSK_IINF_SOL_ITG_PROSTA_                            "MSK_IINF_SOL_ITG_PROSTA"
#define MSK_IINF_SOL_ITG_SOLSTA_                            "MSK_IINF_SOL_ITG_SOLSTA"
#define MSK_IINF_SOL_ITR_PROSTA_                            "MSK_IINF_SOL_ITR_PROSTA"
#define MSK_IINF_SOL_ITR_SOLSTA_                            "MSK_IINF_SOL_ITR_SOLSTA"
#define MSK_IINF_STO_NUM_A_REALLOC_                         "MSK_IINF_STO_NUM_A_REALLOC"

#define MSK_DINF_BI_CLEAN_DUAL_TIME_                        "MSK_DINF_BI_CLEAN_DUAL_TIME"
#define MSK_DINF_BI_CLEAN_PRIMAL_TIME_                      "MSK_DINF_BI_CLEAN_PRIMAL_TIME"
#define MSK_DINF_BI_CLEAN_TIME_                             "MSK_DINF_BI_CLEAN_TIME"
#define MSK_DINF_BI_DUAL_TIME_                              "MSK_DINF_BI_DUAL_TIME"
#define MSK_DINF_BI_PRIMAL_TIME_                            "MSK_DINF_BI_PRIMAL_TIME"
#define MSK_DINF_BI_TIME_                                   "MSK_DINF_BI_TIME"
#define MSK_DINF_INTPNT_DUAL_FEAS_                          "MSK_DINF_INTPNT_DUAL_FEAS"
#define MSK_DINF_INTPNT_DUAL_OBJ_                           "MSK_DINF_INTPNT_DUAL_OBJ"
#define MSK_DINF_INTPNT_FACTOR_NUM_FLOPS_                   "MSK_DINF_INTPNT_FACTOR_NUM_FLOPS"
#define MSK_DINF_INTPNT_OPT_STATUS_                         "MSK_DINF_INTPNT_OPT_STATUS"
#define MSK_DINF_INTPNT_ORDER_TIME_                         "MSK_DINF_INTPNT_ORDER_TIME"
#define MSK_DINF_INTPNT_PRIMAL_FEAS_                        "MSK_DINF_INTPNT_PRIMAL_FEAS"
#define MSK_DINF_INTPNT_PRIMAL_OBJ_                         "MSK_DINF_INTPNT_PRIMAL_OBJ"
#define MSK_DINF_INTPNT_TIME_                               "MSK_DINF_INTPNT_TIME"
#define MSK_DINF_MIO_CLIQUE_SEPARATION_TIME_                "MSK_DINF_MIO_CLIQUE_SEPARATION_TIME"
#define MSK_DINF_MIO_CMIR_SEPARATION_TIME_                  "MSK_DINF_MIO_CMIR_SEPARATION_TIME"
#define MSK_DINF_MIO_CONSTRUCT_SOLUTION_OBJ_                "MSK_DINF_MIO_CONSTRUCT_SOLUTION_OBJ"
#define MSK_DINF_MIO_DUAL_BOUND_AFTER_PRESOLVE_             "MSK_DINF_MIO_DUAL_BOUND_AFTER_PRESOLVE"
#define MSK_DINF_MIO_GMI_SEPARATION_TIME_                   "MSK_DINF_MIO_GMI_SEPARATION_TIME"
#define MSK_DINF_MIO_IMPLIED_BOUND_TIME_                    "MSK_DINF_MIO_IMPLIED_BOUND_TIME"
#define MSK_DINF_MIO_KNAPSACK_COVER_SEPARATION_TIME_        "MSK_DINF_MIO_KNAPSACK_COVER_SEPARATION_TIME"
#define MSK_DINF_MIO_OBJ_ABS_GAP_                           "MSK_DINF_MIO_OBJ_ABS_GAP"
#define MSK_DINF_MIO_OBJ_BOUND_                             "MSK_DINF_MIO_OBJ_BOUND"
#define MSK_DINF_MIO_OBJ_INT_                               "MSK_DINF_MIO_OBJ_INT"
#define MSK_DINF_MIO_OBJ_REL_GAP_                           "MSK_DINF_MIO_OBJ_REL_GAP"
#define MSK_DINF_MIO_PROBING_TIME_                          "MSK_DINF_MIO_PROBING_TIME"
#define MSK_DINF_MIO_ROOT_CUTGEN_TIME_                      "MSK_DINF_MIO_ROOT_CUTGEN_TIME"
#define MSK_DINF_MIO_ROOT_OPTIMIZER_TIME_                   "MSK_DINF_MIO_ROOT_OPTIMIZER_TIME"
#define MSK_DINF_MIO_ROOT_PRESOLVE_TIME_                    "MSK_DINF_MIO_ROOT_PRESOLVE_TIME"
#define MSK_DINF_MIO_TIME_                                  "MSK_DINF_MIO_TIME"
#define MSK_DINF_MIO_USER_OBJ_CUT_                          "MSK_DINF_MIO_USER_OBJ_CUT"
#define MSK_DINF_OPTIMIZER_TIME_                            "MSK_DINF_OPTIMIZER_TIME"
#define MSK_DINF_PRESOLVE_ELI_TIME_                         "MSK_DINF_PRESOLVE_ELI_TIME"
#define MSK_DINF_PRESOLVE_LINDEP_TIME_                      "MSK_DINF_PRESOLVE_LINDEP_TIME"
#define MSK_DINF_PRESOLVE_TIME_                             "MSK_DINF_PRESOLVE_TIME"
#define MSK_DINF_PRIMAL_REPAIR_PENALTY_OBJ_                 "MSK_DINF_PRIMAL_REPAIR_PENALTY_OBJ"
#define MSK_DINF_QCQO_REFORMULATE_MAX_PERTURBATION_         "MSK_DINF_QCQO_REFORMULATE_MAX_PERTURBATION"
#define MSK_DINF_QCQO_REFORMULATE_TIME_                     "MSK_DINF_QCQO_REFORMULATE_TIME"
#define MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_COLUMN_SCALING_ "MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_COLUMN_SCALING"
#define MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_DIAG_SCALING_ "MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_DIAG_SCALING"
#define MSK_DINF_RD_TIME_                                   "MSK_DINF_RD_TIME"
#define MSK_DINF_SIM_DUAL_TIME_                             "MSK_DINF_SIM_DUAL_TIME"
#define MSK_DINF_SIM_FEAS_                                  "MSK_DINF_SIM_FEAS"
#define MSK_DINF_SIM_OBJ_                                   "MSK_DINF_SIM_OBJ"
#define MSK_DINF_SIM_PRIMAL_TIME_                           "MSK_DINF_SIM_PRIMAL_TIME"
#define MSK_DINF_SIM_TIME_                                  "MSK_DINF_SIM_TIME"
#define MSK_DINF_SOL_BAS_DUAL_OBJ_                          "MSK_DINF_SOL_BAS_DUAL_OBJ"
#define MSK_DINF_SOL_BAS_DVIOLCON_                          "MSK_DINF_SOL_BAS_DVIOLCON"
#define MSK_DINF_SOL_BAS_DVIOLVAR_                          "MSK_DINF_SOL_BAS_DVIOLVAR"
#define MSK_DINF_SOL_BAS_NRM_BARX_                          "MSK_DINF_SOL_BAS_NRM_BARX"
#define MSK_DINF_SOL_BAS_NRM_SLC_                           "MSK_DINF_SOL_BAS_NRM_SLC"
#define MSK_DINF_SOL_BAS_NRM_SLX_                           "MSK_DINF_SOL_BAS_NRM_SLX"
#define MSK_DINF_SOL_BAS_NRM_SUC_                           "MSK_DINF_SOL_BAS_NRM_SUC"
#define MSK_DINF_SOL_BAS_NRM_SUX_                           "MSK_DINF_SOL_BAS_NRM_SUX"
#define MSK_DINF_SOL_BAS_NRM_XC_                            "MSK_DINF_SOL_BAS_NRM_XC"
#define MSK_DINF_SOL_BAS_NRM_XX_                            "MSK_DINF_SOL_BAS_NRM_XX"
#define MSK_DINF_SOL_BAS_NRM_Y_                             "MSK_DINF_SOL_BAS_NRM_Y"
#define MSK_DINF_SOL_BAS_PRIMAL_OBJ_                        "MSK_DINF_SOL_BAS_PRIMAL_OBJ"
#define MSK_DINF_SOL_BAS_PVIOLCON_                          "MSK_DINF_SOL_BAS_PVIOLCON"
#define MSK_DINF_SOL_BAS_PVIOLVAR_                          "MSK_DINF_SOL_BAS_PVIOLVAR"
#define MSK_DINF_SOL_ITG_NRM_BARX_                          "MSK_DINF_SOL_ITG_NRM_BARX"
#define MSK_DINF_SOL_ITG_NRM_XC_                            "MSK_DINF_SOL_ITG_NRM_XC"
#define MSK_DINF_SOL_ITG_NRM_XX_                            "MSK_DINF_SOL_ITG_NRM_XX"
#define MSK_DINF_SOL_ITG_PRIMAL_OBJ_                        "MSK_DINF_SOL_ITG_PRIMAL_OBJ"
#define MSK_DINF_SOL_ITG_PVIOLBARVAR_                       "MSK_DINF_SOL_ITG_PVIOLBARVAR"
#define MSK_DINF_SOL_ITG_PVIOLCON_                          "MSK_DINF_SOL_ITG_PVIOLCON"
#define MSK_DINF_SOL_ITG_PVIOLCONES_                        "MSK_DINF_SOL_ITG_PVIOLCONES"
#define MSK_DINF_SOL_ITG_PVIOLITG_                          "MSK_DINF_SOL_ITG_PVIOLITG"
#define MSK_DINF_SOL_ITG_PVIOLVAR_                          "MSK_DINF_SOL_ITG_PVIOLVAR"
#define MSK_DINF_SOL_ITR_DUAL_OBJ_                          "MSK_DINF_SOL_ITR_DUAL_OBJ"
#define MSK_DINF_SOL_ITR_DVIOLBARVAR_                       "MSK_DINF_SOL_ITR_DVIOLBARVAR"
#define MSK_DINF_SOL_ITR_DVIOLCON_                          "MSK_DINF_SOL_ITR_DVIOLCON"
#define MSK_DINF_SOL_ITR_DVIOLCONES_                        "MSK_DINF_SOL_ITR_DVIOLCONES"
#define MSK_DINF_SOL_ITR_DVIOLVAR_                          "MSK_DINF_SOL_ITR_DVIOLVAR"
#define MSK_DINF_SOL_ITR_NRM_BARS_                          "MSK_DINF_SOL_ITR_NRM_BARS"
#define MSK_DINF_SOL_ITR_NRM_BARX_                          "MSK_DINF_SOL_ITR_NRM_BARX"
#define MSK_DINF_SOL_ITR_NRM_SLC_                           "MSK_DINF_SOL_ITR_NRM_SLC"
#define MSK_DINF_SOL_ITR_NRM_SLX_                           "MSK_DINF_SOL_ITR_NRM_SLX"
#define MSK_DINF_SOL_ITR_NRM_SNX_                           "MSK_DINF_SOL_ITR_NRM_SNX"
#define MSK_DINF_SOL_ITR_NRM_SUC_                           "MSK_DINF_SOL_ITR_NRM_SUC"
#define MSK_DINF_SOL_ITR_NRM_SUX_                           "MSK_DINF_SOL_ITR_NRM_SUX"
#define MSK_DINF_SOL_ITR_NRM_XC_                            "MSK_DINF_SOL_ITR_NRM_XC"
#define MSK_DINF_SOL_ITR_NRM_XX_                            "MSK_DINF_SOL_ITR_NRM_XX"
#define MSK_DINF_SOL_ITR_NRM_Y_                             "MSK_DINF_SOL_ITR_NRM_Y"
#define MSK_DINF_SOL_ITR_PRIMAL_OBJ_                        "MSK_DINF_SOL_ITR_PRIMAL_OBJ"
#define MSK_DINF_SOL_ITR_PVIOLBARVAR_                       "MSK_DINF_SOL_ITR_PVIOLBARVAR"
#define MSK_DINF_SOL_ITR_PVIOLCON_                          "MSK_DINF_SOL_ITR_PVIOLCON"
#define MSK_DINF_SOL_ITR_PVIOLCONES_                        "MSK_DINF_SOL_ITR_PVIOLCONES"
#define MSK_DINF_SOL_ITR_PVIOLVAR_                          "MSK_DINF_SOL_ITR_PVIOLVAR"
#define MSK_DINF_TO_CONIC_TIME_                             "MSK_DINF_TO_CONIC_TIME"

#define MSK_LIINF_BI_CLEAN_DUAL_DEG_ITER_                   "MSK_LIINF_BI_CLEAN_DUAL_DEG_ITER"
#define MSK_LIINF_BI_CLEAN_DUAL_ITER_                       "MSK_LIINF_BI_CLEAN_DUAL_ITER"
#define MSK_LIINF_BI_CLEAN_PRIMAL_DEG_ITER_                 "MSK_LIINF_BI_CLEAN_PRIMAL_DEG_ITER"
#define MSK_LIINF_BI_CLEAN_PRIMAL_ITER_                     "MSK_LIINF_BI_CLEAN_PRIMAL_ITER"
#define MSK_LIINF_BI_DUAL_ITER_                             "MSK_LIINF_BI_DUAL_ITER"
#define MSK_LIINF_BI_PRIMAL_ITER_                           "MSK_LIINF_BI_PRIMAL_ITER"
#define MSK_LIINF_INTPNT_FACTOR_NUM_NZ_                     "MSK_LIINF_INTPNT_FACTOR_NUM_NZ"
#define MSK_LIINF_MIO_ANZ_                                  "MSK_LIINF_MIO_ANZ"
#define MSK_LIINF_MIO_INTPNT_ITER_                          "MSK_LIINF_MIO_INTPNT_ITER"
#define MSK_LIINF_MIO_PRESOLVED_ANZ_                        "MSK_LIINF_MIO_PRESOLVED_ANZ"
#define MSK_LIINF_MIO_SIMPLEX_ITER_                         "MSK_LIINF_MIO_SIMPLEX_ITER"
#define MSK_LIINF_RD_NUMANZ_                                "MSK_LIINF_RD_NUMANZ"
#define MSK_LIINF_RD_NUMQNZ_                                "MSK_LIINF_RD_NUMQNZ"



/* Typedefs */

typedef char       MSKchart;
typedef void     * MSKvoid_t;

#ifdef  MSKINT64
typedef MSKINT64 __mskint64;
#else
typedef long long __mskint64;
#endif

#if defined(LLONG_MAX) && LLONG_MAX <= INT_MAX
#warning "Expected (long long) to be a 64bit type. MOSEK API functions may not work."
#endif
typedef int          __mskint32;

/*
typedef unsigned int       __mskuint32;
typedef signed   int       __mskint32;
typedef unsigned long long __mskuint64;
typedef signed   long long __mskint64;
*/

/* Enumeration typedefs */
#ifndef MSK_NO_ENUMS
typedef int                     MSKsolveforme;
typedef enum MSKpurify_enum          MSKpurifye;
typedef int                     MSKsensitivitytypee;
typedef enum MSKsimreform_enum       MSKsimreforme;
typedef enum MSKsimhotstart_enum     MSKsimhotstarte;
typedef enum MSKstreamtype_enum      MSKstreamtypee;
typedef enum MSKsymmattype_enum      MSKsymmattypee;
typedef enum MSKiinfitem_enum        MSKiinfiteme;
typedef int                     MSKdataformate;
typedef enum MSKprosta_enum          MSKprostae;
typedef enum MSKboundkey_enum        MSKboundkeye;
typedef enum MSKliinfitem_enum       MSKliinfiteme;
typedef enum MSKinftype_enum         MSKinftypee;
typedef int                     MSKorderingtypee;
typedef int                     MSKbasindtypee;
typedef enum MSKparametertype_enum   MSKparametertypee;
typedef enum MSKdinfitem_enum        MSKdinfiteme;
typedef int                     MSKmiomodee;
typedef int                     MSKmiocontsoltypee;
typedef int                     MSKscalingmethode;
typedef int                     MSKbranchdire;
typedef enum MSKsolitem_enum         MSKsoliteme;
typedef int                     MSKcheckconvexitytypee;
typedef enum MSKintpnthotstart_enum  MSKintpnthotstarte;
typedef int                     MSKmionodeseltypee;
typedef int                     MSKscalingtypee;
typedef enum MSKproblemtype_enum     MSKproblemtypee;
typedef enum MSKmark_enum            MSKmarke;
typedef enum MSKstakey_enum          MSKstakeye;
typedef enum MSKcallbackcode_enum    MSKcallbackcodee;
typedef enum MSKsoltype_enum         MSKsoltypee;
typedef enum MSKfeature_enum         MSKfeaturee;
typedef enum MSKuplo_enum            MSKuploe;
typedef int                     MSKvaluee;
typedef enum MSKscopr_enum           MSKscopre;
typedef enum MSKproblemitem_enum     MSKproblemiteme;
typedef enum MSKtranspose_enum       MSKtransposee;
typedef int                     MSKsimseltypee;
typedef enum MSKxmlwriteroutputtype_enum MSKxmlwriteroutputtypee;
typedef enum MSKobjsense_enum        MSKobjsensee;
typedef enum MSKrescodetype_enum     MSKrescodetypee;
typedef enum MSKsolsta_enum          MSKsolstae;
typedef enum MSKconetype_enum        MSKconetypee;
typedef int                     MSKonoffkeye;
typedef enum MSKsparam_enum          MSKsparame;
typedef int                     MSKcompresstypee;
typedef int                     MSKstartpointtypee;
typedef enum MSKsimdupvec_enum       MSKsimdupvece;
typedef enum MSKsimdegen_enum        MSKsimdegene;
typedef int                     MSKmpsformate;
typedef enum MSKdparam_enum          MSKdparame;
typedef enum MSKiparam_enum          MSKiparame;
typedef int                     MSKoptimizertypee;
typedef enum MSKrescode_enum         MSKrescodee;
typedef int                     MSKpresolvemodee;
typedef int                     MSKiomodee;
typedef enum MSKnametype_enum        MSKnametypee;
typedef enum MSKvariabletype_enum    MSKvariabletypee;
#else
typedef int                     MSKsolveforme;
typedef int                     MSKpurifye;
typedef int                     MSKsensitivitytypee;
typedef int                     MSKsimreforme;
typedef int                     MSKsimhotstarte;
typedef int                     MSKstreamtypee;
typedef int                     MSKsymmattypee;
typedef int                     MSKiinfiteme;
typedef int                     MSKdataformate;
typedef int                     MSKprostae;
typedef int                     MSKboundkeye;
typedef int                     MSKliinfiteme;
typedef int                     MSKinftypee;
typedef int                     MSKorderingtypee;
typedef int                     MSKbasindtypee;
typedef int                     MSKparametertypee;
typedef int                     MSKdinfiteme;
typedef int                     MSKmiomodee;
typedef int                     MSKmiocontsoltypee;
typedef int                     MSKscalingmethode;
typedef int                     MSKbranchdire;
typedef int                     MSKsoliteme;
typedef int                     MSKcheckconvexitytypee;
typedef int                     MSKintpnthotstarte;
typedef int                     MSKmionodeseltypee;
typedef int                     MSKscalingtypee;
typedef int                     MSKproblemtypee;
typedef int                     MSKmarke;
typedef int                     MSKstakeye;
typedef int                     MSKcallbackcodee;
typedef int                     MSKsoltypee;
typedef int                     MSKfeaturee;
typedef int                     MSKuploe;
typedef int                     MSKvaluee;
typedef int                     MSKscopre;
typedef int                     MSKproblemiteme;
typedef int                     MSKtransposee;
typedef int                     MSKsimseltypee;
typedef int                     MSKxmlwriteroutputtypee;
typedef int                     MSKobjsensee;
typedef int                     MSKrescodetypee;
typedef int                     MSKsolstae;
typedef int                     MSKconetypee;
typedef int                     MSKonoffkeye;
typedef int                     MSKsparame;
typedef int                     MSKcompresstypee;
typedef int                     MSKstartpointtypee;
typedef int                     MSKsimdupvece;
typedef int                     MSKsimdegene;
typedef int                     MSKmpsformate;
typedef int                     MSKdparame;
typedef int                     MSKiparame;
typedef int                     MSKoptimizertypee;
typedef int                     MSKrescodee;
typedef int                     MSKpresolvemodee;
typedef int                     MSKiomodee;
typedef int                     MSKnametypee;
typedef int                     MSKvariabletypee;
#endif

/* Simple typedefs */
typedef void * MSKenv_t;

typedef void * MSKtask_t;

typedef void * MSKuserhandle_t;

typedef int MSKbooleant;

typedef __mskint32 MSKint32t;

typedef __mskint64 MSKint64t;

typedef wchar_t MSKwchart;

typedef double MSKrealt;

typedef char * MSKstring_t;

/* Function typedefs */
typedef MSKint32t  (MSKAPI * MSKcallbackfunc) (
	MSKtask_t task,
	MSKuserhandle_t usrptr,
	MSKcallbackcodee caller,
	const MSKrealt * douinf,
	const MSKint32t * intinf,
	const MSKint64t * lintinf);

typedef void  (MSKAPI * MSKexitfunc) (
	MSKuserhandle_t usrptr,
	const char * file,
	MSKint32t line,
	const char * msg);

typedef void  (MSKAPI * MSKfreefunc) (
	MSKuserhandle_t usrptr,
	void * buffer);

typedef void *  (MSKAPI * MSKmallocfunc) (
	MSKuserhandle_t usrptr,
	const size_t size);

typedef void *  (MSKAPI * MSKcallocfunc) (
	MSKuserhandle_t usrptr,
	const size_t num,
	const size_t size);

typedef void *  (MSKAPI * MSKreallocfunc) (
	MSKuserhandle_t usrptr,
	void * ptr,
	const size_t size);

typedef void  (MSKAPI * MSKstreamfunc) (
	MSKuserhandle_t handle,
	const char * str);

typedef MSKrescodee  (MSKAPI * MSKresponsefunc) (
	MSKuserhandle_t handle,
	MSKrescodee r,
	const char * msg);

typedef size_t  (MSKAPI * MSKhreadfunc) (
	MSKuserhandle_t handle,
	void * dest,
	const size_t count);

typedef size_t  (MSKAPI * MSKhwritefunc) (
	MSKuserhandle_t handle,
	void * src,
	const size_t count);




/* Functions */

/* using __cplusplus */
#ifdef __cplusplus
extern "C" {
#endif

/* MSK_analyzeproblem */
MSKrescodee (MSKAPI MSK_analyzeproblem) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_analyzenames */
MSKrescodee (MSKAPI MSK_analyzenames) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	MSKnametypee nametype);

/* MSK_analyzesolution */
MSKrescodee (MSKAPI MSK_analyzesolution) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	MSKsoltypee whichsol);

/* MSK_initbasissolve */
MSKrescodee (MSKAPI MSK_initbasissolve) (
	MSKtask_t task,
	MSKint32t * basis);

/* MSK_solvewithbasis */
MSKrescodee (MSKAPI MSK_solvewithbasis) (
	MSKtask_t task,
	MSKint32t transp,
	MSKint32t * numnz,
	MSKint32t * sub,
	MSKrealt * val);

/* MSK_basiscond */
MSKrescodee (MSKAPI MSK_basiscond) (
	MSKtask_t task,
	MSKrealt * nrmbasis,
	MSKrealt * nrminvbasis);

/* MSK_appendcons */
MSKrescodee (MSKAPI MSK_appendcons) (
	MSKtask_t task,
	MSKint32t num);

/* MSK_appendvars */
MSKrescodee (MSKAPI MSK_appendvars) (
	MSKtask_t task,
	MSKint32t num);

/* MSK_removecons */
MSKrescodee (MSKAPI MSK_removecons) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subset);

/* MSK_removevars */
MSKrescodee (MSKAPI MSK_removevars) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subset);

/* MSK_removebarvars */
MSKrescodee (MSKAPI MSK_removebarvars) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subset);

/* MSK_removecones */
MSKrescodee (MSKAPI MSK_removecones) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subset);

/* MSK_appendbarvars */
MSKrescodee (MSKAPI MSK_appendbarvars) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * dim);

/* MSK_appendcone */
MSKrescodee (MSKAPI MSK_appendcone) (
	MSKtask_t task,
	MSKconetypee ct,
	MSKrealt conepar,
	MSKint32t nummem,
	const MSKint32t * submem);

/* MSK_appendconeseq */
MSKrescodee (MSKAPI MSK_appendconeseq) (
	MSKtask_t task,
	MSKconetypee ct,
	MSKrealt conepar,
	MSKint32t nummem,
	MSKint32t j);

/* MSK_appendconesseq */
MSKrescodee (MSKAPI MSK_appendconesseq) (
	MSKtask_t task,
	MSKint32t num,
	const MSKconetypee * ct,
	const MSKrealt * conepar,
	const MSKint32t * nummem,
	MSKint32t j);

/* MSK_bktostr */
MSKrescodee (MSKAPI MSK_bktostr) (
	MSKtask_t task,
	MSKboundkeye bk,
	char * str);

/* MSK_callbackcodetostr */
MSKrescodee (MSKAPI MSK_callbackcodetostr) (
	MSKcallbackcodee code,
	char * callbackcodestr);

/* MSK_calloctask */
void * (MSKAPI MSK_calloctask) (
	MSKtask_t task,
	const size_t number,
	const size_t size);

/* MSK_callocdbgtask */
void * (MSKAPI MSK_callocdbgtask) (
	MSKtask_t task,
	const size_t number,
	const size_t size,
	const char * file,
	const unsigned line);

/* MSK_chgconbound */
MSKrescodee (MSKAPI MSK_chgconbound) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t lower,
	MSKint32t finite,
	MSKrealt value);

/* MSK_chgvarbound */
MSKrescodee (MSKAPI MSK_chgvarbound) (
	MSKtask_t task,
	MSKint32t j,
	MSKint32t lower,
	MSKint32t finite,
	MSKrealt value);

/* MSK_conetypetostr */
MSKrescodee (MSKAPI MSK_conetypetostr) (
	MSKtask_t task,
	MSKconetypee ct,
	char * str);

/* MSK_deletetask */
MSKrescodee (MSKAPI MSK_deletetask) (
	MSKtask_t * task);

/* MSK_echotask */
MSKrescodee (MSKAPIVA MSK_echotask) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	const char * format,
	...);

/* MSK_freetask */
void (MSKAPI MSK_freetask) (
	MSKtask_t task,
	void * buffer);

/* MSK_freedbgtask */
void (MSKAPI MSK_freedbgtask) (
	MSKtask_t task,
	void * buffer,
	const char * file,
	const unsigned line);

/* MSK_getaij */
MSKrescodee (MSKAPI MSK_getaij) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t j,
	MSKrealt * aij);

/* MSK_getapiecenumnz */
MSKrescodee (MSKAPI MSK_getapiecenumnz) (
	MSKtask_t task,
	MSKint32t firsti,
	MSKint32t lasti,
	MSKint32t firstj,
	MSKint32t lastj,
	MSKint32t * numnz);

/* MSK_getacolnumnz */
MSKrescodee (MSKAPI MSK_getacolnumnz) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * nzj);

/* MSK_getacol */
MSKrescodee (MSKAPI MSK_getacol) (
	MSKtask_t task,
	MSKint32t j,
	MSKint32t * nzj,
	MSKint32t * subj,
	MSKrealt * valj);

/* MSK_getacolslice */
MSKrescodee (MSKAPI MSK_getacolslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint32t maxnumnz,
	MSKint32t * surp,
	MSKint32t * ptrb,
	MSKint32t * ptre,
	MSKint32t * sub,
	MSKrealt * val);

/* MSK_getacolslice64 */
MSKrescodee (MSKAPI MSK_getacolslice64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t maxnumnz,
	MSKint64t * surp,
	MSKint64t * ptrb,
	MSKint64t * ptre,
	MSKint32t * sub,
	MSKrealt * val);

/* MSK_getarownumnz */
MSKrescodee (MSKAPI MSK_getarownumnz) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * nzi);

/* MSK_getarow */
MSKrescodee (MSKAPI MSK_getarow) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * nzi,
	MSKint32t * subi,
	MSKrealt * vali);

/* MSK_getacolslicenumnz */
MSKrescodee (MSKAPI MSK_getacolslicenumnz) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint32t * numnz);

/* MSK_getacolslicenumnz64 */
MSKrescodee (MSKAPI MSK_getacolslicenumnz64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t * numnz);

/* MSK_getarowslicenumnz */
MSKrescodee (MSKAPI MSK_getarowslicenumnz) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint32t * numnz);

/* MSK_getarowslicenumnz64 */
MSKrescodee (MSKAPI MSK_getarowslicenumnz64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t * numnz);

/* MSK_getarowslice */
MSKrescodee (MSKAPI MSK_getarowslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint32t maxnumnz,
	MSKint32t * surp,
	MSKint32t * ptrb,
	MSKint32t * ptre,
	MSKint32t * sub,
	MSKrealt * val);

/* MSK_getarowslice64 */
MSKrescodee (MSKAPI MSK_getarowslice64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t maxnumnz,
	MSKint64t * surp,
	MSKint64t * ptrb,
	MSKint64t * ptre,
	MSKint32t * sub,
	MSKrealt * val);

/* MSK_getarowslicetrip */
MSKrescodee (MSKAPI MSK_getarowslicetrip) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t maxnumnz,
	MSKint64t * surp,
	MSKint32t * subi,
	MSKint32t * subj,
	MSKrealt * val);

/* MSK_getacolslicetrip */
MSKrescodee (MSKAPI MSK_getacolslicetrip) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t maxnumnz,
	MSKint64t * surp,
	MSKint32t * subi,
	MSKint32t * subj,
	MSKrealt * val);

/* MSK_getconbound */
MSKrescodee (MSKAPI MSK_getconbound) (
	MSKtask_t task,
	MSKint32t i,
	MSKboundkeye * bk,
	MSKrealt * bl,
	MSKrealt * bu);

/* MSK_getvarbound */
MSKrescodee (MSKAPI MSK_getvarbound) (
	MSKtask_t task,
	MSKint32t i,
	MSKboundkeye * bk,
	MSKrealt * bl,
	MSKrealt * bu);

/* MSK_getconboundslice */
MSKrescodee (MSKAPI MSK_getconboundslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKboundkeye * bk,
	MSKrealt * bl,
	MSKrealt * bu);

/* MSK_getvarboundslice */
MSKrescodee (MSKAPI MSK_getvarboundslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKboundkeye * bk,
	MSKrealt * bl,
	MSKrealt * bu);

/* MSK_getcj */
MSKrescodee (MSKAPI MSK_getcj) (
	MSKtask_t task,
	MSKint32t j,
	MSKrealt * cj);

/* MSK_getc */
MSKrescodee (MSKAPI MSK_getc) (
	MSKtask_t task,
	MSKrealt * c);

/* MSK_getcallbackfunc */
MSKrescodee (MSKAPI MSK_getcallbackfunc) (
	MSKtask_t task,
	MSKcallbackfunc * func,
	MSKuserhandle_t * handle);

/* MSK_getcfix */
MSKrescodee (MSKAPI MSK_getcfix) (
	MSKtask_t task,
	MSKrealt * cfix);

/* MSK_getcone */
MSKrescodee (MSKAPI MSK_getcone) (
	MSKtask_t task,
	MSKint32t k,
	MSKconetypee * ct,
	MSKrealt * conepar,
	MSKint32t * nummem,
	MSKint32t * submem);

/* MSK_getconeinfo */
MSKrescodee (MSKAPI MSK_getconeinfo) (
	MSKtask_t task,
	MSKint32t k,
	MSKconetypee * ct,
	MSKrealt * conepar,
	MSKint32t * nummem);

/* MSK_getclist */
MSKrescodee (MSKAPI MSK_getclist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	MSKrealt * c);

/* MSK_getcslice */
MSKrescodee (MSKAPI MSK_getcslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * c);

/* MSK_getdouinf */
MSKrescodee (MSKAPI MSK_getdouinf) (
	MSKtask_t task,
	MSKdinfiteme whichdinf,
	MSKrealt * dvalue);

/* MSK_getdouparam */
MSKrescodee (MSKAPI MSK_getdouparam) (
	MSKtask_t task,
	MSKdparame param,
	MSKrealt * parvalue);

/* MSK_getdualobj */
MSKrescodee (MSKAPI MSK_getdualobj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * dualobj);

/* MSK_getenv */
MSKrescodee (MSKAPI MSK_getenv) (
	MSKtask_t task,
	MSKenv_t * env);

/* MSK_getinfindex */
MSKrescodee (MSKAPI MSK_getinfindex) (
	MSKtask_t task,
	MSKinftypee inftype,
	const char * infname,
	MSKint32t * infindex);

/* MSK_getinfmax */
MSKrescodee (MSKAPI MSK_getinfmax) (
	MSKtask_t task,
	MSKinftypee inftype,
	MSKint32t * infmax);

/* MSK_getinfname */
MSKrescodee (MSKAPI MSK_getinfname) (
	MSKtask_t task,
	MSKinftypee inftype,
	MSKint32t whichinf,
	char * infname);

/* MSK_getintinf */
MSKrescodee (MSKAPI MSK_getintinf) (
	MSKtask_t task,
	MSKiinfiteme whichiinf,
	MSKint32t * ivalue);

/* MSK_getlintinf */
MSKrescodee (MSKAPI MSK_getlintinf) (
	MSKtask_t task,
	MSKliinfiteme whichliinf,
	MSKint64t * ivalue);

/* MSK_getintparam */
MSKrescodee (MSKAPI MSK_getintparam) (
	MSKtask_t task,
	MSKiparame param,
	MSKint32t * parvalue);

/* MSK_getmaxnamelen */
MSKrescodee (MSKAPI MSK_getmaxnamelen) (
	MSKtask_t task,
	MSKint32t * maxlen);

/* MSK_getmaxnumanz */
MSKrescodee (MSKAPI MSK_getmaxnumanz) (
	MSKtask_t task,
	MSKint32t * maxnumanz);

/* MSK_getmaxnumanz64 */
MSKrescodee (MSKAPI MSK_getmaxnumanz64) (
	MSKtask_t task,
	MSKint64t * maxnumanz);

/* MSK_getmaxnumcon */
MSKrescodee (MSKAPI MSK_getmaxnumcon) (
	MSKtask_t task,
	MSKint32t * maxnumcon);

/* MSK_getmaxnumvar */
MSKrescodee (MSKAPI MSK_getmaxnumvar) (
	MSKtask_t task,
	MSKint32t * maxnumvar);

/* MSK_getnadouinf */
MSKrescodee (MSKAPI MSK_getnadouinf) (
	MSKtask_t task,
	const char * infitemname,
	MSKrealt * dvalue);

/* MSK_getnadouparam */
MSKrescodee (MSKAPI MSK_getnadouparam) (
	MSKtask_t task,
	const char * paramname,
	MSKrealt * parvalue);

/* MSK_getnaintinf */
MSKrescodee (MSKAPI MSK_getnaintinf) (
	MSKtask_t task,
	const char * infitemname,
	MSKint32t * ivalue);

/* MSK_getnaintparam */
MSKrescodee (MSKAPI MSK_getnaintparam) (
	MSKtask_t task,
	const char * paramname,
	MSKint32t * parvalue);

/* MSK_getbarvarnamelen */
MSKrescodee (MSKAPI MSK_getbarvarnamelen) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * len);

/* MSK_getbarvarname */
MSKrescodee (MSKAPI MSK_getbarvarname) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t sizename,
	char * name);

/* MSK_getbarvarnameindex */
MSKrescodee (MSKAPI MSK_getbarvarnameindex) (
	MSKtask_t task,
	const char * somename,
	MSKint32t * asgn,
	MSKint32t * index);

/* MSK_generatevarnames */
MSKrescodee (MSKAPI MSK_generatevarnames) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	const char * fmt,
	MSKint32t ndims,
	const MSKint32t * dims,
	const MSKint64t * sp);

/* MSK_generateconnames */
MSKrescodee (MSKAPI MSK_generateconnames) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subi,
	const char * fmt,
	MSKint32t ndims,
	const MSKint32t * dims,
	const MSKint64t * sp);

/* MSK_generateconenames */
MSKrescodee (MSKAPI MSK_generateconenames) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subk,
	const char * fmt,
	MSKint32t ndims,
	const MSKint32t * dims,
	const MSKint64t * sp);

/* MSK_putconname */
MSKrescodee (MSKAPI MSK_putconname) (
	MSKtask_t task,
	MSKint32t i,
	const char * name);

/* MSK_putvarname */
MSKrescodee (MSKAPI MSK_putvarname) (
	MSKtask_t task,
	MSKint32t j,
	const char * name);

/* MSK_putconename */
MSKrescodee (MSKAPI MSK_putconename) (
	MSKtask_t task,
	MSKint32t j,
	const char * name);

/* MSK_putbarvarname */
MSKrescodee (MSKAPI MSK_putbarvarname) (
	MSKtask_t task,
	MSKint32t j,
	const char * name);

/* MSK_getvarnamelen */
MSKrescodee (MSKAPI MSK_getvarnamelen) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * len);

/* MSK_getvarname */
MSKrescodee (MSKAPI MSK_getvarname) (
	MSKtask_t task,
	MSKint32t j,
	MSKint32t sizename,
	char * name);

/* MSK_getconnamelen */
MSKrescodee (MSKAPI MSK_getconnamelen) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * len);

/* MSK_getconname */
MSKrescodee (MSKAPI MSK_getconname) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t sizename,
	char * name);

/* MSK_getconnameindex */
MSKrescodee (MSKAPI MSK_getconnameindex) (
	MSKtask_t task,
	const char * somename,
	MSKint32t * asgn,
	MSKint32t * index);

/* MSK_getvarnameindex */
MSKrescodee (MSKAPI MSK_getvarnameindex) (
	MSKtask_t task,
	const char * somename,
	MSKint32t * asgn,
	MSKint32t * index);

/* MSK_getconenamelen */
MSKrescodee (MSKAPI MSK_getconenamelen) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * len);

/* MSK_getconename */
MSKrescodee (MSKAPI MSK_getconename) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t sizename,
	char * name);

/* MSK_getconenameindex */
MSKrescodee (MSKAPI MSK_getconenameindex) (
	MSKtask_t task,
	const char * somename,
	MSKint32t * asgn,
	MSKint32t * index);

/* MSK_getnastrparam */
MSKrescodee (MSKAPI MSK_getnastrparam) (
	MSKtask_t task,
	const char * paramname,
	MSKint32t sizeparamname,
	MSKint32t * len,
	char * parvalue);

/* MSK_getnumanz */
MSKrescodee (MSKAPI MSK_getnumanz) (
	MSKtask_t task,
	MSKint32t * numanz);

/* MSK_getnumanz64 */
MSKrescodee (MSKAPI MSK_getnumanz64) (
	MSKtask_t task,
	MSKint64t * numanz);

/* MSK_getnumcon */
MSKrescodee (MSKAPI MSK_getnumcon) (
	MSKtask_t task,
	MSKint32t * numcon);

/* MSK_getnumcone */
MSKrescodee (MSKAPI MSK_getnumcone) (
	MSKtask_t task,
	MSKint32t * numcone);

/* MSK_getnumconemem */
MSKrescodee (MSKAPI MSK_getnumconemem) (
	MSKtask_t task,
	MSKint32t k,
	MSKint32t * nummem);

/* MSK_getnumintvar */
MSKrescodee (MSKAPI MSK_getnumintvar) (
	MSKtask_t task,
	MSKint32t * numintvar);

/* MSK_getnumparam */
MSKrescodee (MSKAPI MSK_getnumparam) (
	MSKtask_t task,
	MSKparametertypee partype,
	MSKint32t * numparam);

/* MSK_getnumqconknz */
MSKrescodee (MSKAPI MSK_getnumqconknz) (
	MSKtask_t task,
	MSKint32t k,
	MSKint32t * numqcnz);

/* MSK_getnumqconknz64 */
MSKrescodee (MSKAPI MSK_getnumqconknz64) (
	MSKtask_t task,
	MSKint32t k,
	MSKint64t * numqcnz);

/* MSK_getnumqobjnz */
MSKrescodee (MSKAPI MSK_getnumqobjnz) (
	MSKtask_t task,
	MSKint32t * numqonz);

/* MSK_getnumqobjnz64 */
MSKrescodee (MSKAPI MSK_getnumqobjnz64) (
	MSKtask_t task,
	MSKint64t * numqonz);

/* MSK_getnumvar */
MSKrescodee (MSKAPI MSK_getnumvar) (
	MSKtask_t task,
	MSKint32t * numvar);

/* MSK_getnumbarvar */
MSKrescodee (MSKAPI MSK_getnumbarvar) (
	MSKtask_t task,
	MSKint32t * numbarvar);

/* MSK_getmaxnumbarvar */
MSKrescodee (MSKAPI MSK_getmaxnumbarvar) (
	MSKtask_t task,
	MSKint32t * maxnumbarvar);

/* MSK_getdimbarvarj */
MSKrescodee (MSKAPI MSK_getdimbarvarj) (
	MSKtask_t task,
	MSKint32t j,
	MSKint32t * dimbarvarj);

/* MSK_getlenbarvarj */
MSKrescodee (MSKAPI MSK_getlenbarvarj) (
	MSKtask_t task,
	MSKint32t j,
	MSKint64t * lenbarvarj);

/* MSK_getobjname */
MSKrescodee (MSKAPI MSK_getobjname) (
	MSKtask_t task,
	MSKint32t sizeobjname,
	char * objname);

/* MSK_getobjnamelen */
MSKrescodee (MSKAPI MSK_getobjnamelen) (
	MSKtask_t task,
	MSKint32t * len);

/* MSK_getparamname */
MSKrescodee (MSKAPI MSK_getparamname) (
	MSKtask_t task,
	MSKparametertypee partype,
	MSKint32t param,
	char * parname);

/* MSK_getparammax */
MSKrescodee (MSKAPI MSK_getparammax) (
	MSKtask_t task,
	MSKparametertypee partype,
	MSKint32t * parammax);

/* MSK_getprimalobj */
MSKrescodee (MSKAPI MSK_getprimalobj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * primalobj);

/* MSK_getprobtype */
MSKrescodee (MSKAPI MSK_getprobtype) (
	MSKtask_t task,
	MSKproblemtypee * probtype);

/* MSK_getqconk64 */
MSKrescodee (MSKAPI MSK_getqconk64) (
	MSKtask_t task,
	MSKint32t k,
	MSKint64t maxnumqcnz,
	MSKint64t * qcsurp,
	MSKint64t * numqcnz,
	MSKint32t * qcsubi,
	MSKint32t * qcsubj,
	MSKrealt * qcval);

/* MSK_getqconk */
MSKrescodee (MSKAPI MSK_getqconk) (
	MSKtask_t task,
	MSKint32t k,
	MSKint32t maxnumqcnz,
	MSKint32t * qcsurp,
	MSKint32t * numqcnz,
	MSKint32t * qcsubi,
	MSKint32t * qcsubj,
	MSKrealt * qcval);

/* MSK_getqobj */
MSKrescodee (MSKAPI MSK_getqobj) (
	MSKtask_t task,
	MSKint32t maxnumqonz,
	MSKint32t * qosurp,
	MSKint32t * numqonz,
	MSKint32t * qosubi,
	MSKint32t * qosubj,
	MSKrealt * qoval);

/* MSK_getqobj64 */
MSKrescodee (MSKAPI MSK_getqobj64) (
	MSKtask_t task,
	MSKint64t maxnumqonz,
	MSKint64t * qosurp,
	MSKint64t * numqonz,
	MSKint32t * qosubi,
	MSKint32t * qosubj,
	MSKrealt * qoval);

/* MSK_getqobjij */
MSKrescodee (MSKAPI MSK_getqobjij) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t j,
	MSKrealt * qoij);

/* MSK_getsolution */
MSKrescodee (MSKAPI MSK_getsolution) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKprostae * prosta,
	MSKsolstae * solsta,
	MSKstakeye * skc,
	MSKstakeye * skx,
	MSKstakeye * skn,
	MSKrealt * xc,
	MSKrealt * xx,
	MSKrealt * y,
	MSKrealt * slc,
	MSKrealt * suc,
	MSKrealt * slx,
	MSKrealt * sux,
	MSKrealt * snx);

/* MSK_getsolsta */
MSKrescodee (MSKAPI MSK_getsolsta) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKsolstae * solsta);

/* MSK_getprosta */
MSKrescodee (MSKAPI MSK_getprosta) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKprostae * prosta);

/* MSK_getskc */
MSKrescodee (MSKAPI MSK_getskc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKstakeye * skc);

/* MSK_getskx */
MSKrescodee (MSKAPI MSK_getskx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKstakeye * skx);

/* MSK_getskn */
MSKrescodee (MSKAPI MSK_getskn) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKstakeye * skn);

/* MSK_getxc */
MSKrescodee (MSKAPI MSK_getxc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * xc);

/* MSK_getxx */
MSKrescodee (MSKAPI MSK_getxx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * xx);

/* MSK_gety */
MSKrescodee (MSKAPI MSK_gety) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * y);

/* MSK_getslc */
MSKrescodee (MSKAPI MSK_getslc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * slc);

/* MSK_getsuc */
MSKrescodee (MSKAPI MSK_getsuc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * suc);

/* MSK_getslx */
MSKrescodee (MSKAPI MSK_getslx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * slx);

/* MSK_getsux */
MSKrescodee (MSKAPI MSK_getsux) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * sux);

/* MSK_getsnx */
MSKrescodee (MSKAPI MSK_getsnx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * snx);

/* MSK_getskcslice */
MSKrescodee (MSKAPI MSK_getskcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKstakeye * skc);

/* MSK_getskxslice */
MSKrescodee (MSKAPI MSK_getskxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKstakeye * skx);

/* MSK_getxcslice */
MSKrescodee (MSKAPI MSK_getxcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * xc);

/* MSK_getxxslice */
MSKrescodee (MSKAPI MSK_getxxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * xx);

/* MSK_getyslice */
MSKrescodee (MSKAPI MSK_getyslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * y);

/* MSK_getslcslice */
MSKrescodee (MSKAPI MSK_getslcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * slc);

/* MSK_getsucslice */
MSKrescodee (MSKAPI MSK_getsucslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * suc);

/* MSK_getslxslice */
MSKrescodee (MSKAPI MSK_getslxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * slx);

/* MSK_getsuxslice */
MSKrescodee (MSKAPI MSK_getsuxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * sux);

/* MSK_getsnxslice */
MSKrescodee (MSKAPI MSK_getsnxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * snx);

/* MSK_getbarxj */
MSKrescodee (MSKAPI MSK_getbarxj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t j,
	MSKrealt * barxj);

/* MSK_getbarxslice */
MSKrescodee (MSKAPI MSK_getbarxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKint64t slicesize,
	MSKrealt * barxslice);

/* MSK_getbarsj */
MSKrescodee (MSKAPI MSK_getbarsj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t j,
	MSKrealt * barsj);

/* MSK_getbarsslice */
MSKrescodee (MSKAPI MSK_getbarsslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKint64t slicesize,
	MSKrealt * barsslice);

/* MSK_putskc */
MSKrescodee (MSKAPI MSK_putskc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKstakeye * skc);

/* MSK_putskx */
MSKrescodee (MSKAPI MSK_putskx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKstakeye * skx);

/* MSK_putxc */
MSKrescodee (MSKAPI MSK_putxc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * xc);

/* MSK_putxx */
MSKrescodee (MSKAPI MSK_putxx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * xx);

/* MSK_puty */
MSKrescodee (MSKAPI MSK_puty) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * y);

/* MSK_putslc */
MSKrescodee (MSKAPI MSK_putslc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * slc);

/* MSK_putsuc */
MSKrescodee (MSKAPI MSK_putsuc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * suc);

/* MSK_putslx */
MSKrescodee (MSKAPI MSK_putslx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * slx);

/* MSK_putsux */
MSKrescodee (MSKAPI MSK_putsux) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * sux);

/* MSK_putsnx */
MSKrescodee (MSKAPI MSK_putsnx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * sux);

/* MSK_putskcslice */
MSKrescodee (MSKAPI MSK_putskcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKstakeye * skc);

/* MSK_putskxslice */
MSKrescodee (MSKAPI MSK_putskxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKstakeye * skx);

/* MSK_putxcslice */
MSKrescodee (MSKAPI MSK_putxcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * xc);

/* MSK_putxxslice */
MSKrescodee (MSKAPI MSK_putxxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * xx);

/* MSK_putyslice */
MSKrescodee (MSKAPI MSK_putyslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * y);

/* MSK_putslcslice */
MSKrescodee (MSKAPI MSK_putslcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * slc);

/* MSK_putsucslice */
MSKrescodee (MSKAPI MSK_putsucslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * suc);

/* MSK_putslxslice */
MSKrescodee (MSKAPI MSK_putslxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * slx);

/* MSK_putsuxslice */
MSKrescodee (MSKAPI MSK_putsuxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * sux);

/* MSK_putsnxslice */
MSKrescodee (MSKAPI MSK_putsnxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * snx);

/* MSK_putbarxj */
MSKrescodee (MSKAPI MSK_putbarxj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t j,
	const MSKrealt * barxj);

/* MSK_putbarsj */
MSKrescodee (MSKAPI MSK_putbarsj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t j,
	const MSKrealt * barsj);

/* MSK_getpviolcon */
MSKrescodee (MSKAPI MSK_getpviolcon) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getpviolvar */
MSKrescodee (MSKAPI MSK_getpviolvar) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getpviolbarvar */
MSKrescodee (MSKAPI MSK_getpviolbarvar) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getpviolcones */
MSKrescodee (MSKAPI MSK_getpviolcones) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getdviolcon */
MSKrescodee (MSKAPI MSK_getdviolcon) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getdviolvar */
MSKrescodee (MSKAPI MSK_getdviolvar) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getdviolbarvar */
MSKrescodee (MSKAPI MSK_getdviolbarvar) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getdviolcones */
MSKrescodee (MSKAPI MSK_getdviolcones) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getsolutioninfo */
MSKrescodee (MSKAPI MSK_getsolutioninfo) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * pobj,
	MSKrealt * pviolcon,
	MSKrealt * pviolvar,
	MSKrealt * pviolbarvar,
	MSKrealt * pviolcone,
	MSKrealt * pviolitg,
	MSKrealt * dobj,
	MSKrealt * dviolcon,
	MSKrealt * dviolvar,
	MSKrealt * dviolbarvar,
	MSKrealt * dviolcone);

/* MSK_getdualsolutionnorms */
MSKrescodee (MSKAPI MSK_getdualsolutionnorms) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * nrmy,
	MSKrealt * nrmslc,
	MSKrealt * nrmsuc,
	MSKrealt * nrmslx,
	MSKrealt * nrmsux,
	MSKrealt * nrmsnx,
	MSKrealt * nrmbars);

/* MSK_getprimalsolutionnorms */
MSKrescodee (MSKAPI MSK_getprimalsolutionnorms) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * nrmxc,
	MSKrealt * nrmxx,
	MSKrealt * nrmbarx);

/* MSK_getsolutionslice */
MSKrescodee (MSKAPI MSK_getsolutionslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKsoliteme solitem,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * values);

/* MSK_getreducedcosts */
MSKrescodee (MSKAPI MSK_getreducedcosts) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * redcosts);

/* MSK_getstrparam */
MSKrescodee (MSKAPI MSK_getstrparam) (
	MSKtask_t task,
	MSKsparame param,
	MSKint32t maxlen,
	MSKint32t * len,
	char * parvalue);

/* MSK_getstrparamlen */
MSKrescodee (MSKAPI MSK_getstrparamlen) (
	MSKtask_t task,
	MSKsparame param,
	MSKint32t * len);

/* MSK_getstrparamal */
MSKrescodee (MSKAPI MSK_getstrparamal) (
	MSKtask_t task,
	MSKsparame param,
	MSKint32t numaddchr,
	MSKstring_t * value);

/* MSK_getnastrparamal */
MSKrescodee (MSKAPI MSK_getnastrparamal) (
	MSKtask_t task,
	const char * paramname,
	MSKint32t numaddchr,
	MSKstring_t * value);

/* MSK_getsymbcon */
MSKrescodee (MSKAPI MSK_getsymbcon) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t sizevalue,
	char * name,
	MSKint32t * value);

/* MSK_gettasknamelen */
MSKrescodee (MSKAPI MSK_gettasknamelen) (
	MSKtask_t task,
	MSKint32t * len);

/* MSK_gettaskname */
MSKrescodee (MSKAPI MSK_gettaskname) (
	MSKtask_t task,
	MSKint32t sizetaskname,
	char * taskname);

/* MSK_getvartype */
MSKrescodee (MSKAPI MSK_getvartype) (
	MSKtask_t task,
	MSKint32t j,
	MSKvariabletypee * vartype);

/* MSK_getvartypelist */
MSKrescodee (MSKAPI MSK_getvartypelist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	MSKvariabletypee * vartype);

/* MSK_inputdata */
MSKrescodee (MSKAPI MSK_inputdata) (
	MSKtask_t task,
	MSKint32t maxnumcon,
	MSKint32t maxnumvar,
	MSKint32t numcon,
	MSKint32t numvar,
	const MSKrealt * c,
	MSKrealt cfix,
	const MSKint32t * aptrb,
	const MSKint32t * aptre,
	const MSKint32t * asub,
	const MSKrealt * aval,
	const MSKboundkeye * bkc,
	const MSKrealt * blc,
	const MSKrealt * buc,
	const MSKboundkeye * bkx,
	const MSKrealt * blx,
	const MSKrealt * bux);

/* MSK_inputdata64 */
MSKrescodee (MSKAPI MSK_inputdata64) (
	MSKtask_t task,
	MSKint32t maxnumcon,
	MSKint32t maxnumvar,
	MSKint32t numcon,
	MSKint32t numvar,
	const MSKrealt * c,
	MSKrealt cfix,
	const MSKint64t * aptrb,
	const MSKint64t * aptre,
	const MSKint32t * asub,
	const MSKrealt * aval,
	const MSKboundkeye * bkc,
	const MSKrealt * blc,
	const MSKrealt * buc,
	const MSKboundkeye * bkx,
	const MSKrealt * blx,
	const MSKrealt * bux);

/* MSK_isdouparname */
MSKrescodee (MSKAPI MSK_isdouparname) (
	MSKtask_t task,
	const char * parname,
	MSKdparame * param);

/* MSK_isintparname */
MSKrescodee (MSKAPI MSK_isintparname) (
	MSKtask_t task,
	const char * parname,
	MSKiparame * param);

/* MSK_isstrparname */
MSKrescodee (MSKAPI MSK_isstrparname) (
	MSKtask_t task,
	const char * parname,
	MSKsparame * param);

/* MSK_linkfiletotaskstream */
MSKrescodee (MSKAPI MSK_linkfiletotaskstream) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	const char * filename,
	MSKint32t append);

/* MSK_linkfunctotaskstream */
MSKrescodee (MSKAPI MSK_linkfunctotaskstream) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	MSKuserhandle_t handle,
	MSKstreamfunc func);

/* MSK_unlinkfuncfromtaskstream */
MSKrescodee (MSKAPI MSK_unlinkfuncfromtaskstream) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_clonetask */
MSKrescodee (MSKAPI MSK_clonetask) (
	MSKtask_t task,
	MSKtask_t * clonedtask);

/* MSK_primalrepair */
MSKrescodee (MSKAPI MSK_primalrepair) (
	MSKtask_t task,
	const MSKrealt * wlc,
	const MSKrealt * wuc,
	const MSKrealt * wlx,
	const MSKrealt * wux);

/* MSK_toconic */
MSKrescodee (MSKAPI MSK_toconic) (
	MSKtask_t task);

/* MSK_optimize */
MSKrescodee (MSKAPI MSK_optimize) (
	MSKtask_t task);

/* MSK_optimizetrm */
MSKrescodee (MSKAPI MSK_optimizetrm) (
	MSKtask_t task,
	MSKrescodee * trmcode);

/* MSK_printparam */
MSKrescodee (MSKAPI MSK_printparam) (
	MSKtask_t task);

/* MSK_probtypetostr */
MSKrescodee (MSKAPI MSK_probtypetostr) (
	MSKtask_t task,
	MSKproblemtypee probtype,
	char * str);

/* MSK_prostatostr */
MSKrescodee (MSKAPI MSK_prostatostr) (
	MSKtask_t task,
	MSKprostae prosta,
	char * str);

/* MSK_putresponsefunc */
MSKrescodee (MSKAPI MSK_putresponsefunc) (
	MSKtask_t task,
	MSKresponsefunc responsefunc,
	MSKuserhandle_t handle);

/* MSK_commitchanges */
MSKrescodee (MSKAPI MSK_commitchanges) (
	MSKtask_t task);

/* MSK_getatruncatetol */
MSKrescodee (MSKAPI MSK_getatruncatetol) (
	MSKtask_t task,
	MSKrealt * tolzero);

/* MSK_putatruncatetol */
MSKrescodee (MSKAPI MSK_putatruncatetol) (
	MSKtask_t task,
	MSKrealt tolzero);

/* MSK_putaij */
MSKrescodee (MSKAPI MSK_putaij) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t j,
	MSKrealt aij);

/* MSK_putaijlist */
MSKrescodee (MSKAPI MSK_putaijlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKrealt * valij);

/* MSK_putaijlist64 */
MSKrescodee (MSKAPI MSK_putaijlist64) (
	MSKtask_t task,
	MSKint64t num,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKrealt * valij);

/* MSK_putacol */
MSKrescodee (MSKAPI MSK_putacol) (
	MSKtask_t task,
	MSKint32t j,
	MSKint32t nzj,
	const MSKint32t * subj,
	const MSKrealt * valj);

/* MSK_putarow */
MSKrescodee (MSKAPI MSK_putarow) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t nzi,
	const MSKint32t * subi,
	const MSKrealt * vali);

/* MSK_putarowslice */
MSKrescodee (MSKAPI MSK_putarowslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKint32t * ptrb,
	const MSKint32t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putarowslice64 */
MSKrescodee (MSKAPI MSK_putarowslice64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKint64t * ptrb,
	const MSKint64t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putarowlist */
MSKrescodee (MSKAPI MSK_putarowlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKint32t * ptrb,
	const MSKint32t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putarowlist64 */
MSKrescodee (MSKAPI MSK_putarowlist64) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKint64t * ptrb,
	const MSKint64t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putacolslice */
MSKrescodee (MSKAPI MSK_putacolslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKint32t * ptrb,
	const MSKint32t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putacolslice64 */
MSKrescodee (MSKAPI MSK_putacolslice64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKint64t * ptrb,
	const MSKint64t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putacollist */
MSKrescodee (MSKAPI MSK_putacollist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKint32t * ptrb,
	const MSKint32t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putacollist64 */
MSKrescodee (MSKAPI MSK_putacollist64) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKint64t * ptrb,
	const MSKint64t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putbaraij */
MSKrescodee (MSKAPI MSK_putbaraij) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t j,
	MSKint64t num,
	const MSKint64t * sub,
	const MSKrealt * weights);

/* MSK_putbaraijlist */
MSKrescodee (MSKAPI MSK_putbaraijlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKint64t * alphaptrb,
	const MSKint64t * alphaptre,
	const MSKint64t * matidx,
	const MSKrealt * weights);

/* MSK_putbararowlist */
MSKrescodee (MSKAPI MSK_putbararowlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subi,
	const MSKint64t * ptrb,
	const MSKint64t * ptre,
	const MSKint32t * subj,
	const MSKint64t * nummat,
	const MSKint64t * matidx,
	const MSKrealt * weights);

/* MSK_getnumbarcnz */
MSKrescodee (MSKAPI MSK_getnumbarcnz) (
	MSKtask_t task,
	MSKint64t * nz);

/* MSK_getnumbaranz */
MSKrescodee (MSKAPI MSK_getnumbaranz) (
	MSKtask_t task,
	MSKint64t * nz);

/* MSK_getbarcsparsity */
MSKrescodee (MSKAPI MSK_getbarcsparsity) (
	MSKtask_t task,
	MSKint64t maxnumnz,
	MSKint64t * numnz,
	MSKint64t * idxj);

/* MSK_getbarasparsity */
MSKrescodee (MSKAPI MSK_getbarasparsity) (
	MSKtask_t task,
	MSKint64t maxnumnz,
	MSKint64t * numnz,
	MSKint64t * idxij);

/* MSK_getbarcidxinfo */
MSKrescodee (MSKAPI MSK_getbarcidxinfo) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint64t * num);

/* MSK_getbarcidxj */
MSKrescodee (MSKAPI MSK_getbarcidxj) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint32t * j);

/* MSK_getbarcidx */
MSKrescodee (MSKAPI MSK_getbarcidx) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint64t maxnum,
	MSKint32t * j,
	MSKint64t * num,
	MSKint64t * sub,
	MSKrealt * weights);

/* MSK_getbaraidxinfo */
MSKrescodee (MSKAPI MSK_getbaraidxinfo) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint64t * num);

/* MSK_getbaraidxij */
MSKrescodee (MSKAPI MSK_getbaraidxij) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint32t * i,
	MSKint32t * j);

/* MSK_getbaraidx */
MSKrescodee (MSKAPI MSK_getbaraidx) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint64t maxnum,
	MSKint32t * i,
	MSKint32t * j,
	MSKint64t * num,
	MSKint64t * sub,
	MSKrealt * weights);

/* MSK_getnumbarcblocktriplets */
MSKrescodee (MSKAPI MSK_getnumbarcblocktriplets) (
	MSKtask_t task,
	MSKint64t * num);

/* MSK_putbarcblocktriplet */
MSKrescodee (MSKAPI MSK_putbarcblocktriplet) (
	MSKtask_t task,
	MSKint64t num,
	const MSKint32t * subj,
	const MSKint32t * subk,
	const MSKint32t * subl,
	const MSKrealt * valjkl);

/* MSK_getbarcblocktriplet */
MSKrescodee (MSKAPI MSK_getbarcblocktriplet) (
	MSKtask_t task,
	MSKint64t maxnum,
	MSKint64t * num,
	MSKint32t * subj,
	MSKint32t * subk,
	MSKint32t * subl,
	MSKrealt * valjkl);

/* MSK_putbarablocktriplet */
MSKrescodee (MSKAPI MSK_putbarablocktriplet) (
	MSKtask_t task,
	MSKint64t num,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKint32t * subk,
	const MSKint32t * subl,
	const MSKrealt * valijkl);

/* MSK_getnumbarablocktriplets */
MSKrescodee (MSKAPI MSK_getnumbarablocktriplets) (
	MSKtask_t task,
	MSKint64t * num);

/* MSK_getbarablocktriplet */
MSKrescodee (MSKAPI MSK_getbarablocktriplet) (
	MSKtask_t task,
	MSKint64t maxnum,
	MSKint64t * num,
	MSKint32t * subi,
	MSKint32t * subj,
	MSKint32t * subk,
	MSKint32t * subl,
	MSKrealt * valijkl);

/* MSK_putconbound */
MSKrescodee (MSKAPI MSK_putconbound) (
	MSKtask_t task,
	MSKint32t i,
	MSKboundkeye bkc,
	MSKrealt blc,
	MSKrealt buc);

/* MSK_putconboundlist */
MSKrescodee (MSKAPI MSK_putconboundlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKboundkeye * bkc,
	const MSKrealt * blc,
	const MSKrealt * buc);

/* MSK_putconboundlistconst */
MSKrescodee (MSKAPI MSK_putconboundlistconst) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	MSKboundkeye bkc,
	MSKrealt blc,
	MSKrealt buc);

/* MSK_putconboundslice */
MSKrescodee (MSKAPI MSK_putconboundslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKboundkeye * bkc,
	const MSKrealt * blc,
	const MSKrealt * buc);

/* MSK_putconboundsliceconst */
MSKrescodee (MSKAPI MSK_putconboundsliceconst) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKboundkeye bkc,
	MSKrealt blc,
	MSKrealt buc);

/* MSK_putvarbound */
MSKrescodee (MSKAPI MSK_putvarbound) (
	MSKtask_t task,
	MSKint32t j,
	MSKboundkeye bkx,
	MSKrealt blx,
	MSKrealt bux);

/* MSK_putvarboundlist */
MSKrescodee (MSKAPI MSK_putvarboundlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKboundkeye * bkx,
	const MSKrealt * blx,
	const MSKrealt * bux);

/* MSK_putvarboundlistconst */
MSKrescodee (MSKAPI MSK_putvarboundlistconst) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	MSKboundkeye bkx,
	MSKrealt blx,
	MSKrealt bux);

/* MSK_putvarboundslice */
MSKrescodee (MSKAPI MSK_putvarboundslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKboundkeye * bkx,
	const MSKrealt * blx,
	const MSKrealt * bux);

/* MSK_putvarboundsliceconst */
MSKrescodee (MSKAPI MSK_putvarboundsliceconst) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKboundkeye bkx,
	MSKrealt blx,
	MSKrealt bux);

/* MSK_putcallbackfunc */
MSKrescodee (MSKAPI MSK_putcallbackfunc) (
	MSKtask_t task,
	MSKcallbackfunc func,
	MSKuserhandle_t handle);

/* MSK_putcfix */
MSKrescodee (MSKAPI MSK_putcfix) (
	MSKtask_t task,
	MSKrealt cfix);

/* MSK_putcj */
MSKrescodee (MSKAPI MSK_putcj) (
	MSKtask_t task,
	MSKint32t j,
	MSKrealt cj);

/* MSK_putobjsense */
MSKrescodee (MSKAPI MSK_putobjsense) (
	MSKtask_t task,
	MSKobjsensee sense);

/* MSK_getobjsense */
MSKrescodee (MSKAPI MSK_getobjsense) (
	MSKtask_t task,
	MSKobjsensee * sense);

/* MSK_putclist */
MSKrescodee (MSKAPI MSK_putclist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	const MSKrealt * val);

/* MSK_putcslice */
MSKrescodee (MSKAPI MSK_putcslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * slice);

/* MSK_putbarcj */
MSKrescodee (MSKAPI MSK_putbarcj) (
	MSKtask_t task,
	MSKint32t j,
	MSKint64t num,
	const MSKint64t * sub,
	const MSKrealt * weights);

/* MSK_putcone */
MSKrescodee (MSKAPI MSK_putcone) (
	MSKtask_t task,
	MSKint32t k,
	MSKconetypee ct,
	MSKrealt conepar,
	MSKint32t nummem,
	const MSKint32t * submem);

/* MSK_appendsparsesymmat */
MSKrescodee (MSKAPI MSK_appendsparsesymmat) (
	MSKtask_t task,
	MSKint32t dim,
	MSKint64t nz,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKrealt * valij,
	MSKint64t * idx);

/* MSK_appendsparsesymmatlist */
MSKrescodee (MSKAPI MSK_appendsparsesymmatlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * dims,
	const MSKint64t * nz,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKrealt * valij,
	MSKint64t * idx);

/* MSK_getsymmatinfo */
MSKrescodee (MSKAPI MSK_getsymmatinfo) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint32t * dim,
	MSKint64t * nz,
	MSKsymmattypee * type);

/* MSK_getnumsymmat */
MSKrescodee (MSKAPI MSK_getnumsymmat) (
	MSKtask_t task,
	MSKint64t * num);

/* MSK_getsparsesymmat */
MSKrescodee (MSKAPI MSK_getsparsesymmat) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint64t maxlen,
	MSKint32t * subi,
	MSKint32t * subj,
	MSKrealt * valij);

/* MSK_putdouparam */
MSKrescodee (MSKAPI MSK_putdouparam) (
	MSKtask_t task,
	MSKdparame param,
	MSKrealt parvalue);

/* MSK_putintparam */
MSKrescodee (MSKAPI MSK_putintparam) (
	MSKtask_t task,
	MSKiparame param,
	MSKint32t parvalue);

/* MSK_putmaxnumcon */
MSKrescodee (MSKAPI MSK_putmaxnumcon) (
	MSKtask_t task,
	MSKint32t maxnumcon);

/* MSK_putmaxnumcone */
MSKrescodee (MSKAPI MSK_putmaxnumcone) (
	MSKtask_t task,
	MSKint32t maxnumcone);

/* MSK_getmaxnumcone */
MSKrescodee (MSKAPI MSK_getmaxnumcone) (
	MSKtask_t task,
	MSKint32t * maxnumcone);

/* MSK_putmaxnumvar */
MSKrescodee (MSKAPI MSK_putmaxnumvar) (
	MSKtask_t task,
	MSKint32t maxnumvar);

/* MSK_putmaxnumbarvar */
MSKrescodee (MSKAPI MSK_putmaxnumbarvar) (
	MSKtask_t task,
	MSKint32t maxnumbarvar);

/* MSK_putmaxnumanz */
MSKrescodee (MSKAPI MSK_putmaxnumanz) (
	MSKtask_t task,
	MSKint64t maxnumanz);

/* MSK_putmaxnumqnz */
MSKrescodee (MSKAPI MSK_putmaxnumqnz) (
	MSKtask_t task,
	MSKint64t maxnumqnz);

/* MSK_getmaxnumqnz */
MSKrescodee (MSKAPI MSK_getmaxnumqnz) (
	MSKtask_t task,
	MSKint32t * maxnumqnz);

/* MSK_getmaxnumqnz64 */
MSKrescodee (MSKAPI MSK_getmaxnumqnz64) (
	MSKtask_t task,
	MSKint64t * maxnumqnz);

/* MSK_putnadouparam */
MSKrescodee (MSKAPI MSK_putnadouparam) (
	MSKtask_t task,
	const char * paramname,
	MSKrealt parvalue);

/* MSK_putnaintparam */
MSKrescodee (MSKAPI MSK_putnaintparam) (
	MSKtask_t task,
	const char * paramname,
	MSKint32t parvalue);

/* MSK_putnastrparam */
MSKrescodee (MSKAPI MSK_putnastrparam) (
	MSKtask_t task,
	const char * paramname,
	const char * parvalue);

/* MSK_putobjname */
MSKrescodee (MSKAPI MSK_putobjname) (
	MSKtask_t task,
	const char * objname);

/* MSK_putparam */
MSKrescodee (MSKAPI MSK_putparam) (
	MSKtask_t task,
	const char * parname,
	const char * parvalue);

/* MSK_putqcon */
MSKrescodee (MSKAPI MSK_putqcon) (
	MSKtask_t task,
	MSKint32t numqcnz,
	const MSKint32t * qcsubk,
	const MSKint32t * qcsubi,
	const MSKint32t * qcsubj,
	const MSKrealt * qcval);

/* MSK_putqconk */
MSKrescodee (MSKAPI MSK_putqconk) (
	MSKtask_t task,
	MSKint32t k,
	MSKint32t numqcnz,
	const MSKint32t * qcsubi,
	const MSKint32t * qcsubj,
	const MSKrealt * qcval);

/* MSK_putqobj */
MSKrescodee (MSKAPI MSK_putqobj) (
	MSKtask_t task,
	MSKint32t numqonz,
	const MSKint32t * qosubi,
	const MSKint32t * qosubj,
	const MSKrealt * qoval);

/* MSK_putqobjij */
MSKrescodee (MSKAPI MSK_putqobjij) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t j,
	MSKrealt qoij);

/* MSK_putsolution */
MSKrescodee (MSKAPI MSK_putsolution) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKstakeye * skc,
	const MSKstakeye * skx,
	const MSKstakeye * skn,
	const MSKrealt * xc,
	const MSKrealt * xx,
	const MSKrealt * y,
	const MSKrealt * slc,
	const MSKrealt * suc,
	const MSKrealt * slx,
	const MSKrealt * sux,
	const MSKrealt * snx);

/* MSK_putconsolutioni */
MSKrescodee (MSKAPI MSK_putconsolutioni) (
	MSKtask_t task,
	MSKint32t i,
	MSKsoltypee whichsol,
	MSKstakeye sk,
	MSKrealt x,
	MSKrealt sl,
	MSKrealt su);

/* MSK_putvarsolutionj */
MSKrescodee (MSKAPI MSK_putvarsolutionj) (
	MSKtask_t task,
	MSKint32t j,
	MSKsoltypee whichsol,
	MSKstakeye sk,
	MSKrealt x,
	MSKrealt sl,
	MSKrealt su,
	MSKrealt sn);

/* MSK_putsolutionyi */
MSKrescodee (MSKAPI MSK_putsolutionyi) (
	MSKtask_t task,
	MSKint32t i,
	MSKsoltypee whichsol,
	MSKrealt y);

/* MSK_putstrparam */
MSKrescodee (MSKAPI MSK_putstrparam) (
	MSKtask_t task,
	MSKsparame param,
	const char * parvalue);

/* MSK_puttaskname */
MSKrescodee (MSKAPI MSK_puttaskname) (
	MSKtask_t task,
	const char * taskname);

/* MSK_putvartype */
MSKrescodee (MSKAPI MSK_putvartype) (
	MSKtask_t task,
	MSKint32t j,
	MSKvariabletypee vartype);

/* MSK_putvartypelist */
MSKrescodee (MSKAPI MSK_putvartypelist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	const MSKvariabletypee * vartype);

/* MSK_readdata */
MSKrescodee (MSKAPI MSK_readdata) (
	MSKtask_t task,
	const char * filename);

/* MSK_readdataformat */
MSKrescodee (MSKAPI MSK_readdataformat) (
	MSKtask_t task,
	const char * filename,
	int format,
	int compress);

/* MSK_readdataautoformat */
MSKrescodee (MSKAPI MSK_readdataautoformat) (
	MSKtask_t task,
	const char * filename);

/* MSK_readparamfile */
MSKrescodee (MSKAPI MSK_readparamfile) (
	MSKtask_t task,
	const char * filename);

/* MSK_readsolution */
MSKrescodee (MSKAPI MSK_readsolution) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const char * filename);

/* MSK_readsummary */
MSKrescodee (MSKAPI MSK_readsummary) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_resizetask */
MSKrescodee (MSKAPI MSK_resizetask) (
	MSKtask_t task,
	MSKint32t maxnumcon,
	MSKint32t maxnumvar,
	MSKint32t maxnumcone,
	MSKint64t maxnumanz,
	MSKint64t maxnumqnz);

/* MSK_checkmemtask */
MSKrescodee (MSKAPI MSK_checkmemtask) (
	MSKtask_t task,
	const char * file,
	MSKint32t line);

/* MSK_getmemusagetask */
MSKrescodee (MSKAPI MSK_getmemusagetask) (
	MSKtask_t task,
	MSKint64t * meminuse,
	MSKint64t * maxmemuse);

/* MSK_setdefaults */
MSKrescodee (MSKAPI MSK_setdefaults) (
	MSKtask_t task);

/* MSK_sktostr */
MSKrescodee (MSKAPI MSK_sktostr) (
	MSKtask_t task,
	MSKstakeye sk,
	char * str);

/* MSK_solstatostr */
MSKrescodee (MSKAPI MSK_solstatostr) (
	MSKtask_t task,
	MSKsolstae solsta,
	char * str);

/* MSK_solutiondef */
MSKrescodee (MSKAPI MSK_solutiondef) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKbooleant * isdef);

/* MSK_deletesolution */
MSKrescodee (MSKAPI MSK_deletesolution) (
	MSKtask_t task,
	MSKsoltypee whichsol);

/* MSK_onesolutionsummary */
MSKrescodee (MSKAPI MSK_onesolutionsummary) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	MSKsoltypee whichsol);

/* MSK_solutionsummary */
MSKrescodee (MSKAPI MSK_solutionsummary) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_updatesolutioninfo */
MSKrescodee (MSKAPI MSK_updatesolutioninfo) (
	MSKtask_t task,
	MSKsoltypee whichsol);

/* MSK_optimizersummary */
MSKrescodee (MSKAPI MSK_optimizersummary) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_strduptask */
char * (MSKAPI MSK_strduptask) (
	MSKtask_t task,
	const char * str);

/* MSK_strdupdbgtask */
char * (MSKAPI MSK_strdupdbgtask) (
	MSKtask_t task,
	const char * str,
	const char * file,
	const unsigned line);

/* MSK_strtoconetype */
MSKrescodee (MSKAPI MSK_strtoconetype) (
	MSKtask_t task,
	const char * str,
	MSKconetypee * conetype);

/* MSK_strtosk */
MSKrescodee (MSKAPI MSK_strtosk) (
	MSKtask_t task,
	const char * str,
	MSKstakeye * sk);

/* MSK_whichparam */
MSKrescodee (MSKAPI MSK_whichparam) (
	MSKtask_t task,
	const char * parname,
	MSKparametertypee * partype,
	MSKint32t * param);

/* MSK_writedata */
MSKrescodee (MSKAPI MSK_writedata) (
	MSKtask_t task,
	const char * filename);

/* MSK_writetask */
MSKrescodee (MSKAPI MSK_writetask) (
	MSKtask_t task,
	const char * filename);

/* MSK_readtask */
MSKrescodee (MSKAPI MSK_readtask) (
	MSKtask_t task,
	const char * filename);

/* MSK_readopfstring */
MSKrescodee (MSKAPI MSK_readopfstring) (
	MSKtask_t task,
	const char * data);

/* MSK_readlpstring */
MSKrescodee (MSKAPI MSK_readlpstring) (
	MSKtask_t task,
	const char * data);

/* MSK_readjsonstring */
MSKrescodee (MSKAPI MSK_readjsonstring) (
	MSKtask_t task,
	const char * data);

/* MSK_readptfstring */
MSKrescodee (MSKAPI MSK_readptfstring) (
	MSKtask_t task,
	const char * data);

/* MSK_writeparamfile */
MSKrescodee (MSKAPI MSK_writeparamfile) (
	MSKtask_t task,
	const char * filename);

/* MSK_getinfeasiblesubproblem */
MSKrescodee (MSKAPI MSK_getinfeasiblesubproblem) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKtask_t * inftask);

/* MSK_writesolution */
MSKrescodee (MSKAPI MSK_writesolution) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const char * filename);

/* MSK_writejsonsol */
MSKrescodee (MSKAPI MSK_writejsonsol) (
	MSKtask_t task,
	const char * filename);

/* MSK_primalsensitivity */
MSKrescodee (MSKAPI MSK_primalsensitivity) (
	MSKtask_t task,
	MSKint32t numi,
	const MSKint32t * subi,
	const MSKmarke * marki,
	MSKint32t numj,
	const MSKint32t * subj,
	const MSKmarke * markj,
	MSKrealt * leftpricei,
	MSKrealt * rightpricei,
	MSKrealt * leftrangei,
	MSKrealt * rightrangei,
	MSKrealt * leftpricej,
	MSKrealt * rightpricej,
	MSKrealt * leftrangej,
	MSKrealt * rightrangej);

/* MSK_sensitivityreport */
MSKrescodee (MSKAPI MSK_sensitivityreport) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_dualsensitivity */
MSKrescodee (MSKAPI MSK_dualsensitivity) (
	MSKtask_t task,
	MSKint32t numj,
	const MSKint32t * subj,
	MSKrealt * leftpricej,
	MSKrealt * rightpricej,
	MSKrealt * leftrangej,
	MSKrealt * rightrangej);

/* MSK_getlasterror */
MSKrescodee (MSKAPI MSK_getlasterror) (
	MSKtask_t task,
	MSKrescodee * lastrescode,
	MSKint32t sizelastmsg,
	MSKint32t * lastmsglen,
	char * lastmsg);

/* MSK_getlasterror64 */
MSKrescodee (MSKAPI MSK_getlasterror64) (
	MSKtask_t task,
	MSKrescodee * lastrescode,
	MSKint64t sizelastmsg,
	MSKint64t * lastmsglen,
	char * lastmsg);

/* MSK_optimizermt */
MSKrescodee (MSKAPI MSK_optimizermt) (
	MSKtask_t task,
	const char * server,
	const char * port,
	MSKrescodee * trmcode);

/* MSK_asyncoptimize */
MSKrescodee (MSKAPI MSK_asyncoptimize) (
	MSKtask_t task,
	const char * server,
	const char * port,
	char * token);

/* MSK_asyncstop */
MSKrescodee (MSKAPI MSK_asyncstop) (
	MSKtask_t task,
	const char * server,
	const char * port,
	const char * token);

/* MSK_asyncpoll */
MSKrescodee (MSKAPI MSK_asyncpoll) (
	MSKtask_t task,
	const char * server,
	const char * port,
	const char * token,
	MSKbooleant * respavailable,
	MSKrescodee * resp,
	MSKrescodee * trm);

/* MSK_asyncgetresult */
MSKrescodee (MSKAPI MSK_asyncgetresult) (
	MSKtask_t task,
	const char * server,
	const char * port,
	const char * token,
	MSKbooleant * respavailable,
	MSKrescodee * resp,
	MSKrescodee * trm);

/* MSK_isinfinity */
MSKbooleant (MSKAPI MSK_isinfinity) (
	MSKrealt value);

/* MSK_checkoutlicense */
MSKrescodee (MSKAPI MSK_checkoutlicense) (
	MSKenv_t env,
	MSKfeaturee feature);

/* MSK_checkinlicense */
MSKrescodee (MSKAPI MSK_checkinlicense) (
	MSKenv_t env,
	MSKfeaturee feature);

/* MSK_checkinall */
MSKrescodee (MSKAPI MSK_checkinall) (
	MSKenv_t env);

/* MSK_setupthreads */
MSKrescodee (MSKAPI MSK_setupthreads) (
	MSKenv_t env,
	MSKint32t numthreads);

/* MSK_getbuildinfo */
MSKrescodee (MSKAPI MSK_getbuildinfo) (
	char * buildstate,
	char * builddate);

/* MSK_getresponseclass */
MSKrescodee (MSKAPI MSK_getresponseclass) (
	MSKrescodee r,
	MSKrescodetypee * rc);

/* MSK_callocenv */
void * (MSKAPI MSK_callocenv) (
	MSKenv_t env,
	const size_t number,
	const size_t size);

/* MSK_callocdbgenv */
void * (MSKAPI MSK_callocdbgenv) (
	MSKenv_t env,
	const size_t number,
	const size_t size,
	const char * file,
	const unsigned line);

/* MSK_deleteenv */
MSKrescodee (MSKAPI MSK_deleteenv) (
	MSKenv_t * env);

/* MSK_echoenv */
MSKrescodee (MSKAPIVA MSK_echoenv) (
	MSKenv_t env,
	MSKstreamtypee whichstream,
	const char * format,
	...);

/* MSK_echointro */
MSKrescodee (MSKAPI MSK_echointro) (
	MSKenv_t env,
	MSKint32t longver);

/* MSK_freeenv */
void (MSKAPI MSK_freeenv) (
	MSKenv_t env,
	void * buffer);

/* MSK_freedbgenv */
void (MSKAPI MSK_freedbgenv) (
	MSKenv_t env,
	void * buffer,
	const char * file,
	const unsigned line);

/* MSK_getcodedesc */
MSKrescodee (MSKAPI MSK_getcodedesc) (
	MSKrescodee code,
	char * symname,
	char * str);

/* MSK_getsymbcondim */
MSKrescodee (MSKAPI MSK_getsymbcondim) (
	MSKenv_t env,
	MSKint32t * num,
	size_t * maxlen);

/* MSK_getversion */
MSKrescodee (MSKAPI MSK_getversion) (
	MSKint32t * major,
	MSKint32t * minor,
	MSKint32t * revision);

/* MSK_checkversion */
MSKrescodee (MSKAPI MSK_checkversion) (
	MSKenv_t env,
	MSKint32t major,
	MSKint32t minor,
	MSKint32t revision);

/* MSK_iparvaltosymnam */
MSKrescodee (MSKAPI MSK_iparvaltosymnam) (
	MSKenv_t env,
	MSKiparame whichparam,
	MSKint32t whichvalue,
	char * symbolicname);

/* MSK_linkfiletoenvstream */
MSKrescodee (MSKAPI MSK_linkfiletoenvstream) (
	MSKenv_t env,
	MSKstreamtypee whichstream,
	const char * filename,
	MSKint32t append);

/* MSK_linkfunctoenvstream */
MSKrescodee (MSKAPI MSK_linkfunctoenvstream) (
	MSKenv_t env,
	MSKstreamtypee whichstream,
	MSKuserhandle_t handle,
	MSKstreamfunc func);

/* MSK_unlinkfuncfromenvstream */
MSKrescodee (MSKAPI MSK_unlinkfuncfromenvstream) (
	MSKenv_t env,
	MSKstreamtypee whichstream);

/* MSK_makeenv */
MSKrescodee (MSKAPI MSK_makeenv) (
	MSKenv_t * env,
	const char * dbgfile);

/* MSK_makeenvalloc */
MSKrescodee (MSKAPI MSK_makeenvalloc) (
	MSKenv_t * env,
	MSKuserhandle_t usrptr,
	MSKmallocfunc usrmalloc,
	MSKcallocfunc usrcalloc,
	MSKreallocfunc usrrealloc,
	MSKfreefunc usrfree,
	const char * dbgfile);

/* MSK_putlicensedebug */
MSKrescodee (MSKAPI MSK_putlicensedebug) (
	MSKenv_t env,
	MSKint32t licdebug);

/* MSK_putlicensecode */
MSKrescodee (MSKAPI MSK_putlicensecode) (
	MSKenv_t env,
	const MSKint32t * code);

/* MSK_putlicensewait */
MSKrescodee (MSKAPI MSK_putlicensewait) (
	MSKenv_t env,
	MSKint32t licwait);

/* MSK_putlicensepath */
MSKrescodee (MSKAPI MSK_putlicensepath) (
	MSKenv_t env,
	const char * licensepath);

/* MSK_maketask */
MSKrescodee (MSKAPI MSK_maketask) (
	MSKenv_t env,
	MSKint32t maxnumcon,
	MSKint32t maxnumvar,
	MSKtask_t * task);

/* MSK_makeemptytask */
MSKrescodee (MSKAPI MSK_makeemptytask) (
	MSKenv_t env,
	MSKtask_t * task);

/* MSK_putexitfunc */
MSKrescodee (MSKAPI MSK_putexitfunc) (
	MSKenv_t env,
	MSKexitfunc exitfunc,
	MSKuserhandle_t handle);

/* MSK_utf8towchar */
MSKrescodee (MSKAPI MSK_utf8towchar) (
	const size_t outputlen,
	size_t * len,
	size_t * conv,
	MSKwchart * output,
	const char * input);

/* MSK_wchartoutf8 */
MSKrescodee (MSKAPI MSK_wchartoutf8) (
	const size_t outputlen,
	size_t * len,
	size_t * conv,
	char * output,
	const MSKwchart * input);

/* MSK_checkmemenv */
MSKrescodee (MSKAPI MSK_checkmemenv) (
	MSKenv_t env,
	const char * file,
	MSKint32t line);

/* MSK_symnamtovalue */
MSKbooleant (MSKAPI MSK_symnamtovalue) (
	const char * name,
	char * value);

/* MSK_axpy */
MSKrescodee (MSKAPI MSK_axpy) (
	MSKenv_t env,
	MSKint32t n,
	MSKrealt alpha,
	const MSKrealt * x,
	MSKrealt * y);

/* MSK_dot */
MSKrescodee (MSKAPI MSK_dot) (
	MSKenv_t env,
	MSKint32t n,
	const MSKrealt * x,
	const MSKrealt * y,
	MSKrealt * xty);

/* MSK_gemv */
MSKrescodee (MSKAPI MSK_gemv) (
	MSKenv_t env,
	MSKtransposee transa,
	MSKint32t m,
	MSKint32t n,
	MSKrealt alpha,
	const MSKrealt * a,
	const MSKrealt * x,
	MSKrealt beta,
	MSKrealt * y);

/* MSK_gemm */
MSKrescodee (MSKAPI MSK_gemm) (
	MSKenv_t env,
	MSKtransposee transa,
	MSKtransposee transb,
	MSKint32t m,
	MSKint32t n,
	MSKint32t k,
	MSKrealt alpha,
	const MSKrealt * a,
	const MSKrealt * b,
	MSKrealt beta,
	MSKrealt * c);

/* MSK_syrk */
MSKrescodee (MSKAPI MSK_syrk) (
	MSKenv_t env,
	MSKuploe uplo,
	MSKtransposee trans,
	MSKint32t n,
	MSKint32t k,
	MSKrealt alpha,
	const MSKrealt * a,
	MSKrealt beta,
	MSKrealt * c);

/* MSK_computesparsecholesky */
MSKrescodee (MSKAPI MSK_computesparsecholesky) (
	MSKenv_t env,
	MSKint32t multithread,
	MSKint32t ordermethod,
	MSKrealt tolsingular,
	MSKint32t n,
	const MSKint32t * anzc,
	const MSKint64t * aptrc,
	const MSKint32t * asubc,
	const MSKrealt * avalc,
	MSKint32t ** perm,
	MSKrealt ** diag,
	MSKint32t ** lnzc,
	MSKint64t ** lptrc,
	MSKint64t * lensubnval,
	MSKint32t ** lsubc,
	MSKrealt ** lvalc);

/* MSK_sparsetriangularsolvedense */
MSKrescodee (MSKAPI MSK_sparsetriangularsolvedense) (
	MSKenv_t env,
	MSKtransposee transposed,
	MSKint32t n,
	const MSKint32t * lnzc,
	const MSKint64t * lptrc,
	MSKint64t lensubnval,
	const MSKint32t * lsubc,
	const MSKrealt * lvalc,
	MSKrealt * b);

/* MSK_potrf */
MSKrescodee (MSKAPI MSK_potrf) (
	MSKenv_t env,
	MSKuploe uplo,
	MSKint32t n,
	MSKrealt * a);

/* MSK_syeig */
MSKrescodee (MSKAPI MSK_syeig) (
	MSKenv_t env,
	MSKuploe uplo,
	MSKint32t n,
	const MSKrealt * a,
	MSKrealt * w);

/* MSK_syevd */
MSKrescodee (MSKAPI MSK_syevd) (
	MSKenv_t env,
	MSKuploe uplo,
	MSKint32t n,
	MSKrealt * a,
	MSKrealt * w);

/* MSK_licensecleanup */
MSKrescodee (MSKAPI MSK_licensecleanup) (
void);



#ifdef __cplusplus
}
#endif


#endif


