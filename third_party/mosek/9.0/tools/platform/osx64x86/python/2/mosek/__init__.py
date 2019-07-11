import _msk
import codecs
import ctypes
import re

class MSKException(Exception):
    pass
class MosekException(MSKException):
    def __init__(self,res,msg):
        MSKException.__init__(self,msg)
        self.msg   = msg
        self.errno = res
    def __str__(self):
        return "(%d) %s" % (self.errno,self.msg)

class Error(MosekException):
    pass

class EnumBase(int):
    """
    Base class for enums.
    """
    enumnamere = re.compile(r'[a-zA-Z][a-zA-Z0-9_]*$')
    def __new__(cls,value):
        if isinstance(value,int):
            return cls._valdict[value]
        elif isinstance(value,str):
            return cls._namedict[value.split('.')[-1]]
        else:
            raise TypeError("Invalid type for enum construction ({0})".format(value))
    def __str__(self):
        return '%s.%s' % (self.__class__.__name__,self.__name__)
    def __repr__(self):
        return self.__name__

    @classmethod
    def members(cls):
        return iter(cls._values)

    @classmethod
    def _initialize(cls, names,values=None):
        for n in names:
            if not cls.enumnamere.match(n):
                raise ValueError("Invalid enum item name '%s' in %s" % (n,cls.__name__))
        if values is None:
            values = range(len(names))
        if len(values) != len(names):
            raise ValueError("Lengths of names and values do not match")

        items = []
        for (n,v) in zip(names,values):
            item = int.__new__(cls,v)
            item.__name__ = n
            setattr(cls,n,item)
            items.append(item)

        cls._values   = items
        cls.values    = items
        cls._namedict = dict([ (v.__name__,v) for v in items ])
        cls._valdict  = dict([ (v,v) for v in items ]) # map int -> enum value (sneaky, eh?)

def Enum(name,names,values=None):
    """
    Create a new enum class with the given names and values.

    Parameters:
     [name]   A string denoting the name of the enum.
     [names]  A list of strings denoting the names of the individual enum values.
     [values] (optional) A list of integer values of the enums. If given, the
       list must have same length as the [names] parameter. If not given, the
       default values 0, 1, ... will be used.
    """
    e = type(name,(EnumBase,),{})
    e._initialize(names,values)
    return e

basindtype = Enum("basindtype", ["always","if_feasible","never","no_error","reservered"], [1,3,0,2,4])
boundkey = Enum("boundkey", ["fr","fx","lo","ra","up"], [3,2,0,4,1])
mark = Enum("mark", ["lo","up"], [0,1])
simdegen = Enum("simdegen", ["aggressive","free","minimum","moderate","none"], [2,1,4,3,0])
transpose = Enum("transpose", ["no","yes"], [0,1])
uplo = Enum("uplo", ["lo","up"], [0,1])
simreform = Enum("simreform", ["aggressive","free","off","on"], [3,2,0,1])
simdupvec = Enum("simdupvec", ["free","off","on"], [2,0,1])
simhotstart = Enum("simhotstart", ["free","none","status_keys"], [1,0,2])
intpnthotstart = Enum("intpnthotstart", ["dual","none","primal","primal_dual"], [2,0,1,3])
purify = Enum("purify", ["auto","dual","none","primal","primal_dual"], [4,2,0,1,3])
callbackcode = Enum("callbackcode", ["begin_bi","begin_conic","begin_dual_bi","begin_dual_sensitivity","begin_dual_setup_bi","begin_dual_simplex","begin_dual_simplex_bi","begin_full_convexity_check","begin_infeas_ana","begin_intpnt","begin_license_wait","begin_mio","begin_optimizer","begin_presolve","begin_primal_bi","begin_primal_repair","begin_primal_sensitivity","begin_primal_setup_bi","begin_primal_simplex","begin_primal_simplex_bi","begin_qcqo_reformulate","begin_read","begin_root_cutgen","begin_simplex","begin_simplex_bi","begin_to_conic","begin_write","conic","dual_simplex","end_bi","end_conic","end_dual_bi","end_dual_sensitivity","end_dual_setup_bi","end_dual_simplex","end_dual_simplex_bi","end_full_convexity_check","end_infeas_ana","end_intpnt","end_license_wait","end_mio","end_optimizer","end_presolve","end_primal_bi","end_primal_repair","end_primal_sensitivity","end_primal_setup_bi","end_primal_simplex","end_primal_simplex_bi","end_qcqo_reformulate","end_read","end_root_cutgen","end_simplex","end_simplex_bi","end_to_conic","end_write","im_bi","im_conic","im_dual_bi","im_dual_sensivity","im_dual_simplex","im_full_convexity_check","im_intpnt","im_license_wait","im_lu","im_mio","im_mio_dual_simplex","im_mio_intpnt","im_mio_primal_simplex","im_order","im_presolve","im_primal_bi","im_primal_sensivity","im_primal_simplex","im_qo_reformulate","im_read","im_root_cutgen","im_simplex","im_simplex_bi","intpnt","new_int_mio","primal_simplex","read_opf","read_opf_section","solving_remote","update_dual_bi","update_dual_simplex","update_dual_simplex_bi","update_presolve","update_primal_bi","update_primal_simplex","update_primal_simplex_bi","write_opf"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92])
checkconvexitytype = Enum("checkconvexitytype", ["full","none","simple"], [2,0,1])
compresstype = Enum("compresstype", ["free","gzip","none","zstd"], [1,2,0,3])
conetype = Enum("conetype", ["dexp","dpow","pexp","ppow","quad","rquad","zero"], [3,5,2,4,0,1,6])
nametype = Enum("nametype", ["gen","lp","mps"], [0,2,1])
scopr = Enum("scopr", ["ent","exp","log","pow","sqrt"], [0,1,2,3,4])
symmattype = Enum("symmattype", ["sparse"], [0])
dataformat = Enum("dataformat", ["cb","extension","free_mps","json_task","lp","mps","op","ptf","task"], [7,0,4,8,2,1,3,6,5])
dinfitem = Enum("dinfitem", ["bi_clean_dual_time","bi_clean_primal_time","bi_clean_time","bi_dual_time","bi_primal_time","bi_time","intpnt_dual_feas","intpnt_dual_obj","intpnt_factor_num_flops","intpnt_opt_status","intpnt_order_time","intpnt_primal_feas","intpnt_primal_obj","intpnt_time","mio_clique_separation_time","mio_cmir_separation_time","mio_construct_solution_obj","mio_dual_bound_after_presolve","mio_gmi_separation_time","mio_implied_bound_time","mio_knapsack_cover_separation_time","mio_obj_abs_gap","mio_obj_bound","mio_obj_int","mio_obj_rel_gap","mio_probing_time","mio_root_cutgen_time","mio_root_optimizer_time","mio_root_presolve_time","mio_time","mio_user_obj_cut","optimizer_time","presolve_eli_time","presolve_lindep_time","presolve_time","primal_repair_penalty_obj","qcqo_reformulate_max_perturbation","qcqo_reformulate_time","qcqo_reformulate_worst_cholesky_column_scaling","qcqo_reformulate_worst_cholesky_diag_scaling","rd_time","sim_dual_time","sim_feas","sim_obj","sim_primal_time","sim_time","sol_bas_dual_obj","sol_bas_dviolcon","sol_bas_dviolvar","sol_bas_nrm_barx","sol_bas_nrm_slc","sol_bas_nrm_slx","sol_bas_nrm_suc","sol_bas_nrm_sux","sol_bas_nrm_xc","sol_bas_nrm_xx","sol_bas_nrm_y","sol_bas_primal_obj","sol_bas_pviolcon","sol_bas_pviolvar","sol_itg_nrm_barx","sol_itg_nrm_xc","sol_itg_nrm_xx","sol_itg_primal_obj","sol_itg_pviolbarvar","sol_itg_pviolcon","sol_itg_pviolcones","sol_itg_pviolitg","sol_itg_pviolvar","sol_itr_dual_obj","sol_itr_dviolbarvar","sol_itr_dviolcon","sol_itr_dviolcones","sol_itr_dviolvar","sol_itr_nrm_bars","sol_itr_nrm_barx","sol_itr_nrm_slc","sol_itr_nrm_slx","sol_itr_nrm_snx","sol_itr_nrm_suc","sol_itr_nrm_sux","sol_itr_nrm_xc","sol_itr_nrm_xx","sol_itr_nrm_y","sol_itr_primal_obj","sol_itr_pviolbarvar","sol_itr_pviolcon","sol_itr_pviolcones","sol_itr_pviolvar","to_conic_time"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89])
feature = Enum("feature", ["pton","pts"], [1,0])
dparam = Enum("dparam", ["ana_sol_infeas_tol","basis_rel_tol_s","basis_tol_s","basis_tol_x","check_convexity_rel_tol","data_sym_mat_tol","data_sym_mat_tol_huge","data_sym_mat_tol_large","data_tol_aij_huge","data_tol_aij_large","data_tol_bound_inf","data_tol_bound_wrn","data_tol_c_huge","data_tol_cj_large","data_tol_qij","data_tol_x","intpnt_co_tol_dfeas","intpnt_co_tol_infeas","intpnt_co_tol_mu_red","intpnt_co_tol_near_rel","intpnt_co_tol_pfeas","intpnt_co_tol_rel_gap","intpnt_qo_tol_dfeas","intpnt_qo_tol_infeas","intpnt_qo_tol_mu_red","intpnt_qo_tol_near_rel","intpnt_qo_tol_pfeas","intpnt_qo_tol_rel_gap","intpnt_tol_dfeas","intpnt_tol_dsafe","intpnt_tol_infeas","intpnt_tol_mu_red","intpnt_tol_path","intpnt_tol_pfeas","intpnt_tol_psafe","intpnt_tol_rel_gap","intpnt_tol_rel_step","intpnt_tol_step_size","lower_obj_cut","lower_obj_cut_finite_trh","mio_max_time","mio_rel_gap_const","mio_tol_abs_gap","mio_tol_abs_relax_int","mio_tol_feas","mio_tol_rel_dual_bound_improvement","mio_tol_rel_gap","optimizer_max_time","presolve_tol_abs_lindep","presolve_tol_aij","presolve_tol_rel_lindep","presolve_tol_s","presolve_tol_x","qcqo_reformulate_rel_drop_tol","semidefinite_tol_approx","sim_lu_tol_rel_piv","simplex_abs_tol_piv","upper_obj_cut","upper_obj_cut_finite_trh"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58])
liinfitem = Enum("liinfitem", ["bi_clean_dual_deg_iter","bi_clean_dual_iter","bi_clean_primal_deg_iter","bi_clean_primal_iter","bi_dual_iter","bi_primal_iter","intpnt_factor_num_nz","mio_anz","mio_intpnt_iter","mio_presolved_anz","mio_simplex_iter","rd_numanz","rd_numqnz"], [0,1,2,3,4,5,6,7,8,9,10,11,12])
iinfitem = Enum("iinfitem", ["ana_pro_num_con","ana_pro_num_con_eq","ana_pro_num_con_fr","ana_pro_num_con_lo","ana_pro_num_con_ra","ana_pro_num_con_up","ana_pro_num_var","ana_pro_num_var_bin","ana_pro_num_var_cont","ana_pro_num_var_eq","ana_pro_num_var_fr","ana_pro_num_var_int","ana_pro_num_var_lo","ana_pro_num_var_ra","ana_pro_num_var_up","intpnt_factor_dim_dense","intpnt_iter","intpnt_num_threads","intpnt_solve_dual","mio_absgap_satisfied","mio_clique_table_size","mio_construct_solution","mio_node_depth","mio_num_active_nodes","mio_num_branch","mio_num_clique_cuts","mio_num_cmir_cuts","mio_num_gomory_cuts","mio_num_implied_bound_cuts","mio_num_int_solutions","mio_num_knapsack_cover_cuts","mio_num_relax","mio_num_repeated_presolve","mio_numbin","mio_numbinconevar","mio_numcon","mio_numcone","mio_numconevar","mio_numcont","mio_numcontconevar","mio_numdexpcones","mio_numdpowcones","mio_numint","mio_numintconevar","mio_numpexpcones","mio_numppowcones","mio_numqcones","mio_numrqcones","mio_numvar","mio_obj_bound_defined","mio_presolved_numbin","mio_presolved_numbinconevar","mio_presolved_numcon","mio_presolved_numcone","mio_presolved_numconevar","mio_presolved_numcont","mio_presolved_numcontconevar","mio_presolved_numdexpcones","mio_presolved_numdpowcones","mio_presolved_numint","mio_presolved_numintconevar","mio_presolved_numpexpcones","mio_presolved_numppowcones","mio_presolved_numqcones","mio_presolved_numrqcones","mio_presolved_numvar","mio_relgap_satisfied","mio_total_num_cuts","mio_user_obj_cut","opt_numcon","opt_numvar","optimize_response","purify_dual_success","purify_primal_success","rd_numbarvar","rd_numcon","rd_numcone","rd_numintvar","rd_numq","rd_numvar","rd_protype","sim_dual_deg_iter","sim_dual_hotstart","sim_dual_hotstart_lu","sim_dual_inf_iter","sim_dual_iter","sim_numcon","sim_numvar","sim_primal_deg_iter","sim_primal_hotstart","sim_primal_hotstart_lu","sim_primal_inf_iter","sim_primal_iter","sim_solve_dual","sol_bas_prosta","sol_bas_solsta","sol_itg_prosta","sol_itg_solsta","sol_itr_prosta","sol_itr_solsta","sto_num_a_realloc"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100])
inftype = Enum("inftype", ["dou_type","int_type","lint_type"], [0,1,2])
iomode = Enum("iomode", ["read","readwrite","write"], [0,2,1])
iparam = Enum("iparam", ["ana_sol_basis","ana_sol_print_violated","auto_sort_a_before_opt","auto_update_sol_info","basis_solve_use_plus_one","bi_clean_optimizer","bi_ignore_max_iter","bi_ignore_num_error","bi_max_iterations","cache_license","check_convexity","compress_statfile","infeas_generic_names","infeas_prefer_primal","infeas_report_auto","infeas_report_level","intpnt_basis","intpnt_diff_step","intpnt_hotstart","intpnt_max_iterations","intpnt_max_num_cor","intpnt_max_num_refinement_steps","intpnt_multi_thread","intpnt_off_col_trh","intpnt_order_gp_num_seeds","intpnt_order_method","intpnt_purify","intpnt_regularization_use","intpnt_scaling","intpnt_solve_form","intpnt_starting_point","license_debug","license_pause_time","license_suppress_expire_wrns","license_trh_expiry_wrn","license_wait","log","log_ana_pro","log_bi","log_bi_freq","log_check_convexity","log_cut_second_opt","log_expand","log_feas_repair","log_file","log_include_summary","log_infeas_ana","log_intpnt","log_local_info","log_mio","log_mio_freq","log_order","log_presolve","log_response","log_sensitivity","log_sensitivity_opt","log_sim","log_sim_freq","log_sim_minor","log_storage","max_num_warnings","mio_branch_dir","mio_conic_outer_approximation","mio_cut_clique","mio_cut_cmir","mio_cut_gmi","mio_cut_implied_bound","mio_cut_knapsack_cover","mio_cut_selection_level","mio_feaspump_level","mio_heuristic_level","mio_max_num_branches","mio_max_num_relaxs","mio_max_num_root_cut_rounds","mio_max_num_solutions","mio_mode","mio_node_optimizer","mio_node_selection","mio_perspective_reformulate","mio_probing_level","mio_propagate_objective_constraint","mio_rins_max_nodes","mio_root_optimizer","mio_root_repeat_presolve_level","mio_seed","mio_vb_detection_level","mt_spincount","num_threads","opf_write_header","opf_write_hints","opf_write_line_length","opf_write_parameters","opf_write_problem","opf_write_sol_bas","opf_write_sol_itg","opf_write_sol_itr","opf_write_solutions","optimizer","param_read_case_name","param_read_ign_error","presolve_eliminator_max_fill","presolve_eliminator_max_num_tries","presolve_level","presolve_lindep_abs_work_trh","presolve_lindep_rel_work_trh","presolve_lindep_use","presolve_max_num_pass","presolve_max_num_reductions","presolve_use","primal_repair_optimizer","ptf_write_transform","read_debug","read_keep_free_con","read_lp_drop_new_vars_in_bou","read_lp_quoted_names","read_mps_format","read_mps_width","read_task_ignore_param","remove_unused_solutions","sensitivity_all","sensitivity_optimizer","sensitivity_type","sim_basis_factor_use","sim_degen","sim_dual_crash","sim_dual_phaseone_method","sim_dual_restrict_selection","sim_dual_selection","sim_exploit_dupvec","sim_hotstart","sim_hotstart_lu","sim_max_iterations","sim_max_num_setbacks","sim_non_singular","sim_primal_crash","sim_primal_phaseone_method","sim_primal_restrict_selection","sim_primal_selection","sim_refactor_freq","sim_reformulation","sim_save_lu","sim_scaling","sim_scaling_method","sim_seed","sim_solve_form","sim_stability_priority","sim_switch_optimizer","sol_filter_keep_basic","sol_filter_keep_ranged","sol_read_name_width","sol_read_width","solution_callback","timing_level","write_bas_constraints","write_bas_head","write_bas_variables","write_compression","write_data_param","write_free_con","write_generic_names","write_generic_names_io","write_ignore_incompatible_items","write_int_constraints","write_int_head","write_int_variables","write_lp_full_obj","write_lp_line_width","write_lp_quoted_names","write_lp_strict_format","write_lp_terms_per_line","write_mps_format","write_mps_int","write_precision","write_sol_barvariables","write_sol_constraints","write_sol_head","write_sol_ignore_invalid_names","write_sol_variables","write_task_inc_sol","write_xml_mode"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179])
branchdir = Enum("branchdir", ["down","far","free","guided","near","pseudocost","root_lp","up"], [2,4,0,6,3,7,5,1])
miocontsoltype = Enum("miocontsoltype", ["itg","itg_rel","none","root"], [2,3,0,1])
miomode = Enum("miomode", ["ignored","satisfied"], [0,1])
mionodeseltype = Enum("mionodeseltype", ["best","first","free","pseudo"], [2,1,0,3])
mpsformat = Enum("mpsformat", ["cplex","free","relaxed","strict"], [3,2,1,0])
objsense = Enum("objsense", ["maximize","minimize"], [1,0])
onoffkey = Enum("onoffkey", ["off","on"], [0,1])
optimizertype = Enum("optimizertype", ["conic","dual_simplex","free","free_simplex","intpnt","mixed_int","primal_simplex"], [0,1,2,3,4,5,6])
orderingtype = Enum("orderingtype", ["appminloc","experimental","force_graphpar","free","none","try_graphpar"], [1,2,4,0,5,3])
presolvemode = Enum("presolvemode", ["free","off","on"], [2,0,1])
parametertype = Enum("parametertype", ["dou_type","int_type","invalid_type","str_type"], [1,2,0,3])
problemitem = Enum("problemitem", ["con","cone","var"], [1,2,0])
problemtype = Enum("problemtype", ["conic","lo","mixed","qcqo","qo"], [3,0,4,2,1])
prosta = Enum("prosta", ["dual_feas","dual_infeas","ill_posed","prim_and_dual_feas","prim_and_dual_infeas","prim_feas","prim_infeas","prim_infeas_or_unbounded","unknown"], [3,5,7,1,6,2,4,8,0])
xmlwriteroutputtype = Enum("xmlwriteroutputtype", ["col","row"], [1,0])
rescode = Enum("rescode", ["err_ad_invalid_codelist","err_api_array_too_small","err_api_cb_connect","err_api_fatal_error","err_api_internal","err_appending_too_big_cone","err_arg_is_too_large","err_arg_is_too_small","err_argument_dimension","err_argument_is_too_large","err_argument_lenneq","err_argument_perm_array","err_argument_type","err_bar_var_dim","err_basis","err_basis_factor","err_basis_singular","err_blank_name","err_cbf_duplicate_acoord","err_cbf_duplicate_bcoord","err_cbf_duplicate_con","err_cbf_duplicate_int","err_cbf_duplicate_obj","err_cbf_duplicate_objacoord","err_cbf_duplicate_pow_cones","err_cbf_duplicate_pow_star_cones","err_cbf_duplicate_psdvar","err_cbf_duplicate_var","err_cbf_invalid_con_type","err_cbf_invalid_dimension_of_cones","err_cbf_invalid_domain_dimension","err_cbf_invalid_exp_dimension","err_cbf_invalid_int_index","err_cbf_invalid_number_of_cones","err_cbf_invalid_power","err_cbf_invalid_power_cone_index","err_cbf_invalid_power_star_cone_index","err_cbf_invalid_psdvar_dimension","err_cbf_invalid_var_type","err_cbf_no_variables","err_cbf_no_version_specified","err_cbf_obj_sense","err_cbf_parse","err_cbf_power_cone_is_too_long","err_cbf_power_cone_mismatch","err_cbf_power_star_cone_mismatch","err_cbf_syntax","err_cbf_too_few_constraints","err_cbf_too_few_ints","err_cbf_too_few_psdvar","err_cbf_too_few_variables","err_cbf_too_many_constraints","err_cbf_too_many_ints","err_cbf_too_many_variables","err_cbf_unhandled_power_cone_type","err_cbf_unhandled_power_star_cone_type","err_cbf_unsupported","err_con_q_not_nsd","err_con_q_not_psd","err_cone_index","err_cone_overlap","err_cone_overlap_append","err_cone_parameter","err_cone_rep_var","err_cone_size","err_cone_type","err_cone_type_str","err_data_file_ext","err_dup_name","err_duplicate_aij","err_duplicate_barvariable_names","err_duplicate_cone_names","err_duplicate_constraint_names","err_duplicate_variable_names","err_end_of_file","err_factor","err_feasrepair_cannot_relax","err_feasrepair_inconsistent_bound","err_feasrepair_solving_relaxed","err_file_license","err_file_open","err_file_read","err_file_write","err_final_solution","err_first","err_firsti","err_firstj","err_fixed_bound_values","err_flexlm","err_format_string","err_global_inv_conic_problem","err_huge_aij","err_huge_c","err_identical_tasks","err_in_argument","err_index","err_index_arr_is_too_large","err_index_arr_is_too_small","err_index_is_too_large","err_index_is_too_small","err_inf_dou_index","err_inf_dou_name","err_inf_int_index","err_inf_int_name","err_inf_lint_index","err_inf_lint_name","err_inf_type","err_infeas_undefined","err_infinite_bound","err_int64_to_int32_cast","err_internal","err_internal_test_failed","err_inv_aptre","err_inv_bk","err_inv_bkc","err_inv_bkx","err_inv_cone_type","err_inv_cone_type_str","err_inv_marki","err_inv_markj","err_inv_name_item","err_inv_numi","err_inv_numj","err_inv_optimizer","err_inv_problem","err_inv_qcon_subi","err_inv_qcon_subj","err_inv_qcon_subk","err_inv_qcon_val","err_inv_qobj_subi","err_inv_qobj_subj","err_inv_qobj_val","err_inv_sk","err_inv_sk_str","err_inv_skc","err_inv_skn","err_inv_skx","err_inv_var_type","err_invalid_aij","err_invalid_ampl_stub","err_invalid_barvar_name","err_invalid_compression","err_invalid_con_name","err_invalid_cone_name","err_invalid_file_format_for_cfix","err_invalid_file_format_for_cones","err_invalid_file_format_for_free_constraints","err_invalid_file_format_for_nonlinear","err_invalid_file_format_for_ranged_constraints","err_invalid_file_format_for_sym_mat","err_invalid_file_name","err_invalid_format_type","err_invalid_idx","err_invalid_iomode","err_invalid_max_num","err_invalid_name_in_sol_file","err_invalid_obj_name","err_invalid_objective_sense","err_invalid_problem_type","err_invalid_sol_file_name","err_invalid_stream","err_invalid_surplus","err_invalid_sym_mat_dim","err_invalid_task","err_invalid_utf8","err_invalid_var_name","err_invalid_wchar","err_invalid_whichsol","err_json_data","err_json_format","err_json_missing_data","err_json_number_overflow","err_json_string","err_json_syntax","err_last","err_lasti","err_lastj","err_lau_arg_k","err_lau_arg_m","err_lau_arg_n","err_lau_arg_trans","err_lau_arg_transa","err_lau_arg_transb","err_lau_arg_uplo","err_lau_invalid_lower_triangular_matrix","err_lau_invalid_sparse_symmetric_matrix","err_lau_not_positive_definite","err_lau_singular_matrix","err_lau_unknown","err_license","err_license_cannot_allocate","err_license_cannot_connect","err_license_expired","err_license_feature","err_license_invalid_hostid","err_license_max","err_license_moseklm_daemon","err_license_no_server_line","err_license_no_server_support","err_license_server","err_license_server_version","err_license_version","err_link_file_dll","err_living_tasks","err_lower_bound_is_a_nan","err_lp_dup_slack_name","err_lp_empty","err_lp_file_format","err_lp_format","err_lp_free_constraint","err_lp_incompatible","err_lp_invalid_con_name","err_lp_invalid_var_name","err_lp_write_conic_problem","err_lp_write_geco_problem","err_lu_max_num_tries","err_max_len_is_too_small","err_maxnumbarvar","err_maxnumcon","err_maxnumcone","err_maxnumqnz","err_maxnumvar","err_mio_internal","err_mio_invalid_node_optimizer","err_mio_invalid_root_optimizer","err_mio_no_optimizer","err_missing_license_file","err_mixed_conic_and_nl","err_mps_cone_overlap","err_mps_cone_repeat","err_mps_cone_type","err_mps_duplicate_q_element","err_mps_file","err_mps_inv_bound_key","err_mps_inv_con_key","err_mps_inv_field","err_mps_inv_marker","err_mps_inv_sec_name","err_mps_inv_sec_order","err_mps_invalid_obj_name","err_mps_invalid_objsense","err_mps_mul_con_name","err_mps_mul_csec","err_mps_mul_qobj","err_mps_mul_qsec","err_mps_no_objective","err_mps_non_symmetric_q","err_mps_null_con_name","err_mps_null_var_name","err_mps_splitted_var","err_mps_tab_in_field2","err_mps_tab_in_field3","err_mps_tab_in_field5","err_mps_undef_con_name","err_mps_undef_var_name","err_mul_a_element","err_name_is_null","err_name_max_len","err_nan_in_blc","err_nan_in_blx","err_nan_in_buc","err_nan_in_bux","err_nan_in_c","err_nan_in_double_data","err_negative_append","err_negative_surplus","err_newer_dll","err_no_bars_for_solution","err_no_barx_for_solution","err_no_basis_sol","err_no_dual_for_itg_sol","err_no_dual_infeas_cer","err_no_init_env","err_no_optimizer_var_type","err_no_primal_infeas_cer","err_no_snx_for_bas_sol","err_no_solution_in_callback","err_non_unique_array","err_nonconvex","err_nonlinear_equality","err_nonlinear_ranged","err_null_env","err_null_pointer","err_null_task","err_num_arguments","err_numconlim","err_numvarlim","err_obj_q_not_nsd","err_obj_q_not_psd","err_objective_range","err_older_dll","err_opf_format","err_opf_new_variable","err_opf_premature_eof","err_optimizer_license","err_overflow","err_param_index","err_param_is_too_large","err_param_is_too_small","err_param_name","err_param_name_dou","err_param_name_int","err_param_name_str","err_param_type","err_param_value_str","err_platform_not_licensed","err_postsolve","err_pro_item","err_prob_license","err_ptf_format","err_qcon_subi_too_large","err_qcon_subi_too_small","err_qcon_upper_triangle","err_qobj_upper_triangle","err_read_format","err_read_lp_missing_end_tag","err_read_lp_nonexisting_name","err_remove_cone_variable","err_repair_invalid_problem","err_repair_optimization_failed","err_sen_bound_invalid_lo","err_sen_bound_invalid_up","err_sen_format","err_sen_index_invalid","err_sen_index_range","err_sen_invalid_regexp","err_sen_numerical","err_sen_solution_status","err_sen_undef_name","err_sen_unhandled_problem_type","err_server_connect","err_server_protocol","err_server_status","err_server_token","err_shape_is_too_large","err_size_license","err_size_license_con","err_size_license_intvar","err_size_license_numcores","err_size_license_var","err_slice_size","err_sol_file_invalid_number","err_solitem","err_solver_probtype","err_space","err_space_leaking","err_space_no_info","err_sym_mat_duplicate","err_sym_mat_huge","err_sym_mat_invalid","err_sym_mat_invalid_col_index","err_sym_mat_invalid_row_index","err_sym_mat_invalid_value","err_sym_mat_not_lower_tringular","err_task_incompatible","err_task_invalid","err_task_write","err_thread_cond_init","err_thread_create","err_thread_mutex_init","err_thread_mutex_lock","err_thread_mutex_unlock","err_toconic_constr_not_conic","err_toconic_constr_q_not_psd","err_toconic_constraint_fx","err_toconic_constraint_ra","err_toconic_objective_not_psd","err_too_small_a_truncation_value","err_too_small_max_num_nz","err_too_small_maxnumanz","err_unb_step_size","err_undef_solution","err_undefined_objective_sense","err_unhandled_solution_status","err_unknown","err_upper_bound_is_a_nan","err_upper_triangle","err_whichitem_not_allowed","err_whichsol","err_write_lp_format","err_write_lp_non_unique_name","err_write_mps_invalid_name","err_write_opf_invalid_var_name","err_writing_file","err_xml_invalid_problem_type","err_y_is_undefined","ok","trm_internal","trm_internal_stop","trm_max_iterations","trm_max_num_setbacks","trm_max_time","trm_mio_num_branches","trm_mio_num_relaxs","trm_num_max_num_int_solutions","trm_numerical_problem","trm_objective_range","trm_stall","trm_user_callback","wrn_ana_almost_int_bounds","wrn_ana_c_zero","wrn_ana_close_bounds","wrn_ana_empty_cols","wrn_ana_large_bounds","wrn_dropped_nz_qobj","wrn_duplicate_barvariable_names","wrn_duplicate_cone_names","wrn_duplicate_constraint_names","wrn_duplicate_variable_names","wrn_eliminator_space","wrn_empty_name","wrn_exp_cones_with_variables_fixed_at_zero","wrn_ignore_integer","wrn_incomplete_linear_dependency_check","wrn_large_aij","wrn_large_bound","wrn_large_cj","wrn_large_con_fx","wrn_large_lo_bound","wrn_large_up_bound","wrn_license_expire","wrn_license_feature_expire","wrn_license_server","wrn_lp_drop_variable","wrn_lp_old_quad_format","wrn_mio_infeasible_final","wrn_mps_split_bou_vector","wrn_mps_split_ran_vector","wrn_mps_split_rhs_vector","wrn_name_max_len","wrn_no_dualizer","wrn_no_global_optimizer","wrn_nz_in_upr_tri","wrn_open_param_file","wrn_param_ignored_cmio","wrn_param_name_dou","wrn_param_name_int","wrn_param_name_str","wrn_param_str_value","wrn_pow_cones_with_root_fixed_at_zero","wrn_presolve_outofspace","wrn_quad_cones_with_root_fixed_at_zero","wrn_rquad_cones_with_root_fixed_at_zero","wrn_sol_file_ignored_con","wrn_sol_file_ignored_var","wrn_sol_filter","wrn_spar_max_len","wrn_sym_mat_large","wrn_too_few_basis_vars","wrn_too_many_basis_vars","wrn_undef_sol_file_name","wrn_using_generic_names","wrn_write_changed_names","wrn_write_discarded_cfix","wrn_zero_aij","wrn_zeros_in_sparse_col","wrn_zeros_in_sparse_row"], [3102,3001,3002,3005,3999,1311,1227,1226,1201,5005,1197,1299,1198,3920,1266,1610,1615,1070,7116,7115,7108,7110,7107,7114,7130,7131,7123,7109,7112,7741,7113,7126,7121,7740,7132,7134,7135,7124,7111,7102,7105,7101,7100,7133,7138,7139,7106,7118,7119,7125,7117,7103,7120,7104,7136,7137,7122,1294,1293,1300,1302,1307,1320,1303,1301,1305,1306,1055,1071,1385,4502,4503,4500,4501,1059,1650,1700,1702,1701,1007,1052,1053,1054,1560,1570,1285,1287,1420,1014,1072,1503,1380,1375,3101,1200,1235,1222,1221,1204,1203,1219,1230,1220,1231,1225,1234,1232,3910,1400,3800,3000,3500,1253,1255,1256,1257,1272,1271,2501,2502,1280,2503,2504,1550,1500,1405,1406,1404,1407,1401,1402,1403,1270,1269,1267,1274,1268,1258,1473,3700,1079,1800,1076,1078,4001,4005,4003,4010,4002,4000,1056,1283,1246,1801,1247,1170,1075,1445,6000,1057,1062,1275,3950,1064,2900,1077,2901,1228,1179,1178,1180,1177,1176,1175,1571,1286,1288,7012,7010,7011,7018,7015,7016,7017,7002,7019,7001,7000,7005,1000,1020,1021,1001,1018,1025,1016,1017,1028,1027,1015,1026,1002,1040,1066,1390,1152,1151,1157,1160,1155,1150,1171,1154,1163,1164,2800,1289,1242,1240,1304,1243,1241,5010,7701,7700,1551,1008,1501,1118,1119,1117,1121,1100,1108,1107,1101,1102,1109,1115,1128,1122,1112,1116,1114,1113,1110,1120,1103,1104,1111,1125,1126,1127,1105,1106,1254,1760,1750,1461,1471,1462,1472,1470,1450,1578,1573,1036,3916,3915,1600,2950,2001,1063,1552,2000,2953,2500,5000,1291,1290,1292,1060,1065,1061,1199,1250,1251,1296,1295,1260,1035,1168,1169,1172,1013,1590,1210,1215,1216,1205,1206,1207,1208,1218,1217,1019,1580,1281,1006,1167,1409,1408,1417,1415,1090,1159,1162,1310,1710,1711,3054,3053,3050,3055,3052,3056,3058,3057,3051,3080,8000,8001,8002,8003,1202,1005,1010,1012,3900,1011,1572,1350,1237,1259,1051,1080,1081,3944,1482,1480,3941,3940,3943,3942,2560,2561,2562,1049,1048,1045,1046,1047,7803,7800,7801,7802,7804,1421,1245,1252,3100,1265,1446,6010,1050,1391,6020,1238,1236,1158,1161,1153,1156,1166,3600,1449,0,10030,10031,10000,10020,10001,10009,10008,10015,10025,10002,10006,10007,904,901,903,902,900,201,852,853,850,851,801,502,932,250,800,62,51,57,54,52,53,500,505,501,85,80,270,72,71,70,65,950,251,200,50,516,510,511,512,515,933,802,930,931,351,352,300,66,960,400,405,350,503,803,804,63,710,705])
rescodetype = Enum("rescodetype", ["err","ok","trm","unk","wrn"], [3,0,2,4,1])
scalingtype = Enum("scalingtype", ["aggressive","free","moderate","none"], [3,0,2,1])
scalingmethod = Enum("scalingmethod", ["free","pow2"], [1,0])
sensitivitytype = Enum("sensitivitytype", ["basis"], [0])
simseltype = Enum("simseltype", ["ase","devex","free","full","partial","se"], [2,3,0,1,5,4])
solitem = Enum("solitem", ["slc","slx","snx","suc","sux","xc","xx","y"], [3,5,7,4,6,0,1,2])
solsta = Enum("solsta", ["dual_feas","dual_illposed_cer","dual_infeas_cer","integer_optimal","optimal","prim_and_dual_feas","prim_feas","prim_illposed_cer","prim_infeas_cer","unknown"], [3,8,6,9,1,4,2,7,5,0])
soltype = Enum("soltype", ["bas","itg","itr"], [1,2,0])
solveform = Enum("solveform", ["dual","free","primal"], [2,0,1])
sparam = Enum("sparam", ["bas_sol_file_name","data_file_name","debug_file_name","int_sol_file_name","itr_sol_file_name","mio_debug_string","param_comment_sign","param_read_file_name","param_write_file_name","read_mps_bou_name","read_mps_obj_name","read_mps_ran_name","read_mps_rhs_name","remote_access_token","sensitivity_file_name","sensitivity_res_file_name","sol_filter_xc_low","sol_filter_xc_upr","sol_filter_xx_low","sol_filter_xx_upr","stat_file_name","stat_key","stat_name","write_lp_gen_var_name"], [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23])
stakey = Enum("stakey", ["bas","fix","inf","low","supbas","unk","upr"], [1,5,6,3,2,0,4])
startpointtype = Enum("startpointtype", ["constant","free","guess","satisfy_bounds"], [2,0,1,3])
streamtype = Enum("streamtype", ["err","log","msg","wrn"], [2,0,1,3])
value = Enum("value", ["license_buffer_length","max_str_len"], [21,1024])
variabletype = Enum("variabletype", ["type_cont","type_int"], [0,1])



class Env:
    def __init__(self,licensefile=None,debugfile=None):
        self.__obj = _msk.Env() if debugfile is None else _msk.Env(debugfile)

        if licensefile is not None:
            if isinstance(licensefile,str):
                res = self.__obj.putlicensepath(licensefile.encode('utf-8', errors='replace'))
                if res != 0:
                    self.__del__()
                    raise Error(rescode(res),"Error %d" % res)

        self.__obj.enablegarcolenv()

    def set_Stream(self,whichstream,func):
        if isinstance(whichstream, streamtype):
            if func is None:
                self.__obj.remove_Stream(whichstream)
            else:
                self.__obj.set_Stream(whichstream,func)
            pass

        else:
            raise TypeError("Invalid stream %s" % whichstream)

    def __del__(self):
        try:
            o = self.__obj
        except AttributeError:
            pass
        else:
            o.dispose()
            del self.__obj

    def __enter__(self):
        return self

    def __exit__(self,exc_type,exc_value,traceback):
        self.__del__()

    def Task(self,numcon=0,numvar=0):
        return Task(self,numcon,numvar)

    def checkoutlicense(self,_arg1_feature_): # 2
      """
      Check out a license feature from the license server ahead of time.
    
      checkoutlicense(self,_arg1_feature_)
        _arg1_feature: mosek.feature. Feature to check out from the license system.
      """
      if not isinstance(_arg1_feature_,feature): raise TypeError("Argument _arg1_feature has wrong type")
      _arg1_feature = _arg1_feature_
      res = self.__obj.checkoutlicense(_arg1_feature_)
      if res != 0:
        raise Error(rescode(res),"")
    
    def checkinlicense(self,_arg1_feature_): # 2
      """
      Check in a license feature back to the license server ahead of time.
    
      checkinlicense(self,_arg1_feature_)
        _arg1_feature: mosek.feature. Feature to check in to the license system.
      """
      if not isinstance(_arg1_feature_,feature): raise TypeError("Argument _arg1_feature has wrong type")
      _arg1_feature = _arg1_feature_
      res = self.__obj.checkinlicense(_arg1_feature_)
      if res != 0:
        raise Error(rescode(res),"")
    
    def checkinall(self): # 2
      """
      Check in all unused license features to the license token server.
    
      checkinall(self)
      """
      res = self.__obj.checkinall()
      if res != 0:
        raise Error(rescode(res),"")
    
    def setupthreads(self,_arg1_numthreads): # 2
      """
      Preallocates a thread pool.
    
      setupthreads(self,_arg1_numthreads)
        _arg1_numthreads: int. Number of threads.
      """
      _arg1_numthreads_ = _arg1_numthreads
      res = self.__obj.setupthreads(_arg1_numthreads_)
      if res != 0:
        raise Error(rescode(res),"")
    
    def echointro(self,_arg1_longver): # 2
      """
      Prints an intro to message stream.
    
      echointro(self,_arg1_longver)
        _arg1_longver: int. If non-zero, then the intro is slightly longer.
      """
      _arg1_longver_ = _arg1_longver
      res = self.__obj.echointro(_arg1_longver_)
      if res != 0:
        raise Error(rescode(res),"")
    
    @staticmethod
    def getcodedesc(_arg0_code_): # 2
      """
      Obtains a short description of a response code.
    
      getcodedesc(_arg0_code_)
        _arg0_code: mosek.rescode. A valid response code.
      returns: _arg1_symname,_arg2_str
        _arg1_symname: str. Symbolic name corresponding to the code.
        _arg2_str: str. Obtains a short description of a response code.
      """
      if not isinstance(_arg0_code_,rescode): raise TypeError("Argument _arg0_code has wrong type")
      _arg0_code = _arg0_code_
      arr__arg1_symname = (ctypes.c_char*value.max_str_len)()
      memview_arr__arg1_symname = memoryview(arr__arg1_symname)
      arr__arg2_str = (ctypes.c_char*value.max_str_len)()
      memview_arr__arg2_str = memoryview(arr__arg2_str)
      res,resargs = _msk.Env.getcodedesc(_arg0_code_,memview_arr__arg1_symname,memview_arr__arg2_str)
      if res != 0:
        raise Error(rescode(res),"")
      retarg__arg1_symname,retarg__arg2_str = resargs
      retarg__arg2_str = arr__arg2_str.value.decode("utf-8",errors="ignore")
      retarg__arg1_symname = arr__arg1_symname.value.decode("utf-8",errors="ignore")
      return retarg__arg1_symname,retarg__arg2_str
    
    @staticmethod
    def getversion(): # 2
      """
      Obtains MOSEK version information.
    
      getversion()
      returns: _arg0_major,_arg1_minor,_arg2_revision
        _arg0_major: int. Major version number.
        _arg1_minor: int. Minor version number.
        _arg2_revision: int. Revision number.
      """
      res,resargs = _msk.Env.getversion()
      if res != 0:
        raise Error(rescode(res),"")
      __arg0_major_return_value,__arg1_minor_return_value,__arg2_revision_return_value = resargs
      return __arg0_major_return_value,__arg1_minor_return_value,__arg2_revision_return_value
    
    def linkfiletostream(self,_arg1_whichstream_,_arg2_filename_,_arg3_append): # 2
      """
      Directs all output from a stream to a file.
    
      linkfiletostream(self,_arg1_whichstream_,_arg2_filename_,_arg3_append)
        _arg1_whichstream: mosek.streamtype. <no description>
        _arg2_filename: str. <no description>
        _arg3_append: int. If this argument is 0 the file will be overwritten, otherwise it will be appended to.
      """
      if not isinstance(_arg1_whichstream_,streamtype): raise TypeError("Argument _arg1_whichstream has wrong type")
      _arg1_whichstream = _arg1_whichstream_
      _arg3_append_ = _arg3_append
      res = self.__obj.linkfiletoenvstream(_arg1_whichstream_,_arg2_filename_,_arg3_append_)
      if res != 0:
        raise Error(rescode(res),"")
    
    def putlicensedebug(self,_arg1_licdebug): # 2
      """
      Enables debug information for the license system.
    
      putlicensedebug(self,_arg1_licdebug)
        _arg1_licdebug: int. Enable output of license check-out debug information.
      """
      _arg1_licdebug_ = _arg1_licdebug
      res = self.__obj.putlicensedebug(_arg1_licdebug_)
      if res != 0:
        raise Error(rescode(res),"")
    
    def putlicensecode(self,_arg1_code): # 2
      """
      Input a runtime license code.
    
      putlicensecode(self,_arg1_code)
        _arg1_code: array of int. A license key string.
      """
      if _arg1_code is None:
        _arg1_code_ = None
      else:
        try:
          _arg1_code_ = memoryview(_arg1_code)
        except TypeError:
          try:
            _tmparr__arg1_code = (ctypes.c_int32*len(_arg1_code))()
            _tmparr__arg1_code[:] = _arg1_code
          except TypeError:
            raise TypeError("Argument _arg1_code has wrong type")
          else:
            _arg1_code_ = memoryview(_tmparr__arg1_code)
      
        else:
          if _arg1_code_.format != "i":
            _tmparr__arg1_code = (ctypes.c_int32*len(_arg1_code))()
            _tmparr__arg1_code[:] = _arg1_code
            _arg1_code_ = memoryview(_tmparr__arg1_code)
      
      if _arg1_code_ is not None and len(_arg1_code_) != value.license_buffer_length:
        raise ValueError("Array argument _arg1_code has wrong length")
      res = self.__obj.putlicensecode(_arg1_code_)
      if res != 0:
        raise Error(rescode(res),"")
    
    def putlicensewait(self,_arg1_licwait): # 2
      """
      Control whether mosek should wait for an available license if no license is available.
    
      putlicensewait(self,_arg1_licwait)
        _arg1_licwait: int. Enable waiting for a license.
      """
      _arg1_licwait_ = _arg1_licwait
      res = self.__obj.putlicensewait(_arg1_licwait_)
      if res != 0:
        raise Error(rescode(res),"")
    
    def putlicensepath(self,_arg1_licensepath_): # 2
      """
      Set the path to the license file.
    
      putlicensepath(self,_arg1_licensepath_)
        _arg1_licensepath: str. A path specifying where to search for the license.
      """
      res = self.__obj.putlicensepath(_arg1_licensepath_)
      if res != 0:
        raise Error(rescode(res),"")
    
    def axpy(self,_arg1_n,_arg2_alpha,_arg3_x,_arg4_y): # 2
      """
      Computes vector addition and multiplication by a scalar.
    
      axpy(self,_arg1_n,_arg2_alpha,_arg3_x,_arg4_y)
        _arg1_n: int. Length of the vectors.
        _arg2_alpha: double. The scalar that multiplies x.
        _arg3_x: array of double. The x vector.
        _arg4_y: array of double. The y vector.
      """
      _arg1_n_ = _arg1_n
      _arg2_alpha_ = _arg2_alpha
      if _arg3_x is None: raise TypeError("Invalid type for argument _arg3_x")
      if _arg3_x is None:
        _arg3_x_ = None
      else:
        try:
          _arg3_x_ = memoryview(_arg3_x)
        except TypeError:
          try:
            _tmparr__arg3_x = (ctypes.c_double*len(_arg3_x))()
            _tmparr__arg3_x[:] = _arg3_x
          except TypeError:
            raise TypeError("Argument _arg3_x has wrong type")
          else:
            _arg3_x_ = memoryview(_tmparr__arg3_x)
      
        else:
          if _arg3_x_.format != "d":
            _tmparr__arg3_x = (ctypes.c_double*len(_arg3_x))()
            _tmparr__arg3_x[:] = _arg3_x
            _arg3_x_ = memoryview(_tmparr__arg3_x)
      
      if _arg3_x_ is not None and len(_arg3_x_) !=  _arg1_n :
        raise ValueError("Array argument _arg3_x has wrong length")
      if _arg4_y is None: raise TypeError("Invalid type for argument _arg4_y")
      _copyback__arg4_y = False
      if _arg4_y is None:
        _arg4_y_ = None
      else:
        try:
          _arg4_y_ = memoryview(_arg4_y)
        except TypeError:
          try:
            _tmparr__arg4_y = (ctypes.c_double*len(_arg4_y))()
            _tmparr__arg4_y[:] = _arg4_y
          except TypeError:
            raise TypeError("Argument _arg4_y has wrong type")
          else:
            _arg4_y_ = memoryview(_tmparr__arg4_y)
            _copyback__arg4_y = True
        else:
          if _arg4_y_.format != "d":
            _tmparr__arg4_y = (ctypes.c_double*len(_arg4_y))()
            _tmparr__arg4_y[:] = _arg4_y
            _arg4_y_ = memoryview(_tmparr__arg4_y)
            _copyback__arg4_y = True
      if _arg4_y_ is not None and len(_arg4_y_) !=  _arg1_n :
        raise ValueError("Array argument _arg4_y has wrong length")
      res = self.__obj.axpy(_arg1_n_,_arg2_alpha_,_arg3_x_,_arg4_y_)
      if res != 0:
        raise Error(rescode(res),"")
      if _copyback__arg4_y:
        _arg4_y[:] = _tmparr__arg4_y
    
    def dot(self,_arg1_n,_arg2_x,_arg3_y): # 2
      """
      Computes the inner product of two vectors.
    
      dot(self,_arg1_n,_arg2_x,_arg3_y)
        _arg1_n: int. Length of the vectors.
        _arg2_x: array of double. The x vector.
        _arg3_y: array of double. The y vector.
      returns: _arg4_xty
        _arg4_xty: double. The result of the inner product.
      """
      _arg1_n_ = _arg1_n
      if _arg2_x is None: raise TypeError("Invalid type for argument _arg2_x")
      if _arg2_x is None:
        _arg2_x_ = None
      else:
        try:
          _arg2_x_ = memoryview(_arg2_x)
        except TypeError:
          try:
            _tmparr__arg2_x = (ctypes.c_double*len(_arg2_x))()
            _tmparr__arg2_x[:] = _arg2_x
          except TypeError:
            raise TypeError("Argument _arg2_x has wrong type")
          else:
            _arg2_x_ = memoryview(_tmparr__arg2_x)
      
        else:
          if _arg2_x_.format != "d":
            _tmparr__arg2_x = (ctypes.c_double*len(_arg2_x))()
            _tmparr__arg2_x[:] = _arg2_x
            _arg2_x_ = memoryview(_tmparr__arg2_x)
      
      if _arg2_x_ is not None and len(_arg2_x_) !=  _arg1_n :
        raise ValueError("Array argument _arg2_x has wrong length")
      if _arg3_y is None: raise TypeError("Invalid type for argument _arg3_y")
      if _arg3_y is None:
        _arg3_y_ = None
      else:
        try:
          _arg3_y_ = memoryview(_arg3_y)
        except TypeError:
          try:
            _tmparr__arg3_y = (ctypes.c_double*len(_arg3_y))()
            _tmparr__arg3_y[:] = _arg3_y
          except TypeError:
            raise TypeError("Argument _arg3_y has wrong type")
          else:
            _arg3_y_ = memoryview(_tmparr__arg3_y)
      
        else:
          if _arg3_y_.format != "d":
            _tmparr__arg3_y = (ctypes.c_double*len(_arg3_y))()
            _tmparr__arg3_y[:] = _arg3_y
            _arg3_y_ = memoryview(_tmparr__arg3_y)
      
      if _arg3_y_ is not None and len(_arg3_y_) !=  _arg1_n :
        raise ValueError("Array argument _arg3_y has wrong length")
      res,resargs = self.__obj.dot(_arg1_n_,_arg2_x_,_arg3_y_)
      if res != 0:
        raise Error(rescode(res),"")
      __arg4_xty_return_value = resargs
      return __arg4_xty_return_value
    
    def gemv(self,_arg1_transa_,_arg2_m,_arg3_n,_arg4_alpha,_arg5_a,_arg6_x,_arg7_beta,_arg8_y): # 2
      """
      Computes dense matrix times a dense vector product.
    
      gemv(self,_arg1_transa_,_arg2_m,_arg3_n,_arg4_alpha,_arg5_a,_arg6_x,_arg7_beta,_arg8_y)
        _arg1_transa: mosek.transpose. Indicates whether the matrix A must be transposed.
        _arg2_m: int. Specifies the number of rows of the matrix A.
        _arg3_n: int. Specifies the number of columns of the matrix A.
        _arg4_alpha: double. A scalar value multiplying the matrix A.
        _arg5_a: array of double. A pointer to the array storing matrix A in a column-major format.
        _arg6_x: array of double. A pointer to the array storing the vector x.
        _arg7_beta: double. A scalar value multiplying the vector y.
        _arg8_y: array of double. A pointer to the array storing the vector y.
      """
      if not isinstance(_arg1_transa_,transpose): raise TypeError("Argument _arg1_transa has wrong type")
      _arg1_transa = _arg1_transa_
      _arg2_m_ = _arg2_m
      _arg3_n_ = _arg3_n
      _arg4_alpha_ = _arg4_alpha
      if _arg5_a is None: raise TypeError("Invalid type for argument _arg5_a")
      if _arg5_a is None:
        _arg5_a_ = None
      else:
        try:
          _arg5_a_ = memoryview(_arg5_a)
        except TypeError:
          try:
            _tmparr__arg5_a = (ctypes.c_double*len(_arg5_a))()
            _tmparr__arg5_a[:] = _arg5_a
          except TypeError:
            raise TypeError("Argument _arg5_a has wrong type")
          else:
            _arg5_a_ = memoryview(_tmparr__arg5_a)
      
        else:
          if _arg5_a_.format != "d":
            _tmparr__arg5_a = (ctypes.c_double*len(_arg5_a))()
            _tmparr__arg5_a[:] = _arg5_a
            _arg5_a_ = memoryview(_tmparr__arg5_a)
      
      if _arg5_a_ is not None and len(_arg5_a_) != ( _arg3_n  *  _arg2_m ):
        raise ValueError("Array argument _arg5_a has wrong length")
      if _arg6_x is None: raise TypeError("Invalid type for argument _arg6_x")
      if _arg6_x is None:
        _arg6_x_ = None
      else:
        try:
          _arg6_x_ = memoryview(_arg6_x)
        except TypeError:
          try:
            _tmparr__arg6_x = (ctypes.c_double*len(_arg6_x))()
            _tmparr__arg6_x[:] = _arg6_x
          except TypeError:
            raise TypeError("Argument _arg6_x has wrong type")
          else:
            _arg6_x_ = memoryview(_tmparr__arg6_x)
      
        else:
          if _arg6_x_.format != "d":
            _tmparr__arg6_x = (ctypes.c_double*len(_arg6_x))()
            _tmparr__arg6_x[:] = _arg6_x
            _arg6_x_ = memoryview(_tmparr__arg6_x)
      
      if ( _arg1_transa  == transpose.no):
        __tmp_var_0 =  _arg3_n ;
      else:
        __tmp_var_0 =  _arg2_m ;
      if _arg6_x_ is not None and len(_arg6_x_) != __tmp_var_0:
        raise ValueError("Array argument _arg6_x has wrong length")
      _arg7_beta_ = _arg7_beta
      if _arg8_y is None: raise TypeError("Invalid type for argument _arg8_y")
      _copyback__arg8_y = False
      if _arg8_y is None:
        _arg8_y_ = None
      else:
        try:
          _arg8_y_ = memoryview(_arg8_y)
        except TypeError:
          try:
            _tmparr__arg8_y = (ctypes.c_double*len(_arg8_y))()
            _tmparr__arg8_y[:] = _arg8_y
          except TypeError:
            raise TypeError("Argument _arg8_y has wrong type")
          else:
            _arg8_y_ = memoryview(_tmparr__arg8_y)
            _copyback__arg8_y = True
        else:
          if _arg8_y_.format != "d":
            _tmparr__arg8_y = (ctypes.c_double*len(_arg8_y))()
            _tmparr__arg8_y[:] = _arg8_y
            _arg8_y_ = memoryview(_tmparr__arg8_y)
            _copyback__arg8_y = True
      if ( _arg1_transa  == transpose.no):
        __tmp_var_1 =  _arg2_m ;
      else:
        __tmp_var_1 =  _arg3_n ;
      if _arg8_y_ is not None and len(_arg8_y_) != __tmp_var_1:
        raise ValueError("Array argument _arg8_y has wrong length")
      res = self.__obj.gemv(_arg1_transa_,_arg2_m_,_arg3_n_,_arg4_alpha_,_arg5_a_,_arg6_x_,_arg7_beta_,_arg8_y_)
      if res != 0:
        raise Error(rescode(res),"")
      if _copyback__arg8_y:
        _arg8_y[:] = _tmparr__arg8_y
    
    def gemm(self,_arg1_transa_,_arg2_transb_,_arg3_m,_arg4_n,_arg5_k,_arg6_alpha,_arg7_a,_arg8_b,_arg9_beta,_arg10_c): # 2
      """
      Performs a dense matrix multiplication.
    
      gemm(self,_arg1_transa_,_arg2_transb_,_arg3_m,_arg4_n,_arg5_k,_arg6_alpha,_arg7_a,_arg8_b,_arg9_beta,_arg10_c)
        _arg1_transa: mosek.transpose. Indicates whether the matrix A must be transposed.
        _arg2_transb: mosek.transpose. Indicates whether the matrix B must be transposed.
        _arg3_m: int. Indicates the number of rows of matrix C.
        _arg4_n: int. Indicates the number of columns of matrix C.
        _arg5_k: int. Specifies the common dimension along which op(A) and op(B) are multiplied.
        _arg6_alpha: double. A scalar value multiplying the result of the matrix multiplication.
        _arg7_a: array of double. The pointer to the array storing matrix A in a column-major format.
        _arg8_b: array of double. The pointer to the array storing matrix B in a column-major format.
        _arg9_beta: double. A scalar value that multiplies C.
        _arg10_c: array of double. The pointer to the array storing matrix C in a column-major format.
      """
      if not isinstance(_arg1_transa_,transpose): raise TypeError("Argument _arg1_transa has wrong type")
      _arg1_transa = _arg1_transa_
      if not isinstance(_arg2_transb_,transpose): raise TypeError("Argument _arg2_transb has wrong type")
      _arg2_transb = _arg2_transb_
      _arg3_m_ = _arg3_m
      _arg4_n_ = _arg4_n
      _arg5_k_ = _arg5_k
      _arg6_alpha_ = _arg6_alpha
      if _arg7_a is None: raise TypeError("Invalid type for argument _arg7_a")
      if _arg7_a is None:
        _arg7_a_ = None
      else:
        try:
          _arg7_a_ = memoryview(_arg7_a)
        except TypeError:
          try:
            _tmparr__arg7_a = (ctypes.c_double*len(_arg7_a))()
            _tmparr__arg7_a[:] = _arg7_a
          except TypeError:
            raise TypeError("Argument _arg7_a has wrong type")
          else:
            _arg7_a_ = memoryview(_tmparr__arg7_a)
      
        else:
          if _arg7_a_.format != "d":
            _tmparr__arg7_a = (ctypes.c_double*len(_arg7_a))()
            _tmparr__arg7_a[:] = _arg7_a
            _arg7_a_ = memoryview(_tmparr__arg7_a)
      
      if _arg7_a_ is not None and len(_arg7_a_) != ( _arg3_m  *  _arg5_k ):
        raise ValueError("Array argument _arg7_a has wrong length")
      if _arg8_b is None: raise TypeError("Invalid type for argument _arg8_b")
      if _arg8_b is None:
        _arg8_b_ = None
      else:
        try:
          _arg8_b_ = memoryview(_arg8_b)
        except TypeError:
          try:
            _tmparr__arg8_b = (ctypes.c_double*len(_arg8_b))()
            _tmparr__arg8_b[:] = _arg8_b
          except TypeError:
            raise TypeError("Argument _arg8_b has wrong type")
          else:
            _arg8_b_ = memoryview(_tmparr__arg8_b)
      
        else:
          if _arg8_b_.format != "d":
            _tmparr__arg8_b = (ctypes.c_double*len(_arg8_b))()
            _tmparr__arg8_b[:] = _arg8_b
            _arg8_b_ = memoryview(_tmparr__arg8_b)
      
      if _arg8_b_ is not None and len(_arg8_b_) != ( _arg5_k  *  _arg4_n ):
        raise ValueError("Array argument _arg8_b has wrong length")
      _arg9_beta_ = _arg9_beta
      if _arg10_c is None: raise TypeError("Invalid type for argument _arg10_c")
      _copyback__arg10_c = False
      if _arg10_c is None:
        _arg10_c_ = None
      else:
        try:
          _arg10_c_ = memoryview(_arg10_c)
        except TypeError:
          try:
            _tmparr__arg10_c = (ctypes.c_double*len(_arg10_c))()
            _tmparr__arg10_c[:] = _arg10_c
          except TypeError:
            raise TypeError("Argument _arg10_c has wrong type")
          else:
            _arg10_c_ = memoryview(_tmparr__arg10_c)
            _copyback__arg10_c = True
        else:
          if _arg10_c_.format != "d":
            _tmparr__arg10_c = (ctypes.c_double*len(_arg10_c))()
            _tmparr__arg10_c[:] = _arg10_c
            _arg10_c_ = memoryview(_tmparr__arg10_c)
            _copyback__arg10_c = True
      if _arg10_c_ is not None and len(_arg10_c_) != ( _arg3_m  *  _arg4_n ):
        raise ValueError("Array argument _arg10_c has wrong length")
      res = self.__obj.gemm(_arg1_transa_,_arg2_transb_,_arg3_m_,_arg4_n_,_arg5_k_,_arg6_alpha_,_arg7_a_,_arg8_b_,_arg9_beta_,_arg10_c_)
      if res != 0:
        raise Error(rescode(res),"")
      if _copyback__arg10_c:
        _arg10_c[:] = _tmparr__arg10_c
    
    def syrk(self,_arg1_uplo_,_arg2_trans_,_arg3_n,_arg4_k,_arg5_alpha,_arg6_a,_arg7_beta,_arg8_c): # 2
      """
      Performs a rank-k update of a symmetric matrix.
    
      syrk(self,_arg1_uplo_,_arg2_trans_,_arg3_n,_arg4_k,_arg5_alpha,_arg6_a,_arg7_beta,_arg8_c)
        _arg1_uplo: mosek.uplo. Indicates whether the upper or lower triangular part of C is used.
        _arg2_trans: mosek.transpose. Indicates whether the matrix A must be transposed.
        _arg3_n: int. Specifies the order of C.
        _arg4_k: int. Indicates the number of rows or columns of A, and its rank.
        _arg5_alpha: double. A scalar value multiplying the result of the matrix multiplication.
        _arg6_a: array of double. The pointer to the array storing matrix A in a column-major format.
        _arg7_beta: double. A scalar value that multiplies C.
        _arg8_c: array of double. The pointer to the array storing matrix C in a column-major format.
      """
      if not isinstance(_arg1_uplo_,uplo): raise TypeError("Argument _arg1_uplo has wrong type")
      _arg1_uplo = _arg1_uplo_
      if not isinstance(_arg2_trans_,transpose): raise TypeError("Argument _arg2_trans has wrong type")
      _arg2_trans = _arg2_trans_
      _arg3_n_ = _arg3_n
      _arg4_k_ = _arg4_k
      _arg5_alpha_ = _arg5_alpha
      if _arg6_a is None: raise TypeError("Invalid type for argument _arg6_a")
      if _arg6_a is None:
        _arg6_a_ = None
      else:
        try:
          _arg6_a_ = memoryview(_arg6_a)
        except TypeError:
          try:
            _tmparr__arg6_a = (ctypes.c_double*len(_arg6_a))()
            _tmparr__arg6_a[:] = _arg6_a
          except TypeError:
            raise TypeError("Argument _arg6_a has wrong type")
          else:
            _arg6_a_ = memoryview(_tmparr__arg6_a)
      
        else:
          if _arg6_a_.format != "d":
            _tmparr__arg6_a = (ctypes.c_double*len(_arg6_a))()
            _tmparr__arg6_a[:] = _arg6_a
            _arg6_a_ = memoryview(_tmparr__arg6_a)
      
      if _arg6_a_ is not None and len(_arg6_a_) != ( _arg3_n  *  _arg4_k ):
        raise ValueError("Array argument _arg6_a has wrong length")
      _arg7_beta_ = _arg7_beta
      if _arg8_c is None: raise TypeError("Invalid type for argument _arg8_c")
      _copyback__arg8_c = False
      if _arg8_c is None:
        _arg8_c_ = None
      else:
        try:
          _arg8_c_ = memoryview(_arg8_c)
        except TypeError:
          try:
            _tmparr__arg8_c = (ctypes.c_double*len(_arg8_c))()
            _tmparr__arg8_c[:] = _arg8_c
          except TypeError:
            raise TypeError("Argument _arg8_c has wrong type")
          else:
            _arg8_c_ = memoryview(_tmparr__arg8_c)
            _copyback__arg8_c = True
        else:
          if _arg8_c_.format != "d":
            _tmparr__arg8_c = (ctypes.c_double*len(_arg8_c))()
            _tmparr__arg8_c[:] = _arg8_c
            _arg8_c_ = memoryview(_tmparr__arg8_c)
            _copyback__arg8_c = True
      if _arg8_c_ is not None and len(_arg8_c_) != ( _arg3_n  *  _arg3_n ):
        raise ValueError("Array argument _arg8_c has wrong length")
      res = self.__obj.syrk(_arg1_uplo_,_arg2_trans_,_arg3_n_,_arg4_k_,_arg5_alpha_,_arg6_a_,_arg7_beta_,_arg8_c_)
      if res != 0:
        raise Error(rescode(res),"")
      if _copyback__arg8_c:
        _arg8_c[:] = _tmparr__arg8_c
    
    def computesparsecholesky(self,_arg1_multithread,_arg2_ordermethod,_arg3_tolsingular,_arg5_anzc,_arg6_aptrc,_arg7_asubc,_arg8_avalc): # 2
      """
      Computes a Cholesky factorization of sparse matrix.
    
      computesparsecholesky(self,_arg1_multithread,_arg2_ordermethod,_arg3_tolsingular,_arg5_anzc,_arg6_aptrc,_arg7_asubc,_arg8_avalc)
        _arg1_multithread: int. If nonzero then the function may exploit multiple threads.
        _arg2_ordermethod: int. If nonzero, then a sparsity preserving ordering will be employed.
        _arg3_tolsingular: double. A positive parameter controlling when a pivot is declared zero.
        _arg5_anzc: array of int. anzc[j] is the number of nonzeros in the jth column of A.
        _arg6_aptrc: array of long. aptrc[j] is a pointer to the first element in column j.
        _arg7_asubc: array of int. Row indexes for each column stored in increasing order.
        _arg8_avalc: array of double. The value corresponding to row indexed stored in asubc.
      returns: _arg13_lensubnval
        _arg13_lensubnval: long. Number of elements in lsubc and lvalc.
      """
      _arg1_multithread_ = _arg1_multithread
      _arg2_ordermethod_ = _arg2_ordermethod
      _arg3_tolsingular_ = _arg3_tolsingular
      _arg4_n_ = None
      if _arg4_n_ is None:
        _arg4_n_ = len(_arg5_anzc)
        _arg4_n = _arg4_n_
      elif _arg4_n_ != len(_arg5_anzc):
        raise IndexError("Inconsistent length of array _arg5_anzc")
      if _arg4_n_ is None:
        _arg4_n_ = len(_arg6_aptrc)
        _arg4_n = _arg4_n_
      elif _arg4_n_ != len(_arg6_aptrc):
        raise IndexError("Inconsistent length of array _arg6_aptrc")
      if _arg4_n_ is None: _arg4_n_ = 0
      if _arg5_anzc is None: raise TypeError("Invalid type for argument _arg5_anzc")
      if _arg5_anzc is None:
        _arg5_anzc_ = None
      else:
        try:
          _arg5_anzc_ = memoryview(_arg5_anzc)
        except TypeError:
          try:
            _tmparr__arg5_anzc = (ctypes.c_int32*len(_arg5_anzc))()
            _tmparr__arg5_anzc[:] = _arg5_anzc
          except TypeError:
            raise TypeError("Argument _arg5_anzc has wrong type")
          else:
            _arg5_anzc_ = memoryview(_tmparr__arg5_anzc)
      
        else:
          if _arg5_anzc_.format != "i":
            _tmparr__arg5_anzc = (ctypes.c_int32*len(_arg5_anzc))()
            _tmparr__arg5_anzc[:] = _arg5_anzc
            _arg5_anzc_ = memoryview(_tmparr__arg5_anzc)
      
      if _arg6_aptrc is None: raise TypeError("Invalid type for argument _arg6_aptrc")
      if _arg6_aptrc is None:
        _arg6_aptrc_ = None
      else:
        try:
          _arg6_aptrc_ = memoryview(_arg6_aptrc)
        except TypeError:
          try:
            _tmparr__arg6_aptrc = (ctypes.c_int64*len(_arg6_aptrc))()
            _tmparr__arg6_aptrc[:] = _arg6_aptrc
          except TypeError:
            raise TypeError("Argument _arg6_aptrc has wrong type")
          else:
            _arg6_aptrc_ = memoryview(_tmparr__arg6_aptrc)
      
        else:
          if _arg6_aptrc_.format != "q":
            _tmparr__arg6_aptrc = (ctypes.c_int64*len(_arg6_aptrc))()
            _tmparr__arg6_aptrc[:] = _arg6_aptrc
            _arg6_aptrc_ = memoryview(_tmparr__arg6_aptrc)
      
      if _arg7_asubc is None: raise TypeError("Invalid type for argument _arg7_asubc")
      if _arg7_asubc is None:
        _arg7_asubc_ = None
      else:
        try:
          _arg7_asubc_ = memoryview(_arg7_asubc)
        except TypeError:
          try:
            _tmparr__arg7_asubc = (ctypes.c_int32*len(_arg7_asubc))()
            _tmparr__arg7_asubc[:] = _arg7_asubc
          except TypeError:
            raise TypeError("Argument _arg7_asubc has wrong type")
          else:
            _arg7_asubc_ = memoryview(_tmparr__arg7_asubc)
      
        else:
          if _arg7_asubc_.format != "i":
            _tmparr__arg7_asubc = (ctypes.c_int32*len(_arg7_asubc))()
            _tmparr__arg7_asubc[:] = _arg7_asubc
            _arg7_asubc_ = memoryview(_tmparr__arg7_asubc)
      
      if _arg8_avalc is None: raise TypeError("Invalid type for argument _arg8_avalc")
      if _arg8_avalc is None:
        _arg8_avalc_ = None
      else:
        try:
          _arg8_avalc_ = memoryview(_arg8_avalc)
        except TypeError:
          try:
            _tmparr__arg8_avalc = (ctypes.c_double*len(_arg8_avalc))()
            _tmparr__arg8_avalc[:] = _arg8_avalc
          except TypeError:
            raise TypeError("Argument _arg8_avalc has wrong type")
          else:
            _arg8_avalc_ = memoryview(_tmparr__arg8_avalc)
      
        else:
          if _arg8_avalc_.format != "d":
            _tmparr__arg8_avalc = (ctypes.c_double*len(_arg8_avalc))()
            _tmparr__arg8_avalc[:] = _arg8_avalc
            _arg8_avalc_ = memoryview(_tmparr__arg8_avalc)
      
      res,resargs = self.__obj.computesparsecholesky(_arg1_multithread_,_arg2_ordermethod_,_arg3_tolsingular_,_arg4_n_,_arg5_anzc_,_arg6_aptrc_,_arg7_asubc_,_arg8_avalc_)
      if res != 0:
        raise Error(rescode(res),"")
      __arg9_perm,__arg10_diag,__arg11_lnzc,__arg12_lptrc,__arg13_lensubnval_return_value,__arg14_lsubc,__arg15_lvalc = resargs
      return __arg9_perm,__arg10_diag,__arg11_lnzc,__arg12_lptrc,__arg13_lensubnval_return_value,__arg14_lsubc,__arg15_lvalc
    
    def sparsetriangularsolvedense(self,_arg1_transposed_,_arg3_lnzc,_arg4_lptrc,_arg6_lsubc,_arg7_lvalc,_arg8_b): # 2
      """
      Solves a sparse triangular system of linear equations.
    
      sparsetriangularsolvedense(self,_arg1_transposed_,_arg3_lnzc,_arg4_lptrc,_arg6_lsubc,_arg7_lvalc,_arg8_b)
        _arg1_transposed: mosek.transpose. Controls whether the solve is with L or the transposed L.
        _arg3_lnzc: array of int. lnzc[j] is the number of nonzeros in column j.
        _arg4_lptrc: array of long. lptrc[j] is a pointer to the first row index and value in column j.
        _arg6_lsubc: array of int. Row indexes for each column stored sequentially.
        _arg7_lvalc: array of double. The value corresponding to row indexed stored lsubc.
        _arg8_b: array of double. The right-hand side of linear equation system to be solved as a dense vector.
      """
      if not isinstance(_arg1_transposed_,transpose): raise TypeError("Argument _arg1_transposed has wrong type")
      _arg1_transposed = _arg1_transposed_
      _arg2_n_ = None
      if _arg2_n_ is None:
        _arg2_n_ = len(_arg8_b)
        _arg2_n = _arg2_n_
      elif _arg2_n_ != len(_arg8_b):
        raise IndexError("Inconsistent length of array _arg8_b")
      if _arg2_n_ is None:
        _arg2_n_ = len(_arg3_lnzc)
        _arg2_n = _arg2_n_
      elif _arg2_n_ != len(_arg3_lnzc):
        raise IndexError("Inconsistent length of array _arg3_lnzc")
      if _arg2_n_ is None:
        _arg2_n_ = len(_arg4_lptrc)
        _arg2_n = _arg2_n_
      elif _arg2_n_ != len(_arg4_lptrc):
        raise IndexError("Inconsistent length of array _arg4_lptrc")
      if _arg2_n_ is None: _arg2_n_ = 0
      if _arg3_lnzc is None: raise TypeError("Invalid type for argument _arg3_lnzc")
      if _arg3_lnzc is None:
        _arg3_lnzc_ = None
      else:
        try:
          _arg3_lnzc_ = memoryview(_arg3_lnzc)
        except TypeError:
          try:
            _tmparr__arg3_lnzc = (ctypes.c_int32*len(_arg3_lnzc))()
            _tmparr__arg3_lnzc[:] = _arg3_lnzc
          except TypeError:
            raise TypeError("Argument _arg3_lnzc has wrong type")
          else:
            _arg3_lnzc_ = memoryview(_tmparr__arg3_lnzc)
      
        else:
          if _arg3_lnzc_.format != "i":
            _tmparr__arg3_lnzc = (ctypes.c_int32*len(_arg3_lnzc))()
            _tmparr__arg3_lnzc[:] = _arg3_lnzc
            _arg3_lnzc_ = memoryview(_tmparr__arg3_lnzc)
      
      if _arg3_lnzc_ is not None and len(_arg3_lnzc_) !=  _arg2_n :
        raise ValueError("Array argument _arg3_lnzc has wrong length")
      if _arg4_lptrc is None: raise TypeError("Invalid type for argument _arg4_lptrc")
      if _arg4_lptrc is None:
        _arg4_lptrc_ = None
      else:
        try:
          _arg4_lptrc_ = memoryview(_arg4_lptrc)
        except TypeError:
          try:
            _tmparr__arg4_lptrc = (ctypes.c_int64*len(_arg4_lptrc))()
            _tmparr__arg4_lptrc[:] = _arg4_lptrc
          except TypeError:
            raise TypeError("Argument _arg4_lptrc has wrong type")
          else:
            _arg4_lptrc_ = memoryview(_tmparr__arg4_lptrc)
      
        else:
          if _arg4_lptrc_.format != "q":
            _tmparr__arg4_lptrc = (ctypes.c_int64*len(_arg4_lptrc))()
            _tmparr__arg4_lptrc[:] = _arg4_lptrc
            _arg4_lptrc_ = memoryview(_tmparr__arg4_lptrc)
      
      if _arg4_lptrc_ is not None and len(_arg4_lptrc_) !=  _arg2_n :
        raise ValueError("Array argument _arg4_lptrc has wrong length")
      _arg5_lensubnval_ = None
      if _arg5_lensubnval_ is None:
        _arg5_lensubnval_ = len(_arg6_lsubc)
        _arg5_lensubnval = _arg5_lensubnval_
      elif _arg5_lensubnval_ != len(_arg6_lsubc):
        raise IndexError("Inconsistent length of array _arg6_lsubc")
      if _arg5_lensubnval_ is None:
        _arg5_lensubnval_ = len(_arg7_lvalc)
        _arg5_lensubnval = _arg5_lensubnval_
      elif _arg5_lensubnval_ != len(_arg7_lvalc):
        raise IndexError("Inconsistent length of array _arg7_lvalc")
      if _arg5_lensubnval_ is None: _arg5_lensubnval_ = 0
      if _arg6_lsubc is None: raise TypeError("Invalid type for argument _arg6_lsubc")
      if _arg6_lsubc is None:
        _arg6_lsubc_ = None
      else:
        try:
          _arg6_lsubc_ = memoryview(_arg6_lsubc)
        except TypeError:
          try:
            _tmparr__arg6_lsubc = (ctypes.c_int32*len(_arg6_lsubc))()
            _tmparr__arg6_lsubc[:] = _arg6_lsubc
          except TypeError:
            raise TypeError("Argument _arg6_lsubc has wrong type")
          else:
            _arg6_lsubc_ = memoryview(_tmparr__arg6_lsubc)
      
        else:
          if _arg6_lsubc_.format != "i":
            _tmparr__arg6_lsubc = (ctypes.c_int32*len(_arg6_lsubc))()
            _tmparr__arg6_lsubc[:] = _arg6_lsubc
            _arg6_lsubc_ = memoryview(_tmparr__arg6_lsubc)
      
      if _arg6_lsubc_ is not None and len(_arg6_lsubc_) !=  _arg5_lensubnval :
        raise ValueError("Array argument _arg6_lsubc has wrong length")
      if _arg7_lvalc is None: raise TypeError("Invalid type for argument _arg7_lvalc")
      if _arg7_lvalc is None:
        _arg7_lvalc_ = None
      else:
        try:
          _arg7_lvalc_ = memoryview(_arg7_lvalc)
        except TypeError:
          try:
            _tmparr__arg7_lvalc = (ctypes.c_double*len(_arg7_lvalc))()
            _tmparr__arg7_lvalc[:] = _arg7_lvalc
          except TypeError:
            raise TypeError("Argument _arg7_lvalc has wrong type")
          else:
            _arg7_lvalc_ = memoryview(_tmparr__arg7_lvalc)
      
        else:
          if _arg7_lvalc_.format != "d":
            _tmparr__arg7_lvalc = (ctypes.c_double*len(_arg7_lvalc))()
            _tmparr__arg7_lvalc[:] = _arg7_lvalc
            _arg7_lvalc_ = memoryview(_tmparr__arg7_lvalc)
      
      if _arg7_lvalc_ is not None and len(_arg7_lvalc_) !=  _arg5_lensubnval :
        raise ValueError("Array argument _arg7_lvalc has wrong length")
      if _arg8_b is None: raise TypeError("Invalid type for argument _arg8_b")
      _copyback__arg8_b = False
      if _arg8_b is None:
        _arg8_b_ = None
      else:
        try:
          _arg8_b_ = memoryview(_arg8_b)
        except TypeError:
          try:
            _tmparr__arg8_b = (ctypes.c_double*len(_arg8_b))()
            _tmparr__arg8_b[:] = _arg8_b
          except TypeError:
            raise TypeError("Argument _arg8_b has wrong type")
          else:
            _arg8_b_ = memoryview(_tmparr__arg8_b)
            _copyback__arg8_b = True
        else:
          if _arg8_b_.format != "d":
            _tmparr__arg8_b = (ctypes.c_double*len(_arg8_b))()
            _tmparr__arg8_b[:] = _arg8_b
            _arg8_b_ = memoryview(_tmparr__arg8_b)
            _copyback__arg8_b = True
      if _arg8_b_ is not None and len(_arg8_b_) !=  _arg2_n :
        raise ValueError("Array argument _arg8_b has wrong length")
      res = self.__obj.sparsetriangularsolvedense(_arg1_transposed_,_arg2_n_,_arg3_lnzc_,_arg4_lptrc_,_arg5_lensubnval_,_arg6_lsubc_,_arg7_lvalc_,_arg8_b_)
      if res != 0:
        raise Error(rescode(res),"")
      if _copyback__arg8_b:
        _arg8_b[:] = _tmparr__arg8_b
    
    def potrf(self,_arg1_uplo_,_arg2_n,_arg3_a): # 2
      """
      Computes a Cholesky factorization of a dense matrix.
    
      potrf(self,_arg1_uplo_,_arg2_n,_arg3_a)
        _arg1_uplo: mosek.uplo. Indicates whether the upper or lower triangular part of the matrix is stored.
        _arg2_n: int. Dimension of the symmetric matrix.
        _arg3_a: array of double. A symmetric matrix stored in column-major order.
      """
      if not isinstance(_arg1_uplo_,uplo): raise TypeError("Argument _arg1_uplo has wrong type")
      _arg1_uplo = _arg1_uplo_
      _arg2_n_ = _arg2_n
      if _arg3_a is None: raise TypeError("Invalid type for argument _arg3_a")
      _copyback__arg3_a = False
      if _arg3_a is None:
        _arg3_a_ = None
      else:
        try:
          _arg3_a_ = memoryview(_arg3_a)
        except TypeError:
          try:
            _tmparr__arg3_a = (ctypes.c_double*len(_arg3_a))()
            _tmparr__arg3_a[:] = _arg3_a
          except TypeError:
            raise TypeError("Argument _arg3_a has wrong type")
          else:
            _arg3_a_ = memoryview(_tmparr__arg3_a)
            _copyback__arg3_a = True
        else:
          if _arg3_a_.format != "d":
            _tmparr__arg3_a = (ctypes.c_double*len(_arg3_a))()
            _tmparr__arg3_a[:] = _arg3_a
            _arg3_a_ = memoryview(_tmparr__arg3_a)
            _copyback__arg3_a = True
      if _arg3_a_ is not None and len(_arg3_a_) != ( _arg2_n  *  _arg2_n ):
        raise ValueError("Array argument _arg3_a has wrong length")
      res = self.__obj.potrf(_arg1_uplo_,_arg2_n_,_arg3_a_)
      if res != 0:
        raise Error(rescode(res),"")
      if _copyback__arg3_a:
        _arg3_a[:] = _tmparr__arg3_a
    
    def syeig(self,_arg1_uplo_,_arg2_n,_arg3_a,_arg4_w): # 2
      """
      Computes all eigenvalues of a symmetric dense matrix.
    
      syeig(self,_arg1_uplo_,_arg2_n,_arg3_a,_arg4_w)
        _arg1_uplo: mosek.uplo. Indicates whether the upper or lower triangular part is used.
        _arg2_n: int. Dimension of the symmetric input matrix.
        _arg3_a: array of double. Input matrix A.
        _arg4_w: array of double. Array of length at least n containing the eigenvalues of A.
      """
      if not isinstance(_arg1_uplo_,uplo): raise TypeError("Argument _arg1_uplo has wrong type")
      _arg1_uplo = _arg1_uplo_
      _arg2_n_ = _arg2_n
      if _arg3_a is None: raise TypeError("Invalid type for argument _arg3_a")
      if _arg3_a is None:
        _arg3_a_ = None
      else:
        try:
          _arg3_a_ = memoryview(_arg3_a)
        except TypeError:
          try:
            _tmparr__arg3_a = (ctypes.c_double*len(_arg3_a))()
            _tmparr__arg3_a[:] = _arg3_a
          except TypeError:
            raise TypeError("Argument _arg3_a has wrong type")
          else:
            _arg3_a_ = memoryview(_tmparr__arg3_a)
      
        else:
          if _arg3_a_.format != "d":
            _tmparr__arg3_a = (ctypes.c_double*len(_arg3_a))()
            _tmparr__arg3_a[:] = _arg3_a
            _arg3_a_ = memoryview(_tmparr__arg3_a)
      
      if _arg3_a_ is not None and len(_arg3_a_) != ( _arg2_n  *  _arg2_n ):
        raise ValueError("Array argument _arg3_a has wrong length")
      if _arg4_w is None: raise TypeError("Invalid type for argument _arg4_w")
      _copyback__arg4_w = False
      if _arg4_w is None:
        _arg4_w_ = None
      else:
        try:
          _arg4_w_ = memoryview(_arg4_w)
        except TypeError:
          try:
            _tmparr__arg4_w = (ctypes.c_double*len(_arg4_w))()
            _tmparr__arg4_w[:] = _arg4_w
          except TypeError:
            raise TypeError("Argument _arg4_w has wrong type")
          else:
            _arg4_w_ = memoryview(_tmparr__arg4_w)
            _copyback__arg4_w = True
        else:
          if _arg4_w_.format != "d":
            _tmparr__arg4_w = (ctypes.c_double*len(_arg4_w))()
            _tmparr__arg4_w[:] = _arg4_w
            _arg4_w_ = memoryview(_tmparr__arg4_w)
            _copyback__arg4_w = True
      if _arg4_w_ is not None and len(_arg4_w_) !=  _arg2_n :
        raise ValueError("Array argument _arg4_w has wrong length")
      res = self.__obj.syeig(_arg1_uplo_,_arg2_n_,_arg3_a_,_arg4_w_)
      if res != 0:
        raise Error(rescode(res),"")
      if _copyback__arg4_w:
        _arg4_w[:] = _tmparr__arg4_w
    
    def syevd(self,_arg1_uplo_,_arg2_n,_arg3_a,_arg4_w): # 2
      """
      Computes all the eigenvalues and eigenvectors of a symmetric dense matrix, and thus its eigenvalue decomposition.
    
      syevd(self,_arg1_uplo_,_arg2_n,_arg3_a,_arg4_w)
        _arg1_uplo: mosek.uplo. Indicates whether the upper or lower triangular part is used.
        _arg2_n: int. Dimension of the symmetric input matrix.
        _arg3_a: array of double. Input matrix A.
        _arg4_w: array of double. Array of length at least n containing the eigenvalues of A.
      """
      if not isinstance(_arg1_uplo_,uplo): raise TypeError("Argument _arg1_uplo has wrong type")
      _arg1_uplo = _arg1_uplo_
      _arg2_n_ = _arg2_n
      if _arg3_a is None: raise TypeError("Invalid type for argument _arg3_a")
      _copyback__arg3_a = False
      if _arg3_a is None:
        _arg3_a_ = None
      else:
        try:
          _arg3_a_ = memoryview(_arg3_a)
        except TypeError:
          try:
            _tmparr__arg3_a = (ctypes.c_double*len(_arg3_a))()
            _tmparr__arg3_a[:] = _arg3_a
          except TypeError:
            raise TypeError("Argument _arg3_a has wrong type")
          else:
            _arg3_a_ = memoryview(_tmparr__arg3_a)
            _copyback__arg3_a = True
        else:
          if _arg3_a_.format != "d":
            _tmparr__arg3_a = (ctypes.c_double*len(_arg3_a))()
            _tmparr__arg3_a[:] = _arg3_a
            _arg3_a_ = memoryview(_tmparr__arg3_a)
            _copyback__arg3_a = True
      if _arg3_a_ is not None and len(_arg3_a_) != ( _arg2_n  *  _arg2_n ):
        raise ValueError("Array argument _arg3_a has wrong length")
      if _arg4_w is None: raise TypeError("Invalid type for argument _arg4_w")
      _copyback__arg4_w = False
      if _arg4_w is None:
        _arg4_w_ = None
      else:
        try:
          _arg4_w_ = memoryview(_arg4_w)
        except TypeError:
          try:
            _tmparr__arg4_w = (ctypes.c_double*len(_arg4_w))()
            _tmparr__arg4_w[:] = _arg4_w
          except TypeError:
            raise TypeError("Argument _arg4_w has wrong type")
          else:
            _arg4_w_ = memoryview(_tmparr__arg4_w)
            _copyback__arg4_w = True
        else:
          if _arg4_w_.format != "d":
            _tmparr__arg4_w = (ctypes.c_double*len(_arg4_w))()
            _tmparr__arg4_w[:] = _arg4_w
            _arg4_w_ = memoryview(_tmparr__arg4_w)
            _copyback__arg4_w = True
      if _arg4_w_ is not None and len(_arg4_w_) !=  _arg2_n :
        raise ValueError("Array argument _arg4_w has wrong length")
      res = self.__obj.syevd(_arg1_uplo_,_arg2_n_,_arg3_a_,_arg4_w_)
      if res != 0:
        raise Error(rescode(res),"")
      if _copyback__arg4_w:
        _arg4_w[:] = _tmparr__arg4_w
      if _copyback__arg3_a:
        _arg3_a[:] = _tmparr__arg3_a
    
    @staticmethod
    def licensecleanup(): # 2
      """
      Stops all threads and delete all handles used by the license system.
    
      licensecleanup()
      """
      res = _msk.Env.licensecleanup()
      if res != 0:
        raise Error(rescode(res),"")
    

class Task:
    def __init__(self,env,numcon=0,numvar=0):
        if isinstance(env,Task):
            self.__obj = _msk.Task(None,numcon,numvar,other=env._Task__obj)
        elif isinstance(env, _msk.Task):
            self.__obj = env
        else:
            self.__obj = _msk.Task(env._Env__obj,numcon,numvar)

    def __del__(self):
        try:
            o = self.__obj
        except AttributeError:
            pass
        else:
            o.dispose()
            del self.__obj

    def __enter__(self):
        return self

    def __exit__(self,exc_type,exc_value,traceback):
        self.__del__()


    def __getlasterror(self,res):
        res,msg = self.__obj.getlasterror()
        return rescode(res),msg

    def set_Stream(self,whichstream,func):
        if isinstance(whichstream, streamtype):
            if func is None:
                self.__obj.remove_Stream(whichstream)
            else:
                self.__obj.set_Stream(whichstream,func)
        else:
            raise TypeError("Invalid stream %s" % whichstream)

    def set_Progress(self,func):
        """
        Set the progress callback function. If func is None, progress callbacks are detached and disabled.
        """
        self.__obj.set_Progress(func)

    def set_InfoCallback(self,func):
        """
        Set the progress callback function. If func is None, progress callbacks are detached and disabled.
        """
        self.__obj.set_InfoCallback(func)


    def analyzeproblem(self,_arg1_whichstream_): # 2
      """
      Analyze the data of a task.
    
      analyzeproblem(self,_arg1_whichstream_)
        _arg1_whichstream: mosek.streamtype. <no description>
      """
      if not isinstance(_arg1_whichstream_,streamtype): raise TypeError("Argument _arg1_whichstream has wrong type")
      _arg1_whichstream = _arg1_whichstream_
      res = self.__obj.analyzeproblem(_arg1_whichstream_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def analyzenames(self,_arg1_whichstream_,_arg2_nametype_): # 2
      """
      Analyze the names and issue an error for the first invalid name.
    
      analyzenames(self,_arg1_whichstream_,_arg2_nametype_)
        _arg1_whichstream: mosek.streamtype. <no description>
        _arg2_nametype: mosek.nametype. The type of names e.g. valid in MPS or LP files.
      """
      if not isinstance(_arg1_whichstream_,streamtype): raise TypeError("Argument _arg1_whichstream has wrong type")
      _arg1_whichstream = _arg1_whichstream_
      if not isinstance(_arg2_nametype_,nametype): raise TypeError("Argument _arg2_nametype has wrong type")
      _arg2_nametype = _arg2_nametype_
      res = self.__obj.analyzenames(_arg1_whichstream_,_arg2_nametype_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def analyzesolution(self,_arg1_whichstream_,_arg2_whichsol_): # 2
      """
      Print information related to the quality of the solution.
    
      analyzesolution(self,_arg1_whichstream_,_arg2_whichsol_)
        _arg1_whichstream: mosek.streamtype. <no description>
        _arg2_whichsol: mosek.soltype. <no description>
      """
      if not isinstance(_arg1_whichstream_,streamtype): raise TypeError("Argument _arg1_whichstream has wrong type")
      _arg1_whichstream = _arg1_whichstream_
      if not isinstance(_arg2_whichsol_,soltype): raise TypeError("Argument _arg2_whichsol has wrong type")
      _arg2_whichsol = _arg2_whichsol_
      res = self.__obj.analyzesolution(_arg1_whichstream_,_arg2_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def initbasissolve(self,_arg1_basis): # 2
      """
      Prepare a task for basis solver.
    
      initbasissolve(self,_arg1_basis)
        _arg1_basis: array of int. The array of basis indexes to use.
      """
      _copyback__arg1_basis = False
      if _arg1_basis is None:
        _arg1_basis_ = None
      else:
        try:
          _arg1_basis_ = memoryview(_arg1_basis)
        except TypeError:
          try:
            _tmparr__arg1_basis = (ctypes.c_int32*len(_arg1_basis))()
            _tmparr__arg1_basis[:] = _arg1_basis
          except TypeError:
            raise TypeError("Argument _arg1_basis has wrong type")
          else:
            _arg1_basis_ = memoryview(_tmparr__arg1_basis)
            _copyback__arg1_basis = True
        else:
          if _arg1_basis_.format != "i":
            _tmparr__arg1_basis = (ctypes.c_int32*len(_arg1_basis))()
            _tmparr__arg1_basis[:] = _arg1_basis
            _arg1_basis_ = memoryview(_tmparr__arg1_basis)
            _copyback__arg1_basis = True
      if _arg1_basis_ is not None and len(_arg1_basis_) != self.getnumcon():
        raise ValueError("Array argument _arg1_basis has wrong length")
      res = self.__obj.initbasissolve(_arg1_basis_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg1_basis:
        _arg1_basis[:] = _tmparr__arg1_basis
    
    def solvewithbasis(self,_arg1_transp,_arg2_numnz_,_arg3_sub,_arg4_val): # 2
      """
      Solve a linear equation system involving a basis matrix.
    
      solvewithbasis(self,_arg1_transp,_arg2_numnz_,_arg3_sub,_arg4_val)
        _arg1_transp: int. Controls which problem formulation is solved.
        _arg2_numnz: int. Input (number of non-zeros in right-hand side) and output (number of non-zeros in solution vector).
        _arg3_sub: array of int. Input (indexes of non-zeros in right-hand side) and output (indexes of non-zeros in solution vector).
        _arg4_val: array of double. Input (right-hand side values) and output (solution vector values).
      returns: _arg2_numnz
        _arg2_numnz: int. Input (number of non-zeros in right-hand side) and output (number of non-zeros in solution vector).
      """
      _arg1_transp_ = _arg1_transp
      _arg2_numnz = _arg2_numnz_
      _copyback__arg3_sub = False
      if _arg3_sub is None:
        _arg3_sub_ = None
      else:
        try:
          _arg3_sub_ = memoryview(_arg3_sub)
        except TypeError:
          try:
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
          except TypeError:
            raise TypeError("Argument _arg3_sub has wrong type")
          else:
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
            _copyback__arg3_sub = True
        else:
          if _arg3_sub_.format != "i":
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
            _copyback__arg3_sub = True
      if _arg3_sub_ is not None and len(_arg3_sub_) != self.getnumcon():
        raise ValueError("Array argument _arg3_sub has wrong length")
      _copyback__arg4_val = False
      if _arg4_val is None:
        _arg4_val_ = None
      else:
        try:
          _arg4_val_ = memoryview(_arg4_val)
        except TypeError:
          try:
            _tmparr__arg4_val = (ctypes.c_double*len(_arg4_val))()
            _tmparr__arg4_val[:] = _arg4_val
          except TypeError:
            raise TypeError("Argument _arg4_val has wrong type")
          else:
            _arg4_val_ = memoryview(_tmparr__arg4_val)
            _copyback__arg4_val = True
        else:
          if _arg4_val_.format != "d":
            _tmparr__arg4_val = (ctypes.c_double*len(_arg4_val))()
            _tmparr__arg4_val[:] = _arg4_val
            _arg4_val_ = memoryview(_tmparr__arg4_val)
            _copyback__arg4_val = True
      if _arg4_val_ is not None and len(_arg4_val_) != self.getnumcon():
        raise ValueError("Array argument _arg4_val has wrong length")
      res,resargs = self.__obj.solvewithbasis(_arg1_transp_,_arg2_numnz_,_arg3_sub_,_arg4_val_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_numnz_return_value = resargs
      if _copyback__arg4_val:
        _arg4_val[:] = _tmparr__arg4_val
      if _copyback__arg3_sub:
        _arg3_sub[:] = _tmparr__arg3_sub
      return __arg2_numnz_return_value
    
    def basiscond(self): # 2
      """
      Computes conditioning information for the basis matrix.
    
      basiscond(self)
      returns: _arg1_nrmbasis,_arg2_nrminvbasis
        _arg1_nrmbasis: double. An estimate for the 1-norm of the basis.
        _arg2_nrminvbasis: double. An estimate for the 1-norm of the inverse of the basis.
      """
      res,resargs = self.__obj.basiscond()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_nrmbasis_return_value,__arg2_nrminvbasis_return_value = resargs
      return __arg1_nrmbasis_return_value,__arg2_nrminvbasis_return_value
    
    def appendcons(self,_arg1_num): # 2
      """
      Appends a number of constraints to the optimization task.
    
      appendcons(self,_arg1_num)
        _arg1_num: int. Number of constraints which should be appended.
      """
      _arg1_num_ = _arg1_num
      res = self.__obj.appendcons(_arg1_num_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def appendvars(self,_arg1_num): # 2
      """
      Appends a number of variables to the optimization task.
    
      appendvars(self,_arg1_num)
        _arg1_num: int. Number of variables which should be appended.
      """
      _arg1_num_ = _arg1_num
      res = self.__obj.appendvars(_arg1_num_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def removecons(self,_arg2_subset): # 2
      """
      Removes a number of constraints.
    
      removecons(self,_arg2_subset)
        _arg2_subset: array of int. Indexes of constraints which should be removed.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subset)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subset):
        raise IndexError("Inconsistent length of array _arg2_subset")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subset is None: raise TypeError("Invalid type for argument _arg2_subset")
      if _arg2_subset is None:
        _arg2_subset_ = None
      else:
        try:
          _arg2_subset_ = memoryview(_arg2_subset)
        except TypeError:
          try:
            _tmparr__arg2_subset = (ctypes.c_int32*len(_arg2_subset))()
            _tmparr__arg2_subset[:] = _arg2_subset
          except TypeError:
            raise TypeError("Argument _arg2_subset has wrong type")
          else:
            _arg2_subset_ = memoryview(_tmparr__arg2_subset)
      
        else:
          if _arg2_subset_.format != "i":
            _tmparr__arg2_subset = (ctypes.c_int32*len(_arg2_subset))()
            _tmparr__arg2_subset[:] = _arg2_subset
            _arg2_subset_ = memoryview(_tmparr__arg2_subset)
      
      res = self.__obj.removecons(_arg1_num_,_arg2_subset_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def removevars(self,_arg2_subset): # 2
      """
      Removes a number of variables.
    
      removevars(self,_arg2_subset)
        _arg2_subset: array of int. Indexes of variables which should be removed.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subset)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subset):
        raise IndexError("Inconsistent length of array _arg2_subset")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subset is None: raise TypeError("Invalid type for argument _arg2_subset")
      if _arg2_subset is None:
        _arg2_subset_ = None
      else:
        try:
          _arg2_subset_ = memoryview(_arg2_subset)
        except TypeError:
          try:
            _tmparr__arg2_subset = (ctypes.c_int32*len(_arg2_subset))()
            _tmparr__arg2_subset[:] = _arg2_subset
          except TypeError:
            raise TypeError("Argument _arg2_subset has wrong type")
          else:
            _arg2_subset_ = memoryview(_tmparr__arg2_subset)
      
        else:
          if _arg2_subset_.format != "i":
            _tmparr__arg2_subset = (ctypes.c_int32*len(_arg2_subset))()
            _tmparr__arg2_subset[:] = _arg2_subset
            _arg2_subset_ = memoryview(_tmparr__arg2_subset)
      
      res = self.__obj.removevars(_arg1_num_,_arg2_subset_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def removebarvars(self,_arg2_subset): # 2
      """
      Removes a number of symmetric matrices.
    
      removebarvars(self,_arg2_subset)
        _arg2_subset: array of int. Indexes of symmetric matrices which should be removed.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subset)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subset):
        raise IndexError("Inconsistent length of array _arg2_subset")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subset is None: raise TypeError("Invalid type for argument _arg2_subset")
      if _arg2_subset is None:
        _arg2_subset_ = None
      else:
        try:
          _arg2_subset_ = memoryview(_arg2_subset)
        except TypeError:
          try:
            _tmparr__arg2_subset = (ctypes.c_int32*len(_arg2_subset))()
            _tmparr__arg2_subset[:] = _arg2_subset
          except TypeError:
            raise TypeError("Argument _arg2_subset has wrong type")
          else:
            _arg2_subset_ = memoryview(_tmparr__arg2_subset)
      
        else:
          if _arg2_subset_.format != "i":
            _tmparr__arg2_subset = (ctypes.c_int32*len(_arg2_subset))()
            _tmparr__arg2_subset[:] = _arg2_subset
            _arg2_subset_ = memoryview(_tmparr__arg2_subset)
      
      res = self.__obj.removebarvars(_arg1_num_,_arg2_subset_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def removecones(self,_arg2_subset): # 2
      """
      Removes a number of conic constraints from the problem.
    
      removecones(self,_arg2_subset)
        _arg2_subset: array of int. Indexes of cones which should be removed.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subset)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subset):
        raise IndexError("Inconsistent length of array _arg2_subset")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subset is None: raise TypeError("Invalid type for argument _arg2_subset")
      if _arg2_subset is None:
        _arg2_subset_ = None
      else:
        try:
          _arg2_subset_ = memoryview(_arg2_subset)
        except TypeError:
          try:
            _tmparr__arg2_subset = (ctypes.c_int32*len(_arg2_subset))()
            _tmparr__arg2_subset[:] = _arg2_subset
          except TypeError:
            raise TypeError("Argument _arg2_subset has wrong type")
          else:
            _arg2_subset_ = memoryview(_tmparr__arg2_subset)
      
        else:
          if _arg2_subset_.format != "i":
            _tmparr__arg2_subset = (ctypes.c_int32*len(_arg2_subset))()
            _tmparr__arg2_subset[:] = _arg2_subset
            _arg2_subset_ = memoryview(_tmparr__arg2_subset)
      
      res = self.__obj.removecones(_arg1_num_,_arg2_subset_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def appendbarvars(self,_arg2_dim): # 2
      """
      Appends semidefinite variables to the problem.
    
      appendbarvars(self,_arg2_dim)
        _arg2_dim: array of int. Dimensions of symmetric matrix variables to be added.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_dim)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_dim):
        raise IndexError("Inconsistent length of array _arg2_dim")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_dim is None: raise TypeError("Invalid type for argument _arg2_dim")
      if _arg2_dim is None:
        _arg2_dim_ = None
      else:
        try:
          _arg2_dim_ = memoryview(_arg2_dim)
        except TypeError:
          try:
            _tmparr__arg2_dim = (ctypes.c_int32*len(_arg2_dim))()
            _tmparr__arg2_dim[:] = _arg2_dim
          except TypeError:
            raise TypeError("Argument _arg2_dim has wrong type")
          else:
            _arg2_dim_ = memoryview(_tmparr__arg2_dim)
      
        else:
          if _arg2_dim_.format != "i":
            _tmparr__arg2_dim = (ctypes.c_int32*len(_arg2_dim))()
            _tmparr__arg2_dim[:] = _arg2_dim
            _arg2_dim_ = memoryview(_tmparr__arg2_dim)
      
      res = self.__obj.appendbarvars(_arg1_num_,_arg2_dim_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def appendcone(self,_arg1_ct_,_arg2_conepar,_arg4_submem): # 2
      """
      Appends a new conic constraint to the problem.
    
      appendcone(self,_arg1_ct_,_arg2_conepar,_arg4_submem)
        _arg1_ct: mosek.conetype. <no description>
        _arg2_conepar: double. <no description>
        _arg4_submem: array of int. <no description>
      """
      if not isinstance(_arg1_ct_,conetype): raise TypeError("Argument _arg1_ct has wrong type")
      _arg1_ct = _arg1_ct_
      _arg2_conepar_ = _arg2_conepar
      _arg3_nummem_ = None
      if _arg3_nummem_ is None:
        _arg3_nummem_ = len(_arg4_submem)
        _arg3_nummem = _arg3_nummem_
      elif _arg3_nummem_ != len(_arg4_submem):
        raise IndexError("Inconsistent length of array _arg4_submem")
      if _arg3_nummem_ is None: _arg3_nummem_ = 0
      if _arg4_submem is None: raise TypeError("Invalid type for argument _arg4_submem")
      if _arg4_submem is None:
        _arg4_submem_ = None
      else:
        try:
          _arg4_submem_ = memoryview(_arg4_submem)
        except TypeError:
          try:
            _tmparr__arg4_submem = (ctypes.c_int32*len(_arg4_submem))()
            _tmparr__arg4_submem[:] = _arg4_submem
          except TypeError:
            raise TypeError("Argument _arg4_submem has wrong type")
          else:
            _arg4_submem_ = memoryview(_tmparr__arg4_submem)
      
        else:
          if _arg4_submem_.format != "i":
            _tmparr__arg4_submem = (ctypes.c_int32*len(_arg4_submem))()
            _tmparr__arg4_submem[:] = _arg4_submem
            _arg4_submem_ = memoryview(_tmparr__arg4_submem)
      
      res = self.__obj.appendcone(_arg1_ct_,_arg2_conepar_,_arg3_nummem_,_arg4_submem_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def appendconeseq(self,_arg1_ct_,_arg2_conepar,_arg3_nummem,_arg4_j): # 2
      """
      Appends a new conic constraint to the problem.
    
      appendconeseq(self,_arg1_ct_,_arg2_conepar,_arg3_nummem,_arg4_j)
        _arg1_ct: mosek.conetype. <no description>
        _arg2_conepar: double. <no description>
        _arg3_nummem: int. <no description>
        _arg4_j: int. Index of the first variable in the conic constraint.
      """
      if not isinstance(_arg1_ct_,conetype): raise TypeError("Argument _arg1_ct has wrong type")
      _arg1_ct = _arg1_ct_
      _arg2_conepar_ = _arg2_conepar
      _arg3_nummem_ = _arg3_nummem
      _arg4_j_ = _arg4_j
      res = self.__obj.appendconeseq(_arg1_ct_,_arg2_conepar_,_arg3_nummem_,_arg4_j_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def appendconesseq(self,_arg2_ct,_arg3_conepar,_arg4_nummem,_arg5_j): # 2
      """
      Appends multiple conic constraints to the problem.
    
      appendconesseq(self,_arg2_ct,_arg3_conepar,_arg4_nummem,_arg5_j)
        _arg2_ct: array of mosek.conetype. <no description>
        _arg3_conepar: array of double. <no description>
        _arg4_nummem: array of int. Numbers of member variables in the cones.
        _arg5_j: int. Index of the first variable in the first cone to be appended.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_ct)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_ct):
        raise IndexError("Inconsistent length of array _arg2_ct")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_conepar)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_conepar):
        raise IndexError("Inconsistent length of array _arg3_conepar")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg4_nummem)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg4_nummem):
        raise IndexError("Inconsistent length of array _arg4_nummem")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_ct is None: raise TypeError("Invalid type for argument _arg2_ct")
      if _arg2_ct is None:
        _arg2_ct_ = None
      else:
        try:
          _arg2_ct_ = memoryview(_arg2_ct)
        except TypeError:
          try:
            _tmparr__arg2_ct = (ctypes.c_int*len(_arg2_ct))()
            _tmparr__arg2_ct[:] = _arg2_ct
          except TypeError:
            raise TypeError("Argument _arg2_ct has wrong type")
          else:
            _arg2_ct_ = memoryview(_tmparr__arg2_ct)
      
        else:
          if _arg2_ct_.format != "i":
            _tmparr__arg2_ct = (ctypes.c_int*len(_arg2_ct))()
            _tmparr__arg2_ct[:] = _arg2_ct
            _arg2_ct_ = memoryview(_tmparr__arg2_ct)
      
      if _arg3_conepar is None: raise TypeError("Invalid type for argument _arg3_conepar")
      if _arg3_conepar is None:
        _arg3_conepar_ = None
      else:
        try:
          _arg3_conepar_ = memoryview(_arg3_conepar)
        except TypeError:
          try:
            _tmparr__arg3_conepar = (ctypes.c_double*len(_arg3_conepar))()
            _tmparr__arg3_conepar[:] = _arg3_conepar
          except TypeError:
            raise TypeError("Argument _arg3_conepar has wrong type")
          else:
            _arg3_conepar_ = memoryview(_tmparr__arg3_conepar)
      
        else:
          if _arg3_conepar_.format != "d":
            _tmparr__arg3_conepar = (ctypes.c_double*len(_arg3_conepar))()
            _tmparr__arg3_conepar[:] = _arg3_conepar
            _arg3_conepar_ = memoryview(_tmparr__arg3_conepar)
      
      if _arg4_nummem is None: raise TypeError("Invalid type for argument _arg4_nummem")
      if _arg4_nummem is None:
        _arg4_nummem_ = None
      else:
        try:
          _arg4_nummem_ = memoryview(_arg4_nummem)
        except TypeError:
          try:
            _tmparr__arg4_nummem = (ctypes.c_int32*len(_arg4_nummem))()
            _tmparr__arg4_nummem[:] = _arg4_nummem
          except TypeError:
            raise TypeError("Argument _arg4_nummem has wrong type")
          else:
            _arg4_nummem_ = memoryview(_tmparr__arg4_nummem)
      
        else:
          if _arg4_nummem_.format != "i":
            _tmparr__arg4_nummem = (ctypes.c_int32*len(_arg4_nummem))()
            _tmparr__arg4_nummem[:] = _arg4_nummem
            _arg4_nummem_ = memoryview(_tmparr__arg4_nummem)
      
      _arg5_j_ = _arg5_j
      res = self.__obj.appendconesseq(_arg1_num_,_arg2_ct_,_arg3_conepar_,_arg4_nummem_,_arg5_j_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def chgconbound(self,_arg1_i,_arg2_lower,_arg3_finite,_arg4_value): # 2
      """
      Changes the bounds for one constraint.
    
      chgconbound(self,_arg1_i,_arg2_lower,_arg3_finite,_arg4_value)
        _arg1_i: int. Index of the constraint for which the bounds should be changed.
        _arg2_lower: int. If non-zero, then the lower bound is changed, otherwise the upper bound is changed.
        _arg3_finite: int. If non-zero, then the given value is assumed to be finite.
        _arg4_value: double. New value for the bound.
      """
      _arg1_i_ = _arg1_i
      _arg2_lower_ = _arg2_lower
      _arg3_finite_ = _arg3_finite
      _arg4_value_ = _arg4_value
      res = self.__obj.chgconbound(_arg1_i_,_arg2_lower_,_arg3_finite_,_arg4_value_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def chgvarbound(self,_arg1_j,_arg2_lower,_arg3_finite,_arg4_value): # 2
      """
      Changes the bounds for one variable.
    
      chgvarbound(self,_arg1_j,_arg2_lower,_arg3_finite,_arg4_value)
        _arg1_j: int. Index of the variable for which the bounds should be changed.
        _arg2_lower: int. If non-zero, then the lower bound is changed, otherwise the upper bound is changed.
        _arg3_finite: int. If non-zero, then the given value is assumed to be finite.
        _arg4_value: double. New value for the bound.
      """
      _arg1_j_ = _arg1_j
      _arg2_lower_ = _arg2_lower
      _arg3_finite_ = _arg3_finite
      _arg4_value_ = _arg4_value
      res = self.__obj.chgvarbound(_arg1_j_,_arg2_lower_,_arg3_finite_,_arg4_value_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getaij(self,_arg1_i,_arg2_j): # 2
      """
      Obtains a single coefficient in linear constraint matrix.
    
      getaij(self,_arg1_i,_arg2_j)
        _arg1_i: int. Row index of the coefficient to be returned.
        _arg2_j: int. Column index of the coefficient to be returned.
      returns: _arg3_aij
        _arg3_aij: double. Returns the requested coefficient.
      """
      _arg1_i_ = _arg1_i
      _arg2_j_ = _arg2_j
      res,resargs = self.__obj.getaij(_arg1_i_,_arg2_j_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg3_aij_return_value = resargs
      return __arg3_aij_return_value
    
    def getapiecenumnz(self,_arg1_firsti,_arg2_lasti,_arg3_firstj,_arg4_lastj): # 2
      """
      Obtains the number non-zeros in a rectangular piece of the linear constraint matrix.
    
      getapiecenumnz(self,_arg1_firsti,_arg2_lasti,_arg3_firstj,_arg4_lastj)
        _arg1_firsti: int. Index of the first row in the rectangular piece.
        _arg2_lasti: int. Index of the last row plus one in the rectangular piece.
        _arg3_firstj: int. Index of the first column in the rectangular piece.
        _arg4_lastj: int. Index of the last column plus one in the rectangular piece.
      returns: _arg5_numnz
        _arg5_numnz: int. Number of non-zero elements in the rectangular piece of the linear constraint matrix.
      """
      _arg1_firsti_ = _arg1_firsti
      _arg2_lasti_ = _arg2_lasti
      _arg3_firstj_ = _arg3_firstj
      _arg4_lastj_ = _arg4_lastj
      res,resargs = self.__obj.getapiecenumnz(_arg1_firsti_,_arg2_lasti_,_arg3_firstj_,_arg4_lastj_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg5_numnz_return_value = resargs
      return __arg5_numnz_return_value
    
    def getacolnumnz(self,_arg1_i): # 2
      """
      Obtains the number of non-zero elements in one column of the linear constraint matrix
    
      getacolnumnz(self,_arg1_i)
        _arg1_i: int. Index of the column.
      returns: _arg2_nzj
        _arg2_nzj: int. Number of non-zeros in the j'th column of (A).
      """
      _arg1_i_ = _arg1_i
      res,resargs = self.__obj.getacolnumnz(_arg1_i_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_nzj_return_value = resargs
      return __arg2_nzj_return_value
    
    def getacol(self,_arg1_j,_arg3_subj,_arg4_valj): # 2
      """
      Obtains one column of the linear constraint matrix.
    
      getacol(self,_arg1_j,_arg3_subj,_arg4_valj)
        _arg1_j: int. Index of the column.
        _arg3_subj: array of int. Row indices of the non-zeros in the column obtained.
        _arg4_valj: array of double. Numerical values in the column obtained.
      returns: _arg2_nzj
        _arg2_nzj: int. Number of non-zeros in the column obtained.
      """
      _arg1_j_ = _arg1_j
      if _arg3_subj is None: raise TypeError("Invalid type for argument _arg3_subj")
      _copyback__arg3_subj = False
      if _arg3_subj is None:
        _arg3_subj_ = None
      else:
        try:
          _arg3_subj_ = memoryview(_arg3_subj)
        except TypeError:
          try:
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
          except TypeError:
            raise TypeError("Argument _arg3_subj has wrong type")
          else:
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
            _copyback__arg3_subj = True
        else:
          if _arg3_subj_.format != "i":
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
            _copyback__arg3_subj = True
      if _arg3_subj_ is not None and len(_arg3_subj_) != self.getacolnumnz( _arg1_j ):
        raise ValueError("Array argument _arg3_subj has wrong length")
      if _arg4_valj is None: raise TypeError("Invalid type for argument _arg4_valj")
      _copyback__arg4_valj = False
      if _arg4_valj is None:
        _arg4_valj_ = None
      else:
        try:
          _arg4_valj_ = memoryview(_arg4_valj)
        except TypeError:
          try:
            _tmparr__arg4_valj = (ctypes.c_double*len(_arg4_valj))()
            _tmparr__arg4_valj[:] = _arg4_valj
          except TypeError:
            raise TypeError("Argument _arg4_valj has wrong type")
          else:
            _arg4_valj_ = memoryview(_tmparr__arg4_valj)
            _copyback__arg4_valj = True
        else:
          if _arg4_valj_.format != "d":
            _tmparr__arg4_valj = (ctypes.c_double*len(_arg4_valj))()
            _tmparr__arg4_valj[:] = _arg4_valj
            _arg4_valj_ = memoryview(_tmparr__arg4_valj)
            _copyback__arg4_valj = True
      if _arg4_valj_ is not None and len(_arg4_valj_) != self.getacolnumnz( _arg1_j ):
        raise ValueError("Array argument _arg4_valj has wrong length")
      res,resargs = self.__obj.getacol(_arg1_j_,_arg3_subj_,_arg4_valj_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_nzj_return_value = resargs
      if _copyback__arg4_valj:
        _arg4_valj[:] = _tmparr__arg4_valj
      if _copyback__arg3_subj:
        _arg3_subj[:] = _tmparr__arg3_subj
      return __arg2_nzj_return_value
    
    def getacolslice(self,_arg1_first,_arg2_last,_arg5_ptrb,_arg6_ptre,_arg7_sub,_arg8_val): # 2
      """
      Obtains a sequence of columns from the coefficient matrix.
    
      getacolslice(self,_arg1_first,_arg2_last,_arg5_ptrb,_arg6_ptre,_arg7_sub,_arg8_val)
        _arg1_first: int. Index of the first column in the sequence.
        _arg2_last: int. Index of the last column in the sequence plus one.
        _arg5_ptrb: array of long. Column start pointers.
        _arg6_ptre: array of long. Column end pointers.
        _arg7_sub: array of int. Contains the row subscripts.
        _arg8_val: array of double. Contains the coefficient values.
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      _arg3_maxnumnz_ = self.getacolslicenumnz( _arg1_first , _arg2_last )
      _arg3_maxnumnz = _arg3_maxnumnz_
      if _arg5_ptrb is None: raise TypeError("Invalid type for argument _arg5_ptrb")
      _copyback__arg5_ptrb = False
      if _arg5_ptrb is None:
        _arg5_ptrb_ = None
      else:
        try:
          _arg5_ptrb_ = memoryview(_arg5_ptrb)
        except TypeError:
          try:
            _tmparr__arg5_ptrb = (ctypes.c_int64*len(_arg5_ptrb))()
            _tmparr__arg5_ptrb[:] = _arg5_ptrb
          except TypeError:
            raise TypeError("Argument _arg5_ptrb has wrong type")
          else:
            _arg5_ptrb_ = memoryview(_tmparr__arg5_ptrb)
            _copyback__arg5_ptrb = True
        else:
          if _arg5_ptrb_.format != "q":
            _tmparr__arg5_ptrb = (ctypes.c_int64*len(_arg5_ptrb))()
            _tmparr__arg5_ptrb[:] = _arg5_ptrb
            _arg5_ptrb_ = memoryview(_tmparr__arg5_ptrb)
            _copyback__arg5_ptrb = True
      if _arg5_ptrb_ is not None and len(_arg5_ptrb_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg5_ptrb has wrong length")
      if _arg6_ptre is None: raise TypeError("Invalid type for argument _arg6_ptre")
      _copyback__arg6_ptre = False
      if _arg6_ptre is None:
        _arg6_ptre_ = None
      else:
        try:
          _arg6_ptre_ = memoryview(_arg6_ptre)
        except TypeError:
          try:
            _tmparr__arg6_ptre = (ctypes.c_int64*len(_arg6_ptre))()
            _tmparr__arg6_ptre[:] = _arg6_ptre
          except TypeError:
            raise TypeError("Argument _arg6_ptre has wrong type")
          else:
            _arg6_ptre_ = memoryview(_tmparr__arg6_ptre)
            _copyback__arg6_ptre = True
        else:
          if _arg6_ptre_.format != "q":
            _tmparr__arg6_ptre = (ctypes.c_int64*len(_arg6_ptre))()
            _tmparr__arg6_ptre[:] = _arg6_ptre
            _arg6_ptre_ = memoryview(_tmparr__arg6_ptre)
            _copyback__arg6_ptre = True
      if _arg6_ptre_ is not None and len(_arg6_ptre_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg6_ptre has wrong length")
      if _arg7_sub is None: raise TypeError("Invalid type for argument _arg7_sub")
      _copyback__arg7_sub = False
      if _arg7_sub is None:
        _arg7_sub_ = None
      else:
        try:
          _arg7_sub_ = memoryview(_arg7_sub)
        except TypeError:
          try:
            _tmparr__arg7_sub = (ctypes.c_int32*len(_arg7_sub))()
            _tmparr__arg7_sub[:] = _arg7_sub
          except TypeError:
            raise TypeError("Argument _arg7_sub has wrong type")
          else:
            _arg7_sub_ = memoryview(_tmparr__arg7_sub)
            _copyback__arg7_sub = True
        else:
          if _arg7_sub_.format != "i":
            _tmparr__arg7_sub = (ctypes.c_int32*len(_arg7_sub))()
            _tmparr__arg7_sub[:] = _arg7_sub
            _arg7_sub_ = memoryview(_tmparr__arg7_sub)
            _copyback__arg7_sub = True
      if _arg7_sub_ is not None and len(_arg7_sub_) !=  _arg3_maxnumnz :
        raise ValueError("Array argument _arg7_sub has wrong length")
      if _arg8_val is None: raise TypeError("Invalid type for argument _arg8_val")
      _copyback__arg8_val = False
      if _arg8_val is None:
        _arg8_val_ = None
      else:
        try:
          _arg8_val_ = memoryview(_arg8_val)
        except TypeError:
          try:
            _tmparr__arg8_val = (ctypes.c_double*len(_arg8_val))()
            _tmparr__arg8_val[:] = _arg8_val
          except TypeError:
            raise TypeError("Argument _arg8_val has wrong type")
          else:
            _arg8_val_ = memoryview(_tmparr__arg8_val)
            _copyback__arg8_val = True
        else:
          if _arg8_val_.format != "d":
            _tmparr__arg8_val = (ctypes.c_double*len(_arg8_val))()
            _tmparr__arg8_val[:] = _arg8_val
            _arg8_val_ = memoryview(_tmparr__arg8_val)
            _copyback__arg8_val = True
      if _arg8_val_ is not None and len(_arg8_val_) !=  _arg3_maxnumnz :
        raise ValueError("Array argument _arg8_val has wrong length")
      res = self.__obj.getacolslice64(_arg1_first_,_arg2_last_,_arg3_maxnumnz_,len(_arg7_sub),_arg5_ptrb_,_arg6_ptre_,_arg7_sub_,_arg8_val_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg8_val:
        _arg8_val[:] = _tmparr__arg8_val
      if _copyback__arg7_sub:
        _arg7_sub[:] = _tmparr__arg7_sub
      if _copyback__arg6_ptre:
        _arg6_ptre[:] = _tmparr__arg6_ptre
      if _copyback__arg5_ptrb:
        _arg5_ptrb[:] = _tmparr__arg5_ptrb
    
    def getarownumnz(self,_arg1_i): # 2
      """
      Obtains the number of non-zero elements in one row of the linear constraint matrix
    
      getarownumnz(self,_arg1_i)
        _arg1_i: int. Index of the row.
      returns: _arg2_nzi
        _arg2_nzi: int. Number of non-zeros in the i'th row of `A`.
      """
      _arg1_i_ = _arg1_i
      res,resargs = self.__obj.getarownumnz(_arg1_i_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_nzi_return_value = resargs
      return __arg2_nzi_return_value
    
    def getarow(self,_arg1_i,_arg3_subi,_arg4_vali): # 2
      """
      Obtains one row of the linear constraint matrix.
    
      getarow(self,_arg1_i,_arg3_subi,_arg4_vali)
        _arg1_i: int. Index of the row.
        _arg3_subi: array of int. Column indices of the non-zeros in the row obtained.
        _arg4_vali: array of double. Numerical values of the row obtained.
      returns: _arg2_nzi
        _arg2_nzi: int. Number of non-zeros in the row obtained.
      """
      _arg1_i_ = _arg1_i
      if _arg3_subi is None: raise TypeError("Invalid type for argument _arg3_subi")
      _copyback__arg3_subi = False
      if _arg3_subi is None:
        _arg3_subi_ = None
      else:
        try:
          _arg3_subi_ = memoryview(_arg3_subi)
        except TypeError:
          try:
            _tmparr__arg3_subi = (ctypes.c_int32*len(_arg3_subi))()
            _tmparr__arg3_subi[:] = _arg3_subi
          except TypeError:
            raise TypeError("Argument _arg3_subi has wrong type")
          else:
            _arg3_subi_ = memoryview(_tmparr__arg3_subi)
            _copyback__arg3_subi = True
        else:
          if _arg3_subi_.format != "i":
            _tmparr__arg3_subi = (ctypes.c_int32*len(_arg3_subi))()
            _tmparr__arg3_subi[:] = _arg3_subi
            _arg3_subi_ = memoryview(_tmparr__arg3_subi)
            _copyback__arg3_subi = True
      if _arg3_subi_ is not None and len(_arg3_subi_) != self.getarownumnz( _arg1_i ):
        raise ValueError("Array argument _arg3_subi has wrong length")
      if _arg4_vali is None: raise TypeError("Invalid type for argument _arg4_vali")
      _copyback__arg4_vali = False
      if _arg4_vali is None:
        _arg4_vali_ = None
      else:
        try:
          _arg4_vali_ = memoryview(_arg4_vali)
        except TypeError:
          try:
            _tmparr__arg4_vali = (ctypes.c_double*len(_arg4_vali))()
            _tmparr__arg4_vali[:] = _arg4_vali
          except TypeError:
            raise TypeError("Argument _arg4_vali has wrong type")
          else:
            _arg4_vali_ = memoryview(_tmparr__arg4_vali)
            _copyback__arg4_vali = True
        else:
          if _arg4_vali_.format != "d":
            _tmparr__arg4_vali = (ctypes.c_double*len(_arg4_vali))()
            _tmparr__arg4_vali[:] = _arg4_vali
            _arg4_vali_ = memoryview(_tmparr__arg4_vali)
            _copyback__arg4_vali = True
      if _arg4_vali_ is not None and len(_arg4_vali_) != self.getarownumnz( _arg1_i ):
        raise ValueError("Array argument _arg4_vali has wrong length")
      res,resargs = self.__obj.getarow(_arg1_i_,_arg3_subi_,_arg4_vali_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_nzi_return_value = resargs
      if _copyback__arg4_vali:
        _arg4_vali[:] = _tmparr__arg4_vali
      if _copyback__arg3_subi:
        _arg3_subi[:] = _tmparr__arg3_subi
      return __arg2_nzi_return_value
    
    def getacolslicenumnz(self,_arg1_first,_arg2_last): # 2
      """
      Obtains the number of non-zeros in a slice of columns of the coefficient matrix.
    
      getacolslicenumnz(self,_arg1_first,_arg2_last)
        _arg1_first: int. Index of the first column in the sequence.
        _arg2_last: int. Index of the last column plus one in the sequence.
      returns: _arg3_numnz
        _arg3_numnz: long. Number of non-zeros in the slice.
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      res,resargs = self.__obj.getacolslicenumnz64(_arg1_first_,_arg2_last_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg3_numnz_return_value = resargs
      return __arg3_numnz_return_value
    
    def getarowslicenumnz(self,_arg1_first,_arg2_last): # 2
      """
      Obtains the number of non-zeros in a slice of rows of the coefficient matrix.
    
      getarowslicenumnz(self,_arg1_first,_arg2_last)
        _arg1_first: int. Index of the first row in the sequence.
        _arg2_last: int. Index of the last row plus one in the sequence.
      returns: _arg3_numnz
        _arg3_numnz: long. Number of non-zeros in the slice.
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      res,resargs = self.__obj.getarowslicenumnz64(_arg1_first_,_arg2_last_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg3_numnz_return_value = resargs
      return __arg3_numnz_return_value
    
    def getarowslice(self,_arg1_first,_arg2_last,_arg5_ptrb,_arg6_ptre,_arg7_sub,_arg8_val): # 2
      """
      Obtains a sequence of rows from the coefficient matrix.
    
      getarowslice(self,_arg1_first,_arg2_last,_arg5_ptrb,_arg6_ptre,_arg7_sub,_arg8_val)
        _arg1_first: int. Index of the first row in the sequence.
        _arg2_last: int. Index of the last row in the sequence plus one.
        _arg5_ptrb: array of long. Row start pointers.
        _arg6_ptre: array of long. Row end pointers.
        _arg7_sub: array of int. Contains the column subscripts.
        _arg8_val: array of double. Contains the coefficient values.
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      _arg3_maxnumnz_ = self.getarowslicenumnz( _arg1_first , _arg2_last )
      _arg3_maxnumnz = _arg3_maxnumnz_
      if _arg5_ptrb is None: raise TypeError("Invalid type for argument _arg5_ptrb")
      _copyback__arg5_ptrb = False
      if _arg5_ptrb is None:
        _arg5_ptrb_ = None
      else:
        try:
          _arg5_ptrb_ = memoryview(_arg5_ptrb)
        except TypeError:
          try:
            _tmparr__arg5_ptrb = (ctypes.c_int64*len(_arg5_ptrb))()
            _tmparr__arg5_ptrb[:] = _arg5_ptrb
          except TypeError:
            raise TypeError("Argument _arg5_ptrb has wrong type")
          else:
            _arg5_ptrb_ = memoryview(_tmparr__arg5_ptrb)
            _copyback__arg5_ptrb = True
        else:
          if _arg5_ptrb_.format != "q":
            _tmparr__arg5_ptrb = (ctypes.c_int64*len(_arg5_ptrb))()
            _tmparr__arg5_ptrb[:] = _arg5_ptrb
            _arg5_ptrb_ = memoryview(_tmparr__arg5_ptrb)
            _copyback__arg5_ptrb = True
      if _arg5_ptrb_ is not None and len(_arg5_ptrb_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg5_ptrb has wrong length")
      if _arg6_ptre is None: raise TypeError("Invalid type for argument _arg6_ptre")
      _copyback__arg6_ptre = False
      if _arg6_ptre is None:
        _arg6_ptre_ = None
      else:
        try:
          _arg6_ptre_ = memoryview(_arg6_ptre)
        except TypeError:
          try:
            _tmparr__arg6_ptre = (ctypes.c_int64*len(_arg6_ptre))()
            _tmparr__arg6_ptre[:] = _arg6_ptre
          except TypeError:
            raise TypeError("Argument _arg6_ptre has wrong type")
          else:
            _arg6_ptre_ = memoryview(_tmparr__arg6_ptre)
            _copyback__arg6_ptre = True
        else:
          if _arg6_ptre_.format != "q":
            _tmparr__arg6_ptre = (ctypes.c_int64*len(_arg6_ptre))()
            _tmparr__arg6_ptre[:] = _arg6_ptre
            _arg6_ptre_ = memoryview(_tmparr__arg6_ptre)
            _copyback__arg6_ptre = True
      if _arg6_ptre_ is not None and len(_arg6_ptre_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg6_ptre has wrong length")
      if _arg7_sub is None: raise TypeError("Invalid type for argument _arg7_sub")
      _copyback__arg7_sub = False
      if _arg7_sub is None:
        _arg7_sub_ = None
      else:
        try:
          _arg7_sub_ = memoryview(_arg7_sub)
        except TypeError:
          try:
            _tmparr__arg7_sub = (ctypes.c_int32*len(_arg7_sub))()
            _tmparr__arg7_sub[:] = _arg7_sub
          except TypeError:
            raise TypeError("Argument _arg7_sub has wrong type")
          else:
            _arg7_sub_ = memoryview(_tmparr__arg7_sub)
            _copyback__arg7_sub = True
        else:
          if _arg7_sub_.format != "i":
            _tmparr__arg7_sub = (ctypes.c_int32*len(_arg7_sub))()
            _tmparr__arg7_sub[:] = _arg7_sub
            _arg7_sub_ = memoryview(_tmparr__arg7_sub)
            _copyback__arg7_sub = True
      if _arg7_sub_ is not None and len(_arg7_sub_) !=  _arg3_maxnumnz :
        raise ValueError("Array argument _arg7_sub has wrong length")
      if _arg8_val is None: raise TypeError("Invalid type for argument _arg8_val")
      _copyback__arg8_val = False
      if _arg8_val is None:
        _arg8_val_ = None
      else:
        try:
          _arg8_val_ = memoryview(_arg8_val)
        except TypeError:
          try:
            _tmparr__arg8_val = (ctypes.c_double*len(_arg8_val))()
            _tmparr__arg8_val[:] = _arg8_val
          except TypeError:
            raise TypeError("Argument _arg8_val has wrong type")
          else:
            _arg8_val_ = memoryview(_tmparr__arg8_val)
            _copyback__arg8_val = True
        else:
          if _arg8_val_.format != "d":
            _tmparr__arg8_val = (ctypes.c_double*len(_arg8_val))()
            _tmparr__arg8_val[:] = _arg8_val
            _arg8_val_ = memoryview(_tmparr__arg8_val)
            _copyback__arg8_val = True
      if _arg8_val_ is not None and len(_arg8_val_) !=  _arg3_maxnumnz :
        raise ValueError("Array argument _arg8_val has wrong length")
      res = self.__obj.getarowslice64(_arg1_first_,_arg2_last_,_arg3_maxnumnz_,len(_arg7_sub),_arg5_ptrb_,_arg6_ptre_,_arg7_sub_,_arg8_val_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg8_val:
        _arg8_val[:] = _tmparr__arg8_val
      if _copyback__arg7_sub:
        _arg7_sub[:] = _tmparr__arg7_sub
      if _copyback__arg6_ptre:
        _arg6_ptre[:] = _tmparr__arg6_ptre
      if _copyback__arg5_ptrb:
        _arg5_ptrb[:] = _tmparr__arg5_ptrb
    
    def getarowslicetrip(self,_arg1_first,_arg2_last,_arg5_subi,_arg6_subj,_arg7_val): # 2
      """
      Obtains a sequence of rows from the coefficient matrix in sparse triplet format.
    
      getarowslicetrip(self,_arg1_first,_arg2_last,_arg5_subi,_arg6_subj,_arg7_val)
        _arg1_first: int. Index of the first row in the sequence.
        _arg2_last: int. Index of the last row in the sequence plus one.
        _arg5_subi: array of int. Constraint subscripts.
        _arg6_subj: array of int. Column subscripts.
        _arg7_val: array of double. Values.
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      _arg3_maxnumnz_ = self.getarowslicenumnz( _arg1_first , _arg2_last )
      _arg3_maxnumnz = _arg3_maxnumnz_
      _copyback__arg5_subi = False
      if _arg5_subi is None:
        _arg5_subi_ = None
      else:
        try:
          _arg5_subi_ = memoryview(_arg5_subi)
        except TypeError:
          try:
            _tmparr__arg5_subi = (ctypes.c_int32*len(_arg5_subi))()
            _tmparr__arg5_subi[:] = _arg5_subi
          except TypeError:
            raise TypeError("Argument _arg5_subi has wrong type")
          else:
            _arg5_subi_ = memoryview(_tmparr__arg5_subi)
            _copyback__arg5_subi = True
        else:
          if _arg5_subi_.format != "i":
            _tmparr__arg5_subi = (ctypes.c_int32*len(_arg5_subi))()
            _tmparr__arg5_subi[:] = _arg5_subi
            _arg5_subi_ = memoryview(_tmparr__arg5_subi)
            _copyback__arg5_subi = True
      if _arg5_subi_ is not None and len(_arg5_subi_) !=  _arg3_maxnumnz :
        raise ValueError("Array argument _arg5_subi has wrong length")
      _copyback__arg6_subj = False
      if _arg6_subj is None:
        _arg6_subj_ = None
      else:
        try:
          _arg6_subj_ = memoryview(_arg6_subj)
        except TypeError:
          try:
            _tmparr__arg6_subj = (ctypes.c_int32*len(_arg6_subj))()
            _tmparr__arg6_subj[:] = _arg6_subj
          except TypeError:
            raise TypeError("Argument _arg6_subj has wrong type")
          else:
            _arg6_subj_ = memoryview(_tmparr__arg6_subj)
            _copyback__arg6_subj = True
        else:
          if _arg6_subj_.format != "i":
            _tmparr__arg6_subj = (ctypes.c_int32*len(_arg6_subj))()
            _tmparr__arg6_subj[:] = _arg6_subj
            _arg6_subj_ = memoryview(_tmparr__arg6_subj)
            _copyback__arg6_subj = True
      if _arg6_subj_ is not None and len(_arg6_subj_) !=  _arg3_maxnumnz :
        raise ValueError("Array argument _arg6_subj has wrong length")
      _copyback__arg7_val = False
      if _arg7_val is None:
        _arg7_val_ = None
      else:
        try:
          _arg7_val_ = memoryview(_arg7_val)
        except TypeError:
          try:
            _tmparr__arg7_val = (ctypes.c_double*len(_arg7_val))()
            _tmparr__arg7_val[:] = _arg7_val
          except TypeError:
            raise TypeError("Argument _arg7_val has wrong type")
          else:
            _arg7_val_ = memoryview(_tmparr__arg7_val)
            _copyback__arg7_val = True
        else:
          if _arg7_val_.format != "d":
            _tmparr__arg7_val = (ctypes.c_double*len(_arg7_val))()
            _tmparr__arg7_val[:] = _arg7_val
            _arg7_val_ = memoryview(_tmparr__arg7_val)
            _copyback__arg7_val = True
      if _arg7_val_ is not None and len(_arg7_val_) !=  _arg3_maxnumnz :
        raise ValueError("Array argument _arg7_val has wrong length")
      res = self.__obj.getarowslicetrip(_arg1_first_,_arg2_last_,_arg3_maxnumnz_,len(_arg5_subi),_arg5_subi_,_arg6_subj_,_arg7_val_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg7_val:
        _arg7_val[:] = _tmparr__arg7_val
      if _copyback__arg6_subj:
        _arg6_subj[:] = _tmparr__arg6_subj
      if _copyback__arg5_subi:
        _arg5_subi[:] = _tmparr__arg5_subi
    
    def getacolslicetrip(self,_arg1_first,_arg2_last,_arg5_subi,_arg6_subj,_arg7_val): # 2
      """
      Obtains a sequence of columns from the coefficient matrix in triplet format.
    
      getacolslicetrip(self,_arg1_first,_arg2_last,_arg5_subi,_arg6_subj,_arg7_val)
        _arg1_first: int. Index of the first column in the sequence.
        _arg2_last: int. Index of the last column in the sequence plus one.
        _arg5_subi: array of int. Constraint subscripts.
        _arg6_subj: array of int. Column subscripts.
        _arg7_val: array of double. Values.
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      _arg3_maxnumnz_ = self.getacolslicenumnz( _arg1_first , _arg2_last )
      _arg3_maxnumnz = _arg3_maxnumnz_
      _copyback__arg5_subi = False
      if _arg5_subi is None:
        _arg5_subi_ = None
      else:
        try:
          _arg5_subi_ = memoryview(_arg5_subi)
        except TypeError:
          try:
            _tmparr__arg5_subi = (ctypes.c_int32*len(_arg5_subi))()
            _tmparr__arg5_subi[:] = _arg5_subi
          except TypeError:
            raise TypeError("Argument _arg5_subi has wrong type")
          else:
            _arg5_subi_ = memoryview(_tmparr__arg5_subi)
            _copyback__arg5_subi = True
        else:
          if _arg5_subi_.format != "i":
            _tmparr__arg5_subi = (ctypes.c_int32*len(_arg5_subi))()
            _tmparr__arg5_subi[:] = _arg5_subi
            _arg5_subi_ = memoryview(_tmparr__arg5_subi)
            _copyback__arg5_subi = True
      if _arg5_subi_ is not None and len(_arg5_subi_) !=  _arg3_maxnumnz :
        raise ValueError("Array argument _arg5_subi has wrong length")
      _copyback__arg6_subj = False
      if _arg6_subj is None:
        _arg6_subj_ = None
      else:
        try:
          _arg6_subj_ = memoryview(_arg6_subj)
        except TypeError:
          try:
            _tmparr__arg6_subj = (ctypes.c_int32*len(_arg6_subj))()
            _tmparr__arg6_subj[:] = _arg6_subj
          except TypeError:
            raise TypeError("Argument _arg6_subj has wrong type")
          else:
            _arg6_subj_ = memoryview(_tmparr__arg6_subj)
            _copyback__arg6_subj = True
        else:
          if _arg6_subj_.format != "i":
            _tmparr__arg6_subj = (ctypes.c_int32*len(_arg6_subj))()
            _tmparr__arg6_subj[:] = _arg6_subj
            _arg6_subj_ = memoryview(_tmparr__arg6_subj)
            _copyback__arg6_subj = True
      if _arg6_subj_ is not None and len(_arg6_subj_) !=  _arg3_maxnumnz :
        raise ValueError("Array argument _arg6_subj has wrong length")
      _copyback__arg7_val = False
      if _arg7_val is None:
        _arg7_val_ = None
      else:
        try:
          _arg7_val_ = memoryview(_arg7_val)
        except TypeError:
          try:
            _tmparr__arg7_val = (ctypes.c_double*len(_arg7_val))()
            _tmparr__arg7_val[:] = _arg7_val
          except TypeError:
            raise TypeError("Argument _arg7_val has wrong type")
          else:
            _arg7_val_ = memoryview(_tmparr__arg7_val)
            _copyback__arg7_val = True
        else:
          if _arg7_val_.format != "d":
            _tmparr__arg7_val = (ctypes.c_double*len(_arg7_val))()
            _tmparr__arg7_val[:] = _arg7_val
            _arg7_val_ = memoryview(_tmparr__arg7_val)
            _copyback__arg7_val = True
      if _arg7_val_ is not None and len(_arg7_val_) !=  _arg3_maxnumnz :
        raise ValueError("Array argument _arg7_val has wrong length")
      res = self.__obj.getacolslicetrip(_arg1_first_,_arg2_last_,_arg3_maxnumnz_,len(_arg5_subi),_arg5_subi_,_arg6_subj_,_arg7_val_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg7_val:
        _arg7_val[:] = _tmparr__arg7_val
      if _copyback__arg6_subj:
        _arg6_subj[:] = _tmparr__arg6_subj
      if _copyback__arg5_subi:
        _arg5_subi[:] = _tmparr__arg5_subi
    
    def getconbound(self,_arg1_i): # 2
      """
      Obtains bound information for one constraint.
    
      getconbound(self,_arg1_i)
        _arg1_i: int. Index of the constraint for which the bound information should be obtained.
      returns: _arg2_bk,_arg3_bl,_arg4_bu
        _arg2_bk: mosek.boundkey. <no description>
        _arg3_bl: double. <no description>
        _arg4_bu: double. <no description>
      """
      _arg1_i_ = _arg1_i
      res,resargs = self.__obj.getconbound(_arg1_i_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_bk_return_value,__arg3_bl_return_value,__arg4_bu_return_value = resargs
      __arg2_bk_return_value = boundkey(__arg2_bk_return_value)
      return __arg2_bk_return_value,__arg3_bl_return_value,__arg4_bu_return_value
    
    def getvarbound(self,_arg1_i): # 2
      """
      Obtains bound information for one variable.
    
      getvarbound(self,_arg1_i)
        _arg1_i: int. Index of the variable for which the bound information should be obtained.
      returns: _arg2_bk,_arg3_bl,_arg4_bu
        _arg2_bk: mosek.boundkey. <no description>
        _arg3_bl: double. <no description>
        _arg4_bu: double. <no description>
      """
      _arg1_i_ = _arg1_i
      res,resargs = self.__obj.getvarbound(_arg1_i_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_bk_return_value,__arg3_bl_return_value,__arg4_bu_return_value = resargs
      __arg2_bk_return_value = boundkey(__arg2_bk_return_value)
      return __arg2_bk_return_value,__arg3_bl_return_value,__arg4_bu_return_value
    
    def getconboundslice(self,_arg1_first,_arg2_last,_arg3_bk,_arg4_bl,_arg5_bu): # 2
      """
      Obtains bounds information for a slice of the constraints.
    
      getconboundslice(self,_arg1_first,_arg2_last,_arg3_bk,_arg4_bl,_arg5_bu)
        _arg1_first: int. <no description>
        _arg2_last: int. <no description>
        _arg3_bk: array of mosek.boundkey. <no description>
        _arg4_bl: array of double. <no description>
        _arg5_bu: array of double. <no description>
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      _copyback__arg3_bk = False
      if _arg3_bk is None:
        _arg3_bk_ = None
      else:
        try:
          _arg3_bk_ = memoryview(_arg3_bk)
        except TypeError:
          try:
            _tmparr__arg3_bk = (ctypes.c_int*len(_arg3_bk))()
            _tmparr__arg3_bk[:] = _arg3_bk
          except TypeError:
            raise TypeError("Argument _arg3_bk has wrong type")
          else:
            _arg3_bk_ = memoryview(_tmparr__arg3_bk)
            _copyback__arg3_bk = True
        else:
          if _arg3_bk_.format != "i":
            _tmparr__arg3_bk = (ctypes.c_int*len(_arg3_bk))()
            _tmparr__arg3_bk[:] = _arg3_bk
            _arg3_bk_ = memoryview(_tmparr__arg3_bk)
            _copyback__arg3_bk = True
      if _arg3_bk_ is not None and len(_arg3_bk_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg3_bk has wrong length")
      _copyback__arg4_bl = False
      if _arg4_bl is None:
        _arg4_bl_ = None
      else:
        try:
          _arg4_bl_ = memoryview(_arg4_bl)
        except TypeError:
          try:
            _tmparr__arg4_bl = (ctypes.c_double*len(_arg4_bl))()
            _tmparr__arg4_bl[:] = _arg4_bl
          except TypeError:
            raise TypeError("Argument _arg4_bl has wrong type")
          else:
            _arg4_bl_ = memoryview(_tmparr__arg4_bl)
            _copyback__arg4_bl = True
        else:
          if _arg4_bl_.format != "d":
            _tmparr__arg4_bl = (ctypes.c_double*len(_arg4_bl))()
            _tmparr__arg4_bl[:] = _arg4_bl
            _arg4_bl_ = memoryview(_tmparr__arg4_bl)
            _copyback__arg4_bl = True
      if _arg4_bl_ is not None and len(_arg4_bl_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg4_bl has wrong length")
      _copyback__arg5_bu = False
      if _arg5_bu is None:
        _arg5_bu_ = None
      else:
        try:
          _arg5_bu_ = memoryview(_arg5_bu)
        except TypeError:
          try:
            _tmparr__arg5_bu = (ctypes.c_double*len(_arg5_bu))()
            _tmparr__arg5_bu[:] = _arg5_bu
          except TypeError:
            raise TypeError("Argument _arg5_bu has wrong type")
          else:
            _arg5_bu_ = memoryview(_tmparr__arg5_bu)
            _copyback__arg5_bu = True
        else:
          if _arg5_bu_.format != "d":
            _tmparr__arg5_bu = (ctypes.c_double*len(_arg5_bu))()
            _tmparr__arg5_bu[:] = _arg5_bu
            _arg5_bu_ = memoryview(_tmparr__arg5_bu)
            _copyback__arg5_bu = True
      if _arg5_bu_ is not None and len(_arg5_bu_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg5_bu has wrong length")
      res = self.__obj.getconboundslice(_arg1_first_,_arg2_last_,_arg3_bk_,_arg4_bl_,_arg5_bu_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg5_bu:
        _arg5_bu[:] = _tmparr__arg5_bu
      if _copyback__arg4_bl:
        _arg4_bl[:] = _tmparr__arg4_bl
      if _copyback__arg3_bk:
        for __tmp_var_0 in range(len(_arg3_bk_)): _arg3_bk[__tmp_var_0] = boundkey(_tmparr__arg3_bk[__tmp_var_0])
    
    def getvarboundslice(self,_arg1_first,_arg2_last,_arg3_bk,_arg4_bl,_arg5_bu): # 2
      """
      Obtains bounds information for a slice of the variables.
    
      getvarboundslice(self,_arg1_first,_arg2_last,_arg3_bk,_arg4_bl,_arg5_bu)
        _arg1_first: int. <no description>
        _arg2_last: int. <no description>
        _arg3_bk: array of mosek.boundkey. <no description>
        _arg4_bl: array of double. <no description>
        _arg5_bu: array of double. <no description>
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      _copyback__arg3_bk = False
      if _arg3_bk is None:
        _arg3_bk_ = None
      else:
        try:
          _arg3_bk_ = memoryview(_arg3_bk)
        except TypeError:
          try:
            _tmparr__arg3_bk = (ctypes.c_int*len(_arg3_bk))()
            _tmparr__arg3_bk[:] = _arg3_bk
          except TypeError:
            raise TypeError("Argument _arg3_bk has wrong type")
          else:
            _arg3_bk_ = memoryview(_tmparr__arg3_bk)
            _copyback__arg3_bk = True
        else:
          if _arg3_bk_.format != "i":
            _tmparr__arg3_bk = (ctypes.c_int*len(_arg3_bk))()
            _tmparr__arg3_bk[:] = _arg3_bk
            _arg3_bk_ = memoryview(_tmparr__arg3_bk)
            _copyback__arg3_bk = True
      if _arg3_bk_ is not None and len(_arg3_bk_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg3_bk has wrong length")
      _copyback__arg4_bl = False
      if _arg4_bl is None:
        _arg4_bl_ = None
      else:
        try:
          _arg4_bl_ = memoryview(_arg4_bl)
        except TypeError:
          try:
            _tmparr__arg4_bl = (ctypes.c_double*len(_arg4_bl))()
            _tmparr__arg4_bl[:] = _arg4_bl
          except TypeError:
            raise TypeError("Argument _arg4_bl has wrong type")
          else:
            _arg4_bl_ = memoryview(_tmparr__arg4_bl)
            _copyback__arg4_bl = True
        else:
          if _arg4_bl_.format != "d":
            _tmparr__arg4_bl = (ctypes.c_double*len(_arg4_bl))()
            _tmparr__arg4_bl[:] = _arg4_bl
            _arg4_bl_ = memoryview(_tmparr__arg4_bl)
            _copyback__arg4_bl = True
      if _arg4_bl_ is not None and len(_arg4_bl_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg4_bl has wrong length")
      _copyback__arg5_bu = False
      if _arg5_bu is None:
        _arg5_bu_ = None
      else:
        try:
          _arg5_bu_ = memoryview(_arg5_bu)
        except TypeError:
          try:
            _tmparr__arg5_bu = (ctypes.c_double*len(_arg5_bu))()
            _tmparr__arg5_bu[:] = _arg5_bu
          except TypeError:
            raise TypeError("Argument _arg5_bu has wrong type")
          else:
            _arg5_bu_ = memoryview(_tmparr__arg5_bu)
            _copyback__arg5_bu = True
        else:
          if _arg5_bu_.format != "d":
            _tmparr__arg5_bu = (ctypes.c_double*len(_arg5_bu))()
            _tmparr__arg5_bu[:] = _arg5_bu
            _arg5_bu_ = memoryview(_tmparr__arg5_bu)
            _copyback__arg5_bu = True
      if _arg5_bu_ is not None and len(_arg5_bu_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg5_bu has wrong length")
      res = self.__obj.getvarboundslice(_arg1_first_,_arg2_last_,_arg3_bk_,_arg4_bl_,_arg5_bu_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg5_bu:
        _arg5_bu[:] = _tmparr__arg5_bu
      if _copyback__arg4_bl:
        _arg4_bl[:] = _tmparr__arg4_bl
      if _copyback__arg3_bk:
        for __tmp_var_0 in range(len(_arg3_bk_)): _arg3_bk[__tmp_var_0] = boundkey(_tmparr__arg3_bk[__tmp_var_0])
    
    def getcj(self,_arg1_j): # 2
      """
      Obtains one objective coefficient.
    
      getcj(self,_arg1_j)
        _arg1_j: int. Index of the variable for which the c coefficient should be obtained.
      returns: _arg2_cj
        _arg2_cj: double. The c coefficient value.
      """
      _arg1_j_ = _arg1_j
      res,resargs = self.__obj.getcj(_arg1_j_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_cj_return_value = resargs
      return __arg2_cj_return_value
    
    def getc(self,_arg1_c): # 2
      """
      Obtains all objective coefficients.
    
      getc(self,_arg1_c)
        _arg1_c: array of double. <no description>
      """
      _copyback__arg1_c = False
      if _arg1_c is None:
        _arg1_c_ = None
      else:
        try:
          _arg1_c_ = memoryview(_arg1_c)
        except TypeError:
          try:
            _tmparr__arg1_c = (ctypes.c_double*len(_arg1_c))()
            _tmparr__arg1_c[:] = _arg1_c
          except TypeError:
            raise TypeError("Argument _arg1_c has wrong type")
          else:
            _arg1_c_ = memoryview(_tmparr__arg1_c)
            _copyback__arg1_c = True
        else:
          if _arg1_c_.format != "d":
            _tmparr__arg1_c = (ctypes.c_double*len(_arg1_c))()
            _tmparr__arg1_c[:] = _arg1_c
            _arg1_c_ = memoryview(_tmparr__arg1_c)
            _copyback__arg1_c = True
      if _arg1_c_ is not None and len(_arg1_c_) != self.getnumvar():
        raise ValueError("Array argument _arg1_c has wrong length")
      res = self.__obj.getc(_arg1_c_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg1_c:
        _arg1_c[:] = _tmparr__arg1_c
    
    def getcfix(self): # 2
      """
      Obtains the fixed term in the objective.
    
      getcfix(self)
      returns: _arg1_cfix
        _arg1_cfix: double. <no description>
      """
      res,resargs = self.__obj.getcfix()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_cfix_return_value = resargs
      return __arg1_cfix_return_value
    
    def getcone(self,_arg1_k,_arg5_submem): # 2
      """
      Obtains a cone.
    
      getcone(self,_arg1_k,_arg5_submem)
        _arg1_k: int. Index of the cone.
        _arg5_submem: array of int. <no description>
      returns: _arg2_ct,_arg3_conepar,_arg4_nummem
        _arg2_ct: mosek.conetype. <no description>
        _arg3_conepar: double. <no description>
        _arg4_nummem: int. <no description>
      """
      _arg1_k_ = _arg1_k
      _copyback__arg5_submem = False
      if _arg5_submem is None:
        _arg5_submem_ = None
      else:
        try:
          _arg5_submem_ = memoryview(_arg5_submem)
        except TypeError:
          try:
            _tmparr__arg5_submem = (ctypes.c_int32*len(_arg5_submem))()
            _tmparr__arg5_submem[:] = _arg5_submem
          except TypeError:
            raise TypeError("Argument _arg5_submem has wrong type")
          else:
            _arg5_submem_ = memoryview(_tmparr__arg5_submem)
            _copyback__arg5_submem = True
        else:
          if _arg5_submem_.format != "i":
            _tmparr__arg5_submem = (ctypes.c_int32*len(_arg5_submem))()
            _tmparr__arg5_submem[:] = _arg5_submem
            _arg5_submem_ = memoryview(_tmparr__arg5_submem)
            _copyback__arg5_submem = True
      if _arg5_submem_ is not None and len(_arg5_submem_) != self.getconeinfo( _arg1_k )[2]:
        raise ValueError("Array argument _arg5_submem has wrong length")
      res,resargs = self.__obj.getcone(_arg1_k_,_arg5_submem_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_ct_return_value,__arg3_conepar_return_value,__arg4_nummem_return_value = resargs
      if _copyback__arg5_submem:
        _arg5_submem[:] = _tmparr__arg5_submem
      __arg2_ct_return_value = conetype(__arg2_ct_return_value)
      return __arg2_ct_return_value,__arg3_conepar_return_value,__arg4_nummem_return_value
    
    def getconeinfo(self,_arg1_k): # 2
      """
      Obtains information about a cone.
    
      getconeinfo(self,_arg1_k)
        _arg1_k: int. Index of the cone.
      returns: _arg2_ct,_arg3_conepar,_arg4_nummem
        _arg2_ct: mosek.conetype. <no description>
        _arg3_conepar: double. <no description>
        _arg4_nummem: int. <no description>
      """
      _arg1_k_ = _arg1_k
      res,resargs = self.__obj.getconeinfo(_arg1_k_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_ct_return_value,__arg3_conepar_return_value,__arg4_nummem_return_value = resargs
      __arg2_ct_return_value = conetype(__arg2_ct_return_value)
      return __arg2_ct_return_value,__arg3_conepar_return_value,__arg4_nummem_return_value
    
    def getclist(self,_arg2_subj,_arg3_c): # 2
      """
      Obtains a sequence of coefficients from the objective.
    
      getclist(self,_arg2_subj,_arg3_c)
        _arg2_subj: array of int. A list of variable indexes.
        _arg3_c: array of double. Linear terms of the requested list of the objective as a dense vector.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subj)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subj):
        raise IndexError("Inconsistent length of array _arg2_subj")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subj is None: raise TypeError("Invalid type for argument _arg2_subj")
      if _arg2_subj is None:
        _arg2_subj_ = None
      else:
        try:
          _arg2_subj_ = memoryview(_arg2_subj)
        except TypeError:
          try:
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
          except TypeError:
            raise TypeError("Argument _arg2_subj has wrong type")
          else:
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
        else:
          if _arg2_subj_.format != "i":
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
      if _arg3_c is None: raise TypeError("Invalid type for argument _arg3_c")
      _copyback__arg3_c = False
      if _arg3_c is None:
        _arg3_c_ = None
      else:
        try:
          _arg3_c_ = memoryview(_arg3_c)
        except TypeError:
          try:
            _tmparr__arg3_c = (ctypes.c_double*len(_arg3_c))()
            _tmparr__arg3_c[:] = _arg3_c
          except TypeError:
            raise TypeError("Argument _arg3_c has wrong type")
          else:
            _arg3_c_ = memoryview(_tmparr__arg3_c)
            _copyback__arg3_c = True
        else:
          if _arg3_c_.format != "d":
            _tmparr__arg3_c = (ctypes.c_double*len(_arg3_c))()
            _tmparr__arg3_c[:] = _arg3_c
            _arg3_c_ = memoryview(_tmparr__arg3_c)
            _copyback__arg3_c = True
      if _arg3_c_ is not None and len(_arg3_c_) !=  _arg1_num :
        raise ValueError("Array argument _arg3_c has wrong length")
      res = self.__obj.getclist(_arg1_num_,_arg2_subj_,_arg3_c_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg3_c:
        _arg3_c[:] = _tmparr__arg3_c
    
    def getcslice(self,_arg1_first,_arg2_last,_arg3_c): # 2
      """
      Obtains a sequence of coefficients from the objective.
    
      getcslice(self,_arg1_first,_arg2_last,_arg3_c)
        _arg1_first: int. <no description>
        _arg2_last: int. <no description>
        _arg3_c: array of double. Linear terms of the requested slice of the objective as a dense vector.
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      _copyback__arg3_c = False
      if _arg3_c is None:
        _arg3_c_ = None
      else:
        try:
          _arg3_c_ = memoryview(_arg3_c)
        except TypeError:
          try:
            _tmparr__arg3_c = (ctypes.c_double*len(_arg3_c))()
            _tmparr__arg3_c[:] = _arg3_c
          except TypeError:
            raise TypeError("Argument _arg3_c has wrong type")
          else:
            _arg3_c_ = memoryview(_tmparr__arg3_c)
            _copyback__arg3_c = True
        else:
          if _arg3_c_.format != "d":
            _tmparr__arg3_c = (ctypes.c_double*len(_arg3_c))()
            _tmparr__arg3_c[:] = _arg3_c
            _arg3_c_ = memoryview(_tmparr__arg3_c)
            _copyback__arg3_c = True
      if _arg3_c_ is not None and len(_arg3_c_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg3_c has wrong length")
      res = self.__obj.getcslice(_arg1_first_,_arg2_last_,_arg3_c_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg3_c:
        _arg3_c[:] = _tmparr__arg3_c
    
    def getdouinf(self,_arg1_whichdinf_): # 2
      """
      Obtains a double information item.
    
      getdouinf(self,_arg1_whichdinf_)
        _arg1_whichdinf: mosek.dinfitem. <no description>
      returns: _arg2_dvalue
        _arg2_dvalue: double. The value of the required double information item.
      """
      if not isinstance(_arg1_whichdinf_,dinfitem): raise TypeError("Argument _arg1_whichdinf has wrong type")
      _arg1_whichdinf = _arg1_whichdinf_
      res,resargs = self.__obj.getdouinf(_arg1_whichdinf_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_dvalue_return_value = resargs
      return __arg2_dvalue_return_value
    
    def getdouparam(self,_arg1_param_): # 2
      """
      Obtains a double parameter.
    
      getdouparam(self,_arg1_param_)
        _arg1_param: mosek.dparam. <no description>
      returns: _arg2_parvalue
        _arg2_parvalue: double. <no description>
      """
      if not isinstance(_arg1_param_,dparam): raise TypeError("Argument _arg1_param has wrong type")
      _arg1_param = _arg1_param_
      res,resargs = self.__obj.getdouparam(_arg1_param_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_parvalue_return_value = resargs
      return __arg2_parvalue_return_value
    
    def getdualobj(self,_arg1_whichsol_): # 2
      """
      Computes the dual objective value associated with the solution.
    
      getdualobj(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. <no description>
      returns: _arg2_dualobj
        _arg2_dualobj: double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res,resargs = self.__obj.getdualobj(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_dualobj_return_value = resargs
      return __arg2_dualobj_return_value
    
    def getintinf(self,_arg1_whichiinf_): # 2
      """
      Obtains an integer information item.
    
      getintinf(self,_arg1_whichiinf_)
        _arg1_whichiinf: mosek.iinfitem. <no description>
      returns: _arg2_ivalue
        _arg2_ivalue: int. The value of the required integer information item.
      """
      if not isinstance(_arg1_whichiinf_,iinfitem): raise TypeError("Argument _arg1_whichiinf has wrong type")
      _arg1_whichiinf = _arg1_whichiinf_
      res,resargs = self.__obj.getintinf(_arg1_whichiinf_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_ivalue_return_value = resargs
      return __arg2_ivalue_return_value
    
    def getlintinf(self,_arg1_whichliinf_): # 2
      """
      Obtains a long integer information item.
    
      getlintinf(self,_arg1_whichliinf_)
        _arg1_whichliinf: mosek.liinfitem. <no description>
      returns: _arg2_ivalue
        _arg2_ivalue: long. The value of the required long integer information item.
      """
      if not isinstance(_arg1_whichliinf_,liinfitem): raise TypeError("Argument _arg1_whichliinf has wrong type")
      _arg1_whichliinf = _arg1_whichliinf_
      res,resargs = self.__obj.getlintinf(_arg1_whichliinf_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_ivalue_return_value = resargs
      return __arg2_ivalue_return_value
    
    def getintparam(self,_arg1_param_): # 2
      """
      Obtains an integer parameter.
    
      getintparam(self,_arg1_param_)
        _arg1_param: mosek.iparam. <no description>
      returns: _arg2_parvalue
        _arg2_parvalue: int. <no description>
      """
      if not isinstance(_arg1_param_,iparam): raise TypeError("Argument _arg1_param has wrong type")
      _arg1_param = _arg1_param_
      res,resargs = self.__obj.getintparam(_arg1_param_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_parvalue_return_value = resargs
      return __arg2_parvalue_return_value
    
    def getmaxnumanz(self): # 2
      """
      Obtains number of preallocated non-zeros in the linear constraint matrix.
    
      getmaxnumanz(self)
      returns: _arg1_maxnumanz
        _arg1_maxnumanz: long. <no description>
      """
      res,resargs = self.__obj.getmaxnumanz64()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_maxnumanz_return_value = resargs
      return __arg1_maxnumanz_return_value
    
    def getmaxnumcon(self): # 2
      """
      Obtains the number of preallocated constraints in the optimization task.
    
      getmaxnumcon(self)
      returns: _arg1_maxnumcon
        _arg1_maxnumcon: int. <no description>
      """
      res,resargs = self.__obj.getmaxnumcon()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_maxnumcon_return_value = resargs
      return __arg1_maxnumcon_return_value
    
    def getmaxnumvar(self): # 2
      """
      Obtains the maximum number variables allowed.
    
      getmaxnumvar(self)
      returns: _arg1_maxnumvar
        _arg1_maxnumvar: int. <no description>
      """
      res,resargs = self.__obj.getmaxnumvar()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_maxnumvar_return_value = resargs
      return __arg1_maxnumvar_return_value
    
    def getbarvarnamelen(self,_arg1_i): # 2
      """
      Obtains the length of the name of a semidefinite variable.
    
      getbarvarnamelen(self,_arg1_i)
        _arg1_i: int. Index of the variable.
      returns: _arg2_len
        _arg2_len: int. Returns the length of the indicated name.
      """
      _arg1_i_ = _arg1_i
      res,resargs = self.__obj.getbarvarnamelen(_arg1_i_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_len_return_value = resargs
      return __arg2_len_return_value
    
    def getbarvarname(self,_arg1_i): # 2
      """
      Obtains the name of a semidefinite variable.
    
      getbarvarname(self,_arg1_i)
        _arg1_i: int. Index of the variable.
      returns: _arg3_name
        _arg3_name: str. The requested name is copied to this buffer.
      """
      _arg1_i_ = _arg1_i
      _arg2_sizename_ = (1 + self.getbarvarnamelen( _arg1_i ))
      _arg2_sizename = _arg2_sizename_
      arr__arg3_name = (ctypes.c_char* _arg2_sizename )()
      memview_arr__arg3_name = memoryview(arr__arg3_name)
      res,resargs = self.__obj.getbarvarname(_arg1_i_,_arg2_sizename_,memview_arr__arg3_name)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      retarg__arg3_name = resargs
      retarg__arg3_name = arr__arg3_name.value.decode("utf-8",errors="ignore")
      return retarg__arg3_name
    
    def getbarvarnameindex(self,_arg1_somename_): # 2
      """
      Obtains the index of semidefinite variable from its name.
    
      getbarvarnameindex(self,_arg1_somename_)
        _arg1_somename: str. The name of the variable.
      returns: _arg2_asgn,_arg3_index
        _arg2_asgn: int. Non-zero if the name somename is assigned to some semidefinite variable.
        _arg3_index: int. The index of a semidefinite variable with the name somename (if one exists).
      """
      res,resargs = self.__obj.getbarvarnameindex(_arg1_somename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_asgn_return_value,__arg3_index_return_value = resargs
      return __arg2_asgn_return_value,__arg3_index_return_value
    
    def generatevarnames(self,_arg2_subj,_arg3_fmt_,_arg5_dims,_arg6_sp): # 2
      """
      Generates systematic names for variables.
    
      generatevarnames(self,_arg2_subj,_arg3_fmt_,_arg5_dims,_arg6_sp)
        _arg2_subj: array of int. Indexes of the variables.
        _arg3_fmt: str. The variable name formatting string.
        _arg5_dims: array of int. Dimensions in the shape.
        _arg6_sp: array of long. Items that should be named.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subj)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subj):
        raise IndexError("Inconsistent length of array _arg2_subj")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subj is None: raise TypeError("Invalid type for argument _arg2_subj")
      if _arg2_subj is None:
        _arg2_subj_ = None
      else:
        try:
          _arg2_subj_ = memoryview(_arg2_subj)
        except TypeError:
          try:
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
          except TypeError:
            raise TypeError("Argument _arg2_subj has wrong type")
          else:
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
        else:
          if _arg2_subj_.format != "i":
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
      _arg4_ndims_ = None
      if _arg4_ndims_ is None:
        _arg4_ndims_ = len(_arg5_dims)
        _arg4_ndims = _arg4_ndims_
      elif _arg4_ndims_ != len(_arg5_dims):
        raise IndexError("Inconsistent length of array _arg5_dims")
      if _arg4_ndims_ is None: _arg4_ndims_ = 0
      if _arg5_dims is None: raise TypeError("Invalid type for argument _arg5_dims")
      if _arg5_dims is None:
        _arg5_dims_ = None
      else:
        try:
          _arg5_dims_ = memoryview(_arg5_dims)
        except TypeError:
          try:
            _tmparr__arg5_dims = (ctypes.c_int32*len(_arg5_dims))()
            _tmparr__arg5_dims[:] = _arg5_dims
          except TypeError:
            raise TypeError("Argument _arg5_dims has wrong type")
          else:
            _arg5_dims_ = memoryview(_tmparr__arg5_dims)
      
        else:
          if _arg5_dims_.format != "i":
            _tmparr__arg5_dims = (ctypes.c_int32*len(_arg5_dims))()
            _tmparr__arg5_dims[:] = _arg5_dims
            _arg5_dims_ = memoryview(_tmparr__arg5_dims)
      
      if _arg6_sp is None:
        _arg6_sp_ = None
      else:
        try:
          _arg6_sp_ = memoryview(_arg6_sp)
        except TypeError:
          try:
            _tmparr__arg6_sp = (ctypes.c_int64*len(_arg6_sp))()
            _tmparr__arg6_sp[:] = _arg6_sp
          except TypeError:
            raise TypeError("Argument _arg6_sp has wrong type")
          else:
            _arg6_sp_ = memoryview(_tmparr__arg6_sp)
      
        else:
          if _arg6_sp_.format != "q":
            _tmparr__arg6_sp = (ctypes.c_int64*len(_arg6_sp))()
            _tmparr__arg6_sp[:] = _arg6_sp
            _arg6_sp_ = memoryview(_tmparr__arg6_sp)
      
      if _arg6_sp_ is not None and len(_arg6_sp_) !=  _arg1_num :
        raise ValueError("Array argument _arg6_sp has wrong length")
      res = self.__obj.generatevarnames(_arg1_num_,_arg2_subj_,_arg3_fmt_,_arg4_ndims_,_arg5_dims_,_arg6_sp_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def generateconnames(self,_arg2_subi,_arg3_fmt_,_arg5_dims,_arg6_sp): # 2
      """
      Generates systematic names for constraints.
    
      generateconnames(self,_arg2_subi,_arg3_fmt_,_arg5_dims,_arg6_sp)
        _arg2_subi: array of int. Indexes of the constraints.
        _arg3_fmt: str. The constraint name formatting string.
        _arg5_dims: array of int. Dimensions in the shape.
        _arg6_sp: array of long. Items that should be named.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subi)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subi):
        raise IndexError("Inconsistent length of array _arg2_subi")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subi is None: raise TypeError("Invalid type for argument _arg2_subi")
      if _arg2_subi is None:
        _arg2_subi_ = None
      else:
        try:
          _arg2_subi_ = memoryview(_arg2_subi)
        except TypeError:
          try:
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
          except TypeError:
            raise TypeError("Argument _arg2_subi has wrong type")
          else:
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
        else:
          if _arg2_subi_.format != "i":
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
      _arg4_ndims_ = None
      if _arg4_ndims_ is None:
        _arg4_ndims_ = len(_arg5_dims)
        _arg4_ndims = _arg4_ndims_
      elif _arg4_ndims_ != len(_arg5_dims):
        raise IndexError("Inconsistent length of array _arg5_dims")
      if _arg4_ndims_ is None: _arg4_ndims_ = 0
      if _arg5_dims is None: raise TypeError("Invalid type for argument _arg5_dims")
      if _arg5_dims is None:
        _arg5_dims_ = None
      else:
        try:
          _arg5_dims_ = memoryview(_arg5_dims)
        except TypeError:
          try:
            _tmparr__arg5_dims = (ctypes.c_int32*len(_arg5_dims))()
            _tmparr__arg5_dims[:] = _arg5_dims
          except TypeError:
            raise TypeError("Argument _arg5_dims has wrong type")
          else:
            _arg5_dims_ = memoryview(_tmparr__arg5_dims)
      
        else:
          if _arg5_dims_.format != "i":
            _tmparr__arg5_dims = (ctypes.c_int32*len(_arg5_dims))()
            _tmparr__arg5_dims[:] = _arg5_dims
            _arg5_dims_ = memoryview(_tmparr__arg5_dims)
      
      if _arg6_sp is None:
        _arg6_sp_ = None
      else:
        try:
          _arg6_sp_ = memoryview(_arg6_sp)
        except TypeError:
          try:
            _tmparr__arg6_sp = (ctypes.c_int64*len(_arg6_sp))()
            _tmparr__arg6_sp[:] = _arg6_sp
          except TypeError:
            raise TypeError("Argument _arg6_sp has wrong type")
          else:
            _arg6_sp_ = memoryview(_tmparr__arg6_sp)
      
        else:
          if _arg6_sp_.format != "q":
            _tmparr__arg6_sp = (ctypes.c_int64*len(_arg6_sp))()
            _tmparr__arg6_sp[:] = _arg6_sp
            _arg6_sp_ = memoryview(_tmparr__arg6_sp)
      
      if _arg6_sp_ is not None and len(_arg6_sp_) !=  _arg1_num :
        raise ValueError("Array argument _arg6_sp has wrong length")
      res = self.__obj.generateconnames(_arg1_num_,_arg2_subi_,_arg3_fmt_,_arg4_ndims_,_arg5_dims_,_arg6_sp_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def generateconenames(self,_arg2_subk,_arg3_fmt_,_arg5_dims,_arg6_sp): # 2
      """
      Generates systematic names for cone.
    
      generateconenames(self,_arg2_subk,_arg3_fmt_,_arg5_dims,_arg6_sp)
        _arg2_subk: array of int. Indexes of the cone.
        _arg3_fmt: str. The cone name formatting string.
        _arg5_dims: array of int. Dimensions in the shape.
        _arg6_sp: array of long. Items that should be named.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subk)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subk):
        raise IndexError("Inconsistent length of array _arg2_subk")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subk is None: raise TypeError("Invalid type for argument _arg2_subk")
      if _arg2_subk is None:
        _arg2_subk_ = None
      else:
        try:
          _arg2_subk_ = memoryview(_arg2_subk)
        except TypeError:
          try:
            _tmparr__arg2_subk = (ctypes.c_int32*len(_arg2_subk))()
            _tmparr__arg2_subk[:] = _arg2_subk
          except TypeError:
            raise TypeError("Argument _arg2_subk has wrong type")
          else:
            _arg2_subk_ = memoryview(_tmparr__arg2_subk)
      
        else:
          if _arg2_subk_.format != "i":
            _tmparr__arg2_subk = (ctypes.c_int32*len(_arg2_subk))()
            _tmparr__arg2_subk[:] = _arg2_subk
            _arg2_subk_ = memoryview(_tmparr__arg2_subk)
      
      _arg4_ndims_ = None
      if _arg4_ndims_ is None:
        _arg4_ndims_ = len(_arg5_dims)
        _arg4_ndims = _arg4_ndims_
      elif _arg4_ndims_ != len(_arg5_dims):
        raise IndexError("Inconsistent length of array _arg5_dims")
      if _arg4_ndims_ is None: _arg4_ndims_ = 0
      if _arg5_dims is None: raise TypeError("Invalid type for argument _arg5_dims")
      if _arg5_dims is None:
        _arg5_dims_ = None
      else:
        try:
          _arg5_dims_ = memoryview(_arg5_dims)
        except TypeError:
          try:
            _tmparr__arg5_dims = (ctypes.c_int32*len(_arg5_dims))()
            _tmparr__arg5_dims[:] = _arg5_dims
          except TypeError:
            raise TypeError("Argument _arg5_dims has wrong type")
          else:
            _arg5_dims_ = memoryview(_tmparr__arg5_dims)
      
        else:
          if _arg5_dims_.format != "i":
            _tmparr__arg5_dims = (ctypes.c_int32*len(_arg5_dims))()
            _tmparr__arg5_dims[:] = _arg5_dims
            _arg5_dims_ = memoryview(_tmparr__arg5_dims)
      
      if _arg6_sp is None:
        _arg6_sp_ = None
      else:
        try:
          _arg6_sp_ = memoryview(_arg6_sp)
        except TypeError:
          try:
            _tmparr__arg6_sp = (ctypes.c_int64*len(_arg6_sp))()
            _tmparr__arg6_sp[:] = _arg6_sp
          except TypeError:
            raise TypeError("Argument _arg6_sp has wrong type")
          else:
            _arg6_sp_ = memoryview(_tmparr__arg6_sp)
      
        else:
          if _arg6_sp_.format != "q":
            _tmparr__arg6_sp = (ctypes.c_int64*len(_arg6_sp))()
            _tmparr__arg6_sp[:] = _arg6_sp
            _arg6_sp_ = memoryview(_tmparr__arg6_sp)
      
      if _arg6_sp_ is not None and len(_arg6_sp_) !=  _arg1_num :
        raise ValueError("Array argument _arg6_sp has wrong length")
      res = self.__obj.generateconenames(_arg1_num_,_arg2_subk_,_arg3_fmt_,_arg4_ndims_,_arg5_dims_,_arg6_sp_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putconname(self,_arg1_i,_arg2_name_): # 2
      """
      Sets the name of a constraint.
    
      putconname(self,_arg1_i,_arg2_name_)
        _arg1_i: int. Index of the constraint.
        _arg2_name: str. The name of the constraint.
      """
      _arg1_i_ = _arg1_i
      res = self.__obj.putconname(_arg1_i_,_arg2_name_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putvarname(self,_arg1_j,_arg2_name_): # 2
      """
      Sets the name of a variable.
    
      putvarname(self,_arg1_j,_arg2_name_)
        _arg1_j: int. Index of the variable.
        _arg2_name: str. The variable name.
      """
      _arg1_j_ = _arg1_j
      res = self.__obj.putvarname(_arg1_j_,_arg2_name_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putconename(self,_arg1_j,_arg2_name_): # 2
      """
      Sets the name of a cone.
    
      putconename(self,_arg1_j,_arg2_name_)
        _arg1_j: int. Index of the cone.
        _arg2_name: str. The name of the cone.
      """
      _arg1_j_ = _arg1_j
      res = self.__obj.putconename(_arg1_j_,_arg2_name_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putbarvarname(self,_arg1_j,_arg2_name_): # 2
      """
      Sets the name of a semidefinite variable.
    
      putbarvarname(self,_arg1_j,_arg2_name_)
        _arg1_j: int. Index of the variable.
        _arg2_name: str. The variable name.
      """
      _arg1_j_ = _arg1_j
      res = self.__obj.putbarvarname(_arg1_j_,_arg2_name_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getvarnamelen(self,_arg1_i): # 2
      """
      Obtains the length of the name of a variable.
    
      getvarnamelen(self,_arg1_i)
        _arg1_i: int. Index of a variable.
      returns: _arg2_len
        _arg2_len: int. Returns the length of the indicated name.
      """
      _arg1_i_ = _arg1_i
      res,resargs = self.__obj.getvarnamelen(_arg1_i_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_len_return_value = resargs
      return __arg2_len_return_value
    
    def getvarname(self,_arg1_j): # 2
      """
      Obtains the name of a variable.
    
      getvarname(self,_arg1_j)
        _arg1_j: int. Index of a variable.
      returns: _arg3_name
        _arg3_name: str. Returns the required name.
      """
      _arg1_j_ = _arg1_j
      _arg2_sizename_ = (1 + self.getvarnamelen( _arg1_j ))
      _arg2_sizename = _arg2_sizename_
      arr__arg3_name = (ctypes.c_char* _arg2_sizename )()
      memview_arr__arg3_name = memoryview(arr__arg3_name)
      res,resargs = self.__obj.getvarname(_arg1_j_,_arg2_sizename_,memview_arr__arg3_name)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      retarg__arg3_name = resargs
      retarg__arg3_name = arr__arg3_name.value.decode("utf-8",errors="ignore")
      return retarg__arg3_name
    
    def getconnamelen(self,_arg1_i): # 2
      """
      Obtains the length of the name of a constraint.
    
      getconnamelen(self,_arg1_i)
        _arg1_i: int. Index of the constraint.
      returns: _arg2_len
        _arg2_len: int. Returns the length of the indicated name.
      """
      _arg1_i_ = _arg1_i
      res,resargs = self.__obj.getconnamelen(_arg1_i_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_len_return_value = resargs
      return __arg2_len_return_value
    
    def getconname(self,_arg1_i): # 2
      """
      Obtains the name of a constraint.
    
      getconname(self,_arg1_i)
        _arg1_i: int. Index of the constraint.
      returns: _arg3_name
        _arg3_name: str. The required name.
      """
      _arg1_i_ = _arg1_i
      _arg2_sizename_ = (1 + self.getconnamelen( _arg1_i ))
      _arg2_sizename = _arg2_sizename_
      arr__arg3_name = (ctypes.c_char* _arg2_sizename )()
      memview_arr__arg3_name = memoryview(arr__arg3_name)
      res,resargs = self.__obj.getconname(_arg1_i_,_arg2_sizename_,memview_arr__arg3_name)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      retarg__arg3_name = resargs
      retarg__arg3_name = arr__arg3_name.value.decode("utf-8",errors="ignore")
      return retarg__arg3_name
    
    def getconnameindex(self,_arg1_somename_): # 2
      """
      Checks whether the name has been assigned to any constraint.
    
      getconnameindex(self,_arg1_somename_)
        _arg1_somename: str. The name which should be checked.
      returns: _arg2_asgn,_arg3_index
        _arg2_asgn: int. Is non-zero if the name somename is assigned to some constraint.
        _arg3_index: int. If the name somename is assigned to a constraint, then return the index of the constraint.
      """
      res,resargs = self.__obj.getconnameindex(_arg1_somename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_asgn_return_value,__arg3_index_return_value = resargs
      return __arg2_asgn_return_value,__arg3_index_return_value
    
    def getvarnameindex(self,_arg1_somename_): # 2
      """
      Checks whether the name has been assigned to any variable.
    
      getvarnameindex(self,_arg1_somename_)
        _arg1_somename: str. The name which should be checked.
      returns: _arg2_asgn,_arg3_index
        _arg2_asgn: int. Is non-zero if the name somename is assigned to a variable.
        _arg3_index: int. If the name somename is assigned to a variable, then return the index of the variable.
      """
      res,resargs = self.__obj.getvarnameindex(_arg1_somename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_asgn_return_value,__arg3_index_return_value = resargs
      return __arg2_asgn_return_value,__arg3_index_return_value
    
    def getconenamelen(self,_arg1_i): # 2
      """
      Obtains the length of the name of a cone.
    
      getconenamelen(self,_arg1_i)
        _arg1_i: int. Index of the cone.
      returns: _arg2_len
        _arg2_len: int. Returns the length of the indicated name.
      """
      _arg1_i_ = _arg1_i
      res,resargs = self.__obj.getconenamelen(_arg1_i_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_len_return_value = resargs
      return __arg2_len_return_value
    
    def getconename(self,_arg1_i): # 2
      """
      Obtains the name of a cone.
    
      getconename(self,_arg1_i)
        _arg1_i: int. Index of the cone.
      returns: _arg3_name
        _arg3_name: str. The required name.
      """
      _arg1_i_ = _arg1_i
      _arg2_sizename_ = (1 + self.getconenamelen( _arg1_i ))
      _arg2_sizename = _arg2_sizename_
      arr__arg3_name = (ctypes.c_char* _arg2_sizename )()
      memview_arr__arg3_name = memoryview(arr__arg3_name)
      res,resargs = self.__obj.getconename(_arg1_i_,_arg2_sizename_,memview_arr__arg3_name)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      retarg__arg3_name = resargs
      retarg__arg3_name = arr__arg3_name.value.decode("utf-8",errors="ignore")
      return retarg__arg3_name
    
    def getconenameindex(self,_arg1_somename_): # 2
      """
      Checks whether the name has been assigned to any cone.
    
      getconenameindex(self,_arg1_somename_)
        _arg1_somename: str. The name which should be checked.
      returns: _arg2_asgn,_arg3_index
        _arg2_asgn: int. Is non-zero if the name somename is assigned to some cone.
        _arg3_index: int. If the name somename is assigned to some cone, this is the index of the cone.
      """
      res,resargs = self.__obj.getconenameindex(_arg1_somename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_asgn_return_value,__arg3_index_return_value = resargs
      return __arg2_asgn_return_value,__arg3_index_return_value
    
    def getnumanz(self): # 2
      """
      Obtains the number of non-zeros in the coefficient matrix.
    
      getnumanz(self)
      returns: _arg1_numanz
        _arg1_numanz: int. <no description>
      """
      res,resargs = self.__obj.getnumanz()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_numanz_return_value = resargs
      return __arg1_numanz_return_value
    
    def getnumanz64(self): # 2
      """
      Obtains the number of non-zeros in the coefficient matrix.
    
      getnumanz64(self)
      returns: _arg1_numanz
        _arg1_numanz: long. <no description>
      """
      res,resargs = self.__obj.getnumanz64()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_numanz_return_value = resargs
      return __arg1_numanz_return_value
    
    def getnumcon(self): # 2
      """
      Obtains the number of constraints.
    
      getnumcon(self)
      returns: _arg1_numcon
        _arg1_numcon: int. <no description>
      """
      res,resargs = self.__obj.getnumcon()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_numcon_return_value = resargs
      return __arg1_numcon_return_value
    
    def getnumcone(self): # 2
      """
      Obtains the number of cones.
    
      getnumcone(self)
      returns: _arg1_numcone
        _arg1_numcone: int. Number of conic constraints.
      """
      res,resargs = self.__obj.getnumcone()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_numcone_return_value = resargs
      return __arg1_numcone_return_value
    
    def getnumconemem(self,_arg1_k): # 2
      """
      Obtains the number of members in a cone.
    
      getnumconemem(self,_arg1_k)
        _arg1_k: int. Index of the cone.
      returns: _arg2_nummem
        _arg2_nummem: int. <no description>
      """
      _arg1_k_ = _arg1_k
      res,resargs = self.__obj.getnumconemem(_arg1_k_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_nummem_return_value = resargs
      return __arg2_nummem_return_value
    
    def getnumintvar(self): # 2
      """
      Obtains the number of integer-constrained variables.
    
      getnumintvar(self)
      returns: _arg1_numintvar
        _arg1_numintvar: int. Number of integer variables.
      """
      res,resargs = self.__obj.getnumintvar()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_numintvar_return_value = resargs
      return __arg1_numintvar_return_value
    
    def getnumparam(self,_arg1_partype_): # 2
      """
      Obtains the number of parameters of a given type.
    
      getnumparam(self,_arg1_partype_)
        _arg1_partype: mosek.parametertype. <no description>
      returns: _arg2_numparam
        _arg2_numparam: int. Returns the number of parameters of the requested type.
      """
      if not isinstance(_arg1_partype_,parametertype): raise TypeError("Argument _arg1_partype has wrong type")
      _arg1_partype = _arg1_partype_
      res,resargs = self.__obj.getnumparam(_arg1_partype_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_numparam_return_value = resargs
      return __arg2_numparam_return_value
    
    def getnumqconknz(self,_arg1_k): # 2
      """
      Obtains the number of non-zero quadratic terms in a constraint.
    
      getnumqconknz(self,_arg1_k)
        _arg1_k: int. Index of the constraint for which the number quadratic terms should be obtained.
      returns: _arg2_numqcnz
        _arg2_numqcnz: long. <no description>
      """
      _arg1_k_ = _arg1_k
      res,resargs = self.__obj.getnumqconknz64(_arg1_k_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_numqcnz_return_value = resargs
      return __arg2_numqcnz_return_value
    
    def getnumqobjnz(self): # 2
      """
      Obtains the number of non-zero quadratic terms in the objective.
    
      getnumqobjnz(self)
      returns: _arg1_numqonz
        _arg1_numqonz: long. <no description>
      """
      res,resargs = self.__obj.getnumqobjnz64()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_numqonz_return_value = resargs
      return __arg1_numqonz_return_value
    
    def getnumvar(self): # 2
      """
      Obtains the number of variables.
    
      getnumvar(self)
      returns: _arg1_numvar
        _arg1_numvar: int. <no description>
      """
      res,resargs = self.__obj.getnumvar()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_numvar_return_value = resargs
      return __arg1_numvar_return_value
    
    def getnumbarvar(self): # 2
      """
      Obtains the number of semidefinite variables.
    
      getnumbarvar(self)
      returns: _arg1_numbarvar
        _arg1_numbarvar: int. Number of semidefinite variables in the problem.
      """
      res,resargs = self.__obj.getnumbarvar()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_numbarvar_return_value = resargs
      return __arg1_numbarvar_return_value
    
    def getmaxnumbarvar(self): # 2
      """
      Obtains maximum number of symmetric matrix variables for which space is currently preallocated.
    
      getmaxnumbarvar(self)
      returns: _arg1_maxnumbarvar
        _arg1_maxnumbarvar: int. Maximum number of symmetric matrix variables for which space is currently preallocated.
      """
      res,resargs = self.__obj.getmaxnumbarvar()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_maxnumbarvar_return_value = resargs
      return __arg1_maxnumbarvar_return_value
    
    def getdimbarvarj(self,_arg1_j): # 2
      """
      Obtains the dimension of a symmetric matrix variable.
    
      getdimbarvarj(self,_arg1_j)
        _arg1_j: int. Index of the semidefinite variable whose dimension is requested.
      returns: _arg2_dimbarvarj
        _arg2_dimbarvarj: int. The dimension of the j'th semidefinite variable.
      """
      _arg1_j_ = _arg1_j
      res,resargs = self.__obj.getdimbarvarj(_arg1_j_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_dimbarvarj_return_value = resargs
      return __arg2_dimbarvarj_return_value
    
    def getlenbarvarj(self,_arg1_j): # 2
      """
      Obtains the length of one semidefinite variable.
    
      getlenbarvarj(self,_arg1_j)
        _arg1_j: int. Index of the semidefinite variable whose length if requested.
      returns: _arg2_lenbarvarj
        _arg2_lenbarvarj: long. Number of scalar elements in the lower triangular part of the semidefinite variable.
      """
      _arg1_j_ = _arg1_j
      res,resargs = self.__obj.getlenbarvarj(_arg1_j_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_lenbarvarj_return_value = resargs
      return __arg2_lenbarvarj_return_value
    
    def getobjname(self): # 2
      """
      Obtains the name assigned to the objective function.
    
      getobjname(self)
      returns: _arg2_objname
        _arg2_objname: str. Assigned the objective name.
      """
      _arg1_sizeobjname_ = (1 + self.getobjnamelen())
      _arg1_sizeobjname = _arg1_sizeobjname_
      arr__arg2_objname = (ctypes.c_char* _arg1_sizeobjname )()
      memview_arr__arg2_objname = memoryview(arr__arg2_objname)
      res,resargs = self.__obj.getobjname(_arg1_sizeobjname_,memview_arr__arg2_objname)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      retarg__arg2_objname = resargs
      retarg__arg2_objname = arr__arg2_objname.value.decode("utf-8",errors="ignore")
      return retarg__arg2_objname
    
    def getobjnamelen(self): # 2
      """
      Obtains the length of the name assigned to the objective function.
    
      getobjnamelen(self)
      returns: _arg1_len
        _arg1_len: int. Assigned the length of the objective name.
      """
      res,resargs = self.__obj.getobjnamelen()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_len_return_value = resargs
      return __arg1_len_return_value
    
    def getprimalobj(self,_arg1_whichsol_): # 2
      """
      Computes the primal objective value for the desired solution.
    
      getprimalobj(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. <no description>
      returns: _arg2_primalobj
        _arg2_primalobj: double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res,resargs = self.__obj.getprimalobj(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_primalobj_return_value = resargs
      return __arg2_primalobj_return_value
    
    def getprobtype(self): # 2
      """
      Obtains the problem type.
    
      getprobtype(self)
      returns: _arg1_probtype
        _arg1_probtype: mosek.problemtype. The problem type.
      """
      res,resargs = self.__obj.getprobtype()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_probtype_return_value = resargs
      __arg1_probtype_return_value = problemtype(__arg1_probtype_return_value)
      return __arg1_probtype_return_value
    
    def getqconk(self,_arg1_k,_arg5_qcsubi,_arg6_qcsubj,_arg7_qcval): # 2
      """
      Obtains all the quadratic terms in a constraint.
    
      getqconk(self,_arg1_k,_arg5_qcsubi,_arg6_qcsubj,_arg7_qcval)
        _arg1_k: int. Which constraint.
        _arg5_qcsubi: array of int. <no description>
        _arg6_qcsubj: array of int. <no description>
        _arg7_qcval: array of double. <no description>
      returns: _arg4_numqcnz
        _arg4_numqcnz: long. <no description>
      """
      _arg1_k_ = _arg1_k
      _arg2_maxnumqcnz_ = self.getnumqconknz( _arg1_k )
      _arg2_maxnumqcnz = _arg2_maxnumqcnz_
      if _arg5_qcsubi is None: raise TypeError("Invalid type for argument _arg5_qcsubi")
      _copyback__arg5_qcsubi = False
      if _arg5_qcsubi is None:
        _arg5_qcsubi_ = None
      else:
        try:
          _arg5_qcsubi_ = memoryview(_arg5_qcsubi)
        except TypeError:
          try:
            _tmparr__arg5_qcsubi = (ctypes.c_int32*len(_arg5_qcsubi))()
            _tmparr__arg5_qcsubi[:] = _arg5_qcsubi
          except TypeError:
            raise TypeError("Argument _arg5_qcsubi has wrong type")
          else:
            _arg5_qcsubi_ = memoryview(_tmparr__arg5_qcsubi)
            _copyback__arg5_qcsubi = True
        else:
          if _arg5_qcsubi_.format != "i":
            _tmparr__arg5_qcsubi = (ctypes.c_int32*len(_arg5_qcsubi))()
            _tmparr__arg5_qcsubi[:] = _arg5_qcsubi
            _arg5_qcsubi_ = memoryview(_tmparr__arg5_qcsubi)
            _copyback__arg5_qcsubi = True
      if _arg5_qcsubi_ is not None and len(_arg5_qcsubi_) != self.getnumqconknz( _arg1_k ):
        raise ValueError("Array argument _arg5_qcsubi has wrong length")
      if _arg6_qcsubj is None: raise TypeError("Invalid type for argument _arg6_qcsubj")
      _copyback__arg6_qcsubj = False
      if _arg6_qcsubj is None:
        _arg6_qcsubj_ = None
      else:
        try:
          _arg6_qcsubj_ = memoryview(_arg6_qcsubj)
        except TypeError:
          try:
            _tmparr__arg6_qcsubj = (ctypes.c_int32*len(_arg6_qcsubj))()
            _tmparr__arg6_qcsubj[:] = _arg6_qcsubj
          except TypeError:
            raise TypeError("Argument _arg6_qcsubj has wrong type")
          else:
            _arg6_qcsubj_ = memoryview(_tmparr__arg6_qcsubj)
            _copyback__arg6_qcsubj = True
        else:
          if _arg6_qcsubj_.format != "i":
            _tmparr__arg6_qcsubj = (ctypes.c_int32*len(_arg6_qcsubj))()
            _tmparr__arg6_qcsubj[:] = _arg6_qcsubj
            _arg6_qcsubj_ = memoryview(_tmparr__arg6_qcsubj)
            _copyback__arg6_qcsubj = True
      if _arg6_qcsubj_ is not None and len(_arg6_qcsubj_) != self.getnumqconknz( _arg1_k ):
        raise ValueError("Array argument _arg6_qcsubj has wrong length")
      if _arg7_qcval is None: raise TypeError("Invalid type for argument _arg7_qcval")
      _copyback__arg7_qcval = False
      if _arg7_qcval is None:
        _arg7_qcval_ = None
      else:
        try:
          _arg7_qcval_ = memoryview(_arg7_qcval)
        except TypeError:
          try:
            _tmparr__arg7_qcval = (ctypes.c_double*len(_arg7_qcval))()
            _tmparr__arg7_qcval[:] = _arg7_qcval
          except TypeError:
            raise TypeError("Argument _arg7_qcval has wrong type")
          else:
            _arg7_qcval_ = memoryview(_tmparr__arg7_qcval)
            _copyback__arg7_qcval = True
        else:
          if _arg7_qcval_.format != "d":
            _tmparr__arg7_qcval = (ctypes.c_double*len(_arg7_qcval))()
            _tmparr__arg7_qcval[:] = _arg7_qcval
            _arg7_qcval_ = memoryview(_tmparr__arg7_qcval)
            _copyback__arg7_qcval = True
      if _arg7_qcval_ is not None and len(_arg7_qcval_) != self.getnumqconknz( _arg1_k ):
        raise ValueError("Array argument _arg7_qcval has wrong length")
      res,resargs = self.__obj.getqconk64(_arg1_k_,_arg2_maxnumqcnz_,len(_arg5_qcsubi),_arg5_qcsubi_,_arg6_qcsubj_,_arg7_qcval_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg4_numqcnz_return_value = resargs
      if _copyback__arg7_qcval:
        _arg7_qcval[:] = _tmparr__arg7_qcval
      if _copyback__arg6_qcsubj:
        _arg6_qcsubj[:] = _tmparr__arg6_qcsubj
      if _copyback__arg5_qcsubi:
        _arg5_qcsubi[:] = _tmparr__arg5_qcsubi
      return __arg4_numqcnz_return_value
    
    def getqobj(self,_arg4_qosubi,_arg5_qosubj,_arg6_qoval): # 2
      """
      Obtains all the quadratic terms in the objective.
    
      getqobj(self,_arg4_qosubi,_arg5_qosubj,_arg6_qoval)
        _arg4_qosubi: array of int. <no description>
        _arg5_qosubj: array of int. <no description>
        _arg6_qoval: array of double. <no description>
      returns: _arg3_numqonz
        _arg3_numqonz: long. <no description>
      """
      _arg1_maxnumqonz_ = self.getnumqobjnz()
      _arg1_maxnumqonz = _arg1_maxnumqonz_
      if _arg4_qosubi is None: raise TypeError("Invalid type for argument _arg4_qosubi")
      _copyback__arg4_qosubi = False
      if _arg4_qosubi is None:
        _arg4_qosubi_ = None
      else:
        try:
          _arg4_qosubi_ = memoryview(_arg4_qosubi)
        except TypeError:
          try:
            _tmparr__arg4_qosubi = (ctypes.c_int32*len(_arg4_qosubi))()
            _tmparr__arg4_qosubi[:] = _arg4_qosubi
          except TypeError:
            raise TypeError("Argument _arg4_qosubi has wrong type")
          else:
            _arg4_qosubi_ = memoryview(_tmparr__arg4_qosubi)
            _copyback__arg4_qosubi = True
        else:
          if _arg4_qosubi_.format != "i":
            _tmparr__arg4_qosubi = (ctypes.c_int32*len(_arg4_qosubi))()
            _tmparr__arg4_qosubi[:] = _arg4_qosubi
            _arg4_qosubi_ = memoryview(_tmparr__arg4_qosubi)
            _copyback__arg4_qosubi = True
      if _arg4_qosubi_ is not None and len(_arg4_qosubi_) !=  _arg1_maxnumqonz :
        raise ValueError("Array argument _arg4_qosubi has wrong length")
      if _arg5_qosubj is None: raise TypeError("Invalid type for argument _arg5_qosubj")
      _copyback__arg5_qosubj = False
      if _arg5_qosubj is None:
        _arg5_qosubj_ = None
      else:
        try:
          _arg5_qosubj_ = memoryview(_arg5_qosubj)
        except TypeError:
          try:
            _tmparr__arg5_qosubj = (ctypes.c_int32*len(_arg5_qosubj))()
            _tmparr__arg5_qosubj[:] = _arg5_qosubj
          except TypeError:
            raise TypeError("Argument _arg5_qosubj has wrong type")
          else:
            _arg5_qosubj_ = memoryview(_tmparr__arg5_qosubj)
            _copyback__arg5_qosubj = True
        else:
          if _arg5_qosubj_.format != "i":
            _tmparr__arg5_qosubj = (ctypes.c_int32*len(_arg5_qosubj))()
            _tmparr__arg5_qosubj[:] = _arg5_qosubj
            _arg5_qosubj_ = memoryview(_tmparr__arg5_qosubj)
            _copyback__arg5_qosubj = True
      if _arg5_qosubj_ is not None and len(_arg5_qosubj_) !=  _arg1_maxnumqonz :
        raise ValueError("Array argument _arg5_qosubj has wrong length")
      if _arg6_qoval is None: raise TypeError("Invalid type for argument _arg6_qoval")
      _copyback__arg6_qoval = False
      if _arg6_qoval is None:
        _arg6_qoval_ = None
      else:
        try:
          _arg6_qoval_ = memoryview(_arg6_qoval)
        except TypeError:
          try:
            _tmparr__arg6_qoval = (ctypes.c_double*len(_arg6_qoval))()
            _tmparr__arg6_qoval[:] = _arg6_qoval
          except TypeError:
            raise TypeError("Argument _arg6_qoval has wrong type")
          else:
            _arg6_qoval_ = memoryview(_tmparr__arg6_qoval)
            _copyback__arg6_qoval = True
        else:
          if _arg6_qoval_.format != "d":
            _tmparr__arg6_qoval = (ctypes.c_double*len(_arg6_qoval))()
            _tmparr__arg6_qoval[:] = _arg6_qoval
            _arg6_qoval_ = memoryview(_tmparr__arg6_qoval)
            _copyback__arg6_qoval = True
      if _arg6_qoval_ is not None and len(_arg6_qoval_) !=  _arg1_maxnumqonz :
        raise ValueError("Array argument _arg6_qoval has wrong length")
      res,resargs = self.__obj.getqobj64(_arg1_maxnumqonz_,len(_arg4_qosubi),_arg4_qosubi_,_arg5_qosubj_,_arg6_qoval_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg3_numqonz_return_value = resargs
      if _copyback__arg6_qoval:
        _arg6_qoval[:] = _tmparr__arg6_qoval
      if _copyback__arg5_qosubj:
        _arg5_qosubj[:] = _tmparr__arg5_qosubj
      if _copyback__arg4_qosubi:
        _arg4_qosubi[:] = _tmparr__arg4_qosubi
      return __arg3_numqonz_return_value
    
    def getqobjij(self,_arg1_i,_arg2_j): # 2
      """
      Obtains one coefficient from the quadratic term of the objective
    
      getqobjij(self,_arg1_i,_arg2_j)
        _arg1_i: int. Row index of the coefficient.
        _arg2_j: int. Column index of coefficient.
      returns: _arg3_qoij
        _arg3_qoij: double. The required coefficient.
      """
      _arg1_i_ = _arg1_i
      _arg2_j_ = _arg2_j
      res,resargs = self.__obj.getqobjij(_arg1_i_,_arg2_j_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg3_qoij_return_value = resargs
      return __arg3_qoij_return_value
    
    def getsolution(self,_arg1_whichsol_,_arg4_skc,_arg5_skx,_arg6_skn,_arg7_xc,_arg8_xx,_arg9_y,_arg10_slc,_arg11_suc,_arg12_slx,_arg13_sux,_arg14_snx): # 2
      """
      Obtains the complete solution.
    
      getsolution(self,_arg1_whichsol_,_arg4_skc,_arg5_skx,_arg6_skn,_arg7_xc,_arg8_xx,_arg9_y,_arg10_slc,_arg11_suc,_arg12_slx,_arg13_sux,_arg14_snx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg4_skc: array of mosek.stakey. <no description>
        _arg5_skx: array of mosek.stakey. <no description>
        _arg6_skn: array of mosek.stakey. <no description>
        _arg7_xc: array of double. <no description>
        _arg8_xx: array of double. <no description>
        _arg9_y: array of double. <no description>
        _arg10_slc: array of double. <no description>
        _arg11_suc: array of double. <no description>
        _arg12_slx: array of double. <no description>
        _arg13_sux: array of double. <no description>
        _arg14_snx: array of double. <no description>
      returns: _arg2_prosta,_arg3_solsta
        _arg2_prosta: mosek.prosta. <no description>
        _arg3_solsta: mosek.solsta. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _copyback__arg4_skc = False
      if _arg4_skc is None:
        _arg4_skc_ = None
      else:
        try:
          _arg4_skc_ = memoryview(_arg4_skc)
        except TypeError:
          try:
            _tmparr__arg4_skc = (ctypes.c_int*len(_arg4_skc))()
            _tmparr__arg4_skc[:] = _arg4_skc
          except TypeError:
            raise TypeError("Argument _arg4_skc has wrong type")
          else:
            _arg4_skc_ = memoryview(_tmparr__arg4_skc)
            _copyback__arg4_skc = True
        else:
          if _arg4_skc_.format != "i":
            _tmparr__arg4_skc = (ctypes.c_int*len(_arg4_skc))()
            _tmparr__arg4_skc[:] = _arg4_skc
            _arg4_skc_ = memoryview(_tmparr__arg4_skc)
            _copyback__arg4_skc = True
      if _arg4_skc_ is not None and len(_arg4_skc_) != self.getnumcon():
        raise ValueError("Array argument _arg4_skc has wrong length")
      _copyback__arg5_skx = False
      if _arg5_skx is None:
        _arg5_skx_ = None
      else:
        try:
          _arg5_skx_ = memoryview(_arg5_skx)
        except TypeError:
          try:
            _tmparr__arg5_skx = (ctypes.c_int*len(_arg5_skx))()
            _tmparr__arg5_skx[:] = _arg5_skx
          except TypeError:
            raise TypeError("Argument _arg5_skx has wrong type")
          else:
            _arg5_skx_ = memoryview(_tmparr__arg5_skx)
            _copyback__arg5_skx = True
        else:
          if _arg5_skx_.format != "i":
            _tmparr__arg5_skx = (ctypes.c_int*len(_arg5_skx))()
            _tmparr__arg5_skx[:] = _arg5_skx
            _arg5_skx_ = memoryview(_tmparr__arg5_skx)
            _copyback__arg5_skx = True
      if _arg5_skx_ is not None and len(_arg5_skx_) != self.getnumvar():
        raise ValueError("Array argument _arg5_skx has wrong length")
      _copyback__arg6_skn = False
      if _arg6_skn is None:
        _arg6_skn_ = None
      else:
        try:
          _arg6_skn_ = memoryview(_arg6_skn)
        except TypeError:
          try:
            _tmparr__arg6_skn = (ctypes.c_int*len(_arg6_skn))()
            _tmparr__arg6_skn[:] = _arg6_skn
          except TypeError:
            raise TypeError("Argument _arg6_skn has wrong type")
          else:
            _arg6_skn_ = memoryview(_tmparr__arg6_skn)
            _copyback__arg6_skn = True
        else:
          if _arg6_skn_.format != "i":
            _tmparr__arg6_skn = (ctypes.c_int*len(_arg6_skn))()
            _tmparr__arg6_skn[:] = _arg6_skn
            _arg6_skn_ = memoryview(_tmparr__arg6_skn)
            _copyback__arg6_skn = True
      if _arg6_skn_ is not None and len(_arg6_skn_) != self.getnumcone():
        raise ValueError("Array argument _arg6_skn has wrong length")
      _copyback__arg7_xc = False
      if _arg7_xc is None:
        _arg7_xc_ = None
      else:
        try:
          _arg7_xc_ = memoryview(_arg7_xc)
        except TypeError:
          try:
            _tmparr__arg7_xc = (ctypes.c_double*len(_arg7_xc))()
            _tmparr__arg7_xc[:] = _arg7_xc
          except TypeError:
            raise TypeError("Argument _arg7_xc has wrong type")
          else:
            _arg7_xc_ = memoryview(_tmparr__arg7_xc)
            _copyback__arg7_xc = True
        else:
          if _arg7_xc_.format != "d":
            _tmparr__arg7_xc = (ctypes.c_double*len(_arg7_xc))()
            _tmparr__arg7_xc[:] = _arg7_xc
            _arg7_xc_ = memoryview(_tmparr__arg7_xc)
            _copyback__arg7_xc = True
      if _arg7_xc_ is not None and len(_arg7_xc_) != self.getnumcon():
        raise ValueError("Array argument _arg7_xc has wrong length")
      _copyback__arg8_xx = False
      if _arg8_xx is None:
        _arg8_xx_ = None
      else:
        try:
          _arg8_xx_ = memoryview(_arg8_xx)
        except TypeError:
          try:
            _tmparr__arg8_xx = (ctypes.c_double*len(_arg8_xx))()
            _tmparr__arg8_xx[:] = _arg8_xx
          except TypeError:
            raise TypeError("Argument _arg8_xx has wrong type")
          else:
            _arg8_xx_ = memoryview(_tmparr__arg8_xx)
            _copyback__arg8_xx = True
        else:
          if _arg8_xx_.format != "d":
            _tmparr__arg8_xx = (ctypes.c_double*len(_arg8_xx))()
            _tmparr__arg8_xx[:] = _arg8_xx
            _arg8_xx_ = memoryview(_tmparr__arg8_xx)
            _copyback__arg8_xx = True
      if _arg8_xx_ is not None and len(_arg8_xx_) != self.getnumvar():
        raise ValueError("Array argument _arg8_xx has wrong length")
      _copyback__arg9_y = False
      if _arg9_y is None:
        _arg9_y_ = None
      else:
        try:
          _arg9_y_ = memoryview(_arg9_y)
        except TypeError:
          try:
            _tmparr__arg9_y = (ctypes.c_double*len(_arg9_y))()
            _tmparr__arg9_y[:] = _arg9_y
          except TypeError:
            raise TypeError("Argument _arg9_y has wrong type")
          else:
            _arg9_y_ = memoryview(_tmparr__arg9_y)
            _copyback__arg9_y = True
        else:
          if _arg9_y_.format != "d":
            _tmparr__arg9_y = (ctypes.c_double*len(_arg9_y))()
            _tmparr__arg9_y[:] = _arg9_y
            _arg9_y_ = memoryview(_tmparr__arg9_y)
            _copyback__arg9_y = True
      if _arg9_y_ is not None and len(_arg9_y_) != self.getnumcon():
        raise ValueError("Array argument _arg9_y has wrong length")
      _copyback__arg10_slc = False
      if _arg10_slc is None:
        _arg10_slc_ = None
      else:
        try:
          _arg10_slc_ = memoryview(_arg10_slc)
        except TypeError:
          try:
            _tmparr__arg10_slc = (ctypes.c_double*len(_arg10_slc))()
            _tmparr__arg10_slc[:] = _arg10_slc
          except TypeError:
            raise TypeError("Argument _arg10_slc has wrong type")
          else:
            _arg10_slc_ = memoryview(_tmparr__arg10_slc)
            _copyback__arg10_slc = True
        else:
          if _arg10_slc_.format != "d":
            _tmparr__arg10_slc = (ctypes.c_double*len(_arg10_slc))()
            _tmparr__arg10_slc[:] = _arg10_slc
            _arg10_slc_ = memoryview(_tmparr__arg10_slc)
            _copyback__arg10_slc = True
      if _arg10_slc_ is not None and len(_arg10_slc_) != self.getnumcon():
        raise ValueError("Array argument _arg10_slc has wrong length")
      _copyback__arg11_suc = False
      if _arg11_suc is None:
        _arg11_suc_ = None
      else:
        try:
          _arg11_suc_ = memoryview(_arg11_suc)
        except TypeError:
          try:
            _tmparr__arg11_suc = (ctypes.c_double*len(_arg11_suc))()
            _tmparr__arg11_suc[:] = _arg11_suc
          except TypeError:
            raise TypeError("Argument _arg11_suc has wrong type")
          else:
            _arg11_suc_ = memoryview(_tmparr__arg11_suc)
            _copyback__arg11_suc = True
        else:
          if _arg11_suc_.format != "d":
            _tmparr__arg11_suc = (ctypes.c_double*len(_arg11_suc))()
            _tmparr__arg11_suc[:] = _arg11_suc
            _arg11_suc_ = memoryview(_tmparr__arg11_suc)
            _copyback__arg11_suc = True
      if _arg11_suc_ is not None and len(_arg11_suc_) != self.getnumcon():
        raise ValueError("Array argument _arg11_suc has wrong length")
      _copyback__arg12_slx = False
      if _arg12_slx is None:
        _arg12_slx_ = None
      else:
        try:
          _arg12_slx_ = memoryview(_arg12_slx)
        except TypeError:
          try:
            _tmparr__arg12_slx = (ctypes.c_double*len(_arg12_slx))()
            _tmparr__arg12_slx[:] = _arg12_slx
          except TypeError:
            raise TypeError("Argument _arg12_slx has wrong type")
          else:
            _arg12_slx_ = memoryview(_tmparr__arg12_slx)
            _copyback__arg12_slx = True
        else:
          if _arg12_slx_.format != "d":
            _tmparr__arg12_slx = (ctypes.c_double*len(_arg12_slx))()
            _tmparr__arg12_slx[:] = _arg12_slx
            _arg12_slx_ = memoryview(_tmparr__arg12_slx)
            _copyback__arg12_slx = True
      if _arg12_slx_ is not None and len(_arg12_slx_) != self.getnumvar():
        raise ValueError("Array argument _arg12_slx has wrong length")
      _copyback__arg13_sux = False
      if _arg13_sux is None:
        _arg13_sux_ = None
      else:
        try:
          _arg13_sux_ = memoryview(_arg13_sux)
        except TypeError:
          try:
            _tmparr__arg13_sux = (ctypes.c_double*len(_arg13_sux))()
            _tmparr__arg13_sux[:] = _arg13_sux
          except TypeError:
            raise TypeError("Argument _arg13_sux has wrong type")
          else:
            _arg13_sux_ = memoryview(_tmparr__arg13_sux)
            _copyback__arg13_sux = True
        else:
          if _arg13_sux_.format != "d":
            _tmparr__arg13_sux = (ctypes.c_double*len(_arg13_sux))()
            _tmparr__arg13_sux[:] = _arg13_sux
            _arg13_sux_ = memoryview(_tmparr__arg13_sux)
            _copyback__arg13_sux = True
      if _arg13_sux_ is not None and len(_arg13_sux_) != self.getnumvar():
        raise ValueError("Array argument _arg13_sux has wrong length")
      _copyback__arg14_snx = False
      if _arg14_snx is None:
        _arg14_snx_ = None
      else:
        try:
          _arg14_snx_ = memoryview(_arg14_snx)
        except TypeError:
          try:
            _tmparr__arg14_snx = (ctypes.c_double*len(_arg14_snx))()
            _tmparr__arg14_snx[:] = _arg14_snx
          except TypeError:
            raise TypeError("Argument _arg14_snx has wrong type")
          else:
            _arg14_snx_ = memoryview(_tmparr__arg14_snx)
            _copyback__arg14_snx = True
        else:
          if _arg14_snx_.format != "d":
            _tmparr__arg14_snx = (ctypes.c_double*len(_arg14_snx))()
            _tmparr__arg14_snx[:] = _arg14_snx
            _arg14_snx_ = memoryview(_tmparr__arg14_snx)
            _copyback__arg14_snx = True
      if _arg14_snx_ is not None and len(_arg14_snx_) != self.getnumvar():
        raise ValueError("Array argument _arg14_snx has wrong length")
      res,resargs = self.__obj.getsolution(_arg1_whichsol_,_arg4_skc_,_arg5_skx_,_arg6_skn_,_arg7_xc_,_arg8_xx_,_arg9_y_,_arg10_slc_,_arg11_suc_,_arg12_slx_,_arg13_sux_,_arg14_snx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_prosta_return_value,__arg3_solsta_return_value = resargs
      if _copyback__arg14_snx:
        _arg14_snx[:] = _tmparr__arg14_snx
      if _copyback__arg13_sux:
        _arg13_sux[:] = _tmparr__arg13_sux
      if _copyback__arg12_slx:
        _arg12_slx[:] = _tmparr__arg12_slx
      if _copyback__arg11_suc:
        _arg11_suc[:] = _tmparr__arg11_suc
      if _copyback__arg10_slc:
        _arg10_slc[:] = _tmparr__arg10_slc
      if _copyback__arg9_y:
        _arg9_y[:] = _tmparr__arg9_y
      if _copyback__arg8_xx:
        _arg8_xx[:] = _tmparr__arg8_xx
      if _copyback__arg7_xc:
        _arg7_xc[:] = _tmparr__arg7_xc
      if _copyback__arg6_skn:
        for __tmp_var_2 in range(len(_arg6_skn_)): _arg6_skn[__tmp_var_2] = stakey(_tmparr__arg6_skn[__tmp_var_2])
      if _copyback__arg5_skx:
        for __tmp_var_1 in range(len(_arg5_skx_)): _arg5_skx[__tmp_var_1] = stakey(_tmparr__arg5_skx[__tmp_var_1])
      if _copyback__arg4_skc:
        for __tmp_var_0 in range(len(_arg4_skc_)): _arg4_skc[__tmp_var_0] = stakey(_tmparr__arg4_skc[__tmp_var_0])
      __arg3_solsta_return_value = solsta(__arg3_solsta_return_value)
      __arg2_prosta_return_value = prosta(__arg2_prosta_return_value)
      return __arg2_prosta_return_value,__arg3_solsta_return_value
    
    def getsolsta(self,_arg1_whichsol_): # 2
      """
      Obtains the solution status.
    
      getsolsta(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. <no description>
      returns: _arg2_solsta
        _arg2_solsta: mosek.solsta. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res,resargs = self.__obj.getsolsta(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_solsta_return_value = resargs
      __arg2_solsta_return_value = solsta(__arg2_solsta_return_value)
      return __arg2_solsta_return_value
    
    def getprosta(self,_arg1_whichsol_): # 2
      """
      Obtains the problem status.
    
      getprosta(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. <no description>
      returns: _arg2_prosta
        _arg2_prosta: mosek.prosta. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res,resargs = self.__obj.getprosta(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_prosta_return_value = resargs
      __arg2_prosta_return_value = prosta(__arg2_prosta_return_value)
      return __arg2_prosta_return_value
    
    def getskc(self,_arg1_whichsol_,_arg2_skc): # 2
      """
      Obtains the status keys for the constraints.
    
      getskc(self,_arg1_whichsol_,_arg2_skc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_skc: array of mosek.stakey. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _copyback__arg2_skc = False
      if _arg2_skc is None:
        _arg2_skc_ = None
      else:
        try:
          _arg2_skc_ = memoryview(_arg2_skc)
        except TypeError:
          try:
            _tmparr__arg2_skc = (ctypes.c_int*len(_arg2_skc))()
            _tmparr__arg2_skc[:] = _arg2_skc
          except TypeError:
            raise TypeError("Argument _arg2_skc has wrong type")
          else:
            _arg2_skc_ = memoryview(_tmparr__arg2_skc)
            _copyback__arg2_skc = True
        else:
          if _arg2_skc_.format != "i":
            _tmparr__arg2_skc = (ctypes.c_int*len(_arg2_skc))()
            _tmparr__arg2_skc[:] = _arg2_skc
            _arg2_skc_ = memoryview(_tmparr__arg2_skc)
            _copyback__arg2_skc = True
      if _arg2_skc_ is not None and len(_arg2_skc_) != self.getnumcon():
        raise ValueError("Array argument _arg2_skc has wrong length")
      res = self.__obj.getskc(_arg1_whichsol_,_arg2_skc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_skc:
        for __tmp_var_0 in range(len(_arg2_skc_)): _arg2_skc[__tmp_var_0] = stakey(_tmparr__arg2_skc[__tmp_var_0])
    
    def getskx(self,_arg1_whichsol_,_arg2_skx): # 2
      """
      Obtains the status keys for the scalar variables.
    
      getskx(self,_arg1_whichsol_,_arg2_skx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_skx: array of mosek.stakey. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _copyback__arg2_skx = False
      if _arg2_skx is None:
        _arg2_skx_ = None
      else:
        try:
          _arg2_skx_ = memoryview(_arg2_skx)
        except TypeError:
          try:
            _tmparr__arg2_skx = (ctypes.c_int*len(_arg2_skx))()
            _tmparr__arg2_skx[:] = _arg2_skx
          except TypeError:
            raise TypeError("Argument _arg2_skx has wrong type")
          else:
            _arg2_skx_ = memoryview(_tmparr__arg2_skx)
            _copyback__arg2_skx = True
        else:
          if _arg2_skx_.format != "i":
            _tmparr__arg2_skx = (ctypes.c_int*len(_arg2_skx))()
            _tmparr__arg2_skx[:] = _arg2_skx
            _arg2_skx_ = memoryview(_tmparr__arg2_skx)
            _copyback__arg2_skx = True
      if _arg2_skx_ is not None and len(_arg2_skx_) != self.getnumvar():
        raise ValueError("Array argument _arg2_skx has wrong length")
      res = self.__obj.getskx(_arg1_whichsol_,_arg2_skx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_skx:
        for __tmp_var_0 in range(len(_arg2_skx_)): _arg2_skx[__tmp_var_0] = stakey(_tmparr__arg2_skx[__tmp_var_0])
    
    def getskn(self,_arg1_whichsol_,_arg2_skn): # 2
      """
      Obtains the status keys for the conic constraints.
    
      getskn(self,_arg1_whichsol_,_arg2_skn)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_skn: array of mosek.stakey. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _copyback__arg2_skn = False
      if _arg2_skn is None:
        _arg2_skn_ = None
      else:
        try:
          _arg2_skn_ = memoryview(_arg2_skn)
        except TypeError:
          try:
            _tmparr__arg2_skn = (ctypes.c_int*len(_arg2_skn))()
            _tmparr__arg2_skn[:] = _arg2_skn
          except TypeError:
            raise TypeError("Argument _arg2_skn has wrong type")
          else:
            _arg2_skn_ = memoryview(_tmparr__arg2_skn)
            _copyback__arg2_skn = True
        else:
          if _arg2_skn_.format != "i":
            _tmparr__arg2_skn = (ctypes.c_int*len(_arg2_skn))()
            _tmparr__arg2_skn[:] = _arg2_skn
            _arg2_skn_ = memoryview(_tmparr__arg2_skn)
            _copyback__arg2_skn = True
      if _arg2_skn_ is not None and len(_arg2_skn_) != self.getnumcone():
        raise ValueError("Array argument _arg2_skn has wrong length")
      res = self.__obj.getskn(_arg1_whichsol_,_arg2_skn_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_skn:
        for __tmp_var_0 in range(len(_arg2_skn_)): _arg2_skn[__tmp_var_0] = stakey(_tmparr__arg2_skn[__tmp_var_0])
    
    def getxc(self,_arg1_whichsol_,_arg2_xc): # 2
      """
      Obtains the xc vector for a solution.
    
      getxc(self,_arg1_whichsol_,_arg2_xc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_xc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_xc is None: raise TypeError("Invalid type for argument _arg2_xc")
      _copyback__arg2_xc = False
      if _arg2_xc is None:
        _arg2_xc_ = None
      else:
        try:
          _arg2_xc_ = memoryview(_arg2_xc)
        except TypeError:
          try:
            _tmparr__arg2_xc = (ctypes.c_double*len(_arg2_xc))()
            _tmparr__arg2_xc[:] = _arg2_xc
          except TypeError:
            raise TypeError("Argument _arg2_xc has wrong type")
          else:
            _arg2_xc_ = memoryview(_tmparr__arg2_xc)
            _copyback__arg2_xc = True
        else:
          if _arg2_xc_.format != "d":
            _tmparr__arg2_xc = (ctypes.c_double*len(_arg2_xc))()
            _tmparr__arg2_xc[:] = _arg2_xc
            _arg2_xc_ = memoryview(_tmparr__arg2_xc)
            _copyback__arg2_xc = True
      if _arg2_xc_ is not None and len(_arg2_xc_) != self.getnumcon():
        raise ValueError("Array argument _arg2_xc has wrong length")
      res = self.__obj.getxc(_arg1_whichsol_,_arg2_xc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_xc:
        _arg2_xc[:] = _tmparr__arg2_xc
    
    def getxx(self,_arg1_whichsol_,_arg2_xx): # 2
      """
      Obtains the xx vector for a solution.
    
      getxx(self,_arg1_whichsol_,_arg2_xx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_xx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_xx is None: raise TypeError("Invalid type for argument _arg2_xx")
      _copyback__arg2_xx = False
      if _arg2_xx is None:
        _arg2_xx_ = None
      else:
        try:
          _arg2_xx_ = memoryview(_arg2_xx)
        except TypeError:
          try:
            _tmparr__arg2_xx = (ctypes.c_double*len(_arg2_xx))()
            _tmparr__arg2_xx[:] = _arg2_xx
          except TypeError:
            raise TypeError("Argument _arg2_xx has wrong type")
          else:
            _arg2_xx_ = memoryview(_tmparr__arg2_xx)
            _copyback__arg2_xx = True
        else:
          if _arg2_xx_.format != "d":
            _tmparr__arg2_xx = (ctypes.c_double*len(_arg2_xx))()
            _tmparr__arg2_xx[:] = _arg2_xx
            _arg2_xx_ = memoryview(_tmparr__arg2_xx)
            _copyback__arg2_xx = True
      if _arg2_xx_ is not None and len(_arg2_xx_) != self.getnumvar():
        raise ValueError("Array argument _arg2_xx has wrong length")
      res = self.__obj.getxx(_arg1_whichsol_,_arg2_xx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_xx:
        _arg2_xx[:] = _tmparr__arg2_xx
    
    def gety(self,_arg1_whichsol_,_arg2_y): # 2
      """
      Obtains the y vector for a solution.
    
      gety(self,_arg1_whichsol_,_arg2_y)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_y: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_y is None: raise TypeError("Invalid type for argument _arg2_y")
      _copyback__arg2_y = False
      if _arg2_y is None:
        _arg2_y_ = None
      else:
        try:
          _arg2_y_ = memoryview(_arg2_y)
        except TypeError:
          try:
            _tmparr__arg2_y = (ctypes.c_double*len(_arg2_y))()
            _tmparr__arg2_y[:] = _arg2_y
          except TypeError:
            raise TypeError("Argument _arg2_y has wrong type")
          else:
            _arg2_y_ = memoryview(_tmparr__arg2_y)
            _copyback__arg2_y = True
        else:
          if _arg2_y_.format != "d":
            _tmparr__arg2_y = (ctypes.c_double*len(_arg2_y))()
            _tmparr__arg2_y[:] = _arg2_y
            _arg2_y_ = memoryview(_tmparr__arg2_y)
            _copyback__arg2_y = True
      if _arg2_y_ is not None and len(_arg2_y_) != self.getnumcon():
        raise ValueError("Array argument _arg2_y has wrong length")
      res = self.__obj.gety(_arg1_whichsol_,_arg2_y_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_y:
        _arg2_y[:] = _tmparr__arg2_y
    
    def getslc(self,_arg1_whichsol_,_arg2_slc): # 2
      """
      Obtains the slc vector for a solution.
    
      getslc(self,_arg1_whichsol_,_arg2_slc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_slc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_slc is None: raise TypeError("Invalid type for argument _arg2_slc")
      _copyback__arg2_slc = False
      if _arg2_slc is None:
        _arg2_slc_ = None
      else:
        try:
          _arg2_slc_ = memoryview(_arg2_slc)
        except TypeError:
          try:
            _tmparr__arg2_slc = (ctypes.c_double*len(_arg2_slc))()
            _tmparr__arg2_slc[:] = _arg2_slc
          except TypeError:
            raise TypeError("Argument _arg2_slc has wrong type")
          else:
            _arg2_slc_ = memoryview(_tmparr__arg2_slc)
            _copyback__arg2_slc = True
        else:
          if _arg2_slc_.format != "d":
            _tmparr__arg2_slc = (ctypes.c_double*len(_arg2_slc))()
            _tmparr__arg2_slc[:] = _arg2_slc
            _arg2_slc_ = memoryview(_tmparr__arg2_slc)
            _copyback__arg2_slc = True
      if _arg2_slc_ is not None and len(_arg2_slc_) != self.getnumcon():
        raise ValueError("Array argument _arg2_slc has wrong length")
      res = self.__obj.getslc(_arg1_whichsol_,_arg2_slc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_slc:
        _arg2_slc[:] = _tmparr__arg2_slc
    
    def getsuc(self,_arg1_whichsol_,_arg2_suc): # 2
      """
      Obtains the suc vector for a solution.
    
      getsuc(self,_arg1_whichsol_,_arg2_suc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_suc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_suc is None: raise TypeError("Invalid type for argument _arg2_suc")
      _copyback__arg2_suc = False
      if _arg2_suc is None:
        _arg2_suc_ = None
      else:
        try:
          _arg2_suc_ = memoryview(_arg2_suc)
        except TypeError:
          try:
            _tmparr__arg2_suc = (ctypes.c_double*len(_arg2_suc))()
            _tmparr__arg2_suc[:] = _arg2_suc
          except TypeError:
            raise TypeError("Argument _arg2_suc has wrong type")
          else:
            _arg2_suc_ = memoryview(_tmparr__arg2_suc)
            _copyback__arg2_suc = True
        else:
          if _arg2_suc_.format != "d":
            _tmparr__arg2_suc = (ctypes.c_double*len(_arg2_suc))()
            _tmparr__arg2_suc[:] = _arg2_suc
            _arg2_suc_ = memoryview(_tmparr__arg2_suc)
            _copyback__arg2_suc = True
      if _arg2_suc_ is not None and len(_arg2_suc_) != self.getnumcon():
        raise ValueError("Array argument _arg2_suc has wrong length")
      res = self.__obj.getsuc(_arg1_whichsol_,_arg2_suc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_suc:
        _arg2_suc[:] = _tmparr__arg2_suc
    
    def getslx(self,_arg1_whichsol_,_arg2_slx): # 2
      """
      Obtains the slx vector for a solution.
    
      getslx(self,_arg1_whichsol_,_arg2_slx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_slx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_slx is None: raise TypeError("Invalid type for argument _arg2_slx")
      _copyback__arg2_slx = False
      if _arg2_slx is None:
        _arg2_slx_ = None
      else:
        try:
          _arg2_slx_ = memoryview(_arg2_slx)
        except TypeError:
          try:
            _tmparr__arg2_slx = (ctypes.c_double*len(_arg2_slx))()
            _tmparr__arg2_slx[:] = _arg2_slx
          except TypeError:
            raise TypeError("Argument _arg2_slx has wrong type")
          else:
            _arg2_slx_ = memoryview(_tmparr__arg2_slx)
            _copyback__arg2_slx = True
        else:
          if _arg2_slx_.format != "d":
            _tmparr__arg2_slx = (ctypes.c_double*len(_arg2_slx))()
            _tmparr__arg2_slx[:] = _arg2_slx
            _arg2_slx_ = memoryview(_tmparr__arg2_slx)
            _copyback__arg2_slx = True
      if _arg2_slx_ is not None and len(_arg2_slx_) != self.getnumvar():
        raise ValueError("Array argument _arg2_slx has wrong length")
      res = self.__obj.getslx(_arg1_whichsol_,_arg2_slx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_slx:
        _arg2_slx[:] = _tmparr__arg2_slx
    
    def getsux(self,_arg1_whichsol_,_arg2_sux): # 2
      """
      Obtains the sux vector for a solution.
    
      getsux(self,_arg1_whichsol_,_arg2_sux)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_sux: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_sux is None: raise TypeError("Invalid type for argument _arg2_sux")
      _copyback__arg2_sux = False
      if _arg2_sux is None:
        _arg2_sux_ = None
      else:
        try:
          _arg2_sux_ = memoryview(_arg2_sux)
        except TypeError:
          try:
            _tmparr__arg2_sux = (ctypes.c_double*len(_arg2_sux))()
            _tmparr__arg2_sux[:] = _arg2_sux
          except TypeError:
            raise TypeError("Argument _arg2_sux has wrong type")
          else:
            _arg2_sux_ = memoryview(_tmparr__arg2_sux)
            _copyback__arg2_sux = True
        else:
          if _arg2_sux_.format != "d":
            _tmparr__arg2_sux = (ctypes.c_double*len(_arg2_sux))()
            _tmparr__arg2_sux[:] = _arg2_sux
            _arg2_sux_ = memoryview(_tmparr__arg2_sux)
            _copyback__arg2_sux = True
      if _arg2_sux_ is not None and len(_arg2_sux_) != self.getnumvar():
        raise ValueError("Array argument _arg2_sux has wrong length")
      res = self.__obj.getsux(_arg1_whichsol_,_arg2_sux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_sux:
        _arg2_sux[:] = _tmparr__arg2_sux
    
    def getsnx(self,_arg1_whichsol_,_arg2_snx): # 2
      """
      Obtains the snx vector for a solution.
    
      getsnx(self,_arg1_whichsol_,_arg2_snx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_snx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_snx is None: raise TypeError("Invalid type for argument _arg2_snx")
      _copyback__arg2_snx = False
      if _arg2_snx is None:
        _arg2_snx_ = None
      else:
        try:
          _arg2_snx_ = memoryview(_arg2_snx)
        except TypeError:
          try:
            _tmparr__arg2_snx = (ctypes.c_double*len(_arg2_snx))()
            _tmparr__arg2_snx[:] = _arg2_snx
          except TypeError:
            raise TypeError("Argument _arg2_snx has wrong type")
          else:
            _arg2_snx_ = memoryview(_tmparr__arg2_snx)
            _copyback__arg2_snx = True
        else:
          if _arg2_snx_.format != "d":
            _tmparr__arg2_snx = (ctypes.c_double*len(_arg2_snx))()
            _tmparr__arg2_snx[:] = _arg2_snx
            _arg2_snx_ = memoryview(_tmparr__arg2_snx)
            _copyback__arg2_snx = True
      if _arg2_snx_ is not None and len(_arg2_snx_) != self.getnumvar():
        raise ValueError("Array argument _arg2_snx has wrong length")
      res = self.__obj.getsnx(_arg1_whichsol_,_arg2_snx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_snx:
        _arg2_snx[:] = _tmparr__arg2_snx
    
    def getskcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_skc): # 2
      """
      Obtains the status keys for a slice of the constraints.
    
      getskcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_skc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_skc: array of mosek.stakey. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_skc = False
      if _arg4_skc is None:
        _arg4_skc_ = None
      else:
        try:
          _arg4_skc_ = memoryview(_arg4_skc)
        except TypeError:
          try:
            _tmparr__arg4_skc = (ctypes.c_int*len(_arg4_skc))()
            _tmparr__arg4_skc[:] = _arg4_skc
          except TypeError:
            raise TypeError("Argument _arg4_skc has wrong type")
          else:
            _arg4_skc_ = memoryview(_tmparr__arg4_skc)
            _copyback__arg4_skc = True
        else:
          if _arg4_skc_.format != "i":
            _tmparr__arg4_skc = (ctypes.c_int*len(_arg4_skc))()
            _tmparr__arg4_skc[:] = _arg4_skc
            _arg4_skc_ = memoryview(_tmparr__arg4_skc)
            _copyback__arg4_skc = True
      if _arg4_skc_ is not None and len(_arg4_skc_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_skc has wrong length")
      res = self.__obj.getskcslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_skc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_skc:
        for __tmp_var_0 in range(len(_arg4_skc_)): _arg4_skc[__tmp_var_0] = stakey(_tmparr__arg4_skc[__tmp_var_0])
    
    def getskxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_skx): # 2
      """
      Obtains the status keys for a slice of the scalar variables.
    
      getskxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_skx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_skx: array of mosek.stakey. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_skx = False
      if _arg4_skx is None:
        _arg4_skx_ = None
      else:
        try:
          _arg4_skx_ = memoryview(_arg4_skx)
        except TypeError:
          try:
            _tmparr__arg4_skx = (ctypes.c_int*len(_arg4_skx))()
            _tmparr__arg4_skx[:] = _arg4_skx
          except TypeError:
            raise TypeError("Argument _arg4_skx has wrong type")
          else:
            _arg4_skx_ = memoryview(_tmparr__arg4_skx)
            _copyback__arg4_skx = True
        else:
          if _arg4_skx_.format != "i":
            _tmparr__arg4_skx = (ctypes.c_int*len(_arg4_skx))()
            _tmparr__arg4_skx[:] = _arg4_skx
            _arg4_skx_ = memoryview(_tmparr__arg4_skx)
            _copyback__arg4_skx = True
      if _arg4_skx_ is not None and len(_arg4_skx_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_skx has wrong length")
      res = self.__obj.getskxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_skx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_skx:
        for __tmp_var_0 in range(len(_arg4_skx_)): _arg4_skx[__tmp_var_0] = stakey(_tmparr__arg4_skx[__tmp_var_0])
    
    def getxcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_xc): # 2
      """
      Obtains a slice of the xc vector for a solution.
    
      getxcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_xc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_xc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_xc = False
      if _arg4_xc is None:
        _arg4_xc_ = None
      else:
        try:
          _arg4_xc_ = memoryview(_arg4_xc)
        except TypeError:
          try:
            _tmparr__arg4_xc = (ctypes.c_double*len(_arg4_xc))()
            _tmparr__arg4_xc[:] = _arg4_xc
          except TypeError:
            raise TypeError("Argument _arg4_xc has wrong type")
          else:
            _arg4_xc_ = memoryview(_tmparr__arg4_xc)
            _copyback__arg4_xc = True
        else:
          if _arg4_xc_.format != "d":
            _tmparr__arg4_xc = (ctypes.c_double*len(_arg4_xc))()
            _tmparr__arg4_xc[:] = _arg4_xc
            _arg4_xc_ = memoryview(_tmparr__arg4_xc)
            _copyback__arg4_xc = True
      if _arg4_xc_ is not None and len(_arg4_xc_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_xc has wrong length")
      res = self.__obj.getxcslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_xc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_xc:
        _arg4_xc[:] = _tmparr__arg4_xc
    
    def getxxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_xx): # 2
      """
      Obtains a slice of the xx vector for a solution.
    
      getxxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_xx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_xx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_xx = False
      if _arg4_xx is None:
        _arg4_xx_ = None
      else:
        try:
          _arg4_xx_ = memoryview(_arg4_xx)
        except TypeError:
          try:
            _tmparr__arg4_xx = (ctypes.c_double*len(_arg4_xx))()
            _tmparr__arg4_xx[:] = _arg4_xx
          except TypeError:
            raise TypeError("Argument _arg4_xx has wrong type")
          else:
            _arg4_xx_ = memoryview(_tmparr__arg4_xx)
            _copyback__arg4_xx = True
        else:
          if _arg4_xx_.format != "d":
            _tmparr__arg4_xx = (ctypes.c_double*len(_arg4_xx))()
            _tmparr__arg4_xx[:] = _arg4_xx
            _arg4_xx_ = memoryview(_tmparr__arg4_xx)
            _copyback__arg4_xx = True
      if _arg4_xx_ is not None and len(_arg4_xx_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_xx has wrong length")
      res = self.__obj.getxxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_xx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_xx:
        _arg4_xx[:] = _tmparr__arg4_xx
    
    def getyslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_y): # 2
      """
      Obtains a slice of the y vector for a solution.
    
      getyslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_y)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_y: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_y = False
      if _arg4_y is None:
        _arg4_y_ = None
      else:
        try:
          _arg4_y_ = memoryview(_arg4_y)
        except TypeError:
          try:
            _tmparr__arg4_y = (ctypes.c_double*len(_arg4_y))()
            _tmparr__arg4_y[:] = _arg4_y
          except TypeError:
            raise TypeError("Argument _arg4_y has wrong type")
          else:
            _arg4_y_ = memoryview(_tmparr__arg4_y)
            _copyback__arg4_y = True
        else:
          if _arg4_y_.format != "d":
            _tmparr__arg4_y = (ctypes.c_double*len(_arg4_y))()
            _tmparr__arg4_y[:] = _arg4_y
            _arg4_y_ = memoryview(_tmparr__arg4_y)
            _copyback__arg4_y = True
      if _arg4_y_ is not None and len(_arg4_y_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_y has wrong length")
      res = self.__obj.getyslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_y_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_y:
        _arg4_y[:] = _tmparr__arg4_y
    
    def getslcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slc): # 2
      """
      Obtains a slice of the slc vector for a solution.
    
      getslcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_slc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_slc = False
      if _arg4_slc is None:
        _arg4_slc_ = None
      else:
        try:
          _arg4_slc_ = memoryview(_arg4_slc)
        except TypeError:
          try:
            _tmparr__arg4_slc = (ctypes.c_double*len(_arg4_slc))()
            _tmparr__arg4_slc[:] = _arg4_slc
          except TypeError:
            raise TypeError("Argument _arg4_slc has wrong type")
          else:
            _arg4_slc_ = memoryview(_tmparr__arg4_slc)
            _copyback__arg4_slc = True
        else:
          if _arg4_slc_.format != "d":
            _tmparr__arg4_slc = (ctypes.c_double*len(_arg4_slc))()
            _tmparr__arg4_slc[:] = _arg4_slc
            _arg4_slc_ = memoryview(_tmparr__arg4_slc)
            _copyback__arg4_slc = True
      if _arg4_slc_ is not None and len(_arg4_slc_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_slc has wrong length")
      res = self.__obj.getslcslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_slc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_slc:
        _arg4_slc[:] = _tmparr__arg4_slc
    
    def getsucslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_suc): # 2
      """
      Obtains a slice of the suc vector for a solution.
    
      getsucslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_suc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_suc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_suc = False
      if _arg4_suc is None:
        _arg4_suc_ = None
      else:
        try:
          _arg4_suc_ = memoryview(_arg4_suc)
        except TypeError:
          try:
            _tmparr__arg4_suc = (ctypes.c_double*len(_arg4_suc))()
            _tmparr__arg4_suc[:] = _arg4_suc
          except TypeError:
            raise TypeError("Argument _arg4_suc has wrong type")
          else:
            _arg4_suc_ = memoryview(_tmparr__arg4_suc)
            _copyback__arg4_suc = True
        else:
          if _arg4_suc_.format != "d":
            _tmparr__arg4_suc = (ctypes.c_double*len(_arg4_suc))()
            _tmparr__arg4_suc[:] = _arg4_suc
            _arg4_suc_ = memoryview(_tmparr__arg4_suc)
            _copyback__arg4_suc = True
      if _arg4_suc_ is not None and len(_arg4_suc_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_suc has wrong length")
      res = self.__obj.getsucslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_suc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_suc:
        _arg4_suc[:] = _tmparr__arg4_suc
    
    def getslxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slx): # 2
      """
      Obtains a slice of the slx vector for a solution.
    
      getslxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_slx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_slx = False
      if _arg4_slx is None:
        _arg4_slx_ = None
      else:
        try:
          _arg4_slx_ = memoryview(_arg4_slx)
        except TypeError:
          try:
            _tmparr__arg4_slx = (ctypes.c_double*len(_arg4_slx))()
            _tmparr__arg4_slx[:] = _arg4_slx
          except TypeError:
            raise TypeError("Argument _arg4_slx has wrong type")
          else:
            _arg4_slx_ = memoryview(_tmparr__arg4_slx)
            _copyback__arg4_slx = True
        else:
          if _arg4_slx_.format != "d":
            _tmparr__arg4_slx = (ctypes.c_double*len(_arg4_slx))()
            _tmparr__arg4_slx[:] = _arg4_slx
            _arg4_slx_ = memoryview(_tmparr__arg4_slx)
            _copyback__arg4_slx = True
      if _arg4_slx_ is not None and len(_arg4_slx_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_slx has wrong length")
      res = self.__obj.getslxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_slx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_slx:
        _arg4_slx[:] = _tmparr__arg4_slx
    
    def getsuxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_sux): # 2
      """
      Obtains a slice of the sux vector for a solution.
    
      getsuxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_sux)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_sux: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_sux = False
      if _arg4_sux is None:
        _arg4_sux_ = None
      else:
        try:
          _arg4_sux_ = memoryview(_arg4_sux)
        except TypeError:
          try:
            _tmparr__arg4_sux = (ctypes.c_double*len(_arg4_sux))()
            _tmparr__arg4_sux[:] = _arg4_sux
          except TypeError:
            raise TypeError("Argument _arg4_sux has wrong type")
          else:
            _arg4_sux_ = memoryview(_tmparr__arg4_sux)
            _copyback__arg4_sux = True
        else:
          if _arg4_sux_.format != "d":
            _tmparr__arg4_sux = (ctypes.c_double*len(_arg4_sux))()
            _tmparr__arg4_sux[:] = _arg4_sux
            _arg4_sux_ = memoryview(_tmparr__arg4_sux)
            _copyback__arg4_sux = True
      if _arg4_sux_ is not None and len(_arg4_sux_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_sux has wrong length")
      res = self.__obj.getsuxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_sux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_sux:
        _arg4_sux[:] = _tmparr__arg4_sux
    
    def getsnxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_snx): # 2
      """
      Obtains a slice of the snx vector for a solution.
    
      getsnxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_snx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_snx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_snx = False
      if _arg4_snx is None:
        _arg4_snx_ = None
      else:
        try:
          _arg4_snx_ = memoryview(_arg4_snx)
        except TypeError:
          try:
            _tmparr__arg4_snx = (ctypes.c_double*len(_arg4_snx))()
            _tmparr__arg4_snx[:] = _arg4_snx
          except TypeError:
            raise TypeError("Argument _arg4_snx has wrong type")
          else:
            _arg4_snx_ = memoryview(_tmparr__arg4_snx)
            _copyback__arg4_snx = True
        else:
          if _arg4_snx_.format != "d":
            _tmparr__arg4_snx = (ctypes.c_double*len(_arg4_snx))()
            _tmparr__arg4_snx[:] = _arg4_snx
            _arg4_snx_ = memoryview(_tmparr__arg4_snx)
            _copyback__arg4_snx = True
      if _arg4_snx_ is not None and len(_arg4_snx_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_snx has wrong length")
      res = self.__obj.getsnxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_snx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_snx:
        _arg4_snx[:] = _tmparr__arg4_snx
    
    def getbarxj(self,_arg1_whichsol_,_arg2_j,_arg3_barxj): # 2
      """
      Obtains the primal solution for a semidefinite variable.
    
      getbarxj(self,_arg1_whichsol_,_arg2_j,_arg3_barxj)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_j: int. Index of the semidefinite variable.
        _arg3_barxj: array of double. Value of the j'th variable of barx.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_j_ = _arg2_j
      if _arg3_barxj is None: raise TypeError("Invalid type for argument _arg3_barxj")
      _copyback__arg3_barxj = False
      if _arg3_barxj is None:
        _arg3_barxj_ = None
      else:
        try:
          _arg3_barxj_ = memoryview(_arg3_barxj)
        except TypeError:
          try:
            _tmparr__arg3_barxj = (ctypes.c_double*len(_arg3_barxj))()
            _tmparr__arg3_barxj[:] = _arg3_barxj
          except TypeError:
            raise TypeError("Argument _arg3_barxj has wrong type")
          else:
            _arg3_barxj_ = memoryview(_tmparr__arg3_barxj)
            _copyback__arg3_barxj = True
        else:
          if _arg3_barxj_.format != "d":
            _tmparr__arg3_barxj = (ctypes.c_double*len(_arg3_barxj))()
            _tmparr__arg3_barxj[:] = _arg3_barxj
            _arg3_barxj_ = memoryview(_tmparr__arg3_barxj)
            _copyback__arg3_barxj = True
      if _arg3_barxj_ is not None and len(_arg3_barxj_) != self.getlenbarvarj( _arg2_j ):
        raise ValueError("Array argument _arg3_barxj has wrong length")
      res = self.__obj.getbarxj(_arg1_whichsol_,_arg2_j_,_arg3_barxj_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg3_barxj:
        _arg3_barxj[:] = _tmparr__arg3_barxj
    
    def getbarxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slicesize,_arg5_barxslice): # 2
      """
      Obtains the primal solution for a sequence of semidefinite variables.
    
      getbarxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slicesize,_arg5_barxslice)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. Index of the first semidefinite variable in the slice.
        _arg3_last: int. Index of the last semidefinite variable in the slice plus one.
        _arg4_slicesize: long. Denotes the length of the array barxslice.
        _arg5_barxslice: array of double. Solution values of symmetric matrix variables in the slice, stored sequentially.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _arg4_slicesize_ = _arg4_slicesize
      if _arg5_barxslice is None: raise TypeError("Invalid type for argument _arg5_barxslice")
      _copyback__arg5_barxslice = False
      if _arg5_barxslice is None:
        _arg5_barxslice_ = None
      else:
        try:
          _arg5_barxslice_ = memoryview(_arg5_barxslice)
        except TypeError:
          try:
            _tmparr__arg5_barxslice = (ctypes.c_double*len(_arg5_barxslice))()
            _tmparr__arg5_barxslice[:] = _arg5_barxslice
          except TypeError:
            raise TypeError("Argument _arg5_barxslice has wrong type")
          else:
            _arg5_barxslice_ = memoryview(_tmparr__arg5_barxslice)
            _copyback__arg5_barxslice = True
        else:
          if _arg5_barxslice_.format != "d":
            _tmparr__arg5_barxslice = (ctypes.c_double*len(_arg5_barxslice))()
            _tmparr__arg5_barxslice[:] = _arg5_barxslice
            _arg5_barxslice_ = memoryview(_tmparr__arg5_barxslice)
            _copyback__arg5_barxslice = True
      if _arg5_barxslice_ is not None and len(_arg5_barxslice_) !=  _arg4_slicesize :
        raise ValueError("Array argument _arg5_barxslice has wrong length")
      res = self.__obj.getbarxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_slicesize_,_arg5_barxslice_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg5_barxslice:
        _arg5_barxslice[:] = _tmparr__arg5_barxslice
    
    def getbarsj(self,_arg1_whichsol_,_arg2_j,_arg3_barsj): # 2
      """
      Obtains the dual solution for a semidefinite variable.
    
      getbarsj(self,_arg1_whichsol_,_arg2_j,_arg3_barsj)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_j: int. Index of the semidefinite variable.
        _arg3_barsj: array of double. Value of the j'th dual variable of barx.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_j_ = _arg2_j
      if _arg3_barsj is None: raise TypeError("Invalid type for argument _arg3_barsj")
      _copyback__arg3_barsj = False
      if _arg3_barsj is None:
        _arg3_barsj_ = None
      else:
        try:
          _arg3_barsj_ = memoryview(_arg3_barsj)
        except TypeError:
          try:
            _tmparr__arg3_barsj = (ctypes.c_double*len(_arg3_barsj))()
            _tmparr__arg3_barsj[:] = _arg3_barsj
          except TypeError:
            raise TypeError("Argument _arg3_barsj has wrong type")
          else:
            _arg3_barsj_ = memoryview(_tmparr__arg3_barsj)
            _copyback__arg3_barsj = True
        else:
          if _arg3_barsj_.format != "d":
            _tmparr__arg3_barsj = (ctypes.c_double*len(_arg3_barsj))()
            _tmparr__arg3_barsj[:] = _arg3_barsj
            _arg3_barsj_ = memoryview(_tmparr__arg3_barsj)
            _copyback__arg3_barsj = True
      if _arg3_barsj_ is not None and len(_arg3_barsj_) != self.getlenbarvarj( _arg2_j ):
        raise ValueError("Array argument _arg3_barsj has wrong length")
      res = self.__obj.getbarsj(_arg1_whichsol_,_arg2_j_,_arg3_barsj_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg3_barsj:
        _arg3_barsj[:] = _tmparr__arg3_barsj
    
    def getbarsslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slicesize,_arg5_barsslice): # 2
      """
      Obtains the dual solution for a sequence of semidefinite variables.
    
      getbarsslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slicesize,_arg5_barsslice)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. Index of the first semidefinite variable in the slice.
        _arg3_last: int. Index of the last semidefinite variable in the slice plus one.
        _arg4_slicesize: long. Denotes the length of the array barsslice.
        _arg5_barsslice: array of double. Dual solution values of symmetric matrix variables in the slice, stored sequentially.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _arg4_slicesize_ = _arg4_slicesize
      if _arg5_barsslice is None: raise TypeError("Invalid type for argument _arg5_barsslice")
      _copyback__arg5_barsslice = False
      if _arg5_barsslice is None:
        _arg5_barsslice_ = None
      else:
        try:
          _arg5_barsslice_ = memoryview(_arg5_barsslice)
        except TypeError:
          try:
            _tmparr__arg5_barsslice = (ctypes.c_double*len(_arg5_barsslice))()
            _tmparr__arg5_barsslice[:] = _arg5_barsslice
          except TypeError:
            raise TypeError("Argument _arg5_barsslice has wrong type")
          else:
            _arg5_barsslice_ = memoryview(_tmparr__arg5_barsslice)
            _copyback__arg5_barsslice = True
        else:
          if _arg5_barsslice_.format != "d":
            _tmparr__arg5_barsslice = (ctypes.c_double*len(_arg5_barsslice))()
            _tmparr__arg5_barsslice[:] = _arg5_barsslice
            _arg5_barsslice_ = memoryview(_tmparr__arg5_barsslice)
            _copyback__arg5_barsslice = True
      if _arg5_barsslice_ is not None and len(_arg5_barsslice_) !=  _arg4_slicesize :
        raise ValueError("Array argument _arg5_barsslice has wrong length")
      res = self.__obj.getbarsslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_slicesize_,_arg5_barsslice_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg5_barsslice:
        _arg5_barsslice[:] = _tmparr__arg5_barsslice
    
    def putskc(self,_arg1_whichsol_,_arg2_skc): # 2
      """
      Sets the status keys for the constraints.
    
      putskc(self,_arg1_whichsol_,_arg2_skc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_skc: array of mosek.stakey. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_skc is None: raise TypeError("Invalid type for argument _arg2_skc")
      if _arg2_skc is None:
        _arg2_skc_ = None
      else:
        try:
          _arg2_skc_ = memoryview(_arg2_skc)
        except TypeError:
          try:
            _tmparr__arg2_skc = (ctypes.c_int*len(_arg2_skc))()
            _tmparr__arg2_skc[:] = _arg2_skc
          except TypeError:
            raise TypeError("Argument _arg2_skc has wrong type")
          else:
            _arg2_skc_ = memoryview(_tmparr__arg2_skc)
      
        else:
          if _arg2_skc_.format != "i":
            _tmparr__arg2_skc = (ctypes.c_int*len(_arg2_skc))()
            _tmparr__arg2_skc[:] = _arg2_skc
            _arg2_skc_ = memoryview(_tmparr__arg2_skc)
      
      if _arg2_skc_ is not None and len(_arg2_skc_) != self.getnumcon():
        raise ValueError("Array argument _arg2_skc has wrong length")
      res = self.__obj.putskc(_arg1_whichsol_,_arg2_skc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putskx(self,_arg1_whichsol_,_arg2_skx): # 2
      """
      Sets the status keys for the scalar variables.
    
      putskx(self,_arg1_whichsol_,_arg2_skx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_skx: array of mosek.stakey. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_skx is None: raise TypeError("Invalid type for argument _arg2_skx")
      if _arg2_skx is None:
        _arg2_skx_ = None
      else:
        try:
          _arg2_skx_ = memoryview(_arg2_skx)
        except TypeError:
          try:
            _tmparr__arg2_skx = (ctypes.c_int*len(_arg2_skx))()
            _tmparr__arg2_skx[:] = _arg2_skx
          except TypeError:
            raise TypeError("Argument _arg2_skx has wrong type")
          else:
            _arg2_skx_ = memoryview(_tmparr__arg2_skx)
      
        else:
          if _arg2_skx_.format != "i":
            _tmparr__arg2_skx = (ctypes.c_int*len(_arg2_skx))()
            _tmparr__arg2_skx[:] = _arg2_skx
            _arg2_skx_ = memoryview(_tmparr__arg2_skx)
      
      if _arg2_skx_ is not None and len(_arg2_skx_) != self.getnumvar():
        raise ValueError("Array argument _arg2_skx has wrong length")
      res = self.__obj.putskx(_arg1_whichsol_,_arg2_skx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putxc(self,_arg1_whichsol_,_arg2_xc): # 2
      """
      Sets the xc vector for a solution.
    
      putxc(self,_arg1_whichsol_,_arg2_xc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_xc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_xc is None: raise TypeError("Invalid type for argument _arg2_xc")
      _copyback__arg2_xc = False
      if _arg2_xc is None:
        _arg2_xc_ = None
      else:
        try:
          _arg2_xc_ = memoryview(_arg2_xc)
        except TypeError:
          try:
            _tmparr__arg2_xc = (ctypes.c_double*len(_arg2_xc))()
            _tmparr__arg2_xc[:] = _arg2_xc
          except TypeError:
            raise TypeError("Argument _arg2_xc has wrong type")
          else:
            _arg2_xc_ = memoryview(_tmparr__arg2_xc)
            _copyback__arg2_xc = True
        else:
          if _arg2_xc_.format != "d":
            _tmparr__arg2_xc = (ctypes.c_double*len(_arg2_xc))()
            _tmparr__arg2_xc[:] = _arg2_xc
            _arg2_xc_ = memoryview(_tmparr__arg2_xc)
            _copyback__arg2_xc = True
      if _arg2_xc_ is not None and len(_arg2_xc_) != self.getnumcon():
        raise ValueError("Array argument _arg2_xc has wrong length")
      res = self.__obj.putxc(_arg1_whichsol_,_arg2_xc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg2_xc:
        _arg2_xc[:] = _tmparr__arg2_xc
    
    def putxx(self,_arg1_whichsol_,_arg2_xx): # 2
      """
      Sets the xx vector for a solution.
    
      putxx(self,_arg1_whichsol_,_arg2_xx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_xx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_xx is None: raise TypeError("Invalid type for argument _arg2_xx")
      if _arg2_xx is None:
        _arg2_xx_ = None
      else:
        try:
          _arg2_xx_ = memoryview(_arg2_xx)
        except TypeError:
          try:
            _tmparr__arg2_xx = (ctypes.c_double*len(_arg2_xx))()
            _tmparr__arg2_xx[:] = _arg2_xx
          except TypeError:
            raise TypeError("Argument _arg2_xx has wrong type")
          else:
            _arg2_xx_ = memoryview(_tmparr__arg2_xx)
      
        else:
          if _arg2_xx_.format != "d":
            _tmparr__arg2_xx = (ctypes.c_double*len(_arg2_xx))()
            _tmparr__arg2_xx[:] = _arg2_xx
            _arg2_xx_ = memoryview(_tmparr__arg2_xx)
      
      if _arg2_xx_ is not None and len(_arg2_xx_) != self.getnumvar():
        raise ValueError("Array argument _arg2_xx has wrong length")
      res = self.__obj.putxx(_arg1_whichsol_,_arg2_xx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def puty(self,_arg1_whichsol_,_arg2_y): # 2
      """
      Sets the y vector for a solution.
    
      puty(self,_arg1_whichsol_,_arg2_y)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_y: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_y is None: raise TypeError("Invalid type for argument _arg2_y")
      if _arg2_y is None:
        _arg2_y_ = None
      else:
        try:
          _arg2_y_ = memoryview(_arg2_y)
        except TypeError:
          try:
            _tmparr__arg2_y = (ctypes.c_double*len(_arg2_y))()
            _tmparr__arg2_y[:] = _arg2_y
          except TypeError:
            raise TypeError("Argument _arg2_y has wrong type")
          else:
            _arg2_y_ = memoryview(_tmparr__arg2_y)
      
        else:
          if _arg2_y_.format != "d":
            _tmparr__arg2_y = (ctypes.c_double*len(_arg2_y))()
            _tmparr__arg2_y[:] = _arg2_y
            _arg2_y_ = memoryview(_tmparr__arg2_y)
      
      if _arg2_y_ is not None and len(_arg2_y_) != self.getnumcon():
        raise ValueError("Array argument _arg2_y has wrong length")
      res = self.__obj.puty(_arg1_whichsol_,_arg2_y_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putslc(self,_arg1_whichsol_,_arg2_slc): # 2
      """
      Sets the slc vector for a solution.
    
      putslc(self,_arg1_whichsol_,_arg2_slc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_slc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_slc is None: raise TypeError("Invalid type for argument _arg2_slc")
      if _arg2_slc is None:
        _arg2_slc_ = None
      else:
        try:
          _arg2_slc_ = memoryview(_arg2_slc)
        except TypeError:
          try:
            _tmparr__arg2_slc = (ctypes.c_double*len(_arg2_slc))()
            _tmparr__arg2_slc[:] = _arg2_slc
          except TypeError:
            raise TypeError("Argument _arg2_slc has wrong type")
          else:
            _arg2_slc_ = memoryview(_tmparr__arg2_slc)
      
        else:
          if _arg2_slc_.format != "d":
            _tmparr__arg2_slc = (ctypes.c_double*len(_arg2_slc))()
            _tmparr__arg2_slc[:] = _arg2_slc
            _arg2_slc_ = memoryview(_tmparr__arg2_slc)
      
      if _arg2_slc_ is not None and len(_arg2_slc_) != self.getnumcon():
        raise ValueError("Array argument _arg2_slc has wrong length")
      res = self.__obj.putslc(_arg1_whichsol_,_arg2_slc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putsuc(self,_arg1_whichsol_,_arg2_suc): # 2
      """
      Sets the suc vector for a solution.
    
      putsuc(self,_arg1_whichsol_,_arg2_suc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_suc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_suc is None: raise TypeError("Invalid type for argument _arg2_suc")
      if _arg2_suc is None:
        _arg2_suc_ = None
      else:
        try:
          _arg2_suc_ = memoryview(_arg2_suc)
        except TypeError:
          try:
            _tmparr__arg2_suc = (ctypes.c_double*len(_arg2_suc))()
            _tmparr__arg2_suc[:] = _arg2_suc
          except TypeError:
            raise TypeError("Argument _arg2_suc has wrong type")
          else:
            _arg2_suc_ = memoryview(_tmparr__arg2_suc)
      
        else:
          if _arg2_suc_.format != "d":
            _tmparr__arg2_suc = (ctypes.c_double*len(_arg2_suc))()
            _tmparr__arg2_suc[:] = _arg2_suc
            _arg2_suc_ = memoryview(_tmparr__arg2_suc)
      
      if _arg2_suc_ is not None and len(_arg2_suc_) != self.getnumcon():
        raise ValueError("Array argument _arg2_suc has wrong length")
      res = self.__obj.putsuc(_arg1_whichsol_,_arg2_suc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putslx(self,_arg1_whichsol_,_arg2_slx): # 2
      """
      Sets the slx vector for a solution.
    
      putslx(self,_arg1_whichsol_,_arg2_slx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_slx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_slx is None: raise TypeError("Invalid type for argument _arg2_slx")
      if _arg2_slx is None:
        _arg2_slx_ = None
      else:
        try:
          _arg2_slx_ = memoryview(_arg2_slx)
        except TypeError:
          try:
            _tmparr__arg2_slx = (ctypes.c_double*len(_arg2_slx))()
            _tmparr__arg2_slx[:] = _arg2_slx
          except TypeError:
            raise TypeError("Argument _arg2_slx has wrong type")
          else:
            _arg2_slx_ = memoryview(_tmparr__arg2_slx)
      
        else:
          if _arg2_slx_.format != "d":
            _tmparr__arg2_slx = (ctypes.c_double*len(_arg2_slx))()
            _tmparr__arg2_slx[:] = _arg2_slx
            _arg2_slx_ = memoryview(_tmparr__arg2_slx)
      
      if _arg2_slx_ is not None and len(_arg2_slx_) != self.getnumvar():
        raise ValueError("Array argument _arg2_slx has wrong length")
      res = self.__obj.putslx(_arg1_whichsol_,_arg2_slx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putsux(self,_arg1_whichsol_,_arg2_sux): # 2
      """
      Sets the sux vector for a solution.
    
      putsux(self,_arg1_whichsol_,_arg2_sux)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_sux: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_sux is None: raise TypeError("Invalid type for argument _arg2_sux")
      if _arg2_sux is None:
        _arg2_sux_ = None
      else:
        try:
          _arg2_sux_ = memoryview(_arg2_sux)
        except TypeError:
          try:
            _tmparr__arg2_sux = (ctypes.c_double*len(_arg2_sux))()
            _tmparr__arg2_sux[:] = _arg2_sux
          except TypeError:
            raise TypeError("Argument _arg2_sux has wrong type")
          else:
            _arg2_sux_ = memoryview(_tmparr__arg2_sux)
      
        else:
          if _arg2_sux_.format != "d":
            _tmparr__arg2_sux = (ctypes.c_double*len(_arg2_sux))()
            _tmparr__arg2_sux[:] = _arg2_sux
            _arg2_sux_ = memoryview(_tmparr__arg2_sux)
      
      if _arg2_sux_ is not None and len(_arg2_sux_) != self.getnumvar():
        raise ValueError("Array argument _arg2_sux has wrong length")
      res = self.__obj.putsux(_arg1_whichsol_,_arg2_sux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putsnx(self,_arg1_whichsol_,_arg2_sux): # 2
      """
      Sets the snx vector for a solution.
    
      putsnx(self,_arg1_whichsol_,_arg2_sux)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_sux: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_sux is None: raise TypeError("Invalid type for argument _arg2_sux")
      if _arg2_sux is None:
        _arg2_sux_ = None
      else:
        try:
          _arg2_sux_ = memoryview(_arg2_sux)
        except TypeError:
          try:
            _tmparr__arg2_sux = (ctypes.c_double*len(_arg2_sux))()
            _tmparr__arg2_sux[:] = _arg2_sux
          except TypeError:
            raise TypeError("Argument _arg2_sux has wrong type")
          else:
            _arg2_sux_ = memoryview(_tmparr__arg2_sux)
      
        else:
          if _arg2_sux_.format != "d":
            _tmparr__arg2_sux = (ctypes.c_double*len(_arg2_sux))()
            _tmparr__arg2_sux[:] = _arg2_sux
            _arg2_sux_ = memoryview(_tmparr__arg2_sux)
      
      if _arg2_sux_ is not None and len(_arg2_sux_) != self.getnumvar():
        raise ValueError("Array argument _arg2_sux has wrong length")
      res = self.__obj.putsnx(_arg1_whichsol_,_arg2_sux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putskcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_skc): # 2
      """
      Sets the status keys for a slice of the constraints.
    
      putskcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_skc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_skc: array of mosek.stakey. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      if _arg4_skc is None:
        _arg4_skc_ = None
      else:
        try:
          _arg4_skc_ = memoryview(_arg4_skc)
        except TypeError:
          try:
            _tmparr__arg4_skc = (ctypes.c_int*len(_arg4_skc))()
            _tmparr__arg4_skc[:] = _arg4_skc
          except TypeError:
            raise TypeError("Argument _arg4_skc has wrong type")
          else:
            _arg4_skc_ = memoryview(_tmparr__arg4_skc)
      
        else:
          if _arg4_skc_.format != "i":
            _tmparr__arg4_skc = (ctypes.c_int*len(_arg4_skc))()
            _tmparr__arg4_skc[:] = _arg4_skc
            _arg4_skc_ = memoryview(_tmparr__arg4_skc)
      
      if _arg4_skc_ is not None and len(_arg4_skc_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_skc has wrong length")
      res = self.__obj.putskcslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_skc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putskxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_skx): # 2
      """
      Sets the status keys for a slice of the variables.
    
      putskxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_skx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_skx: array of mosek.stakey. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      if _arg4_skx is None: raise TypeError("Invalid type for argument _arg4_skx")
      if _arg4_skx is None:
        _arg4_skx_ = None
      else:
        try:
          _arg4_skx_ = memoryview(_arg4_skx)
        except TypeError:
          try:
            _tmparr__arg4_skx = (ctypes.c_int*len(_arg4_skx))()
            _tmparr__arg4_skx[:] = _arg4_skx
          except TypeError:
            raise TypeError("Argument _arg4_skx has wrong type")
          else:
            _arg4_skx_ = memoryview(_tmparr__arg4_skx)
      
        else:
          if _arg4_skx_.format != "i":
            _tmparr__arg4_skx = (ctypes.c_int*len(_arg4_skx))()
            _tmparr__arg4_skx[:] = _arg4_skx
            _arg4_skx_ = memoryview(_tmparr__arg4_skx)
      
      if _arg4_skx_ is not None and len(_arg4_skx_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_skx has wrong length")
      res = self.__obj.putskxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_skx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putxcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_xc): # 2
      """
      Sets a slice of the xc vector for a solution.
    
      putxcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_xc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_xc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      if _arg4_xc is None: raise TypeError("Invalid type for argument _arg4_xc")
      if _arg4_xc is None:
        _arg4_xc_ = None
      else:
        try:
          _arg4_xc_ = memoryview(_arg4_xc)
        except TypeError:
          try:
            _tmparr__arg4_xc = (ctypes.c_double*len(_arg4_xc))()
            _tmparr__arg4_xc[:] = _arg4_xc
          except TypeError:
            raise TypeError("Argument _arg4_xc has wrong type")
          else:
            _arg4_xc_ = memoryview(_tmparr__arg4_xc)
      
        else:
          if _arg4_xc_.format != "d":
            _tmparr__arg4_xc = (ctypes.c_double*len(_arg4_xc))()
            _tmparr__arg4_xc[:] = _arg4_xc
            _arg4_xc_ = memoryview(_tmparr__arg4_xc)
      
      if _arg4_xc_ is not None and len(_arg4_xc_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_xc has wrong length")
      res = self.__obj.putxcslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_xc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putxxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_xx): # 2
      """
      Sets a slice of the xx vector for a solution.
    
      putxxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_xx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_xx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      if _arg4_xx is None: raise TypeError("Invalid type for argument _arg4_xx")
      if _arg4_xx is None:
        _arg4_xx_ = None
      else:
        try:
          _arg4_xx_ = memoryview(_arg4_xx)
        except TypeError:
          try:
            _tmparr__arg4_xx = (ctypes.c_double*len(_arg4_xx))()
            _tmparr__arg4_xx[:] = _arg4_xx
          except TypeError:
            raise TypeError("Argument _arg4_xx has wrong type")
          else:
            _arg4_xx_ = memoryview(_tmparr__arg4_xx)
      
        else:
          if _arg4_xx_.format != "d":
            _tmparr__arg4_xx = (ctypes.c_double*len(_arg4_xx))()
            _tmparr__arg4_xx[:] = _arg4_xx
            _arg4_xx_ = memoryview(_tmparr__arg4_xx)
      
      if _arg4_xx_ is not None and len(_arg4_xx_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_xx has wrong length")
      res = self.__obj.putxxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_xx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putyslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_y): # 2
      """
      Sets a slice of the y vector for a solution.
    
      putyslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_y)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_y: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      if _arg4_y is None: raise TypeError("Invalid type for argument _arg4_y")
      if _arg4_y is None:
        _arg4_y_ = None
      else:
        try:
          _arg4_y_ = memoryview(_arg4_y)
        except TypeError:
          try:
            _tmparr__arg4_y = (ctypes.c_double*len(_arg4_y))()
            _tmparr__arg4_y[:] = _arg4_y
          except TypeError:
            raise TypeError("Argument _arg4_y has wrong type")
          else:
            _arg4_y_ = memoryview(_tmparr__arg4_y)
      
        else:
          if _arg4_y_.format != "d":
            _tmparr__arg4_y = (ctypes.c_double*len(_arg4_y))()
            _tmparr__arg4_y[:] = _arg4_y
            _arg4_y_ = memoryview(_tmparr__arg4_y)
      
      if _arg4_y_ is not None and len(_arg4_y_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_y has wrong length")
      res = self.__obj.putyslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_y_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putslcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slc): # 2
      """
      Sets a slice of the slc vector for a solution.
    
      putslcslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_slc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      if _arg4_slc is None: raise TypeError("Invalid type for argument _arg4_slc")
      if _arg4_slc is None:
        _arg4_slc_ = None
      else:
        try:
          _arg4_slc_ = memoryview(_arg4_slc)
        except TypeError:
          try:
            _tmparr__arg4_slc = (ctypes.c_double*len(_arg4_slc))()
            _tmparr__arg4_slc[:] = _arg4_slc
          except TypeError:
            raise TypeError("Argument _arg4_slc has wrong type")
          else:
            _arg4_slc_ = memoryview(_tmparr__arg4_slc)
      
        else:
          if _arg4_slc_.format != "d":
            _tmparr__arg4_slc = (ctypes.c_double*len(_arg4_slc))()
            _tmparr__arg4_slc[:] = _arg4_slc
            _arg4_slc_ = memoryview(_tmparr__arg4_slc)
      
      if _arg4_slc_ is not None and len(_arg4_slc_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_slc has wrong length")
      res = self.__obj.putslcslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_slc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putsucslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_suc): # 2
      """
      Sets a slice of the suc vector for a solution.
    
      putsucslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_suc)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_suc: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      if _arg4_suc is None: raise TypeError("Invalid type for argument _arg4_suc")
      if _arg4_suc is None:
        _arg4_suc_ = None
      else:
        try:
          _arg4_suc_ = memoryview(_arg4_suc)
        except TypeError:
          try:
            _tmparr__arg4_suc = (ctypes.c_double*len(_arg4_suc))()
            _tmparr__arg4_suc[:] = _arg4_suc
          except TypeError:
            raise TypeError("Argument _arg4_suc has wrong type")
          else:
            _arg4_suc_ = memoryview(_tmparr__arg4_suc)
      
        else:
          if _arg4_suc_.format != "d":
            _tmparr__arg4_suc = (ctypes.c_double*len(_arg4_suc))()
            _tmparr__arg4_suc[:] = _arg4_suc
            _arg4_suc_ = memoryview(_tmparr__arg4_suc)
      
      if _arg4_suc_ is not None and len(_arg4_suc_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_suc has wrong length")
      res = self.__obj.putsucslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_suc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putslxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slx): # 2
      """
      Sets a slice of the slx vector for a solution.
    
      putslxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_slx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_slx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      if _arg4_slx is None: raise TypeError("Invalid type for argument _arg4_slx")
      if _arg4_slx is None:
        _arg4_slx_ = None
      else:
        try:
          _arg4_slx_ = memoryview(_arg4_slx)
        except TypeError:
          try:
            _tmparr__arg4_slx = (ctypes.c_double*len(_arg4_slx))()
            _tmparr__arg4_slx[:] = _arg4_slx
          except TypeError:
            raise TypeError("Argument _arg4_slx has wrong type")
          else:
            _arg4_slx_ = memoryview(_tmparr__arg4_slx)
      
        else:
          if _arg4_slx_.format != "d":
            _tmparr__arg4_slx = (ctypes.c_double*len(_arg4_slx))()
            _tmparr__arg4_slx[:] = _arg4_slx
            _arg4_slx_ = memoryview(_tmparr__arg4_slx)
      
      if _arg4_slx_ is not None and len(_arg4_slx_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_slx has wrong length")
      res = self.__obj.putslxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_slx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putsuxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_sux): # 2
      """
      Sets a slice of the sux vector for a solution.
    
      putsuxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_sux)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_sux: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      if _arg4_sux is None: raise TypeError("Invalid type for argument _arg4_sux")
      if _arg4_sux is None:
        _arg4_sux_ = None
      else:
        try:
          _arg4_sux_ = memoryview(_arg4_sux)
        except TypeError:
          try:
            _tmparr__arg4_sux = (ctypes.c_double*len(_arg4_sux))()
            _tmparr__arg4_sux[:] = _arg4_sux
          except TypeError:
            raise TypeError("Argument _arg4_sux has wrong type")
          else:
            _arg4_sux_ = memoryview(_tmparr__arg4_sux)
      
        else:
          if _arg4_sux_.format != "d":
            _tmparr__arg4_sux = (ctypes.c_double*len(_arg4_sux))()
            _tmparr__arg4_sux[:] = _arg4_sux
            _arg4_sux_ = memoryview(_tmparr__arg4_sux)
      
      if _arg4_sux_ is not None and len(_arg4_sux_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_sux has wrong length")
      res = self.__obj.putsuxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_sux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putsnxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_snx): # 2
      """
      Sets a slice of the snx vector for a solution.
    
      putsnxslice(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_snx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. <no description>
        _arg3_last: int. <no description>
        _arg4_snx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      if _arg4_snx is None: raise TypeError("Invalid type for argument _arg4_snx")
      if _arg4_snx is None:
        _arg4_snx_ = None
      else:
        try:
          _arg4_snx_ = memoryview(_arg4_snx)
        except TypeError:
          try:
            _tmparr__arg4_snx = (ctypes.c_double*len(_arg4_snx))()
            _tmparr__arg4_snx[:] = _arg4_snx
          except TypeError:
            raise TypeError("Argument _arg4_snx has wrong type")
          else:
            _arg4_snx_ = memoryview(_tmparr__arg4_snx)
      
        else:
          if _arg4_snx_.format != "d":
            _tmparr__arg4_snx = (ctypes.c_double*len(_arg4_snx))()
            _tmparr__arg4_snx[:] = _arg4_snx
            _arg4_snx_ = memoryview(_tmparr__arg4_snx)
      
      if _arg4_snx_ is not None and len(_arg4_snx_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_snx has wrong length")
      res = self.__obj.putsnxslice(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_snx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putbarxj(self,_arg1_whichsol_,_arg2_j,_arg3_barxj): # 2
      """
      Sets the primal solution for a semidefinite variable.
    
      putbarxj(self,_arg1_whichsol_,_arg2_j,_arg3_barxj)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_j: int. Index of the semidefinite variable.
        _arg3_barxj: array of double. Value of the j'th variable of barx.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_j_ = _arg2_j
      if _arg3_barxj is None: raise TypeError("Invalid type for argument _arg3_barxj")
      if _arg3_barxj is None:
        _arg3_barxj_ = None
      else:
        try:
          _arg3_barxj_ = memoryview(_arg3_barxj)
        except TypeError:
          try:
            _tmparr__arg3_barxj = (ctypes.c_double*len(_arg3_barxj))()
            _tmparr__arg3_barxj[:] = _arg3_barxj
          except TypeError:
            raise TypeError("Argument _arg3_barxj has wrong type")
          else:
            _arg3_barxj_ = memoryview(_tmparr__arg3_barxj)
      
        else:
          if _arg3_barxj_.format != "d":
            _tmparr__arg3_barxj = (ctypes.c_double*len(_arg3_barxj))()
            _tmparr__arg3_barxj[:] = _arg3_barxj
            _arg3_barxj_ = memoryview(_tmparr__arg3_barxj)
      
      if _arg3_barxj_ is not None and len(_arg3_barxj_) != self.getlenbarvarj( _arg2_j ):
        raise ValueError("Array argument _arg3_barxj has wrong length")
      res = self.__obj.putbarxj(_arg1_whichsol_,_arg2_j_,_arg3_barxj_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putbarsj(self,_arg1_whichsol_,_arg2_j,_arg3_barsj): # 2
      """
      Sets the dual solution for a semidefinite variable.
    
      putbarsj(self,_arg1_whichsol_,_arg2_j,_arg3_barsj)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_j: int. Index of the semidefinite variable.
        _arg3_barsj: array of double. Value of the j'th variable of barx.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_j_ = _arg2_j
      if _arg3_barsj is None: raise TypeError("Invalid type for argument _arg3_barsj")
      if _arg3_barsj is None:
        _arg3_barsj_ = None
      else:
        try:
          _arg3_barsj_ = memoryview(_arg3_barsj)
        except TypeError:
          try:
            _tmparr__arg3_barsj = (ctypes.c_double*len(_arg3_barsj))()
            _tmparr__arg3_barsj[:] = _arg3_barsj
          except TypeError:
            raise TypeError("Argument _arg3_barsj has wrong type")
          else:
            _arg3_barsj_ = memoryview(_tmparr__arg3_barsj)
      
        else:
          if _arg3_barsj_.format != "d":
            _tmparr__arg3_barsj = (ctypes.c_double*len(_arg3_barsj))()
            _tmparr__arg3_barsj[:] = _arg3_barsj
            _arg3_barsj_ = memoryview(_tmparr__arg3_barsj)
      
      if _arg3_barsj_ is not None and len(_arg3_barsj_) != self.getlenbarvarj( _arg2_j ):
        raise ValueError("Array argument _arg3_barsj has wrong length")
      res = self.__obj.putbarsj(_arg1_whichsol_,_arg2_j_,_arg3_barsj_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getpviolcon(self,_arg1_whichsol_,_arg3_sub,_arg4_viol): # 2
      """
      Computes the violation of a primal solution associated to a constraint.
    
      getpviolcon(self,_arg1_whichsol_,_arg3_sub,_arg4_viol)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg3_sub: array of int. An array of indexes of constraints.
        _arg4_viol: array of double. List of violations corresponding to sub.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_num_ = None
      if _arg2_num_ is None:
        _arg2_num_ = len(_arg3_sub)
        _arg2_num = _arg2_num_
      elif _arg2_num_ != len(_arg3_sub):
        raise IndexError("Inconsistent length of array _arg3_sub")
      if _arg2_num_ is None: _arg2_num_ = 0
      if _arg3_sub is None: raise TypeError("Invalid type for argument _arg3_sub")
      if _arg3_sub is None:
        _arg3_sub_ = None
      else:
        try:
          _arg3_sub_ = memoryview(_arg3_sub)
        except TypeError:
          try:
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
          except TypeError:
            raise TypeError("Argument _arg3_sub has wrong type")
          else:
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
        else:
          if _arg3_sub_.format != "i":
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
      if _arg4_viol is None: raise TypeError("Invalid type for argument _arg4_viol")
      _copyback__arg4_viol = False
      if _arg4_viol is None:
        _arg4_viol_ = None
      else:
        try:
          _arg4_viol_ = memoryview(_arg4_viol)
        except TypeError:
          try:
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
          except TypeError:
            raise TypeError("Argument _arg4_viol has wrong type")
          else:
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
        else:
          if _arg4_viol_.format != "d":
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
      if _arg4_viol_ is not None and len(_arg4_viol_) !=  _arg2_num :
        raise ValueError("Array argument _arg4_viol has wrong length")
      res = self.__obj.getpviolcon(_arg1_whichsol_,_arg2_num_,_arg3_sub_,_arg4_viol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_viol:
        _arg4_viol[:] = _tmparr__arg4_viol
    
    def getpviolvar(self,_arg1_whichsol_,_arg3_sub,_arg4_viol): # 2
      """
      Computes the violation of a primal solution for a list of scalar variables.
    
      getpviolvar(self,_arg1_whichsol_,_arg3_sub,_arg4_viol)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg3_sub: array of int. An array of indexes of x variables.
        _arg4_viol: array of double. List of violations corresponding to sub.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_num_ = None
      if _arg2_num_ is None:
        _arg2_num_ = len(_arg3_sub)
        _arg2_num = _arg2_num_
      elif _arg2_num_ != len(_arg3_sub):
        raise IndexError("Inconsistent length of array _arg3_sub")
      if _arg2_num_ is None: _arg2_num_ = 0
      if _arg3_sub is None: raise TypeError("Invalid type for argument _arg3_sub")
      if _arg3_sub is None:
        _arg3_sub_ = None
      else:
        try:
          _arg3_sub_ = memoryview(_arg3_sub)
        except TypeError:
          try:
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
          except TypeError:
            raise TypeError("Argument _arg3_sub has wrong type")
          else:
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
        else:
          if _arg3_sub_.format != "i":
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
      if _arg4_viol is None: raise TypeError("Invalid type for argument _arg4_viol")
      _copyback__arg4_viol = False
      if _arg4_viol is None:
        _arg4_viol_ = None
      else:
        try:
          _arg4_viol_ = memoryview(_arg4_viol)
        except TypeError:
          try:
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
          except TypeError:
            raise TypeError("Argument _arg4_viol has wrong type")
          else:
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
        else:
          if _arg4_viol_.format != "d":
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
      if _arg4_viol_ is not None and len(_arg4_viol_) !=  _arg2_num :
        raise ValueError("Array argument _arg4_viol has wrong length")
      res = self.__obj.getpviolvar(_arg1_whichsol_,_arg2_num_,_arg3_sub_,_arg4_viol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_viol:
        _arg4_viol[:] = _tmparr__arg4_viol
    
    def getpviolbarvar(self,_arg1_whichsol_,_arg3_sub,_arg4_viol): # 2
      """
      Computes the violation of a primal solution for a list of semidefinite variables.
    
      getpviolbarvar(self,_arg1_whichsol_,_arg3_sub,_arg4_viol)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg3_sub: array of int. An array of indexes of barX variables.
        _arg4_viol: array of double. List of violations corresponding to sub.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_num_ = None
      if _arg2_num_ is None:
        _arg2_num_ = len(_arg3_sub)
        _arg2_num = _arg2_num_
      elif _arg2_num_ != len(_arg3_sub):
        raise IndexError("Inconsistent length of array _arg3_sub")
      if _arg2_num_ is None: _arg2_num_ = 0
      if _arg3_sub is None: raise TypeError("Invalid type for argument _arg3_sub")
      if _arg3_sub is None:
        _arg3_sub_ = None
      else:
        try:
          _arg3_sub_ = memoryview(_arg3_sub)
        except TypeError:
          try:
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
          except TypeError:
            raise TypeError("Argument _arg3_sub has wrong type")
          else:
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
        else:
          if _arg3_sub_.format != "i":
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
      if _arg4_viol is None: raise TypeError("Invalid type for argument _arg4_viol")
      _copyback__arg4_viol = False
      if _arg4_viol is None:
        _arg4_viol_ = None
      else:
        try:
          _arg4_viol_ = memoryview(_arg4_viol)
        except TypeError:
          try:
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
          except TypeError:
            raise TypeError("Argument _arg4_viol has wrong type")
          else:
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
        else:
          if _arg4_viol_.format != "d":
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
      if _arg4_viol_ is not None and len(_arg4_viol_) !=  _arg2_num :
        raise ValueError("Array argument _arg4_viol has wrong length")
      res = self.__obj.getpviolbarvar(_arg1_whichsol_,_arg2_num_,_arg3_sub_,_arg4_viol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_viol:
        _arg4_viol[:] = _tmparr__arg4_viol
    
    def getpviolcones(self,_arg1_whichsol_,_arg3_sub,_arg4_viol): # 2
      """
      Computes the violation of a solution for set of conic constraints.
    
      getpviolcones(self,_arg1_whichsol_,_arg3_sub,_arg4_viol)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg3_sub: array of int. An array of indexes of conic constraints.
        _arg4_viol: array of double. List of violations corresponding to sub.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_num_ = None
      if _arg2_num_ is None:
        _arg2_num_ = len(_arg3_sub)
        _arg2_num = _arg2_num_
      elif _arg2_num_ != len(_arg3_sub):
        raise IndexError("Inconsistent length of array _arg3_sub")
      if _arg2_num_ is None: _arg2_num_ = 0
      if _arg3_sub is None: raise TypeError("Invalid type for argument _arg3_sub")
      if _arg3_sub is None:
        _arg3_sub_ = None
      else:
        try:
          _arg3_sub_ = memoryview(_arg3_sub)
        except TypeError:
          try:
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
          except TypeError:
            raise TypeError("Argument _arg3_sub has wrong type")
          else:
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
        else:
          if _arg3_sub_.format != "i":
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
      if _arg4_viol is None: raise TypeError("Invalid type for argument _arg4_viol")
      _copyback__arg4_viol = False
      if _arg4_viol is None:
        _arg4_viol_ = None
      else:
        try:
          _arg4_viol_ = memoryview(_arg4_viol)
        except TypeError:
          try:
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
          except TypeError:
            raise TypeError("Argument _arg4_viol has wrong type")
          else:
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
        else:
          if _arg4_viol_.format != "d":
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
      if _arg4_viol_ is not None and len(_arg4_viol_) !=  _arg2_num :
        raise ValueError("Array argument _arg4_viol has wrong length")
      res = self.__obj.getpviolcones(_arg1_whichsol_,_arg2_num_,_arg3_sub_,_arg4_viol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_viol:
        _arg4_viol[:] = _tmparr__arg4_viol
    
    def getdviolcon(self,_arg1_whichsol_,_arg3_sub,_arg4_viol): # 2
      """
      Computes the violation of a dual solution associated with a set of constraints.
    
      getdviolcon(self,_arg1_whichsol_,_arg3_sub,_arg4_viol)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg3_sub: array of int. An array of indexes of constraints.
        _arg4_viol: array of double. List of violations corresponding to sub.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_num_ = None
      if _arg2_num_ is None:
        _arg2_num_ = len(_arg3_sub)
        _arg2_num = _arg2_num_
      elif _arg2_num_ != len(_arg3_sub):
        raise IndexError("Inconsistent length of array _arg3_sub")
      if _arg2_num_ is None: _arg2_num_ = 0
      if _arg3_sub is None: raise TypeError("Invalid type for argument _arg3_sub")
      if _arg3_sub is None:
        _arg3_sub_ = None
      else:
        try:
          _arg3_sub_ = memoryview(_arg3_sub)
        except TypeError:
          try:
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
          except TypeError:
            raise TypeError("Argument _arg3_sub has wrong type")
          else:
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
        else:
          if _arg3_sub_.format != "i":
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
      if _arg4_viol is None: raise TypeError("Invalid type for argument _arg4_viol")
      _copyback__arg4_viol = False
      if _arg4_viol is None:
        _arg4_viol_ = None
      else:
        try:
          _arg4_viol_ = memoryview(_arg4_viol)
        except TypeError:
          try:
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
          except TypeError:
            raise TypeError("Argument _arg4_viol has wrong type")
          else:
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
        else:
          if _arg4_viol_.format != "d":
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
      if _arg4_viol_ is not None and len(_arg4_viol_) !=  _arg2_num :
        raise ValueError("Array argument _arg4_viol has wrong length")
      res = self.__obj.getdviolcon(_arg1_whichsol_,_arg2_num_,_arg3_sub_,_arg4_viol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_viol:
        _arg4_viol[:] = _tmparr__arg4_viol
    
    def getdviolvar(self,_arg1_whichsol_,_arg3_sub,_arg4_viol): # 2
      """
      Computes the violation of a dual solution associated with a set of scalar variables.
    
      getdviolvar(self,_arg1_whichsol_,_arg3_sub,_arg4_viol)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg3_sub: array of int. An array of indexes of x variables.
        _arg4_viol: array of double. List of violations corresponding to sub.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_num_ = None
      if _arg2_num_ is None:
        _arg2_num_ = len(_arg3_sub)
        _arg2_num = _arg2_num_
      elif _arg2_num_ != len(_arg3_sub):
        raise IndexError("Inconsistent length of array _arg3_sub")
      if _arg2_num_ is None: _arg2_num_ = 0
      if _arg3_sub is None: raise TypeError("Invalid type for argument _arg3_sub")
      if _arg3_sub is None:
        _arg3_sub_ = None
      else:
        try:
          _arg3_sub_ = memoryview(_arg3_sub)
        except TypeError:
          try:
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
          except TypeError:
            raise TypeError("Argument _arg3_sub has wrong type")
          else:
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
        else:
          if _arg3_sub_.format != "i":
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
      if _arg4_viol is None: raise TypeError("Invalid type for argument _arg4_viol")
      _copyback__arg4_viol = False
      if _arg4_viol is None:
        _arg4_viol_ = None
      else:
        try:
          _arg4_viol_ = memoryview(_arg4_viol)
        except TypeError:
          try:
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
          except TypeError:
            raise TypeError("Argument _arg4_viol has wrong type")
          else:
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
        else:
          if _arg4_viol_.format != "d":
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
      if _arg4_viol_ is not None and len(_arg4_viol_) !=  _arg2_num :
        raise ValueError("Array argument _arg4_viol has wrong length")
      res = self.__obj.getdviolvar(_arg1_whichsol_,_arg2_num_,_arg3_sub_,_arg4_viol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_viol:
        _arg4_viol[:] = _tmparr__arg4_viol
    
    def getdviolbarvar(self,_arg1_whichsol_,_arg3_sub,_arg4_viol): # 2
      """
      Computes the violation of dual solution for a set of semidefinite variables.
    
      getdviolbarvar(self,_arg1_whichsol_,_arg3_sub,_arg4_viol)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg3_sub: array of int. An array of indexes of barx variables.
        _arg4_viol: array of double. List of violations corresponding to sub.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_num_ = None
      if _arg2_num_ is None:
        _arg2_num_ = len(_arg3_sub)
        _arg2_num = _arg2_num_
      elif _arg2_num_ != len(_arg3_sub):
        raise IndexError("Inconsistent length of array _arg3_sub")
      if _arg2_num_ is None: _arg2_num_ = 0
      if _arg3_sub is None: raise TypeError("Invalid type for argument _arg3_sub")
      if _arg3_sub is None:
        _arg3_sub_ = None
      else:
        try:
          _arg3_sub_ = memoryview(_arg3_sub)
        except TypeError:
          try:
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
          except TypeError:
            raise TypeError("Argument _arg3_sub has wrong type")
          else:
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
        else:
          if _arg3_sub_.format != "i":
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
      if _arg4_viol is None: raise TypeError("Invalid type for argument _arg4_viol")
      _copyback__arg4_viol = False
      if _arg4_viol is None:
        _arg4_viol_ = None
      else:
        try:
          _arg4_viol_ = memoryview(_arg4_viol)
        except TypeError:
          try:
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
          except TypeError:
            raise TypeError("Argument _arg4_viol has wrong type")
          else:
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
        else:
          if _arg4_viol_.format != "d":
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
      if _arg4_viol_ is not None and len(_arg4_viol_) !=  _arg2_num :
        raise ValueError("Array argument _arg4_viol has wrong length")
      res = self.__obj.getdviolbarvar(_arg1_whichsol_,_arg2_num_,_arg3_sub_,_arg4_viol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_viol:
        _arg4_viol[:] = _tmparr__arg4_viol
    
    def getdviolcones(self,_arg1_whichsol_,_arg3_sub,_arg4_viol): # 2
      """
      Computes the violation of a solution for set of dual conic constraints.
    
      getdviolcones(self,_arg1_whichsol_,_arg3_sub,_arg4_viol)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg3_sub: array of int. An array of indexes of conic constraints.
        _arg4_viol: array of double. List of violations corresponding to sub.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_num_ = None
      if _arg2_num_ is None:
        _arg2_num_ = len(_arg3_sub)
        _arg2_num = _arg2_num_
      elif _arg2_num_ != len(_arg3_sub):
        raise IndexError("Inconsistent length of array _arg3_sub")
      if _arg2_num_ is None: _arg2_num_ = 0
      if _arg3_sub is None: raise TypeError("Invalid type for argument _arg3_sub")
      if _arg3_sub is None:
        _arg3_sub_ = None
      else:
        try:
          _arg3_sub_ = memoryview(_arg3_sub)
        except TypeError:
          try:
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
          except TypeError:
            raise TypeError("Argument _arg3_sub has wrong type")
          else:
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
        else:
          if _arg3_sub_.format != "i":
            _tmparr__arg3_sub = (ctypes.c_int32*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
      if _arg4_viol is None: raise TypeError("Invalid type for argument _arg4_viol")
      _copyback__arg4_viol = False
      if _arg4_viol is None:
        _arg4_viol_ = None
      else:
        try:
          _arg4_viol_ = memoryview(_arg4_viol)
        except TypeError:
          try:
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
          except TypeError:
            raise TypeError("Argument _arg4_viol has wrong type")
          else:
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
        else:
          if _arg4_viol_.format != "d":
            _tmparr__arg4_viol = (ctypes.c_double*len(_arg4_viol))()
            _tmparr__arg4_viol[:] = _arg4_viol
            _arg4_viol_ = memoryview(_tmparr__arg4_viol)
            _copyback__arg4_viol = True
      if _arg4_viol_ is not None and len(_arg4_viol_) !=  _arg2_num :
        raise ValueError("Array argument _arg4_viol has wrong length")
      res = self.__obj.getdviolcones(_arg1_whichsol_,_arg2_num_,_arg3_sub_,_arg4_viol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_viol:
        _arg4_viol[:] = _tmparr__arg4_viol
    
    def getsolutioninfo(self,_arg1_whichsol_): # 2
      """
      Obtains information about of a solution.
    
      getsolutioninfo(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. <no description>
      returns: _arg2_pobj,_arg3_pviolcon,_arg4_pviolvar,_arg5_pviolbarvar,_arg6_pviolcone,_arg7_pviolitg,_arg8_dobj,_arg9_dviolcon,_arg10_dviolvar,_arg11_dviolbarvar,_arg12_dviolcone
        _arg2_pobj: double. The primal objective value.
        _arg3_pviolcon: double. Maximal primal bound violation for a xc variable.
        _arg4_pviolvar: double. Maximal primal bound violation for a xx variable.
        _arg5_pviolbarvar: double. Maximal primal bound violation for a barx variable.
        _arg6_pviolcone: double. Maximal primal violation of the solution with respect to the conic constraints.
        _arg7_pviolitg: double. Maximal violation in the integer constraints.
        _arg8_dobj: double. Dual objective value.
        _arg9_dviolcon: double. Maximal dual bound violation for a xc variable.
        _arg10_dviolvar: double. Maximal dual bound violation for a xx variable.
        _arg11_dviolbarvar: double. Maximal dual bound violation for a bars variable.
        _arg12_dviolcone: double. Maximum violation of the dual solution in the dual conic constraints.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res,resargs = self.__obj.getsolutioninfo(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_pobj_return_value,__arg3_pviolcon_return_value,__arg4_pviolvar_return_value,__arg5_pviolbarvar_return_value,__arg6_pviolcone_return_value,__arg7_pviolitg_return_value,__arg8_dobj_return_value,__arg9_dviolcon_return_value,__arg10_dviolvar_return_value,__arg11_dviolbarvar_return_value,__arg12_dviolcone_return_value = resargs
      return __arg2_pobj_return_value,__arg3_pviolcon_return_value,__arg4_pviolvar_return_value,__arg5_pviolbarvar_return_value,__arg6_pviolcone_return_value,__arg7_pviolitg_return_value,__arg8_dobj_return_value,__arg9_dviolcon_return_value,__arg10_dviolvar_return_value,__arg11_dviolbarvar_return_value,__arg12_dviolcone_return_value
    
    def getdualsolutionnorms(self,_arg1_whichsol_): # 2
      """
      Compute norms of the dual solution.
    
      getdualsolutionnorms(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. <no description>
      returns: _arg2_nrmy,_arg3_nrmslc,_arg4_nrmsuc,_arg5_nrmslx,_arg6_nrmsux,_arg7_nrmsnx,_arg8_nrmbars
        _arg2_nrmy: double. The norm of the y vector.
        _arg3_nrmslc: double. The norm of the slc vector.
        _arg4_nrmsuc: double. The norm of the suc vector.
        _arg5_nrmslx: double. The norm of the slx vector.
        _arg6_nrmsux: double. The norm of the sux vector.
        _arg7_nrmsnx: double. The norm of the snx vector.
        _arg8_nrmbars: double. The norm of the bars vector.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res,resargs = self.__obj.getdualsolutionnorms(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_nrmy_return_value,__arg3_nrmslc_return_value,__arg4_nrmsuc_return_value,__arg5_nrmslx_return_value,__arg6_nrmsux_return_value,__arg7_nrmsnx_return_value,__arg8_nrmbars_return_value = resargs
      return __arg2_nrmy_return_value,__arg3_nrmslc_return_value,__arg4_nrmsuc_return_value,__arg5_nrmslx_return_value,__arg6_nrmsux_return_value,__arg7_nrmsnx_return_value,__arg8_nrmbars_return_value
    
    def getprimalsolutionnorms(self,_arg1_whichsol_): # 2
      """
      Compute norms of the primal solution.
    
      getprimalsolutionnorms(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. <no description>
      returns: _arg2_nrmxc,_arg3_nrmxx,_arg4_nrmbarx
        _arg2_nrmxc: double. The norm of the xc vector.
        _arg3_nrmxx: double. The norm of the xx vector.
        _arg4_nrmbarx: double. The norm of the barX vector.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res,resargs = self.__obj.getprimalsolutionnorms(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_nrmxc_return_value,__arg3_nrmxx_return_value,__arg4_nrmbarx_return_value = resargs
      return __arg2_nrmxc_return_value,__arg3_nrmxx_return_value,__arg4_nrmbarx_return_value
    
    def getsolutionslice(self,_arg1_whichsol_,_arg2_solitem_,_arg3_first,_arg4_last,_arg5_values): # 2
      """
      Obtains a slice of the solution.
    
      getsolutionslice(self,_arg1_whichsol_,_arg2_solitem_,_arg3_first,_arg4_last,_arg5_values)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_solitem: mosek.solitem. <no description>
        _arg3_first: int. <no description>
        _arg4_last: int. <no description>
        _arg5_values: array of double. The values of the requested solution elements.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if not isinstance(_arg2_solitem_,solitem): raise TypeError("Argument _arg2_solitem has wrong type")
      _arg2_solitem = _arg2_solitem_
      _arg3_first_ = _arg3_first
      _arg4_last_ = _arg4_last
      _copyback__arg5_values = False
      if _arg5_values is None:
        _arg5_values_ = None
      else:
        try:
          _arg5_values_ = memoryview(_arg5_values)
        except TypeError:
          try:
            _tmparr__arg5_values = (ctypes.c_double*len(_arg5_values))()
            _tmparr__arg5_values[:] = _arg5_values
          except TypeError:
            raise TypeError("Argument _arg5_values has wrong type")
          else:
            _arg5_values_ = memoryview(_tmparr__arg5_values)
            _copyback__arg5_values = True
        else:
          if _arg5_values_.format != "d":
            _tmparr__arg5_values = (ctypes.c_double*len(_arg5_values))()
            _tmparr__arg5_values[:] = _arg5_values
            _arg5_values_ = memoryview(_tmparr__arg5_values)
            _copyback__arg5_values = True
      if _arg5_values_ is not None and len(_arg5_values_) != ( _arg4_last  -  _arg3_first ):
        raise ValueError("Array argument _arg5_values has wrong length")
      res = self.__obj.getsolutionslice(_arg1_whichsol_,_arg2_solitem_,_arg3_first_,_arg4_last_,_arg5_values_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg5_values:
        _arg5_values[:] = _tmparr__arg5_values
    
    def getreducedcosts(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_redcosts): # 2
      """
      Obtains the reduced costs for a sequence of variables.
    
      getreducedcosts(self,_arg1_whichsol_,_arg2_first,_arg3_last,_arg4_redcosts)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_first: int. The index of the first variable in the sequence.
        _arg3_last: int. The index of the last variable in the sequence plus 1.
        _arg4_redcosts: array of double. Returns the requested reduced costs.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      _arg2_first_ = _arg2_first
      _arg3_last_ = _arg3_last
      _copyback__arg4_redcosts = False
      if _arg4_redcosts is None:
        _arg4_redcosts_ = None
      else:
        try:
          _arg4_redcosts_ = memoryview(_arg4_redcosts)
        except TypeError:
          try:
            _tmparr__arg4_redcosts = (ctypes.c_double*len(_arg4_redcosts))()
            _tmparr__arg4_redcosts[:] = _arg4_redcosts
          except TypeError:
            raise TypeError("Argument _arg4_redcosts has wrong type")
          else:
            _arg4_redcosts_ = memoryview(_tmparr__arg4_redcosts)
            _copyback__arg4_redcosts = True
        else:
          if _arg4_redcosts_.format != "d":
            _tmparr__arg4_redcosts = (ctypes.c_double*len(_arg4_redcosts))()
            _tmparr__arg4_redcosts[:] = _arg4_redcosts
            _arg4_redcosts_ = memoryview(_tmparr__arg4_redcosts)
            _copyback__arg4_redcosts = True
      if _arg4_redcosts_ is not None and len(_arg4_redcosts_) != ( _arg3_last  -  _arg2_first ):
        raise ValueError("Array argument _arg4_redcosts has wrong length")
      res = self.__obj.getreducedcosts(_arg1_whichsol_,_arg2_first_,_arg3_last_,_arg4_redcosts_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg4_redcosts:
        _arg4_redcosts[:] = _tmparr__arg4_redcosts
    
    def getstrparam(self,_arg1_param_): # 2
      """
      Obtains the value of a string parameter.
    
      getstrparam(self,_arg1_param_)
        _arg1_param: mosek.sparam. <no description>
      returns: _arg3_len,_arg4_parvalue
        _arg3_len: int. The length of the parameter value.
        _arg4_parvalue: str. If this is not a null pointer, the parameter value is stored here.
      """
      if not isinstance(_arg1_param_,sparam): raise TypeError("Argument _arg1_param has wrong type")
      _arg1_param = _arg1_param_
      _arg2_maxlen_ = (1 + self.getstrparamlen( _arg1_param ))
      _arg2_maxlen = _arg2_maxlen_
      arr__arg4_parvalue = (ctypes.c_char* _arg2_maxlen )()
      memview_arr__arg4_parvalue = memoryview(arr__arg4_parvalue)
      res,resargs = self.__obj.getstrparam(_arg1_param_,_arg2_maxlen_,memview_arr__arg4_parvalue)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg3_len_return_value,retarg__arg4_parvalue = resargs
      retarg__arg4_parvalue = arr__arg4_parvalue.value.decode("utf-8",errors="ignore")
      return __arg3_len_return_value,retarg__arg4_parvalue
    
    def getstrparamlen(self,_arg1_param_): # 2
      """
      Obtains the length of a string parameter.
    
      getstrparamlen(self,_arg1_param_)
        _arg1_param: mosek.sparam. <no description>
      returns: _arg2_len
        _arg2_len: int. The length of the parameter value.
      """
      if not isinstance(_arg1_param_,sparam): raise TypeError("Argument _arg1_param has wrong type")
      _arg1_param = _arg1_param_
      res,resargs = self.__obj.getstrparamlen(_arg1_param_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_len_return_value = resargs
      return __arg2_len_return_value
    
    def gettasknamelen(self): # 2
      """
      Obtains the length the task name.
    
      gettasknamelen(self)
      returns: _arg1_len
        _arg1_len: int. Returns the length of the task name.
      """
      res,resargs = self.__obj.gettasknamelen()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_len_return_value = resargs
      return __arg1_len_return_value
    
    def gettaskname(self): # 2
      """
      Obtains the task name.
    
      gettaskname(self)
      returns: _arg2_taskname
        _arg2_taskname: str. Returns the task name.
      """
      _arg1_sizetaskname_ = (1 + self.gettasknamelen())
      _arg1_sizetaskname = _arg1_sizetaskname_
      arr__arg2_taskname = (ctypes.c_char* _arg1_sizetaskname )()
      memview_arr__arg2_taskname = memoryview(arr__arg2_taskname)
      res,resargs = self.__obj.gettaskname(_arg1_sizetaskname_,memview_arr__arg2_taskname)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      retarg__arg2_taskname = resargs
      retarg__arg2_taskname = arr__arg2_taskname.value.decode("utf-8",errors="ignore")
      return retarg__arg2_taskname
    
    def getvartype(self,_arg1_j): # 2
      """
      Gets the variable type of one variable.
    
      getvartype(self,_arg1_j)
        _arg1_j: int. Index of the variable.
      returns: _arg2_vartype
        _arg2_vartype: mosek.variabletype. Variable type of variable index j.
      """
      _arg1_j_ = _arg1_j
      res,resargs = self.__obj.getvartype(_arg1_j_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_vartype_return_value = resargs
      __arg2_vartype_return_value = variabletype(__arg2_vartype_return_value)
      return __arg2_vartype_return_value
    
    def getvartypelist(self,_arg2_subj,_arg3_vartype): # 2
      """
      Obtains the variable type for one or more variables.
    
      getvartypelist(self,_arg2_subj,_arg3_vartype)
        _arg2_subj: array of int. A list of variable indexes.
        _arg3_vartype: array of mosek.variabletype. Returns the variables types corresponding the variable indexes requested.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subj)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subj):
        raise IndexError("Inconsistent length of array _arg2_subj")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subj is None: raise TypeError("Invalid type for argument _arg2_subj")
      if _arg2_subj is None:
        _arg2_subj_ = None
      else:
        try:
          _arg2_subj_ = memoryview(_arg2_subj)
        except TypeError:
          try:
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
          except TypeError:
            raise TypeError("Argument _arg2_subj has wrong type")
          else:
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
        else:
          if _arg2_subj_.format != "i":
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
      _copyback__arg3_vartype = False
      if _arg3_vartype is None:
        _arg3_vartype_ = None
      else:
        try:
          _arg3_vartype_ = memoryview(_arg3_vartype)
        except TypeError:
          try:
            _tmparr__arg3_vartype = (ctypes.c_int*len(_arg3_vartype))()
            _tmparr__arg3_vartype[:] = _arg3_vartype
          except TypeError:
            raise TypeError("Argument _arg3_vartype has wrong type")
          else:
            _arg3_vartype_ = memoryview(_tmparr__arg3_vartype)
            _copyback__arg3_vartype = True
        else:
          if _arg3_vartype_.format != "i":
            _tmparr__arg3_vartype = (ctypes.c_int*len(_arg3_vartype))()
            _tmparr__arg3_vartype[:] = _arg3_vartype
            _arg3_vartype_ = memoryview(_tmparr__arg3_vartype)
            _copyback__arg3_vartype = True
      if _arg3_vartype_ is not None and len(_arg3_vartype_) !=  _arg1_num :
        raise ValueError("Array argument _arg3_vartype has wrong length")
      res = self.__obj.getvartypelist(_arg1_num_,_arg2_subj_,_arg3_vartype_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg3_vartype:
        for __tmp_var_0 in range(len(_arg3_vartype_)): _arg3_vartype[__tmp_var_0] = variabletype(_tmparr__arg3_vartype[__tmp_var_0])
    
    def inputdata(self,_arg1_maxnumcon,_arg2_maxnumvar,_arg5_c,_arg6_cfix,_arg7_aptrb,_arg8_aptre,_arg9_asub,_arg10_aval,_arg11_bkc,_arg12_blc,_arg13_buc,_arg14_bkx,_arg15_blx,_arg16_bux): # 2
      """
      Input the linear part of an optimization task in one function call.
    
      inputdata(self,_arg1_maxnumcon,_arg2_maxnumvar,_arg5_c,_arg6_cfix,_arg7_aptrb,_arg8_aptre,_arg9_asub,_arg10_aval,_arg11_bkc,_arg12_blc,_arg13_buc,_arg14_bkx,_arg15_blx,_arg16_bux)
        _arg1_maxnumcon: int. <no description>
        _arg2_maxnumvar: int. <no description>
        _arg5_c: array of double. <no description>
        _arg6_cfix: double. <no description>
        _arg7_aptrb: array of long. <no description>
        _arg8_aptre: array of long. <no description>
        _arg9_asub: array of int. <no description>
        _arg10_aval: array of double. <no description>
        _arg11_bkc: array of mosek.boundkey. <no description>
        _arg12_blc: array of double. <no description>
        _arg13_buc: array of double. <no description>
        _arg14_bkx: array of mosek.boundkey. <no description>
        _arg15_blx: array of double. <no description>
        _arg16_bux: array of double. <no description>
      """
      _arg1_maxnumcon_ = _arg1_maxnumcon
      _arg2_maxnumvar_ = _arg2_maxnumvar
      _arg3_numcon_ = None
      if _arg3_numcon_ is None:
        _arg3_numcon_ = len(_arg13_buc)
        _arg3_numcon = _arg3_numcon_
      elif _arg3_numcon_ != len(_arg13_buc):
        raise IndexError("Inconsistent length of array _arg13_buc")
      if _arg3_numcon_ is None:
        _arg3_numcon_ = len(_arg12_blc)
        _arg3_numcon = _arg3_numcon_
      elif _arg3_numcon_ != len(_arg12_blc):
        raise IndexError("Inconsistent length of array _arg12_blc")
      if _arg3_numcon_ is None:
        _arg3_numcon_ = len(_arg11_bkc)
        _arg3_numcon = _arg3_numcon_
      elif _arg3_numcon_ != len(_arg11_bkc):
        raise IndexError("Inconsistent length of array _arg11_bkc")
      if _arg3_numcon_ is None: _arg3_numcon_ = 0
      _arg4_numvar_ = None
      if _arg4_numvar_ is None:
        _arg4_numvar_ = len(_arg5_c)
        _arg4_numvar = _arg4_numvar_
      elif _arg4_numvar_ != len(_arg5_c):
        raise IndexError("Inconsistent length of array _arg5_c")
      if _arg4_numvar_ is None:
        _arg4_numvar_ = len(_arg16_bux)
        _arg4_numvar = _arg4_numvar_
      elif _arg4_numvar_ != len(_arg16_bux):
        raise IndexError("Inconsistent length of array _arg16_bux")
      if _arg4_numvar_ is None:
        _arg4_numvar_ = len(_arg15_blx)
        _arg4_numvar = _arg4_numvar_
      elif _arg4_numvar_ != len(_arg15_blx):
        raise IndexError("Inconsistent length of array _arg15_blx")
      if _arg4_numvar_ is None:
        _arg4_numvar_ = len(_arg14_bkx)
        _arg4_numvar = _arg4_numvar_
      elif _arg4_numvar_ != len(_arg14_bkx):
        raise IndexError("Inconsistent length of array _arg14_bkx")
      if _arg4_numvar_ is None:
        _arg4_numvar_ = len(_arg7_aptrb)
        _arg4_numvar = _arg4_numvar_
      elif _arg4_numvar_ != len(_arg7_aptrb):
        raise IndexError("Inconsistent length of array _arg7_aptrb")
      if _arg4_numvar_ is None:
        _arg4_numvar_ = len(_arg8_aptre)
        _arg4_numvar = _arg4_numvar_
      elif _arg4_numvar_ != len(_arg8_aptre):
        raise IndexError("Inconsistent length of array _arg8_aptre")
      if _arg4_numvar_ is None: _arg4_numvar_ = 0
      if _arg5_c is None:
        _arg5_c_ = None
      else:
        try:
          _arg5_c_ = memoryview(_arg5_c)
        except TypeError:
          try:
            _tmparr__arg5_c = (ctypes.c_double*len(_arg5_c))()
            _tmparr__arg5_c[:] = _arg5_c
          except TypeError:
            raise TypeError("Argument _arg5_c has wrong type")
          else:
            _arg5_c_ = memoryview(_tmparr__arg5_c)
      
        else:
          if _arg5_c_.format != "d":
            _tmparr__arg5_c = (ctypes.c_double*len(_arg5_c))()
            _tmparr__arg5_c[:] = _arg5_c
            _arg5_c_ = memoryview(_tmparr__arg5_c)
      
      _arg6_cfix_ = _arg6_cfix
      if _arg7_aptrb is None: raise TypeError("Invalid type for argument _arg7_aptrb")
      if _arg7_aptrb is None:
        _arg7_aptrb_ = None
      else:
        try:
          _arg7_aptrb_ = memoryview(_arg7_aptrb)
        except TypeError:
          try:
            _tmparr__arg7_aptrb = (ctypes.c_int64*len(_arg7_aptrb))()
            _tmparr__arg7_aptrb[:] = _arg7_aptrb
          except TypeError:
            raise TypeError("Argument _arg7_aptrb has wrong type")
          else:
            _arg7_aptrb_ = memoryview(_tmparr__arg7_aptrb)
      
        else:
          if _arg7_aptrb_.format != "q":
            _tmparr__arg7_aptrb = (ctypes.c_int64*len(_arg7_aptrb))()
            _tmparr__arg7_aptrb[:] = _arg7_aptrb
            _arg7_aptrb_ = memoryview(_tmparr__arg7_aptrb)
      
      if _arg8_aptre is None: raise TypeError("Invalid type for argument _arg8_aptre")
      if _arg8_aptre is None:
        _arg8_aptre_ = None
      else:
        try:
          _arg8_aptre_ = memoryview(_arg8_aptre)
        except TypeError:
          try:
            _tmparr__arg8_aptre = (ctypes.c_int64*len(_arg8_aptre))()
            _tmparr__arg8_aptre[:] = _arg8_aptre
          except TypeError:
            raise TypeError("Argument _arg8_aptre has wrong type")
          else:
            _arg8_aptre_ = memoryview(_tmparr__arg8_aptre)
      
        else:
          if _arg8_aptre_.format != "q":
            _tmparr__arg8_aptre = (ctypes.c_int64*len(_arg8_aptre))()
            _tmparr__arg8_aptre[:] = _arg8_aptre
            _arg8_aptre_ = memoryview(_tmparr__arg8_aptre)
      
      if _arg9_asub is None: raise TypeError("Invalid type for argument _arg9_asub")
      if _arg9_asub is None:
        _arg9_asub_ = None
      else:
        try:
          _arg9_asub_ = memoryview(_arg9_asub)
        except TypeError:
          try:
            _tmparr__arg9_asub = (ctypes.c_int32*len(_arg9_asub))()
            _tmparr__arg9_asub[:] = _arg9_asub
          except TypeError:
            raise TypeError("Argument _arg9_asub has wrong type")
          else:
            _arg9_asub_ = memoryview(_tmparr__arg9_asub)
      
        else:
          if _arg9_asub_.format != "i":
            _tmparr__arg9_asub = (ctypes.c_int32*len(_arg9_asub))()
            _tmparr__arg9_asub[:] = _arg9_asub
            _arg9_asub_ = memoryview(_tmparr__arg9_asub)
      
      if _arg10_aval is None: raise TypeError("Invalid type for argument _arg10_aval")
      if _arg10_aval is None:
        _arg10_aval_ = None
      else:
        try:
          _arg10_aval_ = memoryview(_arg10_aval)
        except TypeError:
          try:
            _tmparr__arg10_aval = (ctypes.c_double*len(_arg10_aval))()
            _tmparr__arg10_aval[:] = _arg10_aval
          except TypeError:
            raise TypeError("Argument _arg10_aval has wrong type")
          else:
            _arg10_aval_ = memoryview(_tmparr__arg10_aval)
      
        else:
          if _arg10_aval_.format != "d":
            _tmparr__arg10_aval = (ctypes.c_double*len(_arg10_aval))()
            _tmparr__arg10_aval[:] = _arg10_aval
            _arg10_aval_ = memoryview(_tmparr__arg10_aval)
      
      if _arg11_bkc is None: raise TypeError("Invalid type for argument _arg11_bkc")
      if _arg11_bkc is None:
        _arg11_bkc_ = None
      else:
        try:
          _arg11_bkc_ = memoryview(_arg11_bkc)
        except TypeError:
          try:
            _tmparr__arg11_bkc = (ctypes.c_int*len(_arg11_bkc))()
            _tmparr__arg11_bkc[:] = _arg11_bkc
          except TypeError:
            raise TypeError("Argument _arg11_bkc has wrong type")
          else:
            _arg11_bkc_ = memoryview(_tmparr__arg11_bkc)
      
        else:
          if _arg11_bkc_.format != "i":
            _tmparr__arg11_bkc = (ctypes.c_int*len(_arg11_bkc))()
            _tmparr__arg11_bkc[:] = _arg11_bkc
            _arg11_bkc_ = memoryview(_tmparr__arg11_bkc)
      
      if _arg12_blc is None: raise TypeError("Invalid type for argument _arg12_blc")
      if _arg12_blc is None:
        _arg12_blc_ = None
      else:
        try:
          _arg12_blc_ = memoryview(_arg12_blc)
        except TypeError:
          try:
            _tmparr__arg12_blc = (ctypes.c_double*len(_arg12_blc))()
            _tmparr__arg12_blc[:] = _arg12_blc
          except TypeError:
            raise TypeError("Argument _arg12_blc has wrong type")
          else:
            _arg12_blc_ = memoryview(_tmparr__arg12_blc)
      
        else:
          if _arg12_blc_.format != "d":
            _tmparr__arg12_blc = (ctypes.c_double*len(_arg12_blc))()
            _tmparr__arg12_blc[:] = _arg12_blc
            _arg12_blc_ = memoryview(_tmparr__arg12_blc)
      
      if _arg13_buc is None: raise TypeError("Invalid type for argument _arg13_buc")
      if _arg13_buc is None:
        _arg13_buc_ = None
      else:
        try:
          _arg13_buc_ = memoryview(_arg13_buc)
        except TypeError:
          try:
            _tmparr__arg13_buc = (ctypes.c_double*len(_arg13_buc))()
            _tmparr__arg13_buc[:] = _arg13_buc
          except TypeError:
            raise TypeError("Argument _arg13_buc has wrong type")
          else:
            _arg13_buc_ = memoryview(_tmparr__arg13_buc)
      
        else:
          if _arg13_buc_.format != "d":
            _tmparr__arg13_buc = (ctypes.c_double*len(_arg13_buc))()
            _tmparr__arg13_buc[:] = _arg13_buc
            _arg13_buc_ = memoryview(_tmparr__arg13_buc)
      
      if _arg14_bkx is None: raise TypeError("Invalid type for argument _arg14_bkx")
      if _arg14_bkx is None:
        _arg14_bkx_ = None
      else:
        try:
          _arg14_bkx_ = memoryview(_arg14_bkx)
        except TypeError:
          try:
            _tmparr__arg14_bkx = (ctypes.c_int*len(_arg14_bkx))()
            _tmparr__arg14_bkx[:] = _arg14_bkx
          except TypeError:
            raise TypeError("Argument _arg14_bkx has wrong type")
          else:
            _arg14_bkx_ = memoryview(_tmparr__arg14_bkx)
      
        else:
          if _arg14_bkx_.format != "i":
            _tmparr__arg14_bkx = (ctypes.c_int*len(_arg14_bkx))()
            _tmparr__arg14_bkx[:] = _arg14_bkx
            _arg14_bkx_ = memoryview(_tmparr__arg14_bkx)
      
      if _arg15_blx is None: raise TypeError("Invalid type for argument _arg15_blx")
      if _arg15_blx is None:
        _arg15_blx_ = None
      else:
        try:
          _arg15_blx_ = memoryview(_arg15_blx)
        except TypeError:
          try:
            _tmparr__arg15_blx = (ctypes.c_double*len(_arg15_blx))()
            _tmparr__arg15_blx[:] = _arg15_blx
          except TypeError:
            raise TypeError("Argument _arg15_blx has wrong type")
          else:
            _arg15_blx_ = memoryview(_tmparr__arg15_blx)
      
        else:
          if _arg15_blx_.format != "d":
            _tmparr__arg15_blx = (ctypes.c_double*len(_arg15_blx))()
            _tmparr__arg15_blx[:] = _arg15_blx
            _arg15_blx_ = memoryview(_tmparr__arg15_blx)
      
      if _arg16_bux is None: raise TypeError("Invalid type for argument _arg16_bux")
      if _arg16_bux is None:
        _arg16_bux_ = None
      else:
        try:
          _arg16_bux_ = memoryview(_arg16_bux)
        except TypeError:
          try:
            _tmparr__arg16_bux = (ctypes.c_double*len(_arg16_bux))()
            _tmparr__arg16_bux[:] = _arg16_bux
          except TypeError:
            raise TypeError("Argument _arg16_bux has wrong type")
          else:
            _arg16_bux_ = memoryview(_tmparr__arg16_bux)
      
        else:
          if _arg16_bux_.format != "d":
            _tmparr__arg16_bux = (ctypes.c_double*len(_arg16_bux))()
            _tmparr__arg16_bux[:] = _arg16_bux
            _arg16_bux_ = memoryview(_tmparr__arg16_bux)
      
      res = self.__obj.inputdata64(_arg1_maxnumcon_,_arg2_maxnumvar_,_arg3_numcon_,_arg4_numvar_,_arg5_c_,_arg6_cfix_,_arg7_aptrb_,_arg8_aptre_,_arg9_asub_,_arg10_aval_,_arg11_bkc_,_arg12_blc_,_arg13_buc_,_arg14_bkx_,_arg15_blx_,_arg16_bux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def isdouparname(self,_arg1_parname_): # 2
      """
      Checks a double parameter name.
    
      isdouparname(self,_arg1_parname_)
        _arg1_parname: str. <no description>
      returns: _arg2_param
        _arg2_param: mosek.dparam. Returns the parameter corresponding to the name, if one exists.
      """
      res,resargs = self.__obj.isdouparname(_arg1_parname_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_param_return_value = resargs
      __arg2_param_return_value = dparam(__arg2_param_return_value)
      return __arg2_param_return_value
    
    def isintparname(self,_arg1_parname_): # 2
      """
      Checks an integer parameter name.
    
      isintparname(self,_arg1_parname_)
        _arg1_parname: str. <no description>
      returns: _arg2_param
        _arg2_param: mosek.iparam. Returns the parameter corresponding to the name, if one exists.
      """
      res,resargs = self.__obj.isintparname(_arg1_parname_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_param_return_value = resargs
      __arg2_param_return_value = iparam(__arg2_param_return_value)
      return __arg2_param_return_value
    
    def isstrparname(self,_arg1_parname_): # 2
      """
      Checks a string parameter name.
    
      isstrparname(self,_arg1_parname_)
        _arg1_parname: str. <no description>
      returns: _arg2_param
        _arg2_param: mosek.sparam. Returns the parameter corresponding to the name, if one exists.
      """
      res,resargs = self.__obj.isstrparname(_arg1_parname_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_param_return_value = resargs
      __arg2_param_return_value = sparam(__arg2_param_return_value)
      return __arg2_param_return_value
    
    def linkfiletostream(self,_arg1_whichstream_,_arg2_filename_,_arg3_append): # 2
      """
      Directs all output from a task stream to a file.
    
      linkfiletostream(self,_arg1_whichstream_,_arg2_filename_,_arg3_append)
        _arg1_whichstream: mosek.streamtype. <no description>
        _arg2_filename: str. <no description>
        _arg3_append: int. If this argument is 0 the output file will be overwritten, otherwise it will be appended to.
      """
      if not isinstance(_arg1_whichstream_,streamtype): raise TypeError("Argument _arg1_whichstream has wrong type")
      _arg1_whichstream = _arg1_whichstream_
      _arg3_append_ = _arg3_append
      res = self.__obj.linkfiletotaskstream(_arg1_whichstream_,_arg2_filename_,_arg3_append_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def primalrepair(self,_arg1_wlc,_arg2_wuc,_arg3_wlx,_arg4_wux): # 2
      """
      Repairs a primal infeasible optimization problem by adjusting the bounds on the constraints and variables.
    
      primalrepair(self,_arg1_wlc,_arg2_wuc,_arg3_wlx,_arg4_wux)
        _arg1_wlc: array of double. Weights associated with relaxing lower bounds on the constraints.
        _arg2_wuc: array of double. Weights associated with relaxing the upper bound on the constraints.
        _arg3_wlx: array of double. Weights associated with relaxing the lower bounds of the variables.
        _arg4_wux: array of double. Weights associated with relaxing the upper bounds of variables.
      """
      if _arg1_wlc is None:
        _arg1_wlc_ = None
      else:
        try:
          _arg1_wlc_ = memoryview(_arg1_wlc)
        except TypeError:
          try:
            _tmparr__arg1_wlc = (ctypes.c_double*len(_arg1_wlc))()
            _tmparr__arg1_wlc[:] = _arg1_wlc
          except TypeError:
            raise TypeError("Argument _arg1_wlc has wrong type")
          else:
            _arg1_wlc_ = memoryview(_tmparr__arg1_wlc)
      
        else:
          if _arg1_wlc_.format != "d":
            _tmparr__arg1_wlc = (ctypes.c_double*len(_arg1_wlc))()
            _tmparr__arg1_wlc[:] = _arg1_wlc
            _arg1_wlc_ = memoryview(_tmparr__arg1_wlc)
      
      if _arg1_wlc_ is not None and len(_arg1_wlc_) != self.getnumcon():
        raise ValueError("Array argument _arg1_wlc has wrong length")
      if _arg2_wuc is None:
        _arg2_wuc_ = None
      else:
        try:
          _arg2_wuc_ = memoryview(_arg2_wuc)
        except TypeError:
          try:
            _tmparr__arg2_wuc = (ctypes.c_double*len(_arg2_wuc))()
            _tmparr__arg2_wuc[:] = _arg2_wuc
          except TypeError:
            raise TypeError("Argument _arg2_wuc has wrong type")
          else:
            _arg2_wuc_ = memoryview(_tmparr__arg2_wuc)
      
        else:
          if _arg2_wuc_.format != "d":
            _tmparr__arg2_wuc = (ctypes.c_double*len(_arg2_wuc))()
            _tmparr__arg2_wuc[:] = _arg2_wuc
            _arg2_wuc_ = memoryview(_tmparr__arg2_wuc)
      
      if _arg2_wuc_ is not None and len(_arg2_wuc_) != self.getnumcon():
        raise ValueError("Array argument _arg2_wuc has wrong length")
      if _arg3_wlx is None:
        _arg3_wlx_ = None
      else:
        try:
          _arg3_wlx_ = memoryview(_arg3_wlx)
        except TypeError:
          try:
            _tmparr__arg3_wlx = (ctypes.c_double*len(_arg3_wlx))()
            _tmparr__arg3_wlx[:] = _arg3_wlx
          except TypeError:
            raise TypeError("Argument _arg3_wlx has wrong type")
          else:
            _arg3_wlx_ = memoryview(_tmparr__arg3_wlx)
      
        else:
          if _arg3_wlx_.format != "d":
            _tmparr__arg3_wlx = (ctypes.c_double*len(_arg3_wlx))()
            _tmparr__arg3_wlx[:] = _arg3_wlx
            _arg3_wlx_ = memoryview(_tmparr__arg3_wlx)
      
      if _arg3_wlx_ is not None and len(_arg3_wlx_) != self.getnumvar():
        raise ValueError("Array argument _arg3_wlx has wrong length")
      if _arg4_wux is None:
        _arg4_wux_ = None
      else:
        try:
          _arg4_wux_ = memoryview(_arg4_wux)
        except TypeError:
          try:
            _tmparr__arg4_wux = (ctypes.c_double*len(_arg4_wux))()
            _tmparr__arg4_wux[:] = _arg4_wux
          except TypeError:
            raise TypeError("Argument _arg4_wux has wrong type")
          else:
            _arg4_wux_ = memoryview(_tmparr__arg4_wux)
      
        else:
          if _arg4_wux_.format != "d":
            _tmparr__arg4_wux = (ctypes.c_double*len(_arg4_wux))()
            _tmparr__arg4_wux[:] = _arg4_wux
            _arg4_wux_ = memoryview(_tmparr__arg4_wux)
      
      if _arg4_wux_ is not None and len(_arg4_wux_) != self.getnumvar():
        raise ValueError("Array argument _arg4_wux has wrong length")
      res = self.__obj.primalrepair(_arg1_wlc_,_arg2_wuc_,_arg3_wlx_,_arg4_wux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def toconic(self): # 2
      """
      In-place reformulation of a QCQP to a COP
    
      toconic(self)
      """
      res = self.__obj.toconic()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def optimize(self): # 2
      """
      Optimizes the problem.
    
      optimize(self)
      returns: _arg1_trmcode
        _arg1_trmcode: mosek.rescode. Is either OK or a termination response code.
      """
      res,resargs = self.__obj.optimizetrm()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_trmcode_return_value = resargs
      __arg1_trmcode_return_value = rescode(__arg1_trmcode_return_value)
      return __arg1_trmcode_return_value
    
    def commitchanges(self): # 2
      """
      Commits all cached problem changes.
    
      commitchanges(self)
      """
      res = self.__obj.commitchanges()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getatruncatetol(self,_arg1_tolzero): # 2
      """
      Gets the current A matrix truncation threshold.
    
      getatruncatetol(self,_arg1_tolzero)
        _arg1_tolzero: array of double. Truncation tolerance.
      """
      if _arg1_tolzero is None: raise TypeError("Invalid type for argument _arg1_tolzero")
      _copyback__arg1_tolzero = False
      if _arg1_tolzero is None:
        _arg1_tolzero_ = None
      else:
        try:
          _arg1_tolzero_ = memoryview(_arg1_tolzero)
        except TypeError:
          try:
            _tmparr__arg1_tolzero = (ctypes.c_double*len(_arg1_tolzero))()
            _tmparr__arg1_tolzero[:] = _arg1_tolzero
          except TypeError:
            raise TypeError("Argument _arg1_tolzero has wrong type")
          else:
            _arg1_tolzero_ = memoryview(_tmparr__arg1_tolzero)
            _copyback__arg1_tolzero = True
        else:
          if _arg1_tolzero_.format != "d":
            _tmparr__arg1_tolzero = (ctypes.c_double*len(_arg1_tolzero))()
            _tmparr__arg1_tolzero[:] = _arg1_tolzero
            _arg1_tolzero_ = memoryview(_tmparr__arg1_tolzero)
            _copyback__arg1_tolzero = True
      if _arg1_tolzero_ is not None and len(_arg1_tolzero_) != 1:
        raise ValueError("Array argument _arg1_tolzero has wrong length")
      res = self.__obj.getatruncatetol(_arg1_tolzero_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg1_tolzero:
        _arg1_tolzero[:] = _tmparr__arg1_tolzero
    
    def putatruncatetol(self,_arg1_tolzero): # 2
      """
      Truncates all elements in A below a certain tolerance to zero.
    
      putatruncatetol(self,_arg1_tolzero)
        _arg1_tolzero: double. Truncation tolerance.
      """
      _arg1_tolzero_ = _arg1_tolzero
      res = self.__obj.putatruncatetol(_arg1_tolzero_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putaij(self,_arg1_i,_arg2_j,_arg3_aij): # 2
      """
      Changes a single value in the linear coefficient matrix.
    
      putaij(self,_arg1_i,_arg2_j,_arg3_aij)
        _arg1_i: int. Constraint (row) index.
        _arg2_j: int. Variable (column) index.
        _arg3_aij: double. New coefficient.
      """
      _arg1_i_ = _arg1_i
      _arg2_j_ = _arg2_j
      _arg3_aij_ = _arg3_aij
      res = self.__obj.putaij(_arg1_i_,_arg2_j_,_arg3_aij_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putaijlist(self,_arg2_subi,_arg3_subj,_arg4_valij): # 2
      """
      Changes one or more coefficients in the linear constraint matrix.
    
      putaijlist(self,_arg2_subi,_arg3_subj,_arg4_valij)
        _arg2_subi: array of int. Constraint (row) indices.
        _arg3_subj: array of int. Variable (column) indices.
        _arg4_valij: array of double. New coefficient values.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subi)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subi):
        raise IndexError("Inconsistent length of array _arg2_subi")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_subj)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_subj):
        raise IndexError("Inconsistent length of array _arg3_subj")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg4_valij)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg4_valij):
        raise IndexError("Inconsistent length of array _arg4_valij")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subi is None: raise TypeError("Invalid type for argument _arg2_subi")
      if _arg2_subi is None:
        _arg2_subi_ = None
      else:
        try:
          _arg2_subi_ = memoryview(_arg2_subi)
        except TypeError:
          try:
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
          except TypeError:
            raise TypeError("Argument _arg2_subi has wrong type")
          else:
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
        else:
          if _arg2_subi_.format != "i":
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
      if _arg3_subj is None: raise TypeError("Invalid type for argument _arg3_subj")
      if _arg3_subj is None:
        _arg3_subj_ = None
      else:
        try:
          _arg3_subj_ = memoryview(_arg3_subj)
        except TypeError:
          try:
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
          except TypeError:
            raise TypeError("Argument _arg3_subj has wrong type")
          else:
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
      
        else:
          if _arg3_subj_.format != "i":
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
      
      if _arg4_valij is None: raise TypeError("Invalid type for argument _arg4_valij")
      if _arg4_valij is None:
        _arg4_valij_ = None
      else:
        try:
          _arg4_valij_ = memoryview(_arg4_valij)
        except TypeError:
          try:
            _tmparr__arg4_valij = (ctypes.c_double*len(_arg4_valij))()
            _tmparr__arg4_valij[:] = _arg4_valij
          except TypeError:
            raise TypeError("Argument _arg4_valij has wrong type")
          else:
            _arg4_valij_ = memoryview(_tmparr__arg4_valij)
      
        else:
          if _arg4_valij_.format != "d":
            _tmparr__arg4_valij = (ctypes.c_double*len(_arg4_valij))()
            _tmparr__arg4_valij[:] = _arg4_valij
            _arg4_valij_ = memoryview(_tmparr__arg4_valij)
      
      res = self.__obj.putaijlist64(_arg1_num_,_arg2_subi_,_arg3_subj_,_arg4_valij_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putacol(self,_arg1_j,_arg3_subj,_arg4_valj): # 2
      """
      Replaces all elements in one column of the linear constraint matrix.
    
      putacol(self,_arg1_j,_arg3_subj,_arg4_valj)
        _arg1_j: int. Column index.
        _arg3_subj: array of int. Row indexes of non-zero values in column.
        _arg4_valj: array of double. New non-zero values of column.
      """
      _arg1_j_ = _arg1_j
      _arg2_nzj_ = None
      if _arg2_nzj_ is None:
        _arg2_nzj_ = len(_arg3_subj)
        _arg2_nzj = _arg2_nzj_
      elif _arg2_nzj_ != len(_arg3_subj):
        raise IndexError("Inconsistent length of array _arg3_subj")
      if _arg2_nzj_ is None:
        _arg2_nzj_ = len(_arg4_valj)
        _arg2_nzj = _arg2_nzj_
      elif _arg2_nzj_ != len(_arg4_valj):
        raise IndexError("Inconsistent length of array _arg4_valj")
      if _arg2_nzj_ is None: _arg2_nzj_ = 0
      if _arg3_subj is None: raise TypeError("Invalid type for argument _arg3_subj")
      if _arg3_subj is None:
        _arg3_subj_ = None
      else:
        try:
          _arg3_subj_ = memoryview(_arg3_subj)
        except TypeError:
          try:
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
          except TypeError:
            raise TypeError("Argument _arg3_subj has wrong type")
          else:
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
      
        else:
          if _arg3_subj_.format != "i":
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
      
      if _arg4_valj is None: raise TypeError("Invalid type for argument _arg4_valj")
      if _arg4_valj is None:
        _arg4_valj_ = None
      else:
        try:
          _arg4_valj_ = memoryview(_arg4_valj)
        except TypeError:
          try:
            _tmparr__arg4_valj = (ctypes.c_double*len(_arg4_valj))()
            _tmparr__arg4_valj[:] = _arg4_valj
          except TypeError:
            raise TypeError("Argument _arg4_valj has wrong type")
          else:
            _arg4_valj_ = memoryview(_tmparr__arg4_valj)
      
        else:
          if _arg4_valj_.format != "d":
            _tmparr__arg4_valj = (ctypes.c_double*len(_arg4_valj))()
            _tmparr__arg4_valj[:] = _arg4_valj
            _arg4_valj_ = memoryview(_tmparr__arg4_valj)
      
      res = self.__obj.putacol(_arg1_j_,_arg2_nzj_,_arg3_subj_,_arg4_valj_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putarow(self,_arg1_i,_arg3_subi,_arg4_vali): # 2
      """
      Replaces all elements in one row of the linear constraint matrix.
    
      putarow(self,_arg1_i,_arg3_subi,_arg4_vali)
        _arg1_i: int. Row index.
        _arg3_subi: array of int. Column indexes of non-zero values in row.
        _arg4_vali: array of double. New non-zero values of row.
      """
      _arg1_i_ = _arg1_i
      _arg2_nzi_ = None
      if _arg2_nzi_ is None:
        _arg2_nzi_ = len(_arg3_subi)
        _arg2_nzi = _arg2_nzi_
      elif _arg2_nzi_ != len(_arg3_subi):
        raise IndexError("Inconsistent length of array _arg3_subi")
      if _arg2_nzi_ is None:
        _arg2_nzi_ = len(_arg4_vali)
        _arg2_nzi = _arg2_nzi_
      elif _arg2_nzi_ != len(_arg4_vali):
        raise IndexError("Inconsistent length of array _arg4_vali")
      if _arg2_nzi_ is None: _arg2_nzi_ = 0
      if _arg3_subi is None: raise TypeError("Invalid type for argument _arg3_subi")
      if _arg3_subi is None:
        _arg3_subi_ = None
      else:
        try:
          _arg3_subi_ = memoryview(_arg3_subi)
        except TypeError:
          try:
            _tmparr__arg3_subi = (ctypes.c_int32*len(_arg3_subi))()
            _tmparr__arg3_subi[:] = _arg3_subi
          except TypeError:
            raise TypeError("Argument _arg3_subi has wrong type")
          else:
            _arg3_subi_ = memoryview(_tmparr__arg3_subi)
      
        else:
          if _arg3_subi_.format != "i":
            _tmparr__arg3_subi = (ctypes.c_int32*len(_arg3_subi))()
            _tmparr__arg3_subi[:] = _arg3_subi
            _arg3_subi_ = memoryview(_tmparr__arg3_subi)
      
      if _arg4_vali is None: raise TypeError("Invalid type for argument _arg4_vali")
      if _arg4_vali is None:
        _arg4_vali_ = None
      else:
        try:
          _arg4_vali_ = memoryview(_arg4_vali)
        except TypeError:
          try:
            _tmparr__arg4_vali = (ctypes.c_double*len(_arg4_vali))()
            _tmparr__arg4_vali[:] = _arg4_vali
          except TypeError:
            raise TypeError("Argument _arg4_vali has wrong type")
          else:
            _arg4_vali_ = memoryview(_tmparr__arg4_vali)
      
        else:
          if _arg4_vali_.format != "d":
            _tmparr__arg4_vali = (ctypes.c_double*len(_arg4_vali))()
            _tmparr__arg4_vali[:] = _arg4_vali
            _arg4_vali_ = memoryview(_tmparr__arg4_vali)
      
      res = self.__obj.putarow(_arg1_i_,_arg2_nzi_,_arg3_subi_,_arg4_vali_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putarowslice(self,_arg1_first,_arg2_last,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval): # 2
      """
      Replaces all elements in several rows the linear constraint matrix.
    
      putarowslice(self,_arg1_first,_arg2_last,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval)
        _arg1_first: int. First row in the slice.
        _arg2_last: int. Last row plus one in the slice.
        _arg3_ptrb: array of long. Array of pointers to the first element in the rows.
        _arg4_ptre: array of long. Array of pointers to the last element plus one in the rows.
        _arg5_asub: array of int. Column indexes of new elements.
        _arg6_aval: array of double. <no description>
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      if _arg3_ptrb is None: raise TypeError("Invalid type for argument _arg3_ptrb")
      if _arg3_ptrb is None:
        _arg3_ptrb_ = None
      else:
        try:
          _arg3_ptrb_ = memoryview(_arg3_ptrb)
        except TypeError:
          try:
            _tmparr__arg3_ptrb = (ctypes.c_int64*len(_arg3_ptrb))()
            _tmparr__arg3_ptrb[:] = _arg3_ptrb
          except TypeError:
            raise TypeError("Argument _arg3_ptrb has wrong type")
          else:
            _arg3_ptrb_ = memoryview(_tmparr__arg3_ptrb)
      
        else:
          if _arg3_ptrb_.format != "q":
            _tmparr__arg3_ptrb = (ctypes.c_int64*len(_arg3_ptrb))()
            _tmparr__arg3_ptrb[:] = _arg3_ptrb
            _arg3_ptrb_ = memoryview(_tmparr__arg3_ptrb)
      
      if _arg3_ptrb_ is not None and len(_arg3_ptrb_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg3_ptrb has wrong length")
      if _arg4_ptre is None: raise TypeError("Invalid type for argument _arg4_ptre")
      if _arg4_ptre is None:
        _arg4_ptre_ = None
      else:
        try:
          _arg4_ptre_ = memoryview(_arg4_ptre)
        except TypeError:
          try:
            _tmparr__arg4_ptre = (ctypes.c_int64*len(_arg4_ptre))()
            _tmparr__arg4_ptre[:] = _arg4_ptre
          except TypeError:
            raise TypeError("Argument _arg4_ptre has wrong type")
          else:
            _arg4_ptre_ = memoryview(_tmparr__arg4_ptre)
      
        else:
          if _arg4_ptre_.format != "q":
            _tmparr__arg4_ptre = (ctypes.c_int64*len(_arg4_ptre))()
            _tmparr__arg4_ptre[:] = _arg4_ptre
            _arg4_ptre_ = memoryview(_tmparr__arg4_ptre)
      
      if _arg4_ptre_ is not None and len(_arg4_ptre_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg4_ptre has wrong length")
      if _arg5_asub is None: raise TypeError("Invalid type for argument _arg5_asub")
      if _arg5_asub is None:
        _arg5_asub_ = None
      else:
        try:
          _arg5_asub_ = memoryview(_arg5_asub)
        except TypeError:
          try:
            _tmparr__arg5_asub = (ctypes.c_int32*len(_arg5_asub))()
            _tmparr__arg5_asub[:] = _arg5_asub
          except TypeError:
            raise TypeError("Argument _arg5_asub has wrong type")
          else:
            _arg5_asub_ = memoryview(_tmparr__arg5_asub)
      
        else:
          if _arg5_asub_.format != "i":
            _tmparr__arg5_asub = (ctypes.c_int32*len(_arg5_asub))()
            _tmparr__arg5_asub[:] = _arg5_asub
            _arg5_asub_ = memoryview(_tmparr__arg5_asub)
      
      if _arg6_aval is None: raise TypeError("Invalid type for argument _arg6_aval")
      if _arg6_aval is None:
        _arg6_aval_ = None
      else:
        try:
          _arg6_aval_ = memoryview(_arg6_aval)
        except TypeError:
          try:
            _tmparr__arg6_aval = (ctypes.c_double*len(_arg6_aval))()
            _tmparr__arg6_aval[:] = _arg6_aval
          except TypeError:
            raise TypeError("Argument _arg6_aval has wrong type")
          else:
            _arg6_aval_ = memoryview(_tmparr__arg6_aval)
      
        else:
          if _arg6_aval_.format != "d":
            _tmparr__arg6_aval = (ctypes.c_double*len(_arg6_aval))()
            _tmparr__arg6_aval[:] = _arg6_aval
            _arg6_aval_ = memoryview(_tmparr__arg6_aval)
      
      res = self.__obj.putarowslice64(_arg1_first_,_arg2_last_,_arg3_ptrb_,_arg4_ptre_,_arg5_asub_,_arg6_aval_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putarowlist(self,_arg2_sub,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval): # 2
      """
      Replaces all elements in several rows of the linear constraint matrix.
    
      putarowlist(self,_arg2_sub,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval)
        _arg2_sub: array of int. Indexes of rows or columns that should be replaced.
        _arg3_ptrb: array of long. Array of pointers to the first element in the rows.
        _arg4_ptre: array of long. Array of pointers to the last element plus one in the rows.
        _arg5_asub: array of int. Variable indexes.
        _arg6_aval: array of double. <no description>
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_sub)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_sub):
        raise IndexError("Inconsistent length of array _arg2_sub")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_ptrb)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_ptrb):
        raise IndexError("Inconsistent length of array _arg3_ptrb")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg4_ptre)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg4_ptre):
        raise IndexError("Inconsistent length of array _arg4_ptre")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_sub is None: raise TypeError("Invalid type for argument _arg2_sub")
      if _arg2_sub is None:
        _arg2_sub_ = None
      else:
        try:
          _arg2_sub_ = memoryview(_arg2_sub)
        except TypeError:
          try:
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
          except TypeError:
            raise TypeError("Argument _arg2_sub has wrong type")
          else:
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
        else:
          if _arg2_sub_.format != "i":
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
      if _arg3_ptrb is None: raise TypeError("Invalid type for argument _arg3_ptrb")
      if _arg3_ptrb is None:
        _arg3_ptrb_ = None
      else:
        try:
          _arg3_ptrb_ = memoryview(_arg3_ptrb)
        except TypeError:
          try:
            _tmparr__arg3_ptrb = (ctypes.c_int64*len(_arg3_ptrb))()
            _tmparr__arg3_ptrb[:] = _arg3_ptrb
          except TypeError:
            raise TypeError("Argument _arg3_ptrb has wrong type")
          else:
            _arg3_ptrb_ = memoryview(_tmparr__arg3_ptrb)
      
        else:
          if _arg3_ptrb_.format != "q":
            _tmparr__arg3_ptrb = (ctypes.c_int64*len(_arg3_ptrb))()
            _tmparr__arg3_ptrb[:] = _arg3_ptrb
            _arg3_ptrb_ = memoryview(_tmparr__arg3_ptrb)
      
      if _arg4_ptre is None: raise TypeError("Invalid type for argument _arg4_ptre")
      if _arg4_ptre is None:
        _arg4_ptre_ = None
      else:
        try:
          _arg4_ptre_ = memoryview(_arg4_ptre)
        except TypeError:
          try:
            _tmparr__arg4_ptre = (ctypes.c_int64*len(_arg4_ptre))()
            _tmparr__arg4_ptre[:] = _arg4_ptre
          except TypeError:
            raise TypeError("Argument _arg4_ptre has wrong type")
          else:
            _arg4_ptre_ = memoryview(_tmparr__arg4_ptre)
      
        else:
          if _arg4_ptre_.format != "q":
            _tmparr__arg4_ptre = (ctypes.c_int64*len(_arg4_ptre))()
            _tmparr__arg4_ptre[:] = _arg4_ptre
            _arg4_ptre_ = memoryview(_tmparr__arg4_ptre)
      
      if _arg5_asub is None: raise TypeError("Invalid type for argument _arg5_asub")
      if _arg5_asub is None:
        _arg5_asub_ = None
      else:
        try:
          _arg5_asub_ = memoryview(_arg5_asub)
        except TypeError:
          try:
            _tmparr__arg5_asub = (ctypes.c_int32*len(_arg5_asub))()
            _tmparr__arg5_asub[:] = _arg5_asub
          except TypeError:
            raise TypeError("Argument _arg5_asub has wrong type")
          else:
            _arg5_asub_ = memoryview(_tmparr__arg5_asub)
      
        else:
          if _arg5_asub_.format != "i":
            _tmparr__arg5_asub = (ctypes.c_int32*len(_arg5_asub))()
            _tmparr__arg5_asub[:] = _arg5_asub
            _arg5_asub_ = memoryview(_tmparr__arg5_asub)
      
      if _arg6_aval is None: raise TypeError("Invalid type for argument _arg6_aval")
      if _arg6_aval is None:
        _arg6_aval_ = None
      else:
        try:
          _arg6_aval_ = memoryview(_arg6_aval)
        except TypeError:
          try:
            _tmparr__arg6_aval = (ctypes.c_double*len(_arg6_aval))()
            _tmparr__arg6_aval[:] = _arg6_aval
          except TypeError:
            raise TypeError("Argument _arg6_aval has wrong type")
          else:
            _arg6_aval_ = memoryview(_tmparr__arg6_aval)
      
        else:
          if _arg6_aval_.format != "d":
            _tmparr__arg6_aval = (ctypes.c_double*len(_arg6_aval))()
            _tmparr__arg6_aval[:] = _arg6_aval
            _arg6_aval_ = memoryview(_tmparr__arg6_aval)
      
      res = self.__obj.putarowlist64(_arg1_num_,_arg2_sub_,_arg3_ptrb_,_arg4_ptre_,_arg5_asub_,_arg6_aval_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putacolslice(self,_arg1_first,_arg2_last,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval): # 2
      """
      Replaces all elements in a sequence of columns the linear constraint matrix.
    
      putacolslice(self,_arg1_first,_arg2_last,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval)
        _arg1_first: int. First column in the slice.
        _arg2_last: int. Last column plus one in the slice.
        _arg3_ptrb: array of long. Array of pointers to the first element in the columns.
        _arg4_ptre: array of long. Array of pointers to the last element plus one in the columns.
        _arg5_asub: array of int. Row indexes
        _arg6_aval: array of double. <no description>
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      if _arg3_ptrb is None: raise TypeError("Invalid type for argument _arg3_ptrb")
      if _arg3_ptrb is None:
        _arg3_ptrb_ = None
      else:
        try:
          _arg3_ptrb_ = memoryview(_arg3_ptrb)
        except TypeError:
          try:
            _tmparr__arg3_ptrb = (ctypes.c_int64*len(_arg3_ptrb))()
            _tmparr__arg3_ptrb[:] = _arg3_ptrb
          except TypeError:
            raise TypeError("Argument _arg3_ptrb has wrong type")
          else:
            _arg3_ptrb_ = memoryview(_tmparr__arg3_ptrb)
      
        else:
          if _arg3_ptrb_.format != "q":
            _tmparr__arg3_ptrb = (ctypes.c_int64*len(_arg3_ptrb))()
            _tmparr__arg3_ptrb[:] = _arg3_ptrb
            _arg3_ptrb_ = memoryview(_tmparr__arg3_ptrb)
      
      if _arg4_ptre is None: raise TypeError("Invalid type for argument _arg4_ptre")
      if _arg4_ptre is None:
        _arg4_ptre_ = None
      else:
        try:
          _arg4_ptre_ = memoryview(_arg4_ptre)
        except TypeError:
          try:
            _tmparr__arg4_ptre = (ctypes.c_int64*len(_arg4_ptre))()
            _tmparr__arg4_ptre[:] = _arg4_ptre
          except TypeError:
            raise TypeError("Argument _arg4_ptre has wrong type")
          else:
            _arg4_ptre_ = memoryview(_tmparr__arg4_ptre)
      
        else:
          if _arg4_ptre_.format != "q":
            _tmparr__arg4_ptre = (ctypes.c_int64*len(_arg4_ptre))()
            _tmparr__arg4_ptre[:] = _arg4_ptre
            _arg4_ptre_ = memoryview(_tmparr__arg4_ptre)
      
      if _arg5_asub is None: raise TypeError("Invalid type for argument _arg5_asub")
      if _arg5_asub is None:
        _arg5_asub_ = None
      else:
        try:
          _arg5_asub_ = memoryview(_arg5_asub)
        except TypeError:
          try:
            _tmparr__arg5_asub = (ctypes.c_int32*len(_arg5_asub))()
            _tmparr__arg5_asub[:] = _arg5_asub
          except TypeError:
            raise TypeError("Argument _arg5_asub has wrong type")
          else:
            _arg5_asub_ = memoryview(_tmparr__arg5_asub)
      
        else:
          if _arg5_asub_.format != "i":
            _tmparr__arg5_asub = (ctypes.c_int32*len(_arg5_asub))()
            _tmparr__arg5_asub[:] = _arg5_asub
            _arg5_asub_ = memoryview(_tmparr__arg5_asub)
      
      if _arg6_aval is None: raise TypeError("Invalid type for argument _arg6_aval")
      if _arg6_aval is None:
        _arg6_aval_ = None
      else:
        try:
          _arg6_aval_ = memoryview(_arg6_aval)
        except TypeError:
          try:
            _tmparr__arg6_aval = (ctypes.c_double*len(_arg6_aval))()
            _tmparr__arg6_aval[:] = _arg6_aval
          except TypeError:
            raise TypeError("Argument _arg6_aval has wrong type")
          else:
            _arg6_aval_ = memoryview(_tmparr__arg6_aval)
      
        else:
          if _arg6_aval_.format != "d":
            _tmparr__arg6_aval = (ctypes.c_double*len(_arg6_aval))()
            _tmparr__arg6_aval[:] = _arg6_aval
            _arg6_aval_ = memoryview(_tmparr__arg6_aval)
      
      res = self.__obj.putacolslice64(_arg1_first_,_arg2_last_,_arg3_ptrb_,_arg4_ptre_,_arg5_asub_,_arg6_aval_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putacollist(self,_arg2_sub,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval): # 2
      """
      Replaces all elements in several columns the linear constraint matrix.
    
      putacollist(self,_arg2_sub,_arg3_ptrb,_arg4_ptre,_arg5_asub,_arg6_aval)
        _arg2_sub: array of int. Indexes of columns that should be replaced.
        _arg3_ptrb: array of long. Array of pointers to the first element in the columns.
        _arg4_ptre: array of long. Array of pointers to the last element plus one in the columns.
        _arg5_asub: array of int. Row indexes
        _arg6_aval: array of double. <no description>
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_sub)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_sub):
        raise IndexError("Inconsistent length of array _arg2_sub")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_ptrb)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_ptrb):
        raise IndexError("Inconsistent length of array _arg3_ptrb")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg4_ptre)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg4_ptre):
        raise IndexError("Inconsistent length of array _arg4_ptre")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_sub is None: raise TypeError("Invalid type for argument _arg2_sub")
      if _arg2_sub is None:
        _arg2_sub_ = None
      else:
        try:
          _arg2_sub_ = memoryview(_arg2_sub)
        except TypeError:
          try:
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
          except TypeError:
            raise TypeError("Argument _arg2_sub has wrong type")
          else:
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
        else:
          if _arg2_sub_.format != "i":
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
      if _arg3_ptrb is None: raise TypeError("Invalid type for argument _arg3_ptrb")
      if _arg3_ptrb is None:
        _arg3_ptrb_ = None
      else:
        try:
          _arg3_ptrb_ = memoryview(_arg3_ptrb)
        except TypeError:
          try:
            _tmparr__arg3_ptrb = (ctypes.c_int64*len(_arg3_ptrb))()
            _tmparr__arg3_ptrb[:] = _arg3_ptrb
          except TypeError:
            raise TypeError("Argument _arg3_ptrb has wrong type")
          else:
            _arg3_ptrb_ = memoryview(_tmparr__arg3_ptrb)
      
        else:
          if _arg3_ptrb_.format != "q":
            _tmparr__arg3_ptrb = (ctypes.c_int64*len(_arg3_ptrb))()
            _tmparr__arg3_ptrb[:] = _arg3_ptrb
            _arg3_ptrb_ = memoryview(_tmparr__arg3_ptrb)
      
      if _arg4_ptre is None: raise TypeError("Invalid type for argument _arg4_ptre")
      if _arg4_ptre is None:
        _arg4_ptre_ = None
      else:
        try:
          _arg4_ptre_ = memoryview(_arg4_ptre)
        except TypeError:
          try:
            _tmparr__arg4_ptre = (ctypes.c_int64*len(_arg4_ptre))()
            _tmparr__arg4_ptre[:] = _arg4_ptre
          except TypeError:
            raise TypeError("Argument _arg4_ptre has wrong type")
          else:
            _arg4_ptre_ = memoryview(_tmparr__arg4_ptre)
      
        else:
          if _arg4_ptre_.format != "q":
            _tmparr__arg4_ptre = (ctypes.c_int64*len(_arg4_ptre))()
            _tmparr__arg4_ptre[:] = _arg4_ptre
            _arg4_ptre_ = memoryview(_tmparr__arg4_ptre)
      
      if _arg5_asub is None: raise TypeError("Invalid type for argument _arg5_asub")
      if _arg5_asub is None:
        _arg5_asub_ = None
      else:
        try:
          _arg5_asub_ = memoryview(_arg5_asub)
        except TypeError:
          try:
            _tmparr__arg5_asub = (ctypes.c_int32*len(_arg5_asub))()
            _tmparr__arg5_asub[:] = _arg5_asub
          except TypeError:
            raise TypeError("Argument _arg5_asub has wrong type")
          else:
            _arg5_asub_ = memoryview(_tmparr__arg5_asub)
      
        else:
          if _arg5_asub_.format != "i":
            _tmparr__arg5_asub = (ctypes.c_int32*len(_arg5_asub))()
            _tmparr__arg5_asub[:] = _arg5_asub
            _arg5_asub_ = memoryview(_tmparr__arg5_asub)
      
      if _arg6_aval is None: raise TypeError("Invalid type for argument _arg6_aval")
      if _arg6_aval is None:
        _arg6_aval_ = None
      else:
        try:
          _arg6_aval_ = memoryview(_arg6_aval)
        except TypeError:
          try:
            _tmparr__arg6_aval = (ctypes.c_double*len(_arg6_aval))()
            _tmparr__arg6_aval[:] = _arg6_aval
          except TypeError:
            raise TypeError("Argument _arg6_aval has wrong type")
          else:
            _arg6_aval_ = memoryview(_tmparr__arg6_aval)
      
        else:
          if _arg6_aval_.format != "d":
            _tmparr__arg6_aval = (ctypes.c_double*len(_arg6_aval))()
            _tmparr__arg6_aval[:] = _arg6_aval
            _arg6_aval_ = memoryview(_tmparr__arg6_aval)
      
      res = self.__obj.putacollist64(_arg1_num_,_arg2_sub_,_arg3_ptrb_,_arg4_ptre_,_arg5_asub_,_arg6_aval_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putbaraij(self,_arg1_i,_arg2_j,_arg4_sub,_arg5_weights): # 2
      """
      Inputs an element of barA.
    
      putbaraij(self,_arg1_i,_arg2_j,_arg4_sub,_arg5_weights)
        _arg1_i: int. Row index of barA.
        _arg2_j: int. Column index of barA.
        _arg4_sub: array of long. Element indexes in matrix storage.
        _arg5_weights: array of double. Weights in the weighted sum.
      """
      _arg1_i_ = _arg1_i
      _arg2_j_ = _arg2_j
      _arg3_num_ = None
      if _arg3_num_ is None:
        _arg3_num_ = len(_arg4_sub)
        _arg3_num = _arg3_num_
      elif _arg3_num_ != len(_arg4_sub):
        raise IndexError("Inconsistent length of array _arg4_sub")
      if _arg3_num_ is None:
        _arg3_num_ = len(_arg5_weights)
        _arg3_num = _arg3_num_
      elif _arg3_num_ != len(_arg5_weights):
        raise IndexError("Inconsistent length of array _arg5_weights")
      if _arg3_num_ is None: _arg3_num_ = 0
      if _arg4_sub is None: raise TypeError("Invalid type for argument _arg4_sub")
      if _arg4_sub is None:
        _arg4_sub_ = None
      else:
        try:
          _arg4_sub_ = memoryview(_arg4_sub)
        except TypeError:
          try:
            _tmparr__arg4_sub = (ctypes.c_int64*len(_arg4_sub))()
            _tmparr__arg4_sub[:] = _arg4_sub
          except TypeError:
            raise TypeError("Argument _arg4_sub has wrong type")
          else:
            _arg4_sub_ = memoryview(_tmparr__arg4_sub)
      
        else:
          if _arg4_sub_.format != "q":
            _tmparr__arg4_sub = (ctypes.c_int64*len(_arg4_sub))()
            _tmparr__arg4_sub[:] = _arg4_sub
            _arg4_sub_ = memoryview(_tmparr__arg4_sub)
      
      if _arg5_weights is None: raise TypeError("Invalid type for argument _arg5_weights")
      if _arg5_weights is None:
        _arg5_weights_ = None
      else:
        try:
          _arg5_weights_ = memoryview(_arg5_weights)
        except TypeError:
          try:
            _tmparr__arg5_weights = (ctypes.c_double*len(_arg5_weights))()
            _tmparr__arg5_weights[:] = _arg5_weights
          except TypeError:
            raise TypeError("Argument _arg5_weights has wrong type")
          else:
            _arg5_weights_ = memoryview(_tmparr__arg5_weights)
      
        else:
          if _arg5_weights_.format != "d":
            _tmparr__arg5_weights = (ctypes.c_double*len(_arg5_weights))()
            _tmparr__arg5_weights[:] = _arg5_weights
            _arg5_weights_ = memoryview(_tmparr__arg5_weights)
      
      res = self.__obj.putbaraij(_arg1_i_,_arg2_j_,_arg3_num_,_arg4_sub_,_arg5_weights_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putbaraijlist(self,_arg2_subi,_arg3_subj,_arg4_alphaptrb,_arg5_alphaptre,_arg6_matidx,_arg7_weights): # 2
      """
      Inputs list of elements of barA.
    
      putbaraijlist(self,_arg2_subi,_arg3_subj,_arg4_alphaptrb,_arg5_alphaptre,_arg6_matidx,_arg7_weights)
        _arg2_subi: array of int. Row index of barA.
        _arg3_subj: array of int. Column index of barA.
        _arg4_alphaptrb: array of long. Start entries for terms in the weighted sum.
        _arg5_alphaptre: array of long. End entries for terms in the weighted sum.
        _arg6_matidx: array of long. Element indexes in matrix storage.
        _arg7_weights: array of double. Weights in the weighted sum.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subi)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subi):
        raise IndexError("Inconsistent length of array _arg2_subi")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_subj)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_subj):
        raise IndexError("Inconsistent length of array _arg3_subj")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg4_alphaptrb)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg4_alphaptrb):
        raise IndexError("Inconsistent length of array _arg4_alphaptrb")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg5_alphaptre)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg5_alphaptre):
        raise IndexError("Inconsistent length of array _arg5_alphaptre")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subi is None: raise TypeError("Invalid type for argument _arg2_subi")
      if _arg2_subi is None:
        _arg2_subi_ = None
      else:
        try:
          _arg2_subi_ = memoryview(_arg2_subi)
        except TypeError:
          try:
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
          except TypeError:
            raise TypeError("Argument _arg2_subi has wrong type")
          else:
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
        else:
          if _arg2_subi_.format != "i":
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
      if _arg3_subj is None: raise TypeError("Invalid type for argument _arg3_subj")
      if _arg3_subj is None:
        _arg3_subj_ = None
      else:
        try:
          _arg3_subj_ = memoryview(_arg3_subj)
        except TypeError:
          try:
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
          except TypeError:
            raise TypeError("Argument _arg3_subj has wrong type")
          else:
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
      
        else:
          if _arg3_subj_.format != "i":
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
      
      if _arg4_alphaptrb is None: raise TypeError("Invalid type for argument _arg4_alphaptrb")
      if _arg4_alphaptrb is None:
        _arg4_alphaptrb_ = None
      else:
        try:
          _arg4_alphaptrb_ = memoryview(_arg4_alphaptrb)
        except TypeError:
          try:
            _tmparr__arg4_alphaptrb = (ctypes.c_int64*len(_arg4_alphaptrb))()
            _tmparr__arg4_alphaptrb[:] = _arg4_alphaptrb
          except TypeError:
            raise TypeError("Argument _arg4_alphaptrb has wrong type")
          else:
            _arg4_alphaptrb_ = memoryview(_tmparr__arg4_alphaptrb)
      
        else:
          if _arg4_alphaptrb_.format != "q":
            _tmparr__arg4_alphaptrb = (ctypes.c_int64*len(_arg4_alphaptrb))()
            _tmparr__arg4_alphaptrb[:] = _arg4_alphaptrb
            _arg4_alphaptrb_ = memoryview(_tmparr__arg4_alphaptrb)
      
      if _arg5_alphaptre is None: raise TypeError("Invalid type for argument _arg5_alphaptre")
      if _arg5_alphaptre is None:
        _arg5_alphaptre_ = None
      else:
        try:
          _arg5_alphaptre_ = memoryview(_arg5_alphaptre)
        except TypeError:
          try:
            _tmparr__arg5_alphaptre = (ctypes.c_int64*len(_arg5_alphaptre))()
            _tmparr__arg5_alphaptre[:] = _arg5_alphaptre
          except TypeError:
            raise TypeError("Argument _arg5_alphaptre has wrong type")
          else:
            _arg5_alphaptre_ = memoryview(_tmparr__arg5_alphaptre)
      
        else:
          if _arg5_alphaptre_.format != "q":
            _tmparr__arg5_alphaptre = (ctypes.c_int64*len(_arg5_alphaptre))()
            _tmparr__arg5_alphaptre[:] = _arg5_alphaptre
            _arg5_alphaptre_ = memoryview(_tmparr__arg5_alphaptre)
      
      if _arg6_matidx is None: raise TypeError("Invalid type for argument _arg6_matidx")
      if _arg6_matidx is None:
        _arg6_matidx_ = None
      else:
        try:
          _arg6_matidx_ = memoryview(_arg6_matidx)
        except TypeError:
          try:
            _tmparr__arg6_matidx = (ctypes.c_int64*len(_arg6_matidx))()
            _tmparr__arg6_matidx[:] = _arg6_matidx
          except TypeError:
            raise TypeError("Argument _arg6_matidx has wrong type")
          else:
            _arg6_matidx_ = memoryview(_tmparr__arg6_matidx)
      
        else:
          if _arg6_matidx_.format != "q":
            _tmparr__arg6_matidx = (ctypes.c_int64*len(_arg6_matidx))()
            _tmparr__arg6_matidx[:] = _arg6_matidx
            _arg6_matidx_ = memoryview(_tmparr__arg6_matidx)
      
      if _arg7_weights is None: raise TypeError("Invalid type for argument _arg7_weights")
      if _arg7_weights is None:
        _arg7_weights_ = None
      else:
        try:
          _arg7_weights_ = memoryview(_arg7_weights)
        except TypeError:
          try:
            _tmparr__arg7_weights = (ctypes.c_double*len(_arg7_weights))()
            _tmparr__arg7_weights[:] = _arg7_weights
          except TypeError:
            raise TypeError("Argument _arg7_weights has wrong type")
          else:
            _arg7_weights_ = memoryview(_tmparr__arg7_weights)
      
        else:
          if _arg7_weights_.format != "d":
            _tmparr__arg7_weights = (ctypes.c_double*len(_arg7_weights))()
            _tmparr__arg7_weights[:] = _arg7_weights
            _arg7_weights_ = memoryview(_tmparr__arg7_weights)
      
      res = self.__obj.putbaraijlist(_arg1_num_,_arg2_subi_,_arg3_subj_,_arg4_alphaptrb_,_arg5_alphaptre_,_arg6_matidx_,_arg7_weights_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putbararowlist(self,_arg2_subi,_arg3_ptrb,_arg4_ptre,_arg5_subj,_arg6_nummat,_arg7_matidx,_arg8_weights): # 2
      """
      Replace a set of rows of barA
    
      putbararowlist(self,_arg2_subi,_arg3_ptrb,_arg4_ptre,_arg5_subj,_arg6_nummat,_arg7_matidx,_arg8_weights)
        _arg2_subi: array of int. Row indexes of barA.
        _arg3_ptrb: array of long. Start of rows in barA.
        _arg4_ptre: array of long. End of rows in barA.
        _arg5_subj: array of int. Column index of barA.
        _arg6_nummat: array of long. Number of entries in weighted sum of matrixes.
        _arg7_matidx: array of long. Matrix indexes for weighted sum of matrixes.
        _arg8_weights: array of double. Weights for weighted sum of matrixes.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subi)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subi):
        raise IndexError("Inconsistent length of array _arg2_subi")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_ptrb)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_ptrb):
        raise IndexError("Inconsistent length of array _arg3_ptrb")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg4_ptre)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg4_ptre):
        raise IndexError("Inconsistent length of array _arg4_ptre")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subi is None: raise TypeError("Invalid type for argument _arg2_subi")
      if _arg2_subi is None:
        _arg2_subi_ = None
      else:
        try:
          _arg2_subi_ = memoryview(_arg2_subi)
        except TypeError:
          try:
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
          except TypeError:
            raise TypeError("Argument _arg2_subi has wrong type")
          else:
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
        else:
          if _arg2_subi_.format != "i":
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
      if _arg3_ptrb is None: raise TypeError("Invalid type for argument _arg3_ptrb")
      if _arg3_ptrb is None:
        _arg3_ptrb_ = None
      else:
        try:
          _arg3_ptrb_ = memoryview(_arg3_ptrb)
        except TypeError:
          try:
            _tmparr__arg3_ptrb = (ctypes.c_int64*len(_arg3_ptrb))()
            _tmparr__arg3_ptrb[:] = _arg3_ptrb
          except TypeError:
            raise TypeError("Argument _arg3_ptrb has wrong type")
          else:
            _arg3_ptrb_ = memoryview(_tmparr__arg3_ptrb)
      
        else:
          if _arg3_ptrb_.format != "q":
            _tmparr__arg3_ptrb = (ctypes.c_int64*len(_arg3_ptrb))()
            _tmparr__arg3_ptrb[:] = _arg3_ptrb
            _arg3_ptrb_ = memoryview(_tmparr__arg3_ptrb)
      
      if _arg4_ptre is None: raise TypeError("Invalid type for argument _arg4_ptre")
      if _arg4_ptre is None:
        _arg4_ptre_ = None
      else:
        try:
          _arg4_ptre_ = memoryview(_arg4_ptre)
        except TypeError:
          try:
            _tmparr__arg4_ptre = (ctypes.c_int64*len(_arg4_ptre))()
            _tmparr__arg4_ptre[:] = _arg4_ptre
          except TypeError:
            raise TypeError("Argument _arg4_ptre has wrong type")
          else:
            _arg4_ptre_ = memoryview(_tmparr__arg4_ptre)
      
        else:
          if _arg4_ptre_.format != "q":
            _tmparr__arg4_ptre = (ctypes.c_int64*len(_arg4_ptre))()
            _tmparr__arg4_ptre[:] = _arg4_ptre
            _arg4_ptre_ = memoryview(_tmparr__arg4_ptre)
      
      if _arg5_subj is None: raise TypeError("Invalid type for argument _arg5_subj")
      if _arg5_subj is None:
        _arg5_subj_ = None
      else:
        try:
          _arg5_subj_ = memoryview(_arg5_subj)
        except TypeError:
          try:
            _tmparr__arg5_subj = (ctypes.c_int32*len(_arg5_subj))()
            _tmparr__arg5_subj[:] = _arg5_subj
          except TypeError:
            raise TypeError("Argument _arg5_subj has wrong type")
          else:
            _arg5_subj_ = memoryview(_tmparr__arg5_subj)
      
        else:
          if _arg5_subj_.format != "i":
            _tmparr__arg5_subj = (ctypes.c_int32*len(_arg5_subj))()
            _tmparr__arg5_subj[:] = _arg5_subj
            _arg5_subj_ = memoryview(_tmparr__arg5_subj)
      
      if _arg6_nummat is None: raise TypeError("Invalid type for argument _arg6_nummat")
      if _arg6_nummat is None:
        _arg6_nummat_ = None
      else:
        try:
          _arg6_nummat_ = memoryview(_arg6_nummat)
        except TypeError:
          try:
            _tmparr__arg6_nummat = (ctypes.c_int64*len(_arg6_nummat))()
            _tmparr__arg6_nummat[:] = _arg6_nummat
          except TypeError:
            raise TypeError("Argument _arg6_nummat has wrong type")
          else:
            _arg6_nummat_ = memoryview(_tmparr__arg6_nummat)
      
        else:
          if _arg6_nummat_.format != "q":
            _tmparr__arg6_nummat = (ctypes.c_int64*len(_arg6_nummat))()
            _tmparr__arg6_nummat[:] = _arg6_nummat
            _arg6_nummat_ = memoryview(_tmparr__arg6_nummat)
      
      if _arg6_nummat_ is not None and len(_arg6_nummat_) != len( _arg5_subj ):
        raise ValueError("Array argument _arg6_nummat has wrong length")
      if _arg7_matidx is None: raise TypeError("Invalid type for argument _arg7_matidx")
      if _arg7_matidx is None:
        _arg7_matidx_ = None
      else:
        try:
          _arg7_matidx_ = memoryview(_arg7_matidx)
        except TypeError:
          try:
            _tmparr__arg7_matidx = (ctypes.c_int64*len(_arg7_matidx))()
            _tmparr__arg7_matidx[:] = _arg7_matidx
          except TypeError:
            raise TypeError("Argument _arg7_matidx has wrong type")
          else:
            _arg7_matidx_ = memoryview(_tmparr__arg7_matidx)
      
        else:
          if _arg7_matidx_.format != "q":
            _tmparr__arg7_matidx = (ctypes.c_int64*len(_arg7_matidx))()
            _tmparr__arg7_matidx[:] = _arg7_matidx
            _arg7_matidx_ = memoryview(_tmparr__arg7_matidx)
      
      if _arg7_matidx_ is not None and len(_arg7_matidx_) != sum( _arg6_nummat ):
        raise ValueError("Array argument _arg7_matidx has wrong length")
      if _arg8_weights is None: raise TypeError("Invalid type for argument _arg8_weights")
      if _arg8_weights is None:
        _arg8_weights_ = None
      else:
        try:
          _arg8_weights_ = memoryview(_arg8_weights)
        except TypeError:
          try:
            _tmparr__arg8_weights = (ctypes.c_double*len(_arg8_weights))()
            _tmparr__arg8_weights[:] = _arg8_weights
          except TypeError:
            raise TypeError("Argument _arg8_weights has wrong type")
          else:
            _arg8_weights_ = memoryview(_tmparr__arg8_weights)
      
        else:
          if _arg8_weights_.format != "d":
            _tmparr__arg8_weights = (ctypes.c_double*len(_arg8_weights))()
            _tmparr__arg8_weights[:] = _arg8_weights
            _arg8_weights_ = memoryview(_tmparr__arg8_weights)
      
      if _arg8_weights_ is not None and len(_arg8_weights_) != sum( _arg6_nummat ):
        raise ValueError("Array argument _arg8_weights has wrong length")
      res = self.__obj.putbararowlist(_arg1_num_,_arg2_subi_,_arg3_ptrb_,_arg4_ptre_,_arg5_subj_,_arg6_nummat_,_arg7_matidx_,_arg8_weights_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getnumbarcnz(self): # 2
      """
      Obtains the number of nonzero elements in barc.
    
      getnumbarcnz(self)
      returns: _arg1_nz
        _arg1_nz: long. The number of nonzero elements in barc.
      """
      res,resargs = self.__obj.getnumbarcnz()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_nz_return_value = resargs
      return __arg1_nz_return_value
    
    def getnumbaranz(self): # 2
      """
      Get the number of nonzero elements in barA.
    
      getnumbaranz(self)
      returns: _arg1_nz
        _arg1_nz: long. The number of nonzero block elements in barA.
      """
      res,resargs = self.__obj.getnumbaranz()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_nz_return_value = resargs
      return __arg1_nz_return_value
    
    def getbarcsparsity(self,_arg3_idxj): # 2
      """
      Get the positions of the nonzero elements in barc.
    
      getbarcsparsity(self,_arg3_idxj)
        _arg3_idxj: array of long. Internal positions of the nonzeros elements in barc.
      returns: _arg2_numnz
        _arg2_numnz: long. Number of nonzero elements in barc.
      """
      _arg1_maxnumnz_ = self.getnumbarcnz()
      _arg1_maxnumnz = _arg1_maxnumnz_
      if _arg3_idxj is None: raise TypeError("Invalid type for argument _arg3_idxj")
      _copyback__arg3_idxj = False
      if _arg3_idxj is None:
        _arg3_idxj_ = None
      else:
        try:
          _arg3_idxj_ = memoryview(_arg3_idxj)
        except TypeError:
          try:
            _tmparr__arg3_idxj = (ctypes.c_int64*len(_arg3_idxj))()
            _tmparr__arg3_idxj[:] = _arg3_idxj
          except TypeError:
            raise TypeError("Argument _arg3_idxj has wrong type")
          else:
            _arg3_idxj_ = memoryview(_tmparr__arg3_idxj)
            _copyback__arg3_idxj = True
        else:
          if _arg3_idxj_.format != "q":
            _tmparr__arg3_idxj = (ctypes.c_int64*len(_arg3_idxj))()
            _tmparr__arg3_idxj[:] = _arg3_idxj
            _arg3_idxj_ = memoryview(_tmparr__arg3_idxj)
            _copyback__arg3_idxj = True
      if _arg3_idxj_ is not None and len(_arg3_idxj_) !=  _arg1_maxnumnz :
        raise ValueError("Array argument _arg3_idxj has wrong length")
      res,resargs = self.__obj.getbarcsparsity(_arg1_maxnumnz_,_arg3_idxj_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_numnz_return_value = resargs
      if _copyback__arg3_idxj:
        _arg3_idxj[:] = _tmparr__arg3_idxj
      return __arg2_numnz_return_value
    
    def getbarasparsity(self,_arg3_idxij): # 2
      """
      Obtains the sparsity pattern of the barA matrix.
    
      getbarasparsity(self,_arg3_idxij)
        _arg3_idxij: array of long. Position of each nonzero element in the vector representation of barA.
      returns: _arg2_numnz
        _arg2_numnz: long. Number of nonzero elements in barA.
      """
      _arg1_maxnumnz_ = self.getnumbaranz()
      _arg1_maxnumnz = _arg1_maxnumnz_
      if _arg3_idxij is None: raise TypeError("Invalid type for argument _arg3_idxij")
      _copyback__arg3_idxij = False
      if _arg3_idxij is None:
        _arg3_idxij_ = None
      else:
        try:
          _arg3_idxij_ = memoryview(_arg3_idxij)
        except TypeError:
          try:
            _tmparr__arg3_idxij = (ctypes.c_int64*len(_arg3_idxij))()
            _tmparr__arg3_idxij[:] = _arg3_idxij
          except TypeError:
            raise TypeError("Argument _arg3_idxij has wrong type")
          else:
            _arg3_idxij_ = memoryview(_tmparr__arg3_idxij)
            _copyback__arg3_idxij = True
        else:
          if _arg3_idxij_.format != "q":
            _tmparr__arg3_idxij = (ctypes.c_int64*len(_arg3_idxij))()
            _tmparr__arg3_idxij[:] = _arg3_idxij
            _arg3_idxij_ = memoryview(_tmparr__arg3_idxij)
            _copyback__arg3_idxij = True
      if _arg3_idxij_ is not None and len(_arg3_idxij_) !=  _arg1_maxnumnz :
        raise ValueError("Array argument _arg3_idxij has wrong length")
      res,resargs = self.__obj.getbarasparsity(_arg1_maxnumnz_,_arg3_idxij_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_numnz_return_value = resargs
      if _copyback__arg3_idxij:
        _arg3_idxij[:] = _tmparr__arg3_idxij
      return __arg2_numnz_return_value
    
    def getbarcidxinfo(self,_arg1_idx): # 2
      """
      Obtains information about an element in barc.
    
      getbarcidxinfo(self,_arg1_idx)
        _arg1_idx: long. Index of the element for which information should be obtained. The value is an index of a symmetric sparse variable.
      returns: _arg2_num
        _arg2_num: long. Number of terms that appear in the weighted sum that forms the requested element.
      """
      _arg1_idx_ = _arg1_idx
      res,resargs = self.__obj.getbarcidxinfo(_arg1_idx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_num_return_value = resargs
      return __arg2_num_return_value
    
    def getbarcidxj(self,_arg1_idx): # 2
      """
      Obtains the row index of an element in barc.
    
      getbarcidxj(self,_arg1_idx)
        _arg1_idx: long. Index of the element for which information should be obtained.
      returns: _arg2_j
        _arg2_j: int. Row index in barc.
      """
      _arg1_idx_ = _arg1_idx
      res,resargs = self.__obj.getbarcidxj(_arg1_idx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_j_return_value = resargs
      return __arg2_j_return_value
    
    def getbarcidx(self,_arg1_idx,_arg5_sub,_arg6_weights): # 2
      """
      Obtains information about an element in barc.
    
      getbarcidx(self,_arg1_idx,_arg5_sub,_arg6_weights)
        _arg1_idx: long. Index of the element for which information should be obtained.
        _arg5_sub: array of long. Elements appearing the weighted sum.
        _arg6_weights: array of double. Weights of terms in the weighted sum.
      returns: _arg3_j,_arg4_num
        _arg3_j: int. Row index in barc.
        _arg4_num: long. Number of terms in the weighted sum.
      """
      _arg1_idx_ = _arg1_idx
      _arg2_maxnum_ = self.getbarcidxinfo( _arg1_idx )
      _arg2_maxnum = _arg2_maxnum_
      if _arg5_sub is None: raise TypeError("Invalid type for argument _arg5_sub")
      _copyback__arg5_sub = False
      if _arg5_sub is None:
        _arg5_sub_ = None
      else:
        try:
          _arg5_sub_ = memoryview(_arg5_sub)
        except TypeError:
          try:
            _tmparr__arg5_sub = (ctypes.c_int64*len(_arg5_sub))()
            _tmparr__arg5_sub[:] = _arg5_sub
          except TypeError:
            raise TypeError("Argument _arg5_sub has wrong type")
          else:
            _arg5_sub_ = memoryview(_tmparr__arg5_sub)
            _copyback__arg5_sub = True
        else:
          if _arg5_sub_.format != "q":
            _tmparr__arg5_sub = (ctypes.c_int64*len(_arg5_sub))()
            _tmparr__arg5_sub[:] = _arg5_sub
            _arg5_sub_ = memoryview(_tmparr__arg5_sub)
            _copyback__arg5_sub = True
      if _arg5_sub_ is not None and len(_arg5_sub_) !=  _arg2_maxnum :
        raise ValueError("Array argument _arg5_sub has wrong length")
      if _arg6_weights is None: raise TypeError("Invalid type for argument _arg6_weights")
      _copyback__arg6_weights = False
      if _arg6_weights is None:
        _arg6_weights_ = None
      else:
        try:
          _arg6_weights_ = memoryview(_arg6_weights)
        except TypeError:
          try:
            _tmparr__arg6_weights = (ctypes.c_double*len(_arg6_weights))()
            _tmparr__arg6_weights[:] = _arg6_weights
          except TypeError:
            raise TypeError("Argument _arg6_weights has wrong type")
          else:
            _arg6_weights_ = memoryview(_tmparr__arg6_weights)
            _copyback__arg6_weights = True
        else:
          if _arg6_weights_.format != "d":
            _tmparr__arg6_weights = (ctypes.c_double*len(_arg6_weights))()
            _tmparr__arg6_weights[:] = _arg6_weights
            _arg6_weights_ = memoryview(_tmparr__arg6_weights)
            _copyback__arg6_weights = True
      if _arg6_weights_ is not None and len(_arg6_weights_) !=  _arg2_maxnum :
        raise ValueError("Array argument _arg6_weights has wrong length")
      res,resargs = self.__obj.getbarcidx(_arg1_idx_,_arg2_maxnum_,_arg5_sub_,_arg6_weights_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg3_j_return_value,__arg4_num_return_value = resargs
      if _copyback__arg6_weights:
        _arg6_weights[:] = _tmparr__arg6_weights
      if _copyback__arg5_sub:
        _arg5_sub[:] = _tmparr__arg5_sub
      return __arg3_j_return_value,__arg4_num_return_value
    
    def getbaraidxinfo(self,_arg1_idx): # 2
      """
      Obtains the number of terms in the weighted sum that form a particular element in barA.
    
      getbaraidxinfo(self,_arg1_idx)
        _arg1_idx: long. The internal position of the element for which information should be obtained.
      returns: _arg2_num
        _arg2_num: long. Number of terms in the weighted sum that form the specified element in barA.
      """
      _arg1_idx_ = _arg1_idx
      res,resargs = self.__obj.getbaraidxinfo(_arg1_idx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_num_return_value = resargs
      return __arg2_num_return_value
    
    def getbaraidxij(self,_arg1_idx): # 2
      """
      Obtains information about an element in barA.
    
      getbaraidxij(self,_arg1_idx)
        _arg1_idx: long. Position of the element in the vectorized form.
      returns: _arg2_i,_arg3_j
        _arg2_i: int. Row index of the element at position idx.
        _arg3_j: int. Column index of the element at position idx.
      """
      _arg1_idx_ = _arg1_idx
      res,resargs = self.__obj.getbaraidxij(_arg1_idx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_i_return_value,__arg3_j_return_value = resargs
      return __arg2_i_return_value,__arg3_j_return_value
    
    def getbaraidx(self,_arg1_idx,_arg6_sub,_arg7_weights): # 2
      """
      Obtains information about an element in barA.
    
      getbaraidx(self,_arg1_idx,_arg6_sub,_arg7_weights)
        _arg1_idx: long. Position of the element in the vectorized form.
        _arg6_sub: array of long. A list indexes of the elements from symmetric matrix storage that appear in the weighted sum.
        _arg7_weights: array of double. The weights associated with each term in the weighted sum.
      returns: _arg3_i,_arg4_j,_arg5_num
        _arg3_i: int. Row index of the element at position idx.
        _arg4_j: int. Column index of the element at position idx.
        _arg5_num: long. Number of terms in weighted sum that forms the element.
      """
      _arg1_idx_ = _arg1_idx
      _arg2_maxnum_ = self.getbaraidxinfo( _arg1_idx )
      _arg2_maxnum = _arg2_maxnum_
      if _arg6_sub is None: raise TypeError("Invalid type for argument _arg6_sub")
      _copyback__arg6_sub = False
      if _arg6_sub is None:
        _arg6_sub_ = None
      else:
        try:
          _arg6_sub_ = memoryview(_arg6_sub)
        except TypeError:
          try:
            _tmparr__arg6_sub = (ctypes.c_int64*len(_arg6_sub))()
            _tmparr__arg6_sub[:] = _arg6_sub
          except TypeError:
            raise TypeError("Argument _arg6_sub has wrong type")
          else:
            _arg6_sub_ = memoryview(_tmparr__arg6_sub)
            _copyback__arg6_sub = True
        else:
          if _arg6_sub_.format != "q":
            _tmparr__arg6_sub = (ctypes.c_int64*len(_arg6_sub))()
            _tmparr__arg6_sub[:] = _arg6_sub
            _arg6_sub_ = memoryview(_tmparr__arg6_sub)
            _copyback__arg6_sub = True
      if _arg6_sub_ is not None and len(_arg6_sub_) !=  _arg2_maxnum :
        raise ValueError("Array argument _arg6_sub has wrong length")
      if _arg7_weights is None: raise TypeError("Invalid type for argument _arg7_weights")
      _copyback__arg7_weights = False
      if _arg7_weights is None:
        _arg7_weights_ = None
      else:
        try:
          _arg7_weights_ = memoryview(_arg7_weights)
        except TypeError:
          try:
            _tmparr__arg7_weights = (ctypes.c_double*len(_arg7_weights))()
            _tmparr__arg7_weights[:] = _arg7_weights
          except TypeError:
            raise TypeError("Argument _arg7_weights has wrong type")
          else:
            _arg7_weights_ = memoryview(_tmparr__arg7_weights)
            _copyback__arg7_weights = True
        else:
          if _arg7_weights_.format != "d":
            _tmparr__arg7_weights = (ctypes.c_double*len(_arg7_weights))()
            _tmparr__arg7_weights[:] = _arg7_weights
            _arg7_weights_ = memoryview(_tmparr__arg7_weights)
            _copyback__arg7_weights = True
      if _arg7_weights_ is not None and len(_arg7_weights_) !=  _arg2_maxnum :
        raise ValueError("Array argument _arg7_weights has wrong length")
      res,resargs = self.__obj.getbaraidx(_arg1_idx_,_arg2_maxnum_,_arg6_sub_,_arg7_weights_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg3_i_return_value,__arg4_j_return_value,__arg5_num_return_value = resargs
      if _copyback__arg7_weights:
        _arg7_weights[:] = _tmparr__arg7_weights
      if _copyback__arg6_sub:
        _arg6_sub[:] = _tmparr__arg6_sub
      return __arg3_i_return_value,__arg4_j_return_value,__arg5_num_return_value
    
    def getnumbarcblocktriplets(self): # 2
      """
      Obtains an upper bound on the number of elements in the block triplet form of barc.
    
      getnumbarcblocktriplets(self)
      returns: _arg1_num
        _arg1_num: long. An upper bound on the number of elements in the block triplet form of barc.
      """
      res,resargs = self.__obj.getnumbarcblocktriplets()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_num_return_value = resargs
      return __arg1_num_return_value
    
    def putbarcblocktriplet(self,_arg1_num,_arg2_subj,_arg3_subk,_arg4_subl,_arg5_valjkl): # 2
      """
      Inputs barC in block triplet form.
    
      putbarcblocktriplet(self,_arg1_num,_arg2_subj,_arg3_subk,_arg4_subl,_arg5_valjkl)
        _arg1_num: long. Number of elements in the block triplet form.
        _arg2_subj: array of int. Symmetric matrix variable index.
        _arg3_subk: array of int. Block row index.
        _arg4_subl: array of int. Block column index.
        _arg5_valjkl: array of double. The numerical value associated with each block triplet.
      """
      _arg1_num_ = _arg1_num
      if _arg2_subj is None: raise TypeError("Invalid type for argument _arg2_subj")
      if _arg2_subj is None:
        _arg2_subj_ = None
      else:
        try:
          _arg2_subj_ = memoryview(_arg2_subj)
        except TypeError:
          try:
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
          except TypeError:
            raise TypeError("Argument _arg2_subj has wrong type")
          else:
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
        else:
          if _arg2_subj_.format != "i":
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
      if _arg2_subj_ is not None and len(_arg2_subj_) !=  _arg1_num :
        raise ValueError("Array argument _arg2_subj has wrong length")
      if _arg3_subk is None: raise TypeError("Invalid type for argument _arg3_subk")
      if _arg3_subk is None:
        _arg3_subk_ = None
      else:
        try:
          _arg3_subk_ = memoryview(_arg3_subk)
        except TypeError:
          try:
            _tmparr__arg3_subk = (ctypes.c_int32*len(_arg3_subk))()
            _tmparr__arg3_subk[:] = _arg3_subk
          except TypeError:
            raise TypeError("Argument _arg3_subk has wrong type")
          else:
            _arg3_subk_ = memoryview(_tmparr__arg3_subk)
      
        else:
          if _arg3_subk_.format != "i":
            _tmparr__arg3_subk = (ctypes.c_int32*len(_arg3_subk))()
            _tmparr__arg3_subk[:] = _arg3_subk
            _arg3_subk_ = memoryview(_tmparr__arg3_subk)
      
      if _arg3_subk_ is not None and len(_arg3_subk_) !=  _arg1_num :
        raise ValueError("Array argument _arg3_subk has wrong length")
      if _arg4_subl is None: raise TypeError("Invalid type for argument _arg4_subl")
      if _arg4_subl is None:
        _arg4_subl_ = None
      else:
        try:
          _arg4_subl_ = memoryview(_arg4_subl)
        except TypeError:
          try:
            _tmparr__arg4_subl = (ctypes.c_int32*len(_arg4_subl))()
            _tmparr__arg4_subl[:] = _arg4_subl
          except TypeError:
            raise TypeError("Argument _arg4_subl has wrong type")
          else:
            _arg4_subl_ = memoryview(_tmparr__arg4_subl)
      
        else:
          if _arg4_subl_.format != "i":
            _tmparr__arg4_subl = (ctypes.c_int32*len(_arg4_subl))()
            _tmparr__arg4_subl[:] = _arg4_subl
            _arg4_subl_ = memoryview(_tmparr__arg4_subl)
      
      if _arg4_subl_ is not None and len(_arg4_subl_) !=  _arg1_num :
        raise ValueError("Array argument _arg4_subl has wrong length")
      if _arg5_valjkl is None: raise TypeError("Invalid type for argument _arg5_valjkl")
      if _arg5_valjkl is None:
        _arg5_valjkl_ = None
      else:
        try:
          _arg5_valjkl_ = memoryview(_arg5_valjkl)
        except TypeError:
          try:
            _tmparr__arg5_valjkl = (ctypes.c_double*len(_arg5_valjkl))()
            _tmparr__arg5_valjkl[:] = _arg5_valjkl
          except TypeError:
            raise TypeError("Argument _arg5_valjkl has wrong type")
          else:
            _arg5_valjkl_ = memoryview(_tmparr__arg5_valjkl)
      
        else:
          if _arg5_valjkl_.format != "d":
            _tmparr__arg5_valjkl = (ctypes.c_double*len(_arg5_valjkl))()
            _tmparr__arg5_valjkl[:] = _arg5_valjkl
            _arg5_valjkl_ = memoryview(_tmparr__arg5_valjkl)
      
      if _arg5_valjkl_ is not None and len(_arg5_valjkl_) !=  _arg1_num :
        raise ValueError("Array argument _arg5_valjkl has wrong length")
      res = self.__obj.putbarcblocktriplet(_arg1_num_,_arg2_subj_,_arg3_subk_,_arg4_subl_,_arg5_valjkl_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getbarcblocktriplet(self,_arg3_subj,_arg4_subk,_arg5_subl,_arg6_valjkl): # 2
      """
      Obtains barC in block triplet form.
    
      getbarcblocktriplet(self,_arg3_subj,_arg4_subk,_arg5_subl,_arg6_valjkl)
        _arg3_subj: array of int. Symmetric matrix variable index.
        _arg4_subk: array of int. Block row index.
        _arg5_subl: array of int. Block column index.
        _arg6_valjkl: array of double. The numerical value associated with each block triplet.
      returns: _arg2_num
        _arg2_num: long. Number of elements in the block triplet form.
      """
      _arg1_maxnum_ = self.getnumbarcblocktriplets()
      _arg1_maxnum = _arg1_maxnum_
      if _arg3_subj is None: raise TypeError("Invalid type for argument _arg3_subj")
      _copyback__arg3_subj = False
      if _arg3_subj is None:
        _arg3_subj_ = None
      else:
        try:
          _arg3_subj_ = memoryview(_arg3_subj)
        except TypeError:
          try:
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
          except TypeError:
            raise TypeError("Argument _arg3_subj has wrong type")
          else:
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
            _copyback__arg3_subj = True
        else:
          if _arg3_subj_.format != "i":
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
            _copyback__arg3_subj = True
      if _arg3_subj_ is not None and len(_arg3_subj_) !=  _arg1_maxnum :
        raise ValueError("Array argument _arg3_subj has wrong length")
      if _arg4_subk is None: raise TypeError("Invalid type for argument _arg4_subk")
      _copyback__arg4_subk = False
      if _arg4_subk is None:
        _arg4_subk_ = None
      else:
        try:
          _arg4_subk_ = memoryview(_arg4_subk)
        except TypeError:
          try:
            _tmparr__arg4_subk = (ctypes.c_int32*len(_arg4_subk))()
            _tmparr__arg4_subk[:] = _arg4_subk
          except TypeError:
            raise TypeError("Argument _arg4_subk has wrong type")
          else:
            _arg4_subk_ = memoryview(_tmparr__arg4_subk)
            _copyback__arg4_subk = True
        else:
          if _arg4_subk_.format != "i":
            _tmparr__arg4_subk = (ctypes.c_int32*len(_arg4_subk))()
            _tmparr__arg4_subk[:] = _arg4_subk
            _arg4_subk_ = memoryview(_tmparr__arg4_subk)
            _copyback__arg4_subk = True
      if _arg4_subk_ is not None and len(_arg4_subk_) !=  _arg1_maxnum :
        raise ValueError("Array argument _arg4_subk has wrong length")
      if _arg5_subl is None: raise TypeError("Invalid type for argument _arg5_subl")
      _copyback__arg5_subl = False
      if _arg5_subl is None:
        _arg5_subl_ = None
      else:
        try:
          _arg5_subl_ = memoryview(_arg5_subl)
        except TypeError:
          try:
            _tmparr__arg5_subl = (ctypes.c_int32*len(_arg5_subl))()
            _tmparr__arg5_subl[:] = _arg5_subl
          except TypeError:
            raise TypeError("Argument _arg5_subl has wrong type")
          else:
            _arg5_subl_ = memoryview(_tmparr__arg5_subl)
            _copyback__arg5_subl = True
        else:
          if _arg5_subl_.format != "i":
            _tmparr__arg5_subl = (ctypes.c_int32*len(_arg5_subl))()
            _tmparr__arg5_subl[:] = _arg5_subl
            _arg5_subl_ = memoryview(_tmparr__arg5_subl)
            _copyback__arg5_subl = True
      if _arg5_subl_ is not None and len(_arg5_subl_) !=  _arg1_maxnum :
        raise ValueError("Array argument _arg5_subl has wrong length")
      if _arg6_valjkl is None: raise TypeError("Invalid type for argument _arg6_valjkl")
      _copyback__arg6_valjkl = False
      if _arg6_valjkl is None:
        _arg6_valjkl_ = None
      else:
        try:
          _arg6_valjkl_ = memoryview(_arg6_valjkl)
        except TypeError:
          try:
            _tmparr__arg6_valjkl = (ctypes.c_double*len(_arg6_valjkl))()
            _tmparr__arg6_valjkl[:] = _arg6_valjkl
          except TypeError:
            raise TypeError("Argument _arg6_valjkl has wrong type")
          else:
            _arg6_valjkl_ = memoryview(_tmparr__arg6_valjkl)
            _copyback__arg6_valjkl = True
        else:
          if _arg6_valjkl_.format != "d":
            _tmparr__arg6_valjkl = (ctypes.c_double*len(_arg6_valjkl))()
            _tmparr__arg6_valjkl[:] = _arg6_valjkl
            _arg6_valjkl_ = memoryview(_tmparr__arg6_valjkl)
            _copyback__arg6_valjkl = True
      if _arg6_valjkl_ is not None and len(_arg6_valjkl_) !=  _arg1_maxnum :
        raise ValueError("Array argument _arg6_valjkl has wrong length")
      res,resargs = self.__obj.getbarcblocktriplet(_arg1_maxnum_,_arg3_subj_,_arg4_subk_,_arg5_subl_,_arg6_valjkl_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_num_return_value = resargs
      if _copyback__arg6_valjkl:
        _arg6_valjkl[:] = _tmparr__arg6_valjkl
      if _copyback__arg5_subl:
        _arg5_subl[:] = _tmparr__arg5_subl
      if _copyback__arg4_subk:
        _arg4_subk[:] = _tmparr__arg4_subk
      if _copyback__arg3_subj:
        _arg3_subj[:] = _tmparr__arg3_subj
      return __arg2_num_return_value
    
    def putbarablocktriplet(self,_arg1_num,_arg2_subi,_arg3_subj,_arg4_subk,_arg5_subl,_arg6_valijkl): # 2
      """
      Inputs barA in block triplet form.
    
      putbarablocktriplet(self,_arg1_num,_arg2_subi,_arg3_subj,_arg4_subk,_arg5_subl,_arg6_valijkl)
        _arg1_num: long. Number of elements in the block triplet form.
        _arg2_subi: array of int. Constraint index.
        _arg3_subj: array of int. Symmetric matrix variable index.
        _arg4_subk: array of int. Block row index.
        _arg5_subl: array of int. Block column index.
        _arg6_valijkl: array of double. The numerical value associated with each block triplet.
      """
      _arg1_num_ = _arg1_num
      if _arg2_subi is None: raise TypeError("Invalid type for argument _arg2_subi")
      if _arg2_subi is None:
        _arg2_subi_ = None
      else:
        try:
          _arg2_subi_ = memoryview(_arg2_subi)
        except TypeError:
          try:
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
          except TypeError:
            raise TypeError("Argument _arg2_subi has wrong type")
          else:
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
        else:
          if _arg2_subi_.format != "i":
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
      if _arg2_subi_ is not None and len(_arg2_subi_) !=  _arg1_num :
        raise ValueError("Array argument _arg2_subi has wrong length")
      if _arg3_subj is None: raise TypeError("Invalid type for argument _arg3_subj")
      if _arg3_subj is None:
        _arg3_subj_ = None
      else:
        try:
          _arg3_subj_ = memoryview(_arg3_subj)
        except TypeError:
          try:
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
          except TypeError:
            raise TypeError("Argument _arg3_subj has wrong type")
          else:
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
      
        else:
          if _arg3_subj_.format != "i":
            _tmparr__arg3_subj = (ctypes.c_int32*len(_arg3_subj))()
            _tmparr__arg3_subj[:] = _arg3_subj
            _arg3_subj_ = memoryview(_tmparr__arg3_subj)
      
      if _arg3_subj_ is not None and len(_arg3_subj_) !=  _arg1_num :
        raise ValueError("Array argument _arg3_subj has wrong length")
      if _arg4_subk is None: raise TypeError("Invalid type for argument _arg4_subk")
      if _arg4_subk is None:
        _arg4_subk_ = None
      else:
        try:
          _arg4_subk_ = memoryview(_arg4_subk)
        except TypeError:
          try:
            _tmparr__arg4_subk = (ctypes.c_int32*len(_arg4_subk))()
            _tmparr__arg4_subk[:] = _arg4_subk
          except TypeError:
            raise TypeError("Argument _arg4_subk has wrong type")
          else:
            _arg4_subk_ = memoryview(_tmparr__arg4_subk)
      
        else:
          if _arg4_subk_.format != "i":
            _tmparr__arg4_subk = (ctypes.c_int32*len(_arg4_subk))()
            _tmparr__arg4_subk[:] = _arg4_subk
            _arg4_subk_ = memoryview(_tmparr__arg4_subk)
      
      if _arg4_subk_ is not None and len(_arg4_subk_) !=  _arg1_num :
        raise ValueError("Array argument _arg4_subk has wrong length")
      if _arg5_subl is None: raise TypeError("Invalid type for argument _arg5_subl")
      if _arg5_subl is None:
        _arg5_subl_ = None
      else:
        try:
          _arg5_subl_ = memoryview(_arg5_subl)
        except TypeError:
          try:
            _tmparr__arg5_subl = (ctypes.c_int32*len(_arg5_subl))()
            _tmparr__arg5_subl[:] = _arg5_subl
          except TypeError:
            raise TypeError("Argument _arg5_subl has wrong type")
          else:
            _arg5_subl_ = memoryview(_tmparr__arg5_subl)
      
        else:
          if _arg5_subl_.format != "i":
            _tmparr__arg5_subl = (ctypes.c_int32*len(_arg5_subl))()
            _tmparr__arg5_subl[:] = _arg5_subl
            _arg5_subl_ = memoryview(_tmparr__arg5_subl)
      
      if _arg5_subl_ is not None and len(_arg5_subl_) !=  _arg1_num :
        raise ValueError("Array argument _arg5_subl has wrong length")
      if _arg6_valijkl is None: raise TypeError("Invalid type for argument _arg6_valijkl")
      if _arg6_valijkl is None:
        _arg6_valijkl_ = None
      else:
        try:
          _arg6_valijkl_ = memoryview(_arg6_valijkl)
        except TypeError:
          try:
            _tmparr__arg6_valijkl = (ctypes.c_double*len(_arg6_valijkl))()
            _tmparr__arg6_valijkl[:] = _arg6_valijkl
          except TypeError:
            raise TypeError("Argument _arg6_valijkl has wrong type")
          else:
            _arg6_valijkl_ = memoryview(_tmparr__arg6_valijkl)
      
        else:
          if _arg6_valijkl_.format != "d":
            _tmparr__arg6_valijkl = (ctypes.c_double*len(_arg6_valijkl))()
            _tmparr__arg6_valijkl[:] = _arg6_valijkl
            _arg6_valijkl_ = memoryview(_tmparr__arg6_valijkl)
      
      if _arg6_valijkl_ is not None and len(_arg6_valijkl_) !=  _arg1_num :
        raise ValueError("Array argument _arg6_valijkl has wrong length")
      res = self.__obj.putbarablocktriplet(_arg1_num_,_arg2_subi_,_arg3_subj_,_arg4_subk_,_arg5_subl_,_arg6_valijkl_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getnumbarablocktriplets(self): # 2
      """
      Obtains an upper bound on the number of scalar elements in the block triplet form of bara.
    
      getnumbarablocktriplets(self)
      returns: _arg1_num
        _arg1_num: long. An upper bound on the number of elements in the block triplet form of bara.
      """
      res,resargs = self.__obj.getnumbarablocktriplets()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_num_return_value = resargs
      return __arg1_num_return_value
    
    def getbarablocktriplet(self,_arg3_subi,_arg4_subj,_arg5_subk,_arg6_subl,_arg7_valijkl): # 2
      """
      Obtains barA in block triplet form.
    
      getbarablocktriplet(self,_arg3_subi,_arg4_subj,_arg5_subk,_arg6_subl,_arg7_valijkl)
        _arg3_subi: array of int. Constraint index.
        _arg4_subj: array of int. Symmetric matrix variable index.
        _arg5_subk: array of int. Block row index.
        _arg6_subl: array of int. Block column index.
        _arg7_valijkl: array of double. The numerical value associated with each block triplet.
      returns: _arg2_num
        _arg2_num: long. Number of elements in the block triplet form.
      """
      _arg1_maxnum_ = self.getnumbarablocktriplets()
      _arg1_maxnum = _arg1_maxnum_
      if _arg3_subi is None: raise TypeError("Invalid type for argument _arg3_subi")
      _copyback__arg3_subi = False
      if _arg3_subi is None:
        _arg3_subi_ = None
      else:
        try:
          _arg3_subi_ = memoryview(_arg3_subi)
        except TypeError:
          try:
            _tmparr__arg3_subi = (ctypes.c_int32*len(_arg3_subi))()
            _tmparr__arg3_subi[:] = _arg3_subi
          except TypeError:
            raise TypeError("Argument _arg3_subi has wrong type")
          else:
            _arg3_subi_ = memoryview(_tmparr__arg3_subi)
            _copyback__arg3_subi = True
        else:
          if _arg3_subi_.format != "i":
            _tmparr__arg3_subi = (ctypes.c_int32*len(_arg3_subi))()
            _tmparr__arg3_subi[:] = _arg3_subi
            _arg3_subi_ = memoryview(_tmparr__arg3_subi)
            _copyback__arg3_subi = True
      if _arg3_subi_ is not None and len(_arg3_subi_) !=  _arg1_maxnum :
        raise ValueError("Array argument _arg3_subi has wrong length")
      if _arg4_subj is None: raise TypeError("Invalid type for argument _arg4_subj")
      _copyback__arg4_subj = False
      if _arg4_subj is None:
        _arg4_subj_ = None
      else:
        try:
          _arg4_subj_ = memoryview(_arg4_subj)
        except TypeError:
          try:
            _tmparr__arg4_subj = (ctypes.c_int32*len(_arg4_subj))()
            _tmparr__arg4_subj[:] = _arg4_subj
          except TypeError:
            raise TypeError("Argument _arg4_subj has wrong type")
          else:
            _arg4_subj_ = memoryview(_tmparr__arg4_subj)
            _copyback__arg4_subj = True
        else:
          if _arg4_subj_.format != "i":
            _tmparr__arg4_subj = (ctypes.c_int32*len(_arg4_subj))()
            _tmparr__arg4_subj[:] = _arg4_subj
            _arg4_subj_ = memoryview(_tmparr__arg4_subj)
            _copyback__arg4_subj = True
      if _arg4_subj_ is not None and len(_arg4_subj_) !=  _arg1_maxnum :
        raise ValueError("Array argument _arg4_subj has wrong length")
      if _arg5_subk is None: raise TypeError("Invalid type for argument _arg5_subk")
      _copyback__arg5_subk = False
      if _arg5_subk is None:
        _arg5_subk_ = None
      else:
        try:
          _arg5_subk_ = memoryview(_arg5_subk)
        except TypeError:
          try:
            _tmparr__arg5_subk = (ctypes.c_int32*len(_arg5_subk))()
            _tmparr__arg5_subk[:] = _arg5_subk
          except TypeError:
            raise TypeError("Argument _arg5_subk has wrong type")
          else:
            _arg5_subk_ = memoryview(_tmparr__arg5_subk)
            _copyback__arg5_subk = True
        else:
          if _arg5_subk_.format != "i":
            _tmparr__arg5_subk = (ctypes.c_int32*len(_arg5_subk))()
            _tmparr__arg5_subk[:] = _arg5_subk
            _arg5_subk_ = memoryview(_tmparr__arg5_subk)
            _copyback__arg5_subk = True
      if _arg5_subk_ is not None and len(_arg5_subk_) !=  _arg1_maxnum :
        raise ValueError("Array argument _arg5_subk has wrong length")
      if _arg6_subl is None: raise TypeError("Invalid type for argument _arg6_subl")
      _copyback__arg6_subl = False
      if _arg6_subl is None:
        _arg6_subl_ = None
      else:
        try:
          _arg6_subl_ = memoryview(_arg6_subl)
        except TypeError:
          try:
            _tmparr__arg6_subl = (ctypes.c_int32*len(_arg6_subl))()
            _tmparr__arg6_subl[:] = _arg6_subl
          except TypeError:
            raise TypeError("Argument _arg6_subl has wrong type")
          else:
            _arg6_subl_ = memoryview(_tmparr__arg6_subl)
            _copyback__arg6_subl = True
        else:
          if _arg6_subl_.format != "i":
            _tmparr__arg6_subl = (ctypes.c_int32*len(_arg6_subl))()
            _tmparr__arg6_subl[:] = _arg6_subl
            _arg6_subl_ = memoryview(_tmparr__arg6_subl)
            _copyback__arg6_subl = True
      if _arg6_subl_ is not None and len(_arg6_subl_) !=  _arg1_maxnum :
        raise ValueError("Array argument _arg6_subl has wrong length")
      if _arg7_valijkl is None: raise TypeError("Invalid type for argument _arg7_valijkl")
      _copyback__arg7_valijkl = False
      if _arg7_valijkl is None:
        _arg7_valijkl_ = None
      else:
        try:
          _arg7_valijkl_ = memoryview(_arg7_valijkl)
        except TypeError:
          try:
            _tmparr__arg7_valijkl = (ctypes.c_double*len(_arg7_valijkl))()
            _tmparr__arg7_valijkl[:] = _arg7_valijkl
          except TypeError:
            raise TypeError("Argument _arg7_valijkl has wrong type")
          else:
            _arg7_valijkl_ = memoryview(_tmparr__arg7_valijkl)
            _copyback__arg7_valijkl = True
        else:
          if _arg7_valijkl_.format != "d":
            _tmparr__arg7_valijkl = (ctypes.c_double*len(_arg7_valijkl))()
            _tmparr__arg7_valijkl[:] = _arg7_valijkl
            _arg7_valijkl_ = memoryview(_tmparr__arg7_valijkl)
            _copyback__arg7_valijkl = True
      if _arg7_valijkl_ is not None and len(_arg7_valijkl_) !=  _arg1_maxnum :
        raise ValueError("Array argument _arg7_valijkl has wrong length")
      res,resargs = self.__obj.getbarablocktriplet(_arg1_maxnum_,_arg3_subi_,_arg4_subj_,_arg5_subk_,_arg6_subl_,_arg7_valijkl_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_num_return_value = resargs
      if _copyback__arg7_valijkl:
        _arg7_valijkl[:] = _tmparr__arg7_valijkl
      if _copyback__arg6_subl:
        _arg6_subl[:] = _tmparr__arg6_subl
      if _copyback__arg5_subk:
        _arg5_subk[:] = _tmparr__arg5_subk
      if _copyback__arg4_subj:
        _arg4_subj[:] = _tmparr__arg4_subj
      if _copyback__arg3_subi:
        _arg3_subi[:] = _tmparr__arg3_subi
      return __arg2_num_return_value
    
    def putconbound(self,_arg1_i,_arg2_bkc_,_arg3_blc,_arg4_buc): # 2
      """
      Changes the bound for one constraint.
    
      putconbound(self,_arg1_i,_arg2_bkc_,_arg3_blc,_arg4_buc)
        _arg1_i: int. Index of the constraint.
        _arg2_bkc: mosek.boundkey. New bound key.
        _arg3_blc: double. New lower bound.
        _arg4_buc: double. New upper bound.
      """
      _arg1_i_ = _arg1_i
      if not isinstance(_arg2_bkc_,boundkey): raise TypeError("Argument _arg2_bkc has wrong type")
      _arg2_bkc = _arg2_bkc_
      _arg3_blc_ = _arg3_blc
      _arg4_buc_ = _arg4_buc
      res = self.__obj.putconbound(_arg1_i_,_arg2_bkc_,_arg3_blc_,_arg4_buc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putconboundlist(self,_arg2_sub,_arg3_bkc,_arg4_blc,_arg5_buc): # 2
      """
      Changes the bounds of a list of constraints.
    
      putconboundlist(self,_arg2_sub,_arg3_bkc,_arg4_blc,_arg5_buc)
        _arg2_sub: array of int. List of constraint indexes.
        _arg3_bkc: array of mosek.boundkey. <no description>
        _arg4_blc: array of double. <no description>
        _arg5_buc: array of double. <no description>
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_sub)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_sub):
        raise IndexError("Inconsistent length of array _arg2_sub")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_bkc)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_bkc):
        raise IndexError("Inconsistent length of array _arg3_bkc")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg4_blc)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg4_blc):
        raise IndexError("Inconsistent length of array _arg4_blc")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg5_buc)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg5_buc):
        raise IndexError("Inconsistent length of array _arg5_buc")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_sub is None: raise TypeError("Invalid type for argument _arg2_sub")
      if _arg2_sub is None:
        _arg2_sub_ = None
      else:
        try:
          _arg2_sub_ = memoryview(_arg2_sub)
        except TypeError:
          try:
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
          except TypeError:
            raise TypeError("Argument _arg2_sub has wrong type")
          else:
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
        else:
          if _arg2_sub_.format != "i":
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
      if _arg3_bkc is None: raise TypeError("Invalid type for argument _arg3_bkc")
      if _arg3_bkc is None:
        _arg3_bkc_ = None
      else:
        try:
          _arg3_bkc_ = memoryview(_arg3_bkc)
        except TypeError:
          try:
            _tmparr__arg3_bkc = (ctypes.c_int*len(_arg3_bkc))()
            _tmparr__arg3_bkc[:] = _arg3_bkc
          except TypeError:
            raise TypeError("Argument _arg3_bkc has wrong type")
          else:
            _arg3_bkc_ = memoryview(_tmparr__arg3_bkc)
      
        else:
          if _arg3_bkc_.format != "i":
            _tmparr__arg3_bkc = (ctypes.c_int*len(_arg3_bkc))()
            _tmparr__arg3_bkc[:] = _arg3_bkc
            _arg3_bkc_ = memoryview(_tmparr__arg3_bkc)
      
      if _arg4_blc is None: raise TypeError("Invalid type for argument _arg4_blc")
      if _arg4_blc is None:
        _arg4_blc_ = None
      else:
        try:
          _arg4_blc_ = memoryview(_arg4_blc)
        except TypeError:
          try:
            _tmparr__arg4_blc = (ctypes.c_double*len(_arg4_blc))()
            _tmparr__arg4_blc[:] = _arg4_blc
          except TypeError:
            raise TypeError("Argument _arg4_blc has wrong type")
          else:
            _arg4_blc_ = memoryview(_tmparr__arg4_blc)
      
        else:
          if _arg4_blc_.format != "d":
            _tmparr__arg4_blc = (ctypes.c_double*len(_arg4_blc))()
            _tmparr__arg4_blc[:] = _arg4_blc
            _arg4_blc_ = memoryview(_tmparr__arg4_blc)
      
      if _arg5_buc is None: raise TypeError("Invalid type for argument _arg5_buc")
      if _arg5_buc is None:
        _arg5_buc_ = None
      else:
        try:
          _arg5_buc_ = memoryview(_arg5_buc)
        except TypeError:
          try:
            _tmparr__arg5_buc = (ctypes.c_double*len(_arg5_buc))()
            _tmparr__arg5_buc[:] = _arg5_buc
          except TypeError:
            raise TypeError("Argument _arg5_buc has wrong type")
          else:
            _arg5_buc_ = memoryview(_tmparr__arg5_buc)
      
        else:
          if _arg5_buc_.format != "d":
            _tmparr__arg5_buc = (ctypes.c_double*len(_arg5_buc))()
            _tmparr__arg5_buc[:] = _arg5_buc
            _arg5_buc_ = memoryview(_tmparr__arg5_buc)
      
      res = self.__obj.putconboundlist(_arg1_num_,_arg2_sub_,_arg3_bkc_,_arg4_blc_,_arg5_buc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putconboundlistconst(self,_arg2_sub,_arg3_bkc_,_arg4_blc,_arg5_buc): # 2
      """
      Changes the bounds of a list of constraints.
    
      putconboundlistconst(self,_arg2_sub,_arg3_bkc_,_arg4_blc,_arg5_buc)
        _arg2_sub: array of int. List of constraint indexes.
        _arg3_bkc: mosek.boundkey. New bound key for all constraints in the list.
        _arg4_blc: double. New lower bound for all constraints in the list.
        _arg5_buc: double. New upper bound for all constraints in the list.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_sub)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_sub):
        raise IndexError("Inconsistent length of array _arg2_sub")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_sub is None: raise TypeError("Invalid type for argument _arg2_sub")
      if _arg2_sub is None:
        _arg2_sub_ = None
      else:
        try:
          _arg2_sub_ = memoryview(_arg2_sub)
        except TypeError:
          try:
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
          except TypeError:
            raise TypeError("Argument _arg2_sub has wrong type")
          else:
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
        else:
          if _arg2_sub_.format != "i":
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
      if not isinstance(_arg3_bkc_,boundkey): raise TypeError("Argument _arg3_bkc has wrong type")
      _arg3_bkc = _arg3_bkc_
      _arg4_blc_ = _arg4_blc
      _arg5_buc_ = _arg5_buc
      res = self.__obj.putconboundlistconst(_arg1_num_,_arg2_sub_,_arg3_bkc_,_arg4_blc_,_arg5_buc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putconboundslice(self,_arg1_first,_arg2_last,_arg3_bkc,_arg4_blc,_arg5_buc): # 2
      """
      Changes the bounds for a slice of the constraints.
    
      putconboundslice(self,_arg1_first,_arg2_last,_arg3_bkc,_arg4_blc,_arg5_buc)
        _arg1_first: int. <no description>
        _arg2_last: int. <no description>
        _arg3_bkc: array of mosek.boundkey. <no description>
        _arg4_blc: array of double. <no description>
        _arg5_buc: array of double. <no description>
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      if _arg3_bkc is None: raise TypeError("Invalid type for argument _arg3_bkc")
      if _arg3_bkc is None:
        _arg3_bkc_ = None
      else:
        try:
          _arg3_bkc_ = memoryview(_arg3_bkc)
        except TypeError:
          try:
            _tmparr__arg3_bkc = (ctypes.c_int*len(_arg3_bkc))()
            _tmparr__arg3_bkc[:] = _arg3_bkc
          except TypeError:
            raise TypeError("Argument _arg3_bkc has wrong type")
          else:
            _arg3_bkc_ = memoryview(_tmparr__arg3_bkc)
      
        else:
          if _arg3_bkc_.format != "i":
            _tmparr__arg3_bkc = (ctypes.c_int*len(_arg3_bkc))()
            _tmparr__arg3_bkc[:] = _arg3_bkc
            _arg3_bkc_ = memoryview(_tmparr__arg3_bkc)
      
      if _arg3_bkc_ is not None and len(_arg3_bkc_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg3_bkc has wrong length")
      if _arg4_blc is None: raise TypeError("Invalid type for argument _arg4_blc")
      if _arg4_blc is None:
        _arg4_blc_ = None
      else:
        try:
          _arg4_blc_ = memoryview(_arg4_blc)
        except TypeError:
          try:
            _tmparr__arg4_blc = (ctypes.c_double*len(_arg4_blc))()
            _tmparr__arg4_blc[:] = _arg4_blc
          except TypeError:
            raise TypeError("Argument _arg4_blc has wrong type")
          else:
            _arg4_blc_ = memoryview(_tmparr__arg4_blc)
      
        else:
          if _arg4_blc_.format != "d":
            _tmparr__arg4_blc = (ctypes.c_double*len(_arg4_blc))()
            _tmparr__arg4_blc[:] = _arg4_blc
            _arg4_blc_ = memoryview(_tmparr__arg4_blc)
      
      if _arg4_blc_ is not None and len(_arg4_blc_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg4_blc has wrong length")
      if _arg5_buc is None: raise TypeError("Invalid type for argument _arg5_buc")
      if _arg5_buc is None:
        _arg5_buc_ = None
      else:
        try:
          _arg5_buc_ = memoryview(_arg5_buc)
        except TypeError:
          try:
            _tmparr__arg5_buc = (ctypes.c_double*len(_arg5_buc))()
            _tmparr__arg5_buc[:] = _arg5_buc
          except TypeError:
            raise TypeError("Argument _arg5_buc has wrong type")
          else:
            _arg5_buc_ = memoryview(_tmparr__arg5_buc)
      
        else:
          if _arg5_buc_.format != "d":
            _tmparr__arg5_buc = (ctypes.c_double*len(_arg5_buc))()
            _tmparr__arg5_buc[:] = _arg5_buc
            _arg5_buc_ = memoryview(_tmparr__arg5_buc)
      
      if _arg5_buc_ is not None and len(_arg5_buc_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg5_buc has wrong length")
      res = self.__obj.putconboundslice(_arg1_first_,_arg2_last_,_arg3_bkc_,_arg4_blc_,_arg5_buc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putconboundsliceconst(self,_arg1_first,_arg2_last,_arg3_bkc_,_arg4_blc,_arg5_buc): # 2
      """
      Changes the bounds for a slice of the constraints.
    
      putconboundsliceconst(self,_arg1_first,_arg2_last,_arg3_bkc_,_arg4_blc,_arg5_buc)
        _arg1_first: int. <no description>
        _arg2_last: int. <no description>
        _arg3_bkc: mosek.boundkey. New bound key for all constraints in the slice.
        _arg4_blc: double. New lower bound for all constraints in the slice.
        _arg5_buc: double. New upper bound for all constraints in the slice.
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      if not isinstance(_arg3_bkc_,boundkey): raise TypeError("Argument _arg3_bkc has wrong type")
      _arg3_bkc = _arg3_bkc_
      _arg4_blc_ = _arg4_blc
      _arg5_buc_ = _arg5_buc
      res = self.__obj.putconboundsliceconst(_arg1_first_,_arg2_last_,_arg3_bkc_,_arg4_blc_,_arg5_buc_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putvarbound(self,_arg1_j,_arg2_bkx_,_arg3_blx,_arg4_bux): # 2
      """
      Changes the bounds for one variable.
    
      putvarbound(self,_arg1_j,_arg2_bkx_,_arg3_blx,_arg4_bux)
        _arg1_j: int. Index of the variable.
        _arg2_bkx: mosek.boundkey. New bound key.
        _arg3_blx: double. New lower bound.
        _arg4_bux: double. New upper bound.
      """
      _arg1_j_ = _arg1_j
      if not isinstance(_arg2_bkx_,boundkey): raise TypeError("Argument _arg2_bkx has wrong type")
      _arg2_bkx = _arg2_bkx_
      _arg3_blx_ = _arg3_blx
      _arg4_bux_ = _arg4_bux
      res = self.__obj.putvarbound(_arg1_j_,_arg2_bkx_,_arg3_blx_,_arg4_bux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putvarboundlist(self,_arg2_sub,_arg3_bkx,_arg4_blx,_arg5_bux): # 2
      """
      Changes the bounds of a list of variables.
    
      putvarboundlist(self,_arg2_sub,_arg3_bkx,_arg4_blx,_arg5_bux)
        _arg2_sub: array of int. List of variable indexes.
        _arg3_bkx: array of mosek.boundkey. <no description>
        _arg4_blx: array of double. <no description>
        _arg5_bux: array of double. <no description>
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_sub)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_sub):
        raise IndexError("Inconsistent length of array _arg2_sub")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_bkx)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_bkx):
        raise IndexError("Inconsistent length of array _arg3_bkx")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg4_blx)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg4_blx):
        raise IndexError("Inconsistent length of array _arg4_blx")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg5_bux)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg5_bux):
        raise IndexError("Inconsistent length of array _arg5_bux")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_sub is None: raise TypeError("Invalid type for argument _arg2_sub")
      if _arg2_sub is None:
        _arg2_sub_ = None
      else:
        try:
          _arg2_sub_ = memoryview(_arg2_sub)
        except TypeError:
          try:
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
          except TypeError:
            raise TypeError("Argument _arg2_sub has wrong type")
          else:
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
        else:
          if _arg2_sub_.format != "i":
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
      if _arg3_bkx is None: raise TypeError("Invalid type for argument _arg3_bkx")
      if _arg3_bkx is None:
        _arg3_bkx_ = None
      else:
        try:
          _arg3_bkx_ = memoryview(_arg3_bkx)
        except TypeError:
          try:
            _tmparr__arg3_bkx = (ctypes.c_int*len(_arg3_bkx))()
            _tmparr__arg3_bkx[:] = _arg3_bkx
          except TypeError:
            raise TypeError("Argument _arg3_bkx has wrong type")
          else:
            _arg3_bkx_ = memoryview(_tmparr__arg3_bkx)
      
        else:
          if _arg3_bkx_.format != "i":
            _tmparr__arg3_bkx = (ctypes.c_int*len(_arg3_bkx))()
            _tmparr__arg3_bkx[:] = _arg3_bkx
            _arg3_bkx_ = memoryview(_tmparr__arg3_bkx)
      
      if _arg4_blx is None: raise TypeError("Invalid type for argument _arg4_blx")
      if _arg4_blx is None:
        _arg4_blx_ = None
      else:
        try:
          _arg4_blx_ = memoryview(_arg4_blx)
        except TypeError:
          try:
            _tmparr__arg4_blx = (ctypes.c_double*len(_arg4_blx))()
            _tmparr__arg4_blx[:] = _arg4_blx
          except TypeError:
            raise TypeError("Argument _arg4_blx has wrong type")
          else:
            _arg4_blx_ = memoryview(_tmparr__arg4_blx)
      
        else:
          if _arg4_blx_.format != "d":
            _tmparr__arg4_blx = (ctypes.c_double*len(_arg4_blx))()
            _tmparr__arg4_blx[:] = _arg4_blx
            _arg4_blx_ = memoryview(_tmparr__arg4_blx)
      
      if _arg5_bux is None: raise TypeError("Invalid type for argument _arg5_bux")
      if _arg5_bux is None:
        _arg5_bux_ = None
      else:
        try:
          _arg5_bux_ = memoryview(_arg5_bux)
        except TypeError:
          try:
            _tmparr__arg5_bux = (ctypes.c_double*len(_arg5_bux))()
            _tmparr__arg5_bux[:] = _arg5_bux
          except TypeError:
            raise TypeError("Argument _arg5_bux has wrong type")
          else:
            _arg5_bux_ = memoryview(_tmparr__arg5_bux)
      
        else:
          if _arg5_bux_.format != "d":
            _tmparr__arg5_bux = (ctypes.c_double*len(_arg5_bux))()
            _tmparr__arg5_bux[:] = _arg5_bux
            _arg5_bux_ = memoryview(_tmparr__arg5_bux)
      
      res = self.__obj.putvarboundlist(_arg1_num_,_arg2_sub_,_arg3_bkx_,_arg4_blx_,_arg5_bux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putvarboundlistconst(self,_arg2_sub,_arg3_bkx_,_arg4_blx,_arg5_bux): # 2
      """
      Changes the bounds of a list of variables.
    
      putvarboundlistconst(self,_arg2_sub,_arg3_bkx_,_arg4_blx,_arg5_bux)
        _arg2_sub: array of int. List of variable indexes.
        _arg3_bkx: mosek.boundkey. New bound key for all variables in the list.
        _arg4_blx: double. New lower bound for all variables in the list.
        _arg5_bux: double. New upper bound for all variables in the list.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_sub)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_sub):
        raise IndexError("Inconsistent length of array _arg2_sub")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_sub is None: raise TypeError("Invalid type for argument _arg2_sub")
      if _arg2_sub is None:
        _arg2_sub_ = None
      else:
        try:
          _arg2_sub_ = memoryview(_arg2_sub)
        except TypeError:
          try:
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
          except TypeError:
            raise TypeError("Argument _arg2_sub has wrong type")
          else:
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
        else:
          if _arg2_sub_.format != "i":
            _tmparr__arg2_sub = (ctypes.c_int32*len(_arg2_sub))()
            _tmparr__arg2_sub[:] = _arg2_sub
            _arg2_sub_ = memoryview(_tmparr__arg2_sub)
      
      if not isinstance(_arg3_bkx_,boundkey): raise TypeError("Argument _arg3_bkx has wrong type")
      _arg3_bkx = _arg3_bkx_
      _arg4_blx_ = _arg4_blx
      _arg5_bux_ = _arg5_bux
      res = self.__obj.putvarboundlistconst(_arg1_num_,_arg2_sub_,_arg3_bkx_,_arg4_blx_,_arg5_bux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putvarboundslice(self,_arg1_first,_arg2_last,_arg3_bkx,_arg4_blx,_arg5_bux): # 2
      """
      Changes the bounds for a slice of the variables.
    
      putvarboundslice(self,_arg1_first,_arg2_last,_arg3_bkx,_arg4_blx,_arg5_bux)
        _arg1_first: int. <no description>
        _arg2_last: int. <no description>
        _arg3_bkx: array of mosek.boundkey. <no description>
        _arg4_blx: array of double. <no description>
        _arg5_bux: array of double. <no description>
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      if _arg3_bkx is None: raise TypeError("Invalid type for argument _arg3_bkx")
      if _arg3_bkx is None:
        _arg3_bkx_ = None
      else:
        try:
          _arg3_bkx_ = memoryview(_arg3_bkx)
        except TypeError:
          try:
            _tmparr__arg3_bkx = (ctypes.c_int*len(_arg3_bkx))()
            _tmparr__arg3_bkx[:] = _arg3_bkx
          except TypeError:
            raise TypeError("Argument _arg3_bkx has wrong type")
          else:
            _arg3_bkx_ = memoryview(_tmparr__arg3_bkx)
      
        else:
          if _arg3_bkx_.format != "i":
            _tmparr__arg3_bkx = (ctypes.c_int*len(_arg3_bkx))()
            _tmparr__arg3_bkx[:] = _arg3_bkx
            _arg3_bkx_ = memoryview(_tmparr__arg3_bkx)
      
      if _arg3_bkx_ is not None and len(_arg3_bkx_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg3_bkx has wrong length")
      if _arg4_blx is None: raise TypeError("Invalid type for argument _arg4_blx")
      if _arg4_blx is None:
        _arg4_blx_ = None
      else:
        try:
          _arg4_blx_ = memoryview(_arg4_blx)
        except TypeError:
          try:
            _tmparr__arg4_blx = (ctypes.c_double*len(_arg4_blx))()
            _tmparr__arg4_blx[:] = _arg4_blx
          except TypeError:
            raise TypeError("Argument _arg4_blx has wrong type")
          else:
            _arg4_blx_ = memoryview(_tmparr__arg4_blx)
      
        else:
          if _arg4_blx_.format != "d":
            _tmparr__arg4_blx = (ctypes.c_double*len(_arg4_blx))()
            _tmparr__arg4_blx[:] = _arg4_blx
            _arg4_blx_ = memoryview(_tmparr__arg4_blx)
      
      if _arg4_blx_ is not None and len(_arg4_blx_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg4_blx has wrong length")
      if _arg5_bux is None: raise TypeError("Invalid type for argument _arg5_bux")
      if _arg5_bux is None:
        _arg5_bux_ = None
      else:
        try:
          _arg5_bux_ = memoryview(_arg5_bux)
        except TypeError:
          try:
            _tmparr__arg5_bux = (ctypes.c_double*len(_arg5_bux))()
            _tmparr__arg5_bux[:] = _arg5_bux
          except TypeError:
            raise TypeError("Argument _arg5_bux has wrong type")
          else:
            _arg5_bux_ = memoryview(_tmparr__arg5_bux)
      
        else:
          if _arg5_bux_.format != "d":
            _tmparr__arg5_bux = (ctypes.c_double*len(_arg5_bux))()
            _tmparr__arg5_bux[:] = _arg5_bux
            _arg5_bux_ = memoryview(_tmparr__arg5_bux)
      
      if _arg5_bux_ is not None and len(_arg5_bux_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg5_bux has wrong length")
      res = self.__obj.putvarboundslice(_arg1_first_,_arg2_last_,_arg3_bkx_,_arg4_blx_,_arg5_bux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putvarboundsliceconst(self,_arg1_first,_arg2_last,_arg3_bkx_,_arg4_blx,_arg5_bux): # 2
      """
      Changes the bounds for a slice of the variables.
    
      putvarboundsliceconst(self,_arg1_first,_arg2_last,_arg3_bkx_,_arg4_blx,_arg5_bux)
        _arg1_first: int. <no description>
        _arg2_last: int. <no description>
        _arg3_bkx: mosek.boundkey. New bound key for all variables in the slice.
        _arg4_blx: double. New lower bound for all variables in the slice.
        _arg5_bux: double. New upper bound for all variables in the slice.
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      if not isinstance(_arg3_bkx_,boundkey): raise TypeError("Argument _arg3_bkx has wrong type")
      _arg3_bkx = _arg3_bkx_
      _arg4_blx_ = _arg4_blx
      _arg5_bux_ = _arg5_bux
      res = self.__obj.putvarboundsliceconst(_arg1_first_,_arg2_last_,_arg3_bkx_,_arg4_blx_,_arg5_bux_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putcfix(self,_arg1_cfix): # 2
      """
      Replaces the fixed term in the objective.
    
      putcfix(self,_arg1_cfix)
        _arg1_cfix: double. <no description>
      """
      _arg1_cfix_ = _arg1_cfix
      res = self.__obj.putcfix(_arg1_cfix_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putcj(self,_arg1_j,_arg2_cj): # 2
      """
      Modifies one linear coefficient in the objective.
    
      putcj(self,_arg1_j,_arg2_cj)
        _arg1_j: int. Index of the variable whose objective coefficient should be changed.
        _arg2_cj: double. New coefficient value.
      """
      _arg1_j_ = _arg1_j
      _arg2_cj_ = _arg2_cj
      res = self.__obj.putcj(_arg1_j_,_arg2_cj_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putobjsense(self,_arg1_sense_): # 2
      """
      Sets the objective sense.
    
      putobjsense(self,_arg1_sense_)
        _arg1_sense: mosek.objsense. The objective sense of the task
      """
      if not isinstance(_arg1_sense_,objsense): raise TypeError("Argument _arg1_sense has wrong type")
      _arg1_sense = _arg1_sense_
      res = self.__obj.putobjsense(_arg1_sense_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getobjsense(self): # 2
      """
      Gets the objective sense.
    
      getobjsense(self)
      returns: _arg1_sense
        _arg1_sense: mosek.objsense. The returned objective sense.
      """
      res,resargs = self.__obj.getobjsense()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_sense_return_value = resargs
      __arg1_sense_return_value = objsense(__arg1_sense_return_value)
      return __arg1_sense_return_value
    
    def putclist(self,_arg2_subj,_arg3_val): # 2
      """
      Modifies a part of the linear objective coefficients.
    
      putclist(self,_arg2_subj,_arg3_val)
        _arg2_subj: array of int. Indices of variables for which objective coefficients should be changed.
        _arg3_val: array of double. New numerical values for the objective coefficients that should be modified.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subj)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subj):
        raise IndexError("Inconsistent length of array _arg2_subj")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_val)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_val):
        raise IndexError("Inconsistent length of array _arg3_val")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subj is None: raise TypeError("Invalid type for argument _arg2_subj")
      if _arg2_subj is None:
        _arg2_subj_ = None
      else:
        try:
          _arg2_subj_ = memoryview(_arg2_subj)
        except TypeError:
          try:
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
          except TypeError:
            raise TypeError("Argument _arg2_subj has wrong type")
          else:
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
        else:
          if _arg2_subj_.format != "i":
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
      if _arg3_val is None: raise TypeError("Invalid type for argument _arg3_val")
      if _arg3_val is None:
        _arg3_val_ = None
      else:
        try:
          _arg3_val_ = memoryview(_arg3_val)
        except TypeError:
          try:
            _tmparr__arg3_val = (ctypes.c_double*len(_arg3_val))()
            _tmparr__arg3_val[:] = _arg3_val
          except TypeError:
            raise TypeError("Argument _arg3_val has wrong type")
          else:
            _arg3_val_ = memoryview(_tmparr__arg3_val)
      
        else:
          if _arg3_val_.format != "d":
            _tmparr__arg3_val = (ctypes.c_double*len(_arg3_val))()
            _tmparr__arg3_val[:] = _arg3_val
            _arg3_val_ = memoryview(_tmparr__arg3_val)
      
      res = self.__obj.putclist(_arg1_num_,_arg2_subj_,_arg3_val_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putcslice(self,_arg1_first,_arg2_last,_arg3_slice): # 2
      """
      Modifies a slice of the linear objective coefficients.
    
      putcslice(self,_arg1_first,_arg2_last,_arg3_slice)
        _arg1_first: int. First element in the slice of c.
        _arg2_last: int. Last element plus 1 of the slice in c to be changed.
        _arg3_slice: array of double. New numerical values for the objective coefficients that should be modified.
      """
      _arg1_first_ = _arg1_first
      _arg2_last_ = _arg2_last
      if _arg3_slice is None: raise TypeError("Invalid type for argument _arg3_slice")
      if _arg3_slice is None:
        _arg3_slice_ = None
      else:
        try:
          _arg3_slice_ = memoryview(_arg3_slice)
        except TypeError:
          try:
            _tmparr__arg3_slice = (ctypes.c_double*len(_arg3_slice))()
            _tmparr__arg3_slice[:] = _arg3_slice
          except TypeError:
            raise TypeError("Argument _arg3_slice has wrong type")
          else:
            _arg3_slice_ = memoryview(_tmparr__arg3_slice)
      
        else:
          if _arg3_slice_.format != "d":
            _tmparr__arg3_slice = (ctypes.c_double*len(_arg3_slice))()
            _tmparr__arg3_slice[:] = _arg3_slice
            _arg3_slice_ = memoryview(_tmparr__arg3_slice)
      
      if _arg3_slice_ is not None and len(_arg3_slice_) != ( _arg2_last  -  _arg1_first ):
        raise ValueError("Array argument _arg3_slice has wrong length")
      res = self.__obj.putcslice(_arg1_first_,_arg2_last_,_arg3_slice_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putbarcj(self,_arg1_j,_arg3_sub,_arg4_weights): # 2
      """
      Changes one element in barc.
    
      putbarcj(self,_arg1_j,_arg3_sub,_arg4_weights)
        _arg1_j: int. Index of the element in barc` that should be changed.
        _arg3_sub: array of long. sub is list of indexes of those symmetric matrices appearing in sum.
        _arg4_weights: array of double. The weights of the terms in the weighted sum.
      """
      _arg1_j_ = _arg1_j
      _arg2_num_ = None
      if _arg2_num_ is None:
        _arg2_num_ = len(_arg3_sub)
        _arg2_num = _arg2_num_
      elif _arg2_num_ != len(_arg3_sub):
        raise IndexError("Inconsistent length of array _arg3_sub")
      if _arg2_num_ is None:
        _arg2_num_ = len(_arg4_weights)
        _arg2_num = _arg2_num_
      elif _arg2_num_ != len(_arg4_weights):
        raise IndexError("Inconsistent length of array _arg4_weights")
      if _arg2_num_ is None: _arg2_num_ = 0
      if _arg3_sub is None: raise TypeError("Invalid type for argument _arg3_sub")
      if _arg3_sub is None:
        _arg3_sub_ = None
      else:
        try:
          _arg3_sub_ = memoryview(_arg3_sub)
        except TypeError:
          try:
            _tmparr__arg3_sub = (ctypes.c_int64*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
          except TypeError:
            raise TypeError("Argument _arg3_sub has wrong type")
          else:
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
        else:
          if _arg3_sub_.format != "q":
            _tmparr__arg3_sub = (ctypes.c_int64*len(_arg3_sub))()
            _tmparr__arg3_sub[:] = _arg3_sub
            _arg3_sub_ = memoryview(_tmparr__arg3_sub)
      
      if _arg4_weights is None: raise TypeError("Invalid type for argument _arg4_weights")
      if _arg4_weights is None:
        _arg4_weights_ = None
      else:
        try:
          _arg4_weights_ = memoryview(_arg4_weights)
        except TypeError:
          try:
            _tmparr__arg4_weights = (ctypes.c_double*len(_arg4_weights))()
            _tmparr__arg4_weights[:] = _arg4_weights
          except TypeError:
            raise TypeError("Argument _arg4_weights has wrong type")
          else:
            _arg4_weights_ = memoryview(_tmparr__arg4_weights)
      
        else:
          if _arg4_weights_.format != "d":
            _tmparr__arg4_weights = (ctypes.c_double*len(_arg4_weights))()
            _tmparr__arg4_weights[:] = _arg4_weights
            _arg4_weights_ = memoryview(_tmparr__arg4_weights)
      
      res = self.__obj.putbarcj(_arg1_j_,_arg2_num_,_arg3_sub_,_arg4_weights_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putcone(self,_arg1_k,_arg2_ct_,_arg3_conepar,_arg5_submem): # 2
      """
      Replaces a conic constraint.
    
      putcone(self,_arg1_k,_arg2_ct_,_arg3_conepar,_arg5_submem)
        _arg1_k: int. Index of the cone.
        _arg2_ct: mosek.conetype. <no description>
        _arg3_conepar: double. <no description>
        _arg5_submem: array of int. <no description>
      """
      _arg1_k_ = _arg1_k
      if not isinstance(_arg2_ct_,conetype): raise TypeError("Argument _arg2_ct has wrong type")
      _arg2_ct = _arg2_ct_
      _arg3_conepar_ = _arg3_conepar
      _arg4_nummem_ = None
      if _arg4_nummem_ is None:
        _arg4_nummem_ = len(_arg5_submem)
        _arg4_nummem = _arg4_nummem_
      elif _arg4_nummem_ != len(_arg5_submem):
        raise IndexError("Inconsistent length of array _arg5_submem")
      if _arg4_nummem_ is None: _arg4_nummem_ = 0
      if _arg5_submem is None: raise TypeError("Invalid type for argument _arg5_submem")
      if _arg5_submem is None:
        _arg5_submem_ = None
      else:
        try:
          _arg5_submem_ = memoryview(_arg5_submem)
        except TypeError:
          try:
            _tmparr__arg5_submem = (ctypes.c_int32*len(_arg5_submem))()
            _tmparr__arg5_submem[:] = _arg5_submem
          except TypeError:
            raise TypeError("Argument _arg5_submem has wrong type")
          else:
            _arg5_submem_ = memoryview(_tmparr__arg5_submem)
      
        else:
          if _arg5_submem_.format != "i":
            _tmparr__arg5_submem = (ctypes.c_int32*len(_arg5_submem))()
            _tmparr__arg5_submem[:] = _arg5_submem
            _arg5_submem_ = memoryview(_tmparr__arg5_submem)
      
      res = self.__obj.putcone(_arg1_k_,_arg2_ct_,_arg3_conepar_,_arg4_nummem_,_arg5_submem_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def appendsparsesymmat(self,_arg1_dim,_arg3_subi,_arg4_subj,_arg5_valij): # 2
      """
      Appends a general sparse symmetric matrix to the storage of symmetric matrices.
    
      appendsparsesymmat(self,_arg1_dim,_arg3_subi,_arg4_subj,_arg5_valij)
        _arg1_dim: int. Dimension of the symmetric matrix that is appended.
        _arg3_subi: array of int. Row subscript in the triplets.
        _arg4_subj: array of int. Column subscripts in the triplets.
        _arg5_valij: array of double. Values of each triplet.
      returns: _arg6_idx
        _arg6_idx: long. Unique index assigned to the inputted matrix.
      """
      _arg1_dim_ = _arg1_dim
      _arg2_nz_ = None
      if _arg2_nz_ is None:
        _arg2_nz_ = len(_arg3_subi)
        _arg2_nz = _arg2_nz_
      elif _arg2_nz_ != len(_arg3_subi):
        raise IndexError("Inconsistent length of array _arg3_subi")
      if _arg2_nz_ is None:
        _arg2_nz_ = len(_arg4_subj)
        _arg2_nz = _arg2_nz_
      elif _arg2_nz_ != len(_arg4_subj):
        raise IndexError("Inconsistent length of array _arg4_subj")
      if _arg2_nz_ is None:
        _arg2_nz_ = len(_arg5_valij)
        _arg2_nz = _arg2_nz_
      elif _arg2_nz_ != len(_arg5_valij):
        raise IndexError("Inconsistent length of array _arg5_valij")
      if _arg2_nz_ is None: _arg2_nz_ = 0
      if _arg3_subi is None: raise TypeError("Invalid type for argument _arg3_subi")
      if _arg3_subi is None:
        _arg3_subi_ = None
      else:
        try:
          _arg3_subi_ = memoryview(_arg3_subi)
        except TypeError:
          try:
            _tmparr__arg3_subi = (ctypes.c_int32*len(_arg3_subi))()
            _tmparr__arg3_subi[:] = _arg3_subi
          except TypeError:
            raise TypeError("Argument _arg3_subi has wrong type")
          else:
            _arg3_subi_ = memoryview(_tmparr__arg3_subi)
      
        else:
          if _arg3_subi_.format != "i":
            _tmparr__arg3_subi = (ctypes.c_int32*len(_arg3_subi))()
            _tmparr__arg3_subi[:] = _arg3_subi
            _arg3_subi_ = memoryview(_tmparr__arg3_subi)
      
      if _arg4_subj is None: raise TypeError("Invalid type for argument _arg4_subj")
      if _arg4_subj is None:
        _arg4_subj_ = None
      else:
        try:
          _arg4_subj_ = memoryview(_arg4_subj)
        except TypeError:
          try:
            _tmparr__arg4_subj = (ctypes.c_int32*len(_arg4_subj))()
            _tmparr__arg4_subj[:] = _arg4_subj
          except TypeError:
            raise TypeError("Argument _arg4_subj has wrong type")
          else:
            _arg4_subj_ = memoryview(_tmparr__arg4_subj)
      
        else:
          if _arg4_subj_.format != "i":
            _tmparr__arg4_subj = (ctypes.c_int32*len(_arg4_subj))()
            _tmparr__arg4_subj[:] = _arg4_subj
            _arg4_subj_ = memoryview(_tmparr__arg4_subj)
      
      if _arg5_valij is None: raise TypeError("Invalid type for argument _arg5_valij")
      if _arg5_valij is None:
        _arg5_valij_ = None
      else:
        try:
          _arg5_valij_ = memoryview(_arg5_valij)
        except TypeError:
          try:
            _tmparr__arg5_valij = (ctypes.c_double*len(_arg5_valij))()
            _tmparr__arg5_valij[:] = _arg5_valij
          except TypeError:
            raise TypeError("Argument _arg5_valij has wrong type")
          else:
            _arg5_valij_ = memoryview(_tmparr__arg5_valij)
      
        else:
          if _arg5_valij_.format != "d":
            _tmparr__arg5_valij = (ctypes.c_double*len(_arg5_valij))()
            _tmparr__arg5_valij[:] = _arg5_valij
            _arg5_valij_ = memoryview(_tmparr__arg5_valij)
      
      res,resargs = self.__obj.appendsparsesymmat(_arg1_dim_,_arg2_nz_,_arg3_subi_,_arg4_subj_,_arg5_valij_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg6_idx_return_value = resargs
      return __arg6_idx_return_value
    
    def appendsparsesymmatlist(self,_arg2_dims,_arg3_nz,_arg4_subi,_arg5_subj,_arg6_valij,_arg7_idx): # 2
      """
      Appends a general sparse symmetric matrix to the storage of symmetric matrices.
    
      appendsparsesymmatlist(self,_arg2_dims,_arg3_nz,_arg4_subi,_arg5_subj,_arg6_valij,_arg7_idx)
        _arg2_dims: array of int. Dimensions of the symmetric matrixes.
        _arg3_nz: array of long. Number of nonzeros for each matrix.
        _arg4_subi: array of int. Row subscript in the triplets.
        _arg5_subj: array of int. Column subscripts in the triplets.
        _arg6_valij: array of double. Values of each triplet.
        _arg7_idx: array of long. Unique index assigned to the inputted matrix.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_dims)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_dims):
        raise IndexError("Inconsistent length of array _arg2_dims")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_nz)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_nz):
        raise IndexError("Inconsistent length of array _arg3_nz")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_dims is None: raise TypeError("Invalid type for argument _arg2_dims")
      if _arg2_dims is None:
        _arg2_dims_ = None
      else:
        try:
          _arg2_dims_ = memoryview(_arg2_dims)
        except TypeError:
          try:
            _tmparr__arg2_dims = (ctypes.c_int32*len(_arg2_dims))()
            _tmparr__arg2_dims[:] = _arg2_dims
          except TypeError:
            raise TypeError("Argument _arg2_dims has wrong type")
          else:
            _arg2_dims_ = memoryview(_tmparr__arg2_dims)
      
        else:
          if _arg2_dims_.format != "i":
            _tmparr__arg2_dims = (ctypes.c_int32*len(_arg2_dims))()
            _tmparr__arg2_dims[:] = _arg2_dims
            _arg2_dims_ = memoryview(_tmparr__arg2_dims)
      
      if _arg3_nz is None: raise TypeError("Invalid type for argument _arg3_nz")
      if _arg3_nz is None:
        _arg3_nz_ = None
      else:
        try:
          _arg3_nz_ = memoryview(_arg3_nz)
        except TypeError:
          try:
            _tmparr__arg3_nz = (ctypes.c_int64*len(_arg3_nz))()
            _tmparr__arg3_nz[:] = _arg3_nz
          except TypeError:
            raise TypeError("Argument _arg3_nz has wrong type")
          else:
            _arg3_nz_ = memoryview(_tmparr__arg3_nz)
      
        else:
          if _arg3_nz_.format != "q":
            _tmparr__arg3_nz = (ctypes.c_int64*len(_arg3_nz))()
            _tmparr__arg3_nz[:] = _arg3_nz
            _arg3_nz_ = memoryview(_tmparr__arg3_nz)
      
      if _arg4_subi is None: raise TypeError("Invalid type for argument _arg4_subi")
      if _arg4_subi is None:
        _arg4_subi_ = None
      else:
        try:
          _arg4_subi_ = memoryview(_arg4_subi)
        except TypeError:
          try:
            _tmparr__arg4_subi = (ctypes.c_int32*len(_arg4_subi))()
            _tmparr__arg4_subi[:] = _arg4_subi
          except TypeError:
            raise TypeError("Argument _arg4_subi has wrong type")
          else:
            _arg4_subi_ = memoryview(_tmparr__arg4_subi)
      
        else:
          if _arg4_subi_.format != "i":
            _tmparr__arg4_subi = (ctypes.c_int32*len(_arg4_subi))()
            _tmparr__arg4_subi[:] = _arg4_subi
            _arg4_subi_ = memoryview(_tmparr__arg4_subi)
      
      if _arg4_subi_ is not None and len(_arg4_subi_) != sum( _arg3_nz ):
        raise ValueError("Array argument _arg4_subi has wrong length")
      if _arg5_subj is None: raise TypeError("Invalid type for argument _arg5_subj")
      if _arg5_subj is None:
        _arg5_subj_ = None
      else:
        try:
          _arg5_subj_ = memoryview(_arg5_subj)
        except TypeError:
          try:
            _tmparr__arg5_subj = (ctypes.c_int32*len(_arg5_subj))()
            _tmparr__arg5_subj[:] = _arg5_subj
          except TypeError:
            raise TypeError("Argument _arg5_subj has wrong type")
          else:
            _arg5_subj_ = memoryview(_tmparr__arg5_subj)
      
        else:
          if _arg5_subj_.format != "i":
            _tmparr__arg5_subj = (ctypes.c_int32*len(_arg5_subj))()
            _tmparr__arg5_subj[:] = _arg5_subj
            _arg5_subj_ = memoryview(_tmparr__arg5_subj)
      
      if _arg5_subj_ is not None and len(_arg5_subj_) != sum( _arg3_nz ):
        raise ValueError("Array argument _arg5_subj has wrong length")
      if _arg6_valij is None: raise TypeError("Invalid type for argument _arg6_valij")
      if _arg6_valij is None:
        _arg6_valij_ = None
      else:
        try:
          _arg6_valij_ = memoryview(_arg6_valij)
        except TypeError:
          try:
            _tmparr__arg6_valij = (ctypes.c_double*len(_arg6_valij))()
            _tmparr__arg6_valij[:] = _arg6_valij
          except TypeError:
            raise TypeError("Argument _arg6_valij has wrong type")
          else:
            _arg6_valij_ = memoryview(_tmparr__arg6_valij)
      
        else:
          if _arg6_valij_.format != "d":
            _tmparr__arg6_valij = (ctypes.c_double*len(_arg6_valij))()
            _tmparr__arg6_valij[:] = _arg6_valij
            _arg6_valij_ = memoryview(_tmparr__arg6_valij)
      
      if _arg6_valij_ is not None and len(_arg6_valij_) != sum( _arg3_nz ):
        raise ValueError("Array argument _arg6_valij has wrong length")
      if _arg7_idx is None: raise TypeError("Invalid type for argument _arg7_idx")
      _copyback__arg7_idx = False
      if _arg7_idx is None:
        _arg7_idx_ = None
      else:
        try:
          _arg7_idx_ = memoryview(_arg7_idx)
        except TypeError:
          try:
            _tmparr__arg7_idx = (ctypes.c_int64*len(_arg7_idx))()
            _tmparr__arg7_idx[:] = _arg7_idx
          except TypeError:
            raise TypeError("Argument _arg7_idx has wrong type")
          else:
            _arg7_idx_ = memoryview(_tmparr__arg7_idx)
            _copyback__arg7_idx = True
        else:
          if _arg7_idx_.format != "q":
            _tmparr__arg7_idx = (ctypes.c_int64*len(_arg7_idx))()
            _tmparr__arg7_idx[:] = _arg7_idx
            _arg7_idx_ = memoryview(_tmparr__arg7_idx)
            _copyback__arg7_idx = True
      if _arg7_idx_ is not None and len(_arg7_idx_) !=  _arg1_num :
        raise ValueError("Array argument _arg7_idx has wrong length")
      res = self.__obj.appendsparsesymmatlist(_arg1_num_,_arg2_dims_,_arg3_nz_,_arg4_subi_,_arg5_subj_,_arg6_valij_,_arg7_idx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg7_idx:
        _arg7_idx[:] = _tmparr__arg7_idx
    
    def getsymmatinfo(self,_arg1_idx): # 2
      """
      Obtains information about a matrix from the symmetric matrix storage.
    
      getsymmatinfo(self,_arg1_idx)
        _arg1_idx: long. Index of the matrix for which information is requested.
      returns: _arg2_dim,_arg3_nz,_arg4_type
        _arg2_dim: int. Returns the dimension of the requested matrix.
        _arg3_nz: long. Returns the number of non-zeros in the requested matrix.
        _arg4_type: mosek.symmattype. Returns the type of the requested matrix.
      """
      _arg1_idx_ = _arg1_idx
      res,resargs = self.__obj.getsymmatinfo(_arg1_idx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_dim_return_value,__arg3_nz_return_value,__arg4_type_return_value = resargs
      __arg4_type_return_value = symmattype(__arg4_type_return_value)
      return __arg2_dim_return_value,__arg3_nz_return_value,__arg4_type_return_value
    
    def getnumsymmat(self): # 2
      """
      Obtains the number of symmetric matrices stored.
    
      getnumsymmat(self)
      returns: _arg1_num
        _arg1_num: long. The number of symmetric sparse matrices.
      """
      res,resargs = self.__obj.getnumsymmat()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_num_return_value = resargs
      return __arg1_num_return_value
    
    def getsparsesymmat(self,_arg1_idx,_arg3_subi,_arg4_subj,_arg5_valij): # 2
      """
      Gets a single symmetric matrix from the matrix store.
    
      getsparsesymmat(self,_arg1_idx,_arg3_subi,_arg4_subj,_arg5_valij)
        _arg1_idx: long. Index of the matrix to retrieve.
        _arg3_subi: array of int. Row subscripts of the matrix non-zero elements.
        _arg4_subj: array of int. Column subscripts of the matrix non-zero elements.
        _arg5_valij: array of double. Coefficients of the matrix non-zero elements.
      """
      _arg1_idx_ = _arg1_idx
      _arg2_maxlen_ = self.getsymmatinfo( _arg1_idx )[1]
      _arg2_maxlen = _arg2_maxlen_
      _copyback__arg3_subi = False
      if _arg3_subi is None:
        _arg3_subi_ = None
      else:
        try:
          _arg3_subi_ = memoryview(_arg3_subi)
        except TypeError:
          try:
            _tmparr__arg3_subi = (ctypes.c_int32*len(_arg3_subi))()
            _tmparr__arg3_subi[:] = _arg3_subi
          except TypeError:
            raise TypeError("Argument _arg3_subi has wrong type")
          else:
            _arg3_subi_ = memoryview(_tmparr__arg3_subi)
            _copyback__arg3_subi = True
        else:
          if _arg3_subi_.format != "i":
            _tmparr__arg3_subi = (ctypes.c_int32*len(_arg3_subi))()
            _tmparr__arg3_subi[:] = _arg3_subi
            _arg3_subi_ = memoryview(_tmparr__arg3_subi)
            _copyback__arg3_subi = True
      if _arg3_subi_ is not None and len(_arg3_subi_) !=  _arg2_maxlen :
        raise ValueError("Array argument _arg3_subi has wrong length")
      _copyback__arg4_subj = False
      if _arg4_subj is None:
        _arg4_subj_ = None
      else:
        try:
          _arg4_subj_ = memoryview(_arg4_subj)
        except TypeError:
          try:
            _tmparr__arg4_subj = (ctypes.c_int32*len(_arg4_subj))()
            _tmparr__arg4_subj[:] = _arg4_subj
          except TypeError:
            raise TypeError("Argument _arg4_subj has wrong type")
          else:
            _arg4_subj_ = memoryview(_tmparr__arg4_subj)
            _copyback__arg4_subj = True
        else:
          if _arg4_subj_.format != "i":
            _tmparr__arg4_subj = (ctypes.c_int32*len(_arg4_subj))()
            _tmparr__arg4_subj[:] = _arg4_subj
            _arg4_subj_ = memoryview(_tmparr__arg4_subj)
            _copyback__arg4_subj = True
      if _arg4_subj_ is not None and len(_arg4_subj_) !=  _arg2_maxlen :
        raise ValueError("Array argument _arg4_subj has wrong length")
      _copyback__arg5_valij = False
      if _arg5_valij is None:
        _arg5_valij_ = None
      else:
        try:
          _arg5_valij_ = memoryview(_arg5_valij)
        except TypeError:
          try:
            _tmparr__arg5_valij = (ctypes.c_double*len(_arg5_valij))()
            _tmparr__arg5_valij[:] = _arg5_valij
          except TypeError:
            raise TypeError("Argument _arg5_valij has wrong type")
          else:
            _arg5_valij_ = memoryview(_tmparr__arg5_valij)
            _copyback__arg5_valij = True
        else:
          if _arg5_valij_.format != "d":
            _tmparr__arg5_valij = (ctypes.c_double*len(_arg5_valij))()
            _tmparr__arg5_valij[:] = _arg5_valij
            _arg5_valij_ = memoryview(_tmparr__arg5_valij)
            _copyback__arg5_valij = True
      if _arg5_valij_ is not None and len(_arg5_valij_) !=  _arg2_maxlen :
        raise ValueError("Array argument _arg5_valij has wrong length")
      res = self.__obj.getsparsesymmat(_arg1_idx_,_arg2_maxlen_,_arg3_subi_,_arg4_subj_,_arg5_valij_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg5_valij:
        _arg5_valij[:] = _tmparr__arg5_valij
      if _copyback__arg4_subj:
        _arg4_subj[:] = _tmparr__arg4_subj
      if _copyback__arg3_subi:
        _arg3_subi[:] = _tmparr__arg3_subi
    
    def putdouparam(self,_arg1_param_,_arg2_parvalue): # 2
      """
      Sets a double parameter.
    
      putdouparam(self,_arg1_param_,_arg2_parvalue)
        _arg1_param: mosek.dparam. <no description>
        _arg2_parvalue: double. <no description>
      """
      if not isinstance(_arg1_param_,dparam): raise TypeError("Argument _arg1_param has wrong type")
      _arg1_param = _arg1_param_
      _arg2_parvalue_ = _arg2_parvalue
      res = self.__obj.putdouparam(_arg1_param_,_arg2_parvalue_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putintparam(self,_arg1_param_,_arg2_parvalue): # 2
      """
      Sets an integer parameter.
    
      putintparam(self,_arg1_param_,_arg2_parvalue)
        _arg1_param: mosek.iparam. <no description>
        _arg2_parvalue: int. <no description>
      """
      if not isinstance(_arg1_param_,iparam): raise TypeError("Argument _arg1_param has wrong type")
      _arg1_param = _arg1_param_
      _arg2_parvalue_ = _arg2_parvalue
      res = self.__obj.putintparam(_arg1_param_,_arg2_parvalue_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putmaxnumcon(self,_arg1_maxnumcon): # 2
      """
      Sets the number of preallocated constraints in the optimization task.
    
      putmaxnumcon(self,_arg1_maxnumcon)
        _arg1_maxnumcon: int. <no description>
      """
      _arg1_maxnumcon_ = _arg1_maxnumcon
      res = self.__obj.putmaxnumcon(_arg1_maxnumcon_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putmaxnumcone(self,_arg1_maxnumcone): # 2
      """
      Sets the number of preallocated conic constraints in the optimization task.
    
      putmaxnumcone(self,_arg1_maxnumcone)
        _arg1_maxnumcone: int. <no description>
      """
      _arg1_maxnumcone_ = _arg1_maxnumcone
      res = self.__obj.putmaxnumcone(_arg1_maxnumcone_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getmaxnumcone(self): # 2
      """
      Obtains the number of preallocated cones in the optimization task.
    
      getmaxnumcone(self)
      returns: _arg1_maxnumcone
        _arg1_maxnumcone: int. <no description>
      """
      res,resargs = self.__obj.getmaxnumcone()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_maxnumcone_return_value = resargs
      return __arg1_maxnumcone_return_value
    
    def putmaxnumvar(self,_arg1_maxnumvar): # 2
      """
      Sets the number of preallocated variables in the optimization task.
    
      putmaxnumvar(self,_arg1_maxnumvar)
        _arg1_maxnumvar: int. <no description>
      """
      _arg1_maxnumvar_ = _arg1_maxnumvar
      res = self.__obj.putmaxnumvar(_arg1_maxnumvar_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putmaxnumbarvar(self,_arg1_maxnumbarvar): # 2
      """
      Sets the number of preallocated symmetric matrix variables.
    
      putmaxnumbarvar(self,_arg1_maxnumbarvar)
        _arg1_maxnumbarvar: int. Number of preallocated symmetric matrix variables.
      """
      _arg1_maxnumbarvar_ = _arg1_maxnumbarvar
      res = self.__obj.putmaxnumbarvar(_arg1_maxnumbarvar_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putmaxnumanz(self,_arg1_maxnumanz): # 2
      """
      Sets the number of preallocated non-zero entries in the linear coefficient matrix.
    
      putmaxnumanz(self,_arg1_maxnumanz)
        _arg1_maxnumanz: long. New size of the storage reserved for storing the linear coefficient matrix.
      """
      _arg1_maxnumanz_ = _arg1_maxnumanz
      res = self.__obj.putmaxnumanz(_arg1_maxnumanz_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putmaxnumqnz(self,_arg1_maxnumqnz): # 2
      """
      Sets the number of preallocated non-zero entries in quadratic terms.
    
      putmaxnumqnz(self,_arg1_maxnumqnz)
        _arg1_maxnumqnz: long. <no description>
      """
      _arg1_maxnumqnz_ = _arg1_maxnumqnz
      res = self.__obj.putmaxnumqnz(_arg1_maxnumqnz_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getmaxnumqnz(self): # 2
      """
      Obtains the number of preallocated non-zeros for all quadratic terms in objective and constraints.
    
      getmaxnumqnz(self)
      returns: _arg1_maxnumqnz
        _arg1_maxnumqnz: long. <no description>
      """
      res,resargs = self.__obj.getmaxnumqnz64()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_maxnumqnz_return_value = resargs
      return __arg1_maxnumqnz_return_value
    
    def putnadouparam(self,_arg1_paramname_,_arg2_parvalue): # 2
      """
      Sets a double parameter.
    
      putnadouparam(self,_arg1_paramname_,_arg2_parvalue)
        _arg1_paramname: str. <no description>
        _arg2_parvalue: double. <no description>
      """
      _arg2_parvalue_ = _arg2_parvalue
      res = self.__obj.putnadouparam(_arg1_paramname_,_arg2_parvalue_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putnaintparam(self,_arg1_paramname_,_arg2_parvalue): # 2
      """
      Sets an integer parameter.
    
      putnaintparam(self,_arg1_paramname_,_arg2_parvalue)
        _arg1_paramname: str. <no description>
        _arg2_parvalue: int. <no description>
      """
      _arg2_parvalue_ = _arg2_parvalue
      res = self.__obj.putnaintparam(_arg1_paramname_,_arg2_parvalue_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putnastrparam(self,_arg1_paramname_,_arg2_parvalue_): # 2
      """
      Sets a string parameter.
    
      putnastrparam(self,_arg1_paramname_,_arg2_parvalue_)
        _arg1_paramname: str. <no description>
        _arg2_parvalue: str. <no description>
      """
      res = self.__obj.putnastrparam(_arg1_paramname_,_arg2_parvalue_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putobjname(self,_arg1_objname_): # 2
      """
      Assigns a new name to the objective.
    
      putobjname(self,_arg1_objname_)
        _arg1_objname: str. <no description>
      """
      res = self.__obj.putobjname(_arg1_objname_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putparam(self,_arg1_parname_,_arg2_parvalue_): # 2
      """
      Modifies the value of parameter.
    
      putparam(self,_arg1_parname_,_arg2_parvalue_)
        _arg1_parname: str. <no description>
        _arg2_parvalue: str. <no description>
      """
      res = self.__obj.putparam(_arg1_parname_,_arg2_parvalue_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putqcon(self,_arg2_qcsubk,_arg3_qcsubi,_arg4_qcsubj,_arg5_qcval): # 2
      """
      Replaces all quadratic terms in constraints.
    
      putqcon(self,_arg2_qcsubk,_arg3_qcsubi,_arg4_qcsubj,_arg5_qcval)
        _arg2_qcsubk: array of int. <no description>
        _arg3_qcsubi: array of int. <no description>
        _arg4_qcsubj: array of int. <no description>
        _arg5_qcval: array of double. <no description>
      """
      _arg1_numqcnz_ = None
      if _arg1_numqcnz_ is None:
        _arg1_numqcnz_ = len(_arg3_qcsubi)
        _arg1_numqcnz = _arg1_numqcnz_
      elif _arg1_numqcnz_ != len(_arg3_qcsubi):
        raise IndexError("Inconsistent length of array _arg3_qcsubi")
      if _arg1_numqcnz_ is None:
        _arg1_numqcnz_ = len(_arg4_qcsubj)
        _arg1_numqcnz = _arg1_numqcnz_
      elif _arg1_numqcnz_ != len(_arg4_qcsubj):
        raise IndexError("Inconsistent length of array _arg4_qcsubj")
      if _arg1_numqcnz_ is None:
        _arg1_numqcnz_ = len(_arg5_qcval)
        _arg1_numqcnz = _arg1_numqcnz_
      elif _arg1_numqcnz_ != len(_arg5_qcval):
        raise IndexError("Inconsistent length of array _arg5_qcval")
      if _arg1_numqcnz_ is None: _arg1_numqcnz_ = 0
      if _arg2_qcsubk is None: raise TypeError("Invalid type for argument _arg2_qcsubk")
      if _arg2_qcsubk is None:
        _arg2_qcsubk_ = None
      else:
        try:
          _arg2_qcsubk_ = memoryview(_arg2_qcsubk)
        except TypeError:
          try:
            _tmparr__arg2_qcsubk = (ctypes.c_int32*len(_arg2_qcsubk))()
            _tmparr__arg2_qcsubk[:] = _arg2_qcsubk
          except TypeError:
            raise TypeError("Argument _arg2_qcsubk has wrong type")
          else:
            _arg2_qcsubk_ = memoryview(_tmparr__arg2_qcsubk)
      
        else:
          if _arg2_qcsubk_.format != "i":
            _tmparr__arg2_qcsubk = (ctypes.c_int32*len(_arg2_qcsubk))()
            _tmparr__arg2_qcsubk[:] = _arg2_qcsubk
            _arg2_qcsubk_ = memoryview(_tmparr__arg2_qcsubk)
      
      if _arg3_qcsubi is None: raise TypeError("Invalid type for argument _arg3_qcsubi")
      if _arg3_qcsubi is None:
        _arg3_qcsubi_ = None
      else:
        try:
          _arg3_qcsubi_ = memoryview(_arg3_qcsubi)
        except TypeError:
          try:
            _tmparr__arg3_qcsubi = (ctypes.c_int32*len(_arg3_qcsubi))()
            _tmparr__arg3_qcsubi[:] = _arg3_qcsubi
          except TypeError:
            raise TypeError("Argument _arg3_qcsubi has wrong type")
          else:
            _arg3_qcsubi_ = memoryview(_tmparr__arg3_qcsubi)
      
        else:
          if _arg3_qcsubi_.format != "i":
            _tmparr__arg3_qcsubi = (ctypes.c_int32*len(_arg3_qcsubi))()
            _tmparr__arg3_qcsubi[:] = _arg3_qcsubi
            _arg3_qcsubi_ = memoryview(_tmparr__arg3_qcsubi)
      
      if _arg4_qcsubj is None: raise TypeError("Invalid type for argument _arg4_qcsubj")
      if _arg4_qcsubj is None:
        _arg4_qcsubj_ = None
      else:
        try:
          _arg4_qcsubj_ = memoryview(_arg4_qcsubj)
        except TypeError:
          try:
            _tmparr__arg4_qcsubj = (ctypes.c_int32*len(_arg4_qcsubj))()
            _tmparr__arg4_qcsubj[:] = _arg4_qcsubj
          except TypeError:
            raise TypeError("Argument _arg4_qcsubj has wrong type")
          else:
            _arg4_qcsubj_ = memoryview(_tmparr__arg4_qcsubj)
      
        else:
          if _arg4_qcsubj_.format != "i":
            _tmparr__arg4_qcsubj = (ctypes.c_int32*len(_arg4_qcsubj))()
            _tmparr__arg4_qcsubj[:] = _arg4_qcsubj
            _arg4_qcsubj_ = memoryview(_tmparr__arg4_qcsubj)
      
      if _arg5_qcval is None: raise TypeError("Invalid type for argument _arg5_qcval")
      if _arg5_qcval is None:
        _arg5_qcval_ = None
      else:
        try:
          _arg5_qcval_ = memoryview(_arg5_qcval)
        except TypeError:
          try:
            _tmparr__arg5_qcval = (ctypes.c_double*len(_arg5_qcval))()
            _tmparr__arg5_qcval[:] = _arg5_qcval
          except TypeError:
            raise TypeError("Argument _arg5_qcval has wrong type")
          else:
            _arg5_qcval_ = memoryview(_tmparr__arg5_qcval)
      
        else:
          if _arg5_qcval_.format != "d":
            _tmparr__arg5_qcval = (ctypes.c_double*len(_arg5_qcval))()
            _tmparr__arg5_qcval[:] = _arg5_qcval
            _arg5_qcval_ = memoryview(_tmparr__arg5_qcval)
      
      res = self.__obj.putqcon(_arg1_numqcnz_,_arg2_qcsubk_,_arg3_qcsubi_,_arg4_qcsubj_,_arg5_qcval_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putqconk(self,_arg1_k,_arg3_qcsubi,_arg4_qcsubj,_arg5_qcval): # 2
      """
      Replaces all quadratic terms in a single constraint.
    
      putqconk(self,_arg1_k,_arg3_qcsubi,_arg4_qcsubj,_arg5_qcval)
        _arg1_k: int. The constraint in which the new quadratic elements are inserted.
        _arg3_qcsubi: array of int. <no description>
        _arg4_qcsubj: array of int. <no description>
        _arg5_qcval: array of double. <no description>
      """
      _arg1_k_ = _arg1_k
      _arg2_numqcnz_ = None
      if _arg2_numqcnz_ is None:
        _arg2_numqcnz_ = len(_arg3_qcsubi)
        _arg2_numqcnz = _arg2_numqcnz_
      elif _arg2_numqcnz_ != len(_arg3_qcsubi):
        raise IndexError("Inconsistent length of array _arg3_qcsubi")
      if _arg2_numqcnz_ is None:
        _arg2_numqcnz_ = len(_arg4_qcsubj)
        _arg2_numqcnz = _arg2_numqcnz_
      elif _arg2_numqcnz_ != len(_arg4_qcsubj):
        raise IndexError("Inconsistent length of array _arg4_qcsubj")
      if _arg2_numqcnz_ is None:
        _arg2_numqcnz_ = len(_arg5_qcval)
        _arg2_numqcnz = _arg2_numqcnz_
      elif _arg2_numqcnz_ != len(_arg5_qcval):
        raise IndexError("Inconsistent length of array _arg5_qcval")
      if _arg2_numqcnz_ is None: _arg2_numqcnz_ = 0
      if _arg3_qcsubi is None: raise TypeError("Invalid type for argument _arg3_qcsubi")
      if _arg3_qcsubi is None:
        _arg3_qcsubi_ = None
      else:
        try:
          _arg3_qcsubi_ = memoryview(_arg3_qcsubi)
        except TypeError:
          try:
            _tmparr__arg3_qcsubi = (ctypes.c_int32*len(_arg3_qcsubi))()
            _tmparr__arg3_qcsubi[:] = _arg3_qcsubi
          except TypeError:
            raise TypeError("Argument _arg3_qcsubi has wrong type")
          else:
            _arg3_qcsubi_ = memoryview(_tmparr__arg3_qcsubi)
      
        else:
          if _arg3_qcsubi_.format != "i":
            _tmparr__arg3_qcsubi = (ctypes.c_int32*len(_arg3_qcsubi))()
            _tmparr__arg3_qcsubi[:] = _arg3_qcsubi
            _arg3_qcsubi_ = memoryview(_tmparr__arg3_qcsubi)
      
      if _arg4_qcsubj is None: raise TypeError("Invalid type for argument _arg4_qcsubj")
      if _arg4_qcsubj is None:
        _arg4_qcsubj_ = None
      else:
        try:
          _arg4_qcsubj_ = memoryview(_arg4_qcsubj)
        except TypeError:
          try:
            _tmparr__arg4_qcsubj = (ctypes.c_int32*len(_arg4_qcsubj))()
            _tmparr__arg4_qcsubj[:] = _arg4_qcsubj
          except TypeError:
            raise TypeError("Argument _arg4_qcsubj has wrong type")
          else:
            _arg4_qcsubj_ = memoryview(_tmparr__arg4_qcsubj)
      
        else:
          if _arg4_qcsubj_.format != "i":
            _tmparr__arg4_qcsubj = (ctypes.c_int32*len(_arg4_qcsubj))()
            _tmparr__arg4_qcsubj[:] = _arg4_qcsubj
            _arg4_qcsubj_ = memoryview(_tmparr__arg4_qcsubj)
      
      if _arg5_qcval is None: raise TypeError("Invalid type for argument _arg5_qcval")
      if _arg5_qcval is None:
        _arg5_qcval_ = None
      else:
        try:
          _arg5_qcval_ = memoryview(_arg5_qcval)
        except TypeError:
          try:
            _tmparr__arg5_qcval = (ctypes.c_double*len(_arg5_qcval))()
            _tmparr__arg5_qcval[:] = _arg5_qcval
          except TypeError:
            raise TypeError("Argument _arg5_qcval has wrong type")
          else:
            _arg5_qcval_ = memoryview(_tmparr__arg5_qcval)
      
        else:
          if _arg5_qcval_.format != "d":
            _tmparr__arg5_qcval = (ctypes.c_double*len(_arg5_qcval))()
            _tmparr__arg5_qcval[:] = _arg5_qcval
            _arg5_qcval_ = memoryview(_tmparr__arg5_qcval)
      
      res = self.__obj.putqconk(_arg1_k_,_arg2_numqcnz_,_arg3_qcsubi_,_arg4_qcsubj_,_arg5_qcval_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putqobj(self,_arg2_qosubi,_arg3_qosubj,_arg4_qoval): # 2
      """
      Replaces all quadratic terms in the objective.
    
      putqobj(self,_arg2_qosubi,_arg3_qosubj,_arg4_qoval)
        _arg2_qosubi: array of int. <no description>
        _arg3_qosubj: array of int. <no description>
        _arg4_qoval: array of double. <no description>
      """
      _arg1_numqonz_ = None
      if _arg1_numqonz_ is None:
        _arg1_numqonz_ = len(_arg2_qosubi)
        _arg1_numqonz = _arg1_numqonz_
      elif _arg1_numqonz_ != len(_arg2_qosubi):
        raise IndexError("Inconsistent length of array _arg2_qosubi")
      if _arg1_numqonz_ is None:
        _arg1_numqonz_ = len(_arg3_qosubj)
        _arg1_numqonz = _arg1_numqonz_
      elif _arg1_numqonz_ != len(_arg3_qosubj):
        raise IndexError("Inconsistent length of array _arg3_qosubj")
      if _arg1_numqonz_ is None:
        _arg1_numqonz_ = len(_arg4_qoval)
        _arg1_numqonz = _arg1_numqonz_
      elif _arg1_numqonz_ != len(_arg4_qoval):
        raise IndexError("Inconsistent length of array _arg4_qoval")
      if _arg1_numqonz_ is None: _arg1_numqonz_ = 0
      if _arg2_qosubi is None: raise TypeError("Invalid type for argument _arg2_qosubi")
      if _arg2_qosubi is None:
        _arg2_qosubi_ = None
      else:
        try:
          _arg2_qosubi_ = memoryview(_arg2_qosubi)
        except TypeError:
          try:
            _tmparr__arg2_qosubi = (ctypes.c_int32*len(_arg2_qosubi))()
            _tmparr__arg2_qosubi[:] = _arg2_qosubi
          except TypeError:
            raise TypeError("Argument _arg2_qosubi has wrong type")
          else:
            _arg2_qosubi_ = memoryview(_tmparr__arg2_qosubi)
      
        else:
          if _arg2_qosubi_.format != "i":
            _tmparr__arg2_qosubi = (ctypes.c_int32*len(_arg2_qosubi))()
            _tmparr__arg2_qosubi[:] = _arg2_qosubi
            _arg2_qosubi_ = memoryview(_tmparr__arg2_qosubi)
      
      if _arg3_qosubj is None: raise TypeError("Invalid type for argument _arg3_qosubj")
      if _arg3_qosubj is None:
        _arg3_qosubj_ = None
      else:
        try:
          _arg3_qosubj_ = memoryview(_arg3_qosubj)
        except TypeError:
          try:
            _tmparr__arg3_qosubj = (ctypes.c_int32*len(_arg3_qosubj))()
            _tmparr__arg3_qosubj[:] = _arg3_qosubj
          except TypeError:
            raise TypeError("Argument _arg3_qosubj has wrong type")
          else:
            _arg3_qosubj_ = memoryview(_tmparr__arg3_qosubj)
      
        else:
          if _arg3_qosubj_.format != "i":
            _tmparr__arg3_qosubj = (ctypes.c_int32*len(_arg3_qosubj))()
            _tmparr__arg3_qosubj[:] = _arg3_qosubj
            _arg3_qosubj_ = memoryview(_tmparr__arg3_qosubj)
      
      if _arg4_qoval is None: raise TypeError("Invalid type for argument _arg4_qoval")
      if _arg4_qoval is None:
        _arg4_qoval_ = None
      else:
        try:
          _arg4_qoval_ = memoryview(_arg4_qoval)
        except TypeError:
          try:
            _tmparr__arg4_qoval = (ctypes.c_double*len(_arg4_qoval))()
            _tmparr__arg4_qoval[:] = _arg4_qoval
          except TypeError:
            raise TypeError("Argument _arg4_qoval has wrong type")
          else:
            _arg4_qoval_ = memoryview(_tmparr__arg4_qoval)
      
        else:
          if _arg4_qoval_.format != "d":
            _tmparr__arg4_qoval = (ctypes.c_double*len(_arg4_qoval))()
            _tmparr__arg4_qoval[:] = _arg4_qoval
            _arg4_qoval_ = memoryview(_tmparr__arg4_qoval)
      
      res = self.__obj.putqobj(_arg1_numqonz_,_arg2_qosubi_,_arg3_qosubj_,_arg4_qoval_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putqobjij(self,_arg1_i,_arg2_j,_arg3_qoij): # 2
      """
      Replaces one coefficient in the quadratic term in the objective.
    
      putqobjij(self,_arg1_i,_arg2_j,_arg3_qoij)
        _arg1_i: int. Row index for the coefficient to be replaced.
        _arg2_j: int. Column index for the coefficient to be replaced.
        _arg3_qoij: double. The new coefficient value.
      """
      _arg1_i_ = _arg1_i
      _arg2_j_ = _arg2_j
      _arg3_qoij_ = _arg3_qoij
      res = self.__obj.putqobjij(_arg1_i_,_arg2_j_,_arg3_qoij_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putsolution(self,_arg1_whichsol_,_arg2_skc,_arg3_skx,_arg4_skn,_arg5_xc,_arg6_xx,_arg7_y,_arg8_slc,_arg9_suc,_arg10_slx,_arg11_sux,_arg12_snx): # 2
      """
      Inserts a solution.
    
      putsolution(self,_arg1_whichsol_,_arg2_skc,_arg3_skx,_arg4_skn,_arg5_xc,_arg6_xx,_arg7_y,_arg8_slc,_arg9_suc,_arg10_slx,_arg11_sux,_arg12_snx)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_skc: array of mosek.stakey. <no description>
        _arg3_skx: array of mosek.stakey. <no description>
        _arg4_skn: array of mosek.stakey. <no description>
        _arg5_xc: array of double. <no description>
        _arg6_xx: array of double. <no description>
        _arg7_y: array of double. <no description>
        _arg8_slc: array of double. <no description>
        _arg9_suc: array of double. <no description>
        _arg10_slx: array of double. <no description>
        _arg11_sux: array of double. <no description>
        _arg12_snx: array of double. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      if _arg2_skc is None:
        _arg2_skc_ = None
      else:
        try:
          _arg2_skc_ = memoryview(_arg2_skc)
        except TypeError:
          try:
            _tmparr__arg2_skc = (ctypes.c_int*len(_arg2_skc))()
            _tmparr__arg2_skc[:] = _arg2_skc
          except TypeError:
            raise TypeError("Argument _arg2_skc has wrong type")
          else:
            _arg2_skc_ = memoryview(_tmparr__arg2_skc)
      
        else:
          if _arg2_skc_.format != "i":
            _tmparr__arg2_skc = (ctypes.c_int*len(_arg2_skc))()
            _tmparr__arg2_skc[:] = _arg2_skc
            _arg2_skc_ = memoryview(_tmparr__arg2_skc)
      
      if _arg3_skx is None:
        _arg3_skx_ = None
      else:
        try:
          _arg3_skx_ = memoryview(_arg3_skx)
        except TypeError:
          try:
            _tmparr__arg3_skx = (ctypes.c_int*len(_arg3_skx))()
            _tmparr__arg3_skx[:] = _arg3_skx
          except TypeError:
            raise TypeError("Argument _arg3_skx has wrong type")
          else:
            _arg3_skx_ = memoryview(_tmparr__arg3_skx)
      
        else:
          if _arg3_skx_.format != "i":
            _tmparr__arg3_skx = (ctypes.c_int*len(_arg3_skx))()
            _tmparr__arg3_skx[:] = _arg3_skx
            _arg3_skx_ = memoryview(_tmparr__arg3_skx)
      
      if _arg4_skn is None:
        _arg4_skn_ = None
      else:
        try:
          _arg4_skn_ = memoryview(_arg4_skn)
        except TypeError:
          try:
            _tmparr__arg4_skn = (ctypes.c_int*len(_arg4_skn))()
            _tmparr__arg4_skn[:] = _arg4_skn
          except TypeError:
            raise TypeError("Argument _arg4_skn has wrong type")
          else:
            _arg4_skn_ = memoryview(_tmparr__arg4_skn)
      
        else:
          if _arg4_skn_.format != "i":
            _tmparr__arg4_skn = (ctypes.c_int*len(_arg4_skn))()
            _tmparr__arg4_skn[:] = _arg4_skn
            _arg4_skn_ = memoryview(_tmparr__arg4_skn)
      
      if _arg5_xc is None:
        _arg5_xc_ = None
      else:
        try:
          _arg5_xc_ = memoryview(_arg5_xc)
        except TypeError:
          try:
            _tmparr__arg5_xc = (ctypes.c_double*len(_arg5_xc))()
            _tmparr__arg5_xc[:] = _arg5_xc
          except TypeError:
            raise TypeError("Argument _arg5_xc has wrong type")
          else:
            _arg5_xc_ = memoryview(_tmparr__arg5_xc)
      
        else:
          if _arg5_xc_.format != "d":
            _tmparr__arg5_xc = (ctypes.c_double*len(_arg5_xc))()
            _tmparr__arg5_xc[:] = _arg5_xc
            _arg5_xc_ = memoryview(_tmparr__arg5_xc)
      
      if _arg6_xx is None:
        _arg6_xx_ = None
      else:
        try:
          _arg6_xx_ = memoryview(_arg6_xx)
        except TypeError:
          try:
            _tmparr__arg6_xx = (ctypes.c_double*len(_arg6_xx))()
            _tmparr__arg6_xx[:] = _arg6_xx
          except TypeError:
            raise TypeError("Argument _arg6_xx has wrong type")
          else:
            _arg6_xx_ = memoryview(_tmparr__arg6_xx)
      
        else:
          if _arg6_xx_.format != "d":
            _tmparr__arg6_xx = (ctypes.c_double*len(_arg6_xx))()
            _tmparr__arg6_xx[:] = _arg6_xx
            _arg6_xx_ = memoryview(_tmparr__arg6_xx)
      
      if _arg7_y is None:
        _arg7_y_ = None
      else:
        try:
          _arg7_y_ = memoryview(_arg7_y)
        except TypeError:
          try:
            _tmparr__arg7_y = (ctypes.c_double*len(_arg7_y))()
            _tmparr__arg7_y[:] = _arg7_y
          except TypeError:
            raise TypeError("Argument _arg7_y has wrong type")
          else:
            _arg7_y_ = memoryview(_tmparr__arg7_y)
      
        else:
          if _arg7_y_.format != "d":
            _tmparr__arg7_y = (ctypes.c_double*len(_arg7_y))()
            _tmparr__arg7_y[:] = _arg7_y
            _arg7_y_ = memoryview(_tmparr__arg7_y)
      
      if _arg8_slc is None:
        _arg8_slc_ = None
      else:
        try:
          _arg8_slc_ = memoryview(_arg8_slc)
        except TypeError:
          try:
            _tmparr__arg8_slc = (ctypes.c_double*len(_arg8_slc))()
            _tmparr__arg8_slc[:] = _arg8_slc
          except TypeError:
            raise TypeError("Argument _arg8_slc has wrong type")
          else:
            _arg8_slc_ = memoryview(_tmparr__arg8_slc)
      
        else:
          if _arg8_slc_.format != "d":
            _tmparr__arg8_slc = (ctypes.c_double*len(_arg8_slc))()
            _tmparr__arg8_slc[:] = _arg8_slc
            _arg8_slc_ = memoryview(_tmparr__arg8_slc)
      
      if _arg9_suc is None:
        _arg9_suc_ = None
      else:
        try:
          _arg9_suc_ = memoryview(_arg9_suc)
        except TypeError:
          try:
            _tmparr__arg9_suc = (ctypes.c_double*len(_arg9_suc))()
            _tmparr__arg9_suc[:] = _arg9_suc
          except TypeError:
            raise TypeError("Argument _arg9_suc has wrong type")
          else:
            _arg9_suc_ = memoryview(_tmparr__arg9_suc)
      
        else:
          if _arg9_suc_.format != "d":
            _tmparr__arg9_suc = (ctypes.c_double*len(_arg9_suc))()
            _tmparr__arg9_suc[:] = _arg9_suc
            _arg9_suc_ = memoryview(_tmparr__arg9_suc)
      
      if _arg10_slx is None:
        _arg10_slx_ = None
      else:
        try:
          _arg10_slx_ = memoryview(_arg10_slx)
        except TypeError:
          try:
            _tmparr__arg10_slx = (ctypes.c_double*len(_arg10_slx))()
            _tmparr__arg10_slx[:] = _arg10_slx
          except TypeError:
            raise TypeError("Argument _arg10_slx has wrong type")
          else:
            _arg10_slx_ = memoryview(_tmparr__arg10_slx)
      
        else:
          if _arg10_slx_.format != "d":
            _tmparr__arg10_slx = (ctypes.c_double*len(_arg10_slx))()
            _tmparr__arg10_slx[:] = _arg10_slx
            _arg10_slx_ = memoryview(_tmparr__arg10_slx)
      
      if _arg11_sux is None:
        _arg11_sux_ = None
      else:
        try:
          _arg11_sux_ = memoryview(_arg11_sux)
        except TypeError:
          try:
            _tmparr__arg11_sux = (ctypes.c_double*len(_arg11_sux))()
            _tmparr__arg11_sux[:] = _arg11_sux
          except TypeError:
            raise TypeError("Argument _arg11_sux has wrong type")
          else:
            _arg11_sux_ = memoryview(_tmparr__arg11_sux)
      
        else:
          if _arg11_sux_.format != "d":
            _tmparr__arg11_sux = (ctypes.c_double*len(_arg11_sux))()
            _tmparr__arg11_sux[:] = _arg11_sux
            _arg11_sux_ = memoryview(_tmparr__arg11_sux)
      
      if _arg12_snx is None:
        _arg12_snx_ = None
      else:
        try:
          _arg12_snx_ = memoryview(_arg12_snx)
        except TypeError:
          try:
            _tmparr__arg12_snx = (ctypes.c_double*len(_arg12_snx))()
            _tmparr__arg12_snx[:] = _arg12_snx
          except TypeError:
            raise TypeError("Argument _arg12_snx has wrong type")
          else:
            _arg12_snx_ = memoryview(_tmparr__arg12_snx)
      
        else:
          if _arg12_snx_.format != "d":
            _tmparr__arg12_snx = (ctypes.c_double*len(_arg12_snx))()
            _tmparr__arg12_snx[:] = _arg12_snx
            _arg12_snx_ = memoryview(_tmparr__arg12_snx)
      
      res = self.__obj.putsolution(_arg1_whichsol_,_arg2_skc_,_arg3_skx_,_arg4_skn_,_arg5_xc_,_arg6_xx_,_arg7_y_,_arg8_slc_,_arg9_suc_,_arg10_slx_,_arg11_sux_,_arg12_snx_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putconsolutioni(self,_arg1_i,_arg2_whichsol_,_arg3_sk_,_arg4_x,_arg5_sl,_arg6_su): # 2
      """
      Sets the primal and dual solution information for a single constraint.
    
      putconsolutioni(self,_arg1_i,_arg2_whichsol_,_arg3_sk_,_arg4_x,_arg5_sl,_arg6_su)
        _arg1_i: int. Index of the constraint.
        _arg2_whichsol: mosek.soltype. <no description>
        _arg3_sk: mosek.stakey. Status key of the constraint.
        _arg4_x: double. Primal solution value of the constraint.
        _arg5_sl: double. Solution value of the dual variable associated with the lower bound.
        _arg6_su: double. Solution value of the dual variable associated with the upper bound.
      """
      _arg1_i_ = _arg1_i
      if not isinstance(_arg2_whichsol_,soltype): raise TypeError("Argument _arg2_whichsol has wrong type")
      _arg2_whichsol = _arg2_whichsol_
      if not isinstance(_arg3_sk_,stakey): raise TypeError("Argument _arg3_sk has wrong type")
      _arg3_sk = _arg3_sk_
      _arg4_x_ = _arg4_x
      _arg5_sl_ = _arg5_sl
      _arg6_su_ = _arg6_su
      res = self.__obj.putconsolutioni(_arg1_i_,_arg2_whichsol_,_arg3_sk_,_arg4_x_,_arg5_sl_,_arg6_su_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putvarsolutionj(self,_arg1_j,_arg2_whichsol_,_arg3_sk_,_arg4_x,_arg5_sl,_arg6_su,_arg7_sn): # 2
      """
      Sets the primal and dual solution information for a single variable.
    
      putvarsolutionj(self,_arg1_j,_arg2_whichsol_,_arg3_sk_,_arg4_x,_arg5_sl,_arg6_su,_arg7_sn)
        _arg1_j: int. Index of the variable.
        _arg2_whichsol: mosek.soltype. <no description>
        _arg3_sk: mosek.stakey. Status key of the variable.
        _arg4_x: double. Primal solution value of the variable.
        _arg5_sl: double. Solution value of the dual variable associated with the lower bound.
        _arg6_su: double. Solution value of the dual variable associated with the upper bound.
        _arg7_sn: double. Solution value of the dual variable associated with the conic constraint.
      """
      _arg1_j_ = _arg1_j
      if not isinstance(_arg2_whichsol_,soltype): raise TypeError("Argument _arg2_whichsol has wrong type")
      _arg2_whichsol = _arg2_whichsol_
      if not isinstance(_arg3_sk_,stakey): raise TypeError("Argument _arg3_sk has wrong type")
      _arg3_sk = _arg3_sk_
      _arg4_x_ = _arg4_x
      _arg5_sl_ = _arg5_sl
      _arg6_su_ = _arg6_su
      _arg7_sn_ = _arg7_sn
      res = self.__obj.putvarsolutionj(_arg1_j_,_arg2_whichsol_,_arg3_sk_,_arg4_x_,_arg5_sl_,_arg6_su_,_arg7_sn_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putsolutionyi(self,_arg1_i,_arg2_whichsol_,_arg3_y): # 2
      """
      Inputs the dual variable of a solution.
    
      putsolutionyi(self,_arg1_i,_arg2_whichsol_,_arg3_y)
        _arg1_i: int. Index of the dual variable.
        _arg2_whichsol: mosek.soltype. <no description>
        _arg3_y: double. Solution value of the dual variable.
      """
      _arg1_i_ = _arg1_i
      if not isinstance(_arg2_whichsol_,soltype): raise TypeError("Argument _arg2_whichsol has wrong type")
      _arg2_whichsol = _arg2_whichsol_
      _arg3_y_ = _arg3_y
      res = self.__obj.putsolutionyi(_arg1_i_,_arg2_whichsol_,_arg3_y_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putstrparam(self,_arg1_param_,_arg2_parvalue_): # 2
      """
      Sets a string parameter.
    
      putstrparam(self,_arg1_param_,_arg2_parvalue_)
        _arg1_param: mosek.sparam. <no description>
        _arg2_parvalue: str. <no description>
      """
      if not isinstance(_arg1_param_,sparam): raise TypeError("Argument _arg1_param has wrong type")
      _arg1_param = _arg1_param_
      res = self.__obj.putstrparam(_arg1_param_,_arg2_parvalue_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def puttaskname(self,_arg1_taskname_): # 2
      """
      Assigns a new name to the task.
    
      puttaskname(self,_arg1_taskname_)
        _arg1_taskname: str. <no description>
      """
      res = self.__obj.puttaskname(_arg1_taskname_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putvartype(self,_arg1_j,_arg2_vartype_): # 2
      """
      Sets the variable type of one variable.
    
      putvartype(self,_arg1_j,_arg2_vartype_)
        _arg1_j: int. Index of the variable.
        _arg2_vartype: mosek.variabletype. The new variable type.
      """
      _arg1_j_ = _arg1_j
      if not isinstance(_arg2_vartype_,variabletype): raise TypeError("Argument _arg2_vartype has wrong type")
      _arg2_vartype = _arg2_vartype_
      res = self.__obj.putvartype(_arg1_j_,_arg2_vartype_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def putvartypelist(self,_arg2_subj,_arg3_vartype): # 2
      """
      Sets the variable type for one or more variables.
    
      putvartypelist(self,_arg2_subj,_arg3_vartype)
        _arg2_subj: array of int. A list of variable indexes for which the variable type should be changed.
        _arg3_vartype: array of mosek.variabletype. A list of variable types.
      """
      _arg1_num_ = None
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg2_subj)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg2_subj):
        raise IndexError("Inconsistent length of array _arg2_subj")
      if _arg1_num_ is None:
        _arg1_num_ = len(_arg3_vartype)
        _arg1_num = _arg1_num_
      elif _arg1_num_ != len(_arg3_vartype):
        raise IndexError("Inconsistent length of array _arg3_vartype")
      if _arg1_num_ is None: _arg1_num_ = 0
      if _arg2_subj is None: raise TypeError("Invalid type for argument _arg2_subj")
      if _arg2_subj is None:
        _arg2_subj_ = None
      else:
        try:
          _arg2_subj_ = memoryview(_arg2_subj)
        except TypeError:
          try:
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
          except TypeError:
            raise TypeError("Argument _arg2_subj has wrong type")
          else:
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
        else:
          if _arg2_subj_.format != "i":
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
      if _arg3_vartype is None: raise TypeError("Invalid type for argument _arg3_vartype")
      if _arg3_vartype is None:
        _arg3_vartype_ = None
      else:
        try:
          _arg3_vartype_ = memoryview(_arg3_vartype)
        except TypeError:
          try:
            _tmparr__arg3_vartype = (ctypes.c_int*len(_arg3_vartype))()
            _tmparr__arg3_vartype[:] = _arg3_vartype
          except TypeError:
            raise TypeError("Argument _arg3_vartype has wrong type")
          else:
            _arg3_vartype_ = memoryview(_tmparr__arg3_vartype)
      
        else:
          if _arg3_vartype_.format != "i":
            _tmparr__arg3_vartype = (ctypes.c_int*len(_arg3_vartype))()
            _tmparr__arg3_vartype[:] = _arg3_vartype
            _arg3_vartype_ = memoryview(_tmparr__arg3_vartype)
      
      res = self.__obj.putvartypelist(_arg1_num_,_arg2_subj_,_arg3_vartype_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def readdataformat(self,_arg1_filename_,_arg2_format_,_arg3_compress_): # 2
      """
      Reads problem data from a file.
    
      readdataformat(self,_arg1_filename_,_arg2_format_,_arg3_compress_)
        _arg1_filename: str. <no description>
        _arg2_format: mosek.dataformat. File data format.
        _arg3_compress: mosek.compresstype. File compression type.
      """
      if not isinstance(_arg2_format_,dataformat): raise TypeError("Argument _arg2_format has wrong type")
      _arg2_format = _arg2_format_
      if not isinstance(_arg3_compress_,compresstype): raise TypeError("Argument _arg3_compress has wrong type")
      _arg3_compress = _arg3_compress_
      res = self.__obj.readdataformat(_arg1_filename_,_arg2_format_,_arg3_compress_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def readdata(self,_arg1_filename_): # 2
      """
      Reads problem data from a file.
    
      readdata(self,_arg1_filename_)
        _arg1_filename: str. <no description>
      """
      res = self.__obj.readdataautoformat(_arg1_filename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def readparamfile(self,_arg1_filename_): # 2
      """
      Reads a parameter file.
    
      readparamfile(self,_arg1_filename_)
        _arg1_filename: str. <no description>
      """
      res = self.__obj.readparamfile(_arg1_filename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def readsolution(self,_arg1_whichsol_,_arg2_filename_): # 2
      """
      Reads a solution from a file.
    
      readsolution(self,_arg1_whichsol_,_arg2_filename_)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_filename: str. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res = self.__obj.readsolution(_arg1_whichsol_,_arg2_filename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def readsummary(self,_arg1_whichstream_): # 2
      """
      Prints information about last file read.
    
      readsummary(self,_arg1_whichstream_)
        _arg1_whichstream: mosek.streamtype. <no description>
      """
      if not isinstance(_arg1_whichstream_,streamtype): raise TypeError("Argument _arg1_whichstream has wrong type")
      _arg1_whichstream = _arg1_whichstream_
      res = self.__obj.readsummary(_arg1_whichstream_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def resizetask(self,_arg1_maxnumcon,_arg2_maxnumvar,_arg3_maxnumcone,_arg4_maxnumanz,_arg5_maxnumqnz): # 2
      """
      Resizes an optimization task.
    
      resizetask(self,_arg1_maxnumcon,_arg2_maxnumvar,_arg3_maxnumcone,_arg4_maxnumanz,_arg5_maxnumqnz)
        _arg1_maxnumcon: int. New maximum number of constraints.
        _arg2_maxnumvar: int. New maximum number of variables.
        _arg3_maxnumcone: int. New maximum number of cones.
        _arg4_maxnumanz: long. New maximum number of linear non-zero elements.
        _arg5_maxnumqnz: long. New maximum number of quadratic non-zeros elements.
      """
      _arg1_maxnumcon_ = _arg1_maxnumcon
      _arg2_maxnumvar_ = _arg2_maxnumvar
      _arg3_maxnumcone_ = _arg3_maxnumcone
      _arg4_maxnumanz_ = _arg4_maxnumanz
      _arg5_maxnumqnz_ = _arg5_maxnumqnz
      res = self.__obj.resizetask(_arg1_maxnumcon_,_arg2_maxnumvar_,_arg3_maxnumcone_,_arg4_maxnumanz_,_arg5_maxnumqnz_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def checkmem(self,_arg1_file_,_arg2_line): # 2
      """
      Checks the memory allocated by the task.
    
      checkmem(self,_arg1_file_,_arg2_line)
        _arg1_file: str. File from which the function is called.
        _arg2_line: int. Line in the file from which the function is called.
      """
      _arg2_line_ = _arg2_line
      res = self.__obj.checkmemtask(_arg1_file_,_arg2_line_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getmemusage(self): # 2
      """
      Obtains information about the amount of memory used by a task.
    
      getmemusage(self)
      returns: _arg1_meminuse,_arg2_maxmemuse
        _arg1_meminuse: long. Amount of memory currently used by the task.
        _arg2_maxmemuse: long. Maximum amount of memory used by the task until now.
      """
      res,resargs = self.__obj.getmemusagetask()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg1_meminuse_return_value,__arg2_maxmemuse_return_value = resargs
      return __arg1_meminuse_return_value,__arg2_maxmemuse_return_value
    
    def setdefaults(self): # 2
      """
      Resets all parameter values.
    
      setdefaults(self)
      """
      res = self.__obj.setdefaults()
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def solutiondef(self,_arg1_whichsol_): # 2
      """
      Checks whether a solution is defined.
    
      solutiondef(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. <no description>
      returns: _arg2_isdef
        _arg2_isdef: int. Is non-zero if the requested solution is defined.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res,resargs = self.__obj.solutiondef(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_isdef_return_value = resargs
      return __arg2_isdef_return_value
    
    def deletesolution(self,_arg1_whichsol_): # 2
      """
      Undefine a solution and free the memory it uses.
    
      deletesolution(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res = self.__obj.deletesolution(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def onesolutionsummary(self,_arg1_whichstream_,_arg2_whichsol_): # 2
      """
      Prints a short summary of a specified solution.
    
      onesolutionsummary(self,_arg1_whichstream_,_arg2_whichsol_)
        _arg1_whichstream: mosek.streamtype. <no description>
        _arg2_whichsol: mosek.soltype. <no description>
      """
      if not isinstance(_arg1_whichstream_,streamtype): raise TypeError("Argument _arg1_whichstream has wrong type")
      _arg1_whichstream = _arg1_whichstream_
      if not isinstance(_arg2_whichsol_,soltype): raise TypeError("Argument _arg2_whichsol has wrong type")
      _arg2_whichsol = _arg2_whichsol_
      res = self.__obj.onesolutionsummary(_arg1_whichstream_,_arg2_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def solutionsummary(self,_arg1_whichstream_): # 2
      """
      Prints a short summary of the current solutions.
    
      solutionsummary(self,_arg1_whichstream_)
        _arg1_whichstream: mosek.streamtype. <no description>
      """
      if not isinstance(_arg1_whichstream_,streamtype): raise TypeError("Argument _arg1_whichstream has wrong type")
      _arg1_whichstream = _arg1_whichstream_
      res = self.__obj.solutionsummary(_arg1_whichstream_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def updatesolutioninfo(self,_arg1_whichsol_): # 2
      """
      Update the information items related to the solution.
    
      updatesolutioninfo(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res = self.__obj.updatesolutioninfo(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def optimizersummary(self,_arg1_whichstream_): # 2
      """
      Prints a short summary with optimizer statistics from last optimization.
    
      optimizersummary(self,_arg1_whichstream_)
        _arg1_whichstream: mosek.streamtype. <no description>
      """
      if not isinstance(_arg1_whichstream_,streamtype): raise TypeError("Argument _arg1_whichstream has wrong type")
      _arg1_whichstream = _arg1_whichstream_
      res = self.__obj.optimizersummary(_arg1_whichstream_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def strtoconetype(self,_arg1_str_): # 2
      """
      Obtains a cone type code.
    
      strtoconetype(self,_arg1_str_)
        _arg1_str: str. String corresponding to the cone type code.
      returns: _arg2_conetype
        _arg2_conetype: mosek.conetype. The cone type corresponding to str.
      """
      res,resargs = self.__obj.strtoconetype(_arg1_str_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_conetype_return_value = resargs
      __arg2_conetype_return_value = conetype(__arg2_conetype_return_value)
      return __arg2_conetype_return_value
    
    def strtosk(self,_arg1_str_): # 2
      """
      Obtains a status key.
    
      strtosk(self,_arg1_str_)
        _arg1_str: str. A status key abbreviation string.
      returns: _arg2_sk
        _arg2_sk: mosek.stakey. Status key corresponding to the string.
      """
      res,resargs = self.__obj.strtosk(_arg1_str_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_sk_return_value = resargs
      __arg2_sk_return_value = stakey(__arg2_sk_return_value)
      return __arg2_sk_return_value
    
    def writedata(self,_arg1_filename_): # 2
      """
      Writes problem data to a file.
    
      writedata(self,_arg1_filename_)
        _arg1_filename: str. <no description>
      """
      res = self.__obj.writedata(_arg1_filename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def writetask(self,_arg1_filename_): # 2
      """
      Write a complete binary dump of the task data.
    
      writetask(self,_arg1_filename_)
        _arg1_filename: str. <no description>
      """
      res = self.__obj.writetask(_arg1_filename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def readtask(self,_arg1_filename_): # 2
      """
      Load task data from a file.
    
      readtask(self,_arg1_filename_)
        _arg1_filename: str. <no description>
      """
      res = self.__obj.readtask(_arg1_filename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def readopfstring(self,_arg1_data_): # 2
      """
      Load task data from a string in OPF format.
    
      readopfstring(self,_arg1_data_)
        _arg1_data: str. <no description>
      """
      res = self.__obj.readopfstring(_arg1_data_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def readlpstring(self,_arg1_data_): # 2
      """
      Load task data from a string in LP format.
    
      readlpstring(self,_arg1_data_)
        _arg1_data: str. <no description>
      """
      res = self.__obj.readlpstring(_arg1_data_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def readjsonstring(self,_arg1_data_): # 2
      """
      Load task data from a string in JSON format.
    
      readjsonstring(self,_arg1_data_)
        _arg1_data: str. <no description>
      """
      res = self.__obj.readjsonstring(_arg1_data_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def readptfstring(self,_arg1_data_): # 2
      """
      Load task data from a string in PTF format.
    
      readptfstring(self,_arg1_data_)
        _arg1_data: str. <no description>
      """
      res = self.__obj.readptfstring(_arg1_data_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def writeparamfile(self,_arg1_filename_): # 2
      """
      Writes all the parameters to a parameter file.
    
      writeparamfile(self,_arg1_filename_)
        _arg1_filename: str. <no description>
      """
      res = self.__obj.writeparamfile(_arg1_filename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def getinfeasiblesubproblem(self,_arg1_whichsol_): # 2
      """
      Obtains an infeasible subproblem.
    
      getinfeasiblesubproblem(self,_arg1_whichsol_)
        _arg1_whichsol: mosek.soltype. Which solution to use when determining the infeasible subproblem.
      returns: _arg2_inftask
        _arg2_inftask: Task. A new task containing the infeasible subproblem.
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res,resargs = self.__obj.getinfeasiblesubproblem(_arg1_whichsol_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg2_inftask_return_value = resargs
      __arg2_inftask_return_value = Task(__arg2_inftask_return_value)
      return __arg2_inftask_return_value
    
    def writesolution(self,_arg1_whichsol_,_arg2_filename_): # 2
      """
      Write a solution to a file.
    
      writesolution(self,_arg1_whichsol_,_arg2_filename_)
        _arg1_whichsol: mosek.soltype. <no description>
        _arg2_filename: str. <no description>
      """
      if not isinstance(_arg1_whichsol_,soltype): raise TypeError("Argument _arg1_whichsol has wrong type")
      _arg1_whichsol = _arg1_whichsol_
      res = self.__obj.writesolution(_arg1_whichsol_,_arg2_filename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def writejsonsol(self,_arg1_filename_): # 2
      """
      Writes a solution to a JSON file.
    
      writejsonsol(self,_arg1_filename_)
        _arg1_filename: str. <no description>
      """
      res = self.__obj.writejsonsol(_arg1_filename_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def primalsensitivity(self,_arg2_subi,_arg3_marki,_arg5_subj,_arg6_markj,_arg7_leftpricei,_arg8_rightpricei,_arg9_leftrangei,_arg10_rightrangei,_arg11_leftpricej,_arg12_rightpricej,_arg13_leftrangej,_arg14_rightrangej): # 2
      """
      Perform sensitivity analysis on bounds.
    
      primalsensitivity(self,_arg2_subi,_arg3_marki,_arg5_subj,_arg6_markj,_arg7_leftpricei,_arg8_rightpricei,_arg9_leftrangei,_arg10_rightrangei,_arg11_leftpricej,_arg12_rightpricej,_arg13_leftrangej,_arg14_rightrangej)
        _arg2_subi: array of int. Indexes of constraints to analyze.
        _arg3_marki: array of mosek.mark. Mark which constraint bounds to analyze.
        _arg5_subj: array of int. Indexes of variables to analyze.
        _arg6_markj: array of mosek.mark. Mark which variable bounds to analyze.
        _arg7_leftpricei: array of double. Left shadow price for constraints.
        _arg8_rightpricei: array of double. Right shadow price for constraints.
        _arg9_leftrangei: array of double. Left range for constraints.
        _arg10_rightrangei: array of double. Right range for constraints.
        _arg11_leftpricej: array of double. Left shadow price for variables.
        _arg12_rightpricej: array of double. Right shadow price for variables.
        _arg13_leftrangej: array of double. Left range for variables.
        _arg14_rightrangej: array of double. Right range for variables.
      """
      _arg1_numi_ = None
      if _arg1_numi_ is None:
        _arg1_numi_ = len(_arg2_subi)
        _arg1_numi = _arg1_numi_
      elif _arg1_numi_ != len(_arg2_subi):
        raise IndexError("Inconsistent length of array _arg2_subi")
      if _arg1_numi_ is None:
        _arg1_numi_ = len(_arg3_marki)
        _arg1_numi = _arg1_numi_
      elif _arg1_numi_ != len(_arg3_marki):
        raise IndexError("Inconsistent length of array _arg3_marki")
      if _arg1_numi_ is None: _arg1_numi_ = 0
      if _arg2_subi is None: raise TypeError("Invalid type for argument _arg2_subi")
      if _arg2_subi is None:
        _arg2_subi_ = None
      else:
        try:
          _arg2_subi_ = memoryview(_arg2_subi)
        except TypeError:
          try:
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
          except TypeError:
            raise TypeError("Argument _arg2_subi has wrong type")
          else:
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
        else:
          if _arg2_subi_.format != "i":
            _tmparr__arg2_subi = (ctypes.c_int32*len(_arg2_subi))()
            _tmparr__arg2_subi[:] = _arg2_subi
            _arg2_subi_ = memoryview(_tmparr__arg2_subi)
      
      if _arg3_marki is None: raise TypeError("Invalid type for argument _arg3_marki")
      if _arg3_marki is None:
        _arg3_marki_ = None
      else:
        try:
          _arg3_marki_ = memoryview(_arg3_marki)
        except TypeError:
          try:
            _tmparr__arg3_marki = (ctypes.c_int*len(_arg3_marki))()
            _tmparr__arg3_marki[:] = _arg3_marki
          except TypeError:
            raise TypeError("Argument _arg3_marki has wrong type")
          else:
            _arg3_marki_ = memoryview(_tmparr__arg3_marki)
      
        else:
          if _arg3_marki_.format != "i":
            _tmparr__arg3_marki = (ctypes.c_int*len(_arg3_marki))()
            _tmparr__arg3_marki[:] = _arg3_marki
            _arg3_marki_ = memoryview(_tmparr__arg3_marki)
      
      _arg4_numj_ = None
      if _arg4_numj_ is None:
        _arg4_numj_ = len(_arg5_subj)
        _arg4_numj = _arg4_numj_
      elif _arg4_numj_ != len(_arg5_subj):
        raise IndexError("Inconsistent length of array _arg5_subj")
      if _arg4_numj_ is None:
        _arg4_numj_ = len(_arg6_markj)
        _arg4_numj = _arg4_numj_
      elif _arg4_numj_ != len(_arg6_markj):
        raise IndexError("Inconsistent length of array _arg6_markj")
      if _arg4_numj_ is None: _arg4_numj_ = 0
      if _arg5_subj is None: raise TypeError("Invalid type for argument _arg5_subj")
      if _arg5_subj is None:
        _arg5_subj_ = None
      else:
        try:
          _arg5_subj_ = memoryview(_arg5_subj)
        except TypeError:
          try:
            _tmparr__arg5_subj = (ctypes.c_int32*len(_arg5_subj))()
            _tmparr__arg5_subj[:] = _arg5_subj
          except TypeError:
            raise TypeError("Argument _arg5_subj has wrong type")
          else:
            _arg5_subj_ = memoryview(_tmparr__arg5_subj)
      
        else:
          if _arg5_subj_.format != "i":
            _tmparr__arg5_subj = (ctypes.c_int32*len(_arg5_subj))()
            _tmparr__arg5_subj[:] = _arg5_subj
            _arg5_subj_ = memoryview(_tmparr__arg5_subj)
      
      if _arg6_markj is None: raise TypeError("Invalid type for argument _arg6_markj")
      if _arg6_markj is None:
        _arg6_markj_ = None
      else:
        try:
          _arg6_markj_ = memoryview(_arg6_markj)
        except TypeError:
          try:
            _tmparr__arg6_markj = (ctypes.c_int*len(_arg6_markj))()
            _tmparr__arg6_markj[:] = _arg6_markj
          except TypeError:
            raise TypeError("Argument _arg6_markj has wrong type")
          else:
            _arg6_markj_ = memoryview(_tmparr__arg6_markj)
      
        else:
          if _arg6_markj_.format != "i":
            _tmparr__arg6_markj = (ctypes.c_int*len(_arg6_markj))()
            _tmparr__arg6_markj[:] = _arg6_markj
            _arg6_markj_ = memoryview(_tmparr__arg6_markj)
      
      _copyback__arg7_leftpricei = False
      if _arg7_leftpricei is None:
        _arg7_leftpricei_ = None
      else:
        try:
          _arg7_leftpricei_ = memoryview(_arg7_leftpricei)
        except TypeError:
          try:
            _tmparr__arg7_leftpricei = (ctypes.c_double*len(_arg7_leftpricei))()
            _tmparr__arg7_leftpricei[:] = _arg7_leftpricei
          except TypeError:
            raise TypeError("Argument _arg7_leftpricei has wrong type")
          else:
            _arg7_leftpricei_ = memoryview(_tmparr__arg7_leftpricei)
            _copyback__arg7_leftpricei = True
        else:
          if _arg7_leftpricei_.format != "d":
            _tmparr__arg7_leftpricei = (ctypes.c_double*len(_arg7_leftpricei))()
            _tmparr__arg7_leftpricei[:] = _arg7_leftpricei
            _arg7_leftpricei_ = memoryview(_tmparr__arg7_leftpricei)
            _copyback__arg7_leftpricei = True
      if _arg7_leftpricei_ is not None and len(_arg7_leftpricei_) !=  _arg1_numi :
        raise ValueError("Array argument _arg7_leftpricei has wrong length")
      _copyback__arg8_rightpricei = False
      if _arg8_rightpricei is None:
        _arg8_rightpricei_ = None
      else:
        try:
          _arg8_rightpricei_ = memoryview(_arg8_rightpricei)
        except TypeError:
          try:
            _tmparr__arg8_rightpricei = (ctypes.c_double*len(_arg8_rightpricei))()
            _tmparr__arg8_rightpricei[:] = _arg8_rightpricei
          except TypeError:
            raise TypeError("Argument _arg8_rightpricei has wrong type")
          else:
            _arg8_rightpricei_ = memoryview(_tmparr__arg8_rightpricei)
            _copyback__arg8_rightpricei = True
        else:
          if _arg8_rightpricei_.format != "d":
            _tmparr__arg8_rightpricei = (ctypes.c_double*len(_arg8_rightpricei))()
            _tmparr__arg8_rightpricei[:] = _arg8_rightpricei
            _arg8_rightpricei_ = memoryview(_tmparr__arg8_rightpricei)
            _copyback__arg8_rightpricei = True
      if _arg8_rightpricei_ is not None and len(_arg8_rightpricei_) !=  _arg1_numi :
        raise ValueError("Array argument _arg8_rightpricei has wrong length")
      _copyback__arg9_leftrangei = False
      if _arg9_leftrangei is None:
        _arg9_leftrangei_ = None
      else:
        try:
          _arg9_leftrangei_ = memoryview(_arg9_leftrangei)
        except TypeError:
          try:
            _tmparr__arg9_leftrangei = (ctypes.c_double*len(_arg9_leftrangei))()
            _tmparr__arg9_leftrangei[:] = _arg9_leftrangei
          except TypeError:
            raise TypeError("Argument _arg9_leftrangei has wrong type")
          else:
            _arg9_leftrangei_ = memoryview(_tmparr__arg9_leftrangei)
            _copyback__arg9_leftrangei = True
        else:
          if _arg9_leftrangei_.format != "d":
            _tmparr__arg9_leftrangei = (ctypes.c_double*len(_arg9_leftrangei))()
            _tmparr__arg9_leftrangei[:] = _arg9_leftrangei
            _arg9_leftrangei_ = memoryview(_tmparr__arg9_leftrangei)
            _copyback__arg9_leftrangei = True
      if _arg9_leftrangei_ is not None and len(_arg9_leftrangei_) !=  _arg1_numi :
        raise ValueError("Array argument _arg9_leftrangei has wrong length")
      _copyback__arg10_rightrangei = False
      if _arg10_rightrangei is None:
        _arg10_rightrangei_ = None
      else:
        try:
          _arg10_rightrangei_ = memoryview(_arg10_rightrangei)
        except TypeError:
          try:
            _tmparr__arg10_rightrangei = (ctypes.c_double*len(_arg10_rightrangei))()
            _tmparr__arg10_rightrangei[:] = _arg10_rightrangei
          except TypeError:
            raise TypeError("Argument _arg10_rightrangei has wrong type")
          else:
            _arg10_rightrangei_ = memoryview(_tmparr__arg10_rightrangei)
            _copyback__arg10_rightrangei = True
        else:
          if _arg10_rightrangei_.format != "d":
            _tmparr__arg10_rightrangei = (ctypes.c_double*len(_arg10_rightrangei))()
            _tmparr__arg10_rightrangei[:] = _arg10_rightrangei
            _arg10_rightrangei_ = memoryview(_tmparr__arg10_rightrangei)
            _copyback__arg10_rightrangei = True
      if _arg10_rightrangei_ is not None and len(_arg10_rightrangei_) !=  _arg1_numi :
        raise ValueError("Array argument _arg10_rightrangei has wrong length")
      _copyback__arg11_leftpricej = False
      if _arg11_leftpricej is None:
        _arg11_leftpricej_ = None
      else:
        try:
          _arg11_leftpricej_ = memoryview(_arg11_leftpricej)
        except TypeError:
          try:
            _tmparr__arg11_leftpricej = (ctypes.c_double*len(_arg11_leftpricej))()
            _tmparr__arg11_leftpricej[:] = _arg11_leftpricej
          except TypeError:
            raise TypeError("Argument _arg11_leftpricej has wrong type")
          else:
            _arg11_leftpricej_ = memoryview(_tmparr__arg11_leftpricej)
            _copyback__arg11_leftpricej = True
        else:
          if _arg11_leftpricej_.format != "d":
            _tmparr__arg11_leftpricej = (ctypes.c_double*len(_arg11_leftpricej))()
            _tmparr__arg11_leftpricej[:] = _arg11_leftpricej
            _arg11_leftpricej_ = memoryview(_tmparr__arg11_leftpricej)
            _copyback__arg11_leftpricej = True
      if _arg11_leftpricej_ is not None and len(_arg11_leftpricej_) !=  _arg4_numj :
        raise ValueError("Array argument _arg11_leftpricej has wrong length")
      _copyback__arg12_rightpricej = False
      if _arg12_rightpricej is None:
        _arg12_rightpricej_ = None
      else:
        try:
          _arg12_rightpricej_ = memoryview(_arg12_rightpricej)
        except TypeError:
          try:
            _tmparr__arg12_rightpricej = (ctypes.c_double*len(_arg12_rightpricej))()
            _tmparr__arg12_rightpricej[:] = _arg12_rightpricej
          except TypeError:
            raise TypeError("Argument _arg12_rightpricej has wrong type")
          else:
            _arg12_rightpricej_ = memoryview(_tmparr__arg12_rightpricej)
            _copyback__arg12_rightpricej = True
        else:
          if _arg12_rightpricej_.format != "d":
            _tmparr__arg12_rightpricej = (ctypes.c_double*len(_arg12_rightpricej))()
            _tmparr__arg12_rightpricej[:] = _arg12_rightpricej
            _arg12_rightpricej_ = memoryview(_tmparr__arg12_rightpricej)
            _copyback__arg12_rightpricej = True
      if _arg12_rightpricej_ is not None and len(_arg12_rightpricej_) !=  _arg4_numj :
        raise ValueError("Array argument _arg12_rightpricej has wrong length")
      _copyback__arg13_leftrangej = False
      if _arg13_leftrangej is None:
        _arg13_leftrangej_ = None
      else:
        try:
          _arg13_leftrangej_ = memoryview(_arg13_leftrangej)
        except TypeError:
          try:
            _tmparr__arg13_leftrangej = (ctypes.c_double*len(_arg13_leftrangej))()
            _tmparr__arg13_leftrangej[:] = _arg13_leftrangej
          except TypeError:
            raise TypeError("Argument _arg13_leftrangej has wrong type")
          else:
            _arg13_leftrangej_ = memoryview(_tmparr__arg13_leftrangej)
            _copyback__arg13_leftrangej = True
        else:
          if _arg13_leftrangej_.format != "d":
            _tmparr__arg13_leftrangej = (ctypes.c_double*len(_arg13_leftrangej))()
            _tmparr__arg13_leftrangej[:] = _arg13_leftrangej
            _arg13_leftrangej_ = memoryview(_tmparr__arg13_leftrangej)
            _copyback__arg13_leftrangej = True
      if _arg13_leftrangej_ is not None and len(_arg13_leftrangej_) !=  _arg4_numj :
        raise ValueError("Array argument _arg13_leftrangej has wrong length")
      _copyback__arg14_rightrangej = False
      if _arg14_rightrangej is None:
        _arg14_rightrangej_ = None
      else:
        try:
          _arg14_rightrangej_ = memoryview(_arg14_rightrangej)
        except TypeError:
          try:
            _tmparr__arg14_rightrangej = (ctypes.c_double*len(_arg14_rightrangej))()
            _tmparr__arg14_rightrangej[:] = _arg14_rightrangej
          except TypeError:
            raise TypeError("Argument _arg14_rightrangej has wrong type")
          else:
            _arg14_rightrangej_ = memoryview(_tmparr__arg14_rightrangej)
            _copyback__arg14_rightrangej = True
        else:
          if _arg14_rightrangej_.format != "d":
            _tmparr__arg14_rightrangej = (ctypes.c_double*len(_arg14_rightrangej))()
            _tmparr__arg14_rightrangej[:] = _arg14_rightrangej
            _arg14_rightrangej_ = memoryview(_tmparr__arg14_rightrangej)
            _copyback__arg14_rightrangej = True
      if _arg14_rightrangej_ is not None and len(_arg14_rightrangej_) !=  _arg4_numj :
        raise ValueError("Array argument _arg14_rightrangej has wrong length")
      res = self.__obj.primalsensitivity(_arg1_numi_,_arg2_subi_,_arg3_marki_,_arg4_numj_,_arg5_subj_,_arg6_markj_,_arg7_leftpricei_,_arg8_rightpricei_,_arg9_leftrangei_,_arg10_rightrangei_,_arg11_leftpricej_,_arg12_rightpricej_,_arg13_leftrangej_,_arg14_rightrangej_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg14_rightrangej:
        _arg14_rightrangej[:] = _tmparr__arg14_rightrangej
      if _copyback__arg13_leftrangej:
        _arg13_leftrangej[:] = _tmparr__arg13_leftrangej
      if _copyback__arg12_rightpricej:
        _arg12_rightpricej[:] = _tmparr__arg12_rightpricej
      if _copyback__arg11_leftpricej:
        _arg11_leftpricej[:] = _tmparr__arg11_leftpricej
      if _copyback__arg10_rightrangei:
        _arg10_rightrangei[:] = _tmparr__arg10_rightrangei
      if _copyback__arg9_leftrangei:
        _arg9_leftrangei[:] = _tmparr__arg9_leftrangei
      if _copyback__arg8_rightpricei:
        _arg8_rightpricei[:] = _tmparr__arg8_rightpricei
      if _copyback__arg7_leftpricei:
        _arg7_leftpricei[:] = _tmparr__arg7_leftpricei
    
    def sensitivityreport(self,_arg1_whichstream_): # 2
      """
      Creates a sensitivity report.
    
      sensitivityreport(self,_arg1_whichstream_)
        _arg1_whichstream: mosek.streamtype. <no description>
      """
      if not isinstance(_arg1_whichstream_,streamtype): raise TypeError("Argument _arg1_whichstream has wrong type")
      _arg1_whichstream = _arg1_whichstream_
      res = self.__obj.sensitivityreport(_arg1_whichstream_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def dualsensitivity(self,_arg2_subj,_arg3_leftpricej,_arg4_rightpricej,_arg5_leftrangej,_arg6_rightrangej): # 2
      """
      Performs sensitivity analysis on objective coefficients.
    
      dualsensitivity(self,_arg2_subj,_arg3_leftpricej,_arg4_rightpricej,_arg5_leftrangej,_arg6_rightrangej)
        _arg2_subj: array of int. Indexes of objective coefficients to analyze.
        _arg3_leftpricej: array of double. Left shadow prices for requested coefficients.
        _arg4_rightpricej: array of double. Right shadow prices for requested coefficients.
        _arg5_leftrangej: array of double. Left range for requested coefficients.
        _arg6_rightrangej: array of double. Right range for requested coefficients.
      """
      _arg1_numj_ = None
      if _arg1_numj_ is None:
        _arg1_numj_ = len(_arg2_subj)
        _arg1_numj = _arg1_numj_
      elif _arg1_numj_ != len(_arg2_subj):
        raise IndexError("Inconsistent length of array _arg2_subj")
      if _arg1_numj_ is None: _arg1_numj_ = 0
      if _arg2_subj is None: raise TypeError("Invalid type for argument _arg2_subj")
      if _arg2_subj is None:
        _arg2_subj_ = None
      else:
        try:
          _arg2_subj_ = memoryview(_arg2_subj)
        except TypeError:
          try:
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
          except TypeError:
            raise TypeError("Argument _arg2_subj has wrong type")
          else:
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
        else:
          if _arg2_subj_.format != "i":
            _tmparr__arg2_subj = (ctypes.c_int32*len(_arg2_subj))()
            _tmparr__arg2_subj[:] = _arg2_subj
            _arg2_subj_ = memoryview(_tmparr__arg2_subj)
      
      _copyback__arg3_leftpricej = False
      if _arg3_leftpricej is None:
        _arg3_leftpricej_ = None
      else:
        try:
          _arg3_leftpricej_ = memoryview(_arg3_leftpricej)
        except TypeError:
          try:
            _tmparr__arg3_leftpricej = (ctypes.c_double*len(_arg3_leftpricej))()
            _tmparr__arg3_leftpricej[:] = _arg3_leftpricej
          except TypeError:
            raise TypeError("Argument _arg3_leftpricej has wrong type")
          else:
            _arg3_leftpricej_ = memoryview(_tmparr__arg3_leftpricej)
            _copyback__arg3_leftpricej = True
        else:
          if _arg3_leftpricej_.format != "d":
            _tmparr__arg3_leftpricej = (ctypes.c_double*len(_arg3_leftpricej))()
            _tmparr__arg3_leftpricej[:] = _arg3_leftpricej
            _arg3_leftpricej_ = memoryview(_tmparr__arg3_leftpricej)
            _copyback__arg3_leftpricej = True
      if _arg3_leftpricej_ is not None and len(_arg3_leftpricej_) !=  _arg1_numj :
        raise ValueError("Array argument _arg3_leftpricej has wrong length")
      _copyback__arg4_rightpricej = False
      if _arg4_rightpricej is None:
        _arg4_rightpricej_ = None
      else:
        try:
          _arg4_rightpricej_ = memoryview(_arg4_rightpricej)
        except TypeError:
          try:
            _tmparr__arg4_rightpricej = (ctypes.c_double*len(_arg4_rightpricej))()
            _tmparr__arg4_rightpricej[:] = _arg4_rightpricej
          except TypeError:
            raise TypeError("Argument _arg4_rightpricej has wrong type")
          else:
            _arg4_rightpricej_ = memoryview(_tmparr__arg4_rightpricej)
            _copyback__arg4_rightpricej = True
        else:
          if _arg4_rightpricej_.format != "d":
            _tmparr__arg4_rightpricej = (ctypes.c_double*len(_arg4_rightpricej))()
            _tmparr__arg4_rightpricej[:] = _arg4_rightpricej
            _arg4_rightpricej_ = memoryview(_tmparr__arg4_rightpricej)
            _copyback__arg4_rightpricej = True
      if _arg4_rightpricej_ is not None and len(_arg4_rightpricej_) !=  _arg1_numj :
        raise ValueError("Array argument _arg4_rightpricej has wrong length")
      _copyback__arg5_leftrangej = False
      if _arg5_leftrangej is None:
        _arg5_leftrangej_ = None
      else:
        try:
          _arg5_leftrangej_ = memoryview(_arg5_leftrangej)
        except TypeError:
          try:
            _tmparr__arg5_leftrangej = (ctypes.c_double*len(_arg5_leftrangej))()
            _tmparr__arg5_leftrangej[:] = _arg5_leftrangej
          except TypeError:
            raise TypeError("Argument _arg5_leftrangej has wrong type")
          else:
            _arg5_leftrangej_ = memoryview(_tmparr__arg5_leftrangej)
            _copyback__arg5_leftrangej = True
        else:
          if _arg5_leftrangej_.format != "d":
            _tmparr__arg5_leftrangej = (ctypes.c_double*len(_arg5_leftrangej))()
            _tmparr__arg5_leftrangej[:] = _arg5_leftrangej
            _arg5_leftrangej_ = memoryview(_tmparr__arg5_leftrangej)
            _copyback__arg5_leftrangej = True
      if _arg5_leftrangej_ is not None and len(_arg5_leftrangej_) !=  _arg1_numj :
        raise ValueError("Array argument _arg5_leftrangej has wrong length")
      _copyback__arg6_rightrangej = False
      if _arg6_rightrangej is None:
        _arg6_rightrangej_ = None
      else:
        try:
          _arg6_rightrangej_ = memoryview(_arg6_rightrangej)
        except TypeError:
          try:
            _tmparr__arg6_rightrangej = (ctypes.c_double*len(_arg6_rightrangej))()
            _tmparr__arg6_rightrangej[:] = _arg6_rightrangej
          except TypeError:
            raise TypeError("Argument _arg6_rightrangej has wrong type")
          else:
            _arg6_rightrangej_ = memoryview(_tmparr__arg6_rightrangej)
            _copyback__arg6_rightrangej = True
        else:
          if _arg6_rightrangej_.format != "d":
            _tmparr__arg6_rightrangej = (ctypes.c_double*len(_arg6_rightrangej))()
            _tmparr__arg6_rightrangej[:] = _arg6_rightrangej
            _arg6_rightrangej_ = memoryview(_tmparr__arg6_rightrangej)
            _copyback__arg6_rightrangej = True
      if _arg6_rightrangej_ is not None and len(_arg6_rightrangej_) !=  _arg1_numj :
        raise ValueError("Array argument _arg6_rightrangej has wrong length")
      res = self.__obj.dualsensitivity(_arg1_numj_,_arg2_subj_,_arg3_leftpricej_,_arg4_rightpricej_,_arg5_leftrangej_,_arg6_rightrangej_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      if _copyback__arg6_rightrangej:
        _arg6_rightrangej[:] = _tmparr__arg6_rightrangej
      if _copyback__arg5_leftrangej:
        _arg5_leftrangej[:] = _tmparr__arg5_leftrangej
      if _copyback__arg4_rightpricej:
        _arg4_rightpricej[:] = _tmparr__arg4_rightpricej
      if _copyback__arg3_leftpricej:
        _arg3_leftpricej[:] = _tmparr__arg3_leftpricej
    
    def optimizermt(self,_arg1_server_,_arg2_port_): # 2
      """
      Offload the optimization task to a solver server.
    
      optimizermt(self,_arg1_server_,_arg2_port_)
        _arg1_server: str. Name or IP address of the solver server.
        _arg2_port: str. Network port of the solver server.
      returns: _arg3_trmcode
        _arg3_trmcode: mosek.rescode. Is either OK or a termination response code.
      """
      res,resargs = self.__obj.optimizermt(_arg1_server_,_arg2_port_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg3_trmcode_return_value = resargs
      __arg3_trmcode_return_value = rescode(__arg3_trmcode_return_value)
      return __arg3_trmcode_return_value
    
    def asyncoptimize(self,_arg1_server_,_arg2_port_): # 2
      """
      Offload the optimization task to a solver server.
    
      asyncoptimize(self,_arg1_server_,_arg2_port_)
        _arg1_server: str. Name or IP address of the solver server
        _arg2_port: str. Network port of the solver service
      returns: _arg3_token
        _arg3_token: str. Returns the task token
      """
      arr__arg3_token = (ctypes.c_char*33)()
      memview_arr__arg3_token = memoryview(arr__arg3_token)
      res,resargs = self.__obj.asyncoptimize(_arg1_server_,_arg2_port_,memview_arr__arg3_token)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      retarg__arg3_token = resargs
      retarg__arg3_token = arr__arg3_token.value.decode("utf-8",errors="ignore")
      return retarg__arg3_token
    
    def asyncstop(self,_arg1_server_,_arg2_port_,_arg3_token_): # 2
      """
      Request that the job identified by the token is terminated.
    
      asyncstop(self,_arg1_server_,_arg2_port_,_arg3_token_)
        _arg1_server: str. Name or IP address of the solver server
        _arg2_port: str. Network port of the solver service
        _arg3_token: str. The task token
      """
      res = self.__obj.asyncstop(_arg1_server_,_arg2_port_,_arg3_token_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
    
    def asyncpoll(self,_arg1_server_,_arg2_port_,_arg3_token_): # 2
      """
      Requests information about the status of the remote job.
    
      asyncpoll(self,_arg1_server_,_arg2_port_,_arg3_token_)
        _arg1_server: str. Name or IP address of the solver server
        _arg2_port: str. Network port of the solver service
        _arg3_token: str. The task token
      returns: _arg4_respavailable,_arg5_resp,_arg6_trm
        _arg4_respavailable: int. Indicates if a remote response is available.
        _arg5_resp: mosek.rescode. Is the response code from the remote solver.
        _arg6_trm: mosek.rescode. Is either OK or a termination response code.
      """
      res,resargs = self.__obj.asyncpoll(_arg1_server_,_arg2_port_,_arg3_token_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg4_respavailable_return_value,__arg5_resp_return_value,__arg6_trm_return_value = resargs
      __arg6_trm_return_value = rescode(__arg6_trm_return_value)
      __arg5_resp_return_value = rescode(__arg5_resp_return_value)
      return __arg4_respavailable_return_value,__arg5_resp_return_value,__arg6_trm_return_value
    
    def asyncgetresult(self,_arg1_server_,_arg2_port_,_arg3_token_): # 2
      """
      Request a response from a remote job.
    
      asyncgetresult(self,_arg1_server_,_arg2_port_,_arg3_token_)
        _arg1_server: str. Name or IP address of the solver server.
        _arg2_port: str. Network port of the solver service.
        _arg3_token: str. The task token.
      returns: _arg4_respavailable,_arg5_resp,_arg6_trm
        _arg4_respavailable: int. Indicates if a remote response is available.
        _arg5_resp: mosek.rescode. Is the response code from the remote solver.
        _arg6_trm: mosek.rescode. Is either OK or a termination response code.
      """
      res,resargs = self.__obj.asyncgetresult(_arg1_server_,_arg2_port_,_arg3_token_)
      if res != 0:
        result,msg = self.__getlasterror(res)
        raise Error(rescode(res),msg)
      __arg4_respavailable_return_value,__arg5_resp_return_value,__arg6_trm_return_value = resargs
      __arg6_trm_return_value = rescode(__arg6_trm_return_value)
      __arg5_resp_return_value = rescode(__arg5_resp_return_value)
      return __arg4_respavailable_return_value,__arg5_resp_return_value,__arg6_trm_return_value
    


class LinAlg:
  __env = Env()

  axpy = __env.axpy
  dot  = __env.dot
  gemv = __env.gemv
  gemm = __env.gemm
  syrk = __env.syrk
  syeig = __env.syeig
  syevd = __env.syevd
  potrf = __env.potrf
