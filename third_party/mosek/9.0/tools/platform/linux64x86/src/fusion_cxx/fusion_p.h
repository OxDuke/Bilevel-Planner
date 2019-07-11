#ifndef __FUSION_P_H__
#define __FUSION_P_H__
#include "monty.h"
#include "mosektask_p.h"
#include "list"
#include "vector"
#include "unordered_map"
#include "fusion.h"
namespace mosek
{
namespace fusion
{
// mosek.fusion.BaseModel from file 'src/fusion/cxx/BaseModel_p.h'
// namespace mosek::fusion
struct p_BaseModel
{
  p_BaseModel(BaseModel * _pubthis);

  void _initialize( monty::rc_ptr<BaseModel> m);
  void _initialize( const std::string & name,
                    const std::string & licpath);

  virtual ~p_BaseModel() { /* std::cout << "~p_BaseModel()" << std::endl;*/  }

  static p_BaseModel * _get_impl(Model * _inst) { return _inst->_impl; }

  //----------------------

  bool synched;
  std::string taskname;

  monty::rc_ptr<SolutionStruct> sol_itr;
  monty::rc_ptr<SolutionStruct> sol_itg;
  monty::rc_ptr<SolutionStruct> sol_bas;

  //---------------------

  std::unique_ptr<Task> task;

  //---------------------
  void task_setLogHandler (const msghandler_t & handler);
  void task_setDataCallbackHandler (const datacbhandler_t & handler);
  void task_setCallbackHandler (const cbhandler_t & handler);

  int task_append_barvar(int size, int num);

  void task_var_name   (int index, const std::string & name);
  void task_con_name   (int index, const std::string & name);
  void task_cone_name  (int index, const std::string & name);
  void task_barvar_name(int index, const std::string & name);
  void task_objectivename(         const std::string & name);

  void task_format_var_names(const std::shared_ptr<monty::ndarray<int,1>> subj, const std::string & format,const std::shared_ptr<monty::ndarray<int,1>> dims, const std::shared_ptr<monty::ndarray<long long,1>> sp);
  void task_format_con_names(const std::shared_ptr<monty::ndarray<int,1>> subi, const std::string & format,const std::shared_ptr<monty::ndarray<int,1>> dims, const std::shared_ptr<monty::ndarray<long long,1>> sp);
  void task_format_cone_names(const std::shared_ptr<monty::ndarray<int,1>> subk, const std::string & format,const std::shared_ptr<monty::ndarray<int,1>> dims, const std::shared_ptr<monty::ndarray<long long,1>> sp);

  void task_break_solve();

  //--------------------------

  int task_numvar();
  int task_numcon();
  int task_numcone();
  int task_numbarvar();

  //--------------------------

  void task_put_param(const std::string & name, const std::string & value);
  void task_put_param(const std::string & name, int    value);
  void task_put_param(const std::string & name, double value);
  
  double    task_get_dinf (const std::string & name);
  int       task_get_iinf (const std::string & name);
  long long task_get_liinf(const std::string & name);
  
  //--------------------------

  void task_con_putboundlist_fr(const std::shared_ptr<monty::ndarray<int,1>> idxs);
  void task_con_putboundlist_lo(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs);
  void task_con_putboundlist_up(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs);
  void task_con_putboundlist_fx(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs);
  void task_con_putboundlist_ra(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & lb , 
                                const std::shared_ptr<monty::ndarray<double,1>> & ub );

  void task_var_putboundlist_fr(const std::shared_ptr<monty::ndarray<int,1>> idxs);
  void task_var_putboundlist_lo(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs);
  void task_var_putboundlist_up(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs);
  void task_var_putboundlist_fx(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs);
  void task_var_putboundlist_ra(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & lb , 
                                const std::shared_ptr<monty::ndarray<double,1>> & ub );
  
  void task_var_putintlist(const std::shared_ptr<monty::ndarray<int,1>> & idxs);
  void task_var_putcontlist(const std::shared_ptr<monty::ndarray<int,1>> & idxs); 
 
  //--------------------------

  void task_putbararowlist(const std::shared_ptr<monty::ndarray<int,1>>       subi,
                           const std::shared_ptr<monty::ndarray<long long,1>> ptr,
                           const std::shared_ptr<monty::ndarray<int,1>>       subj,
                           const std::shared_ptr<monty::ndarray<long long,1>> matidx);

  void task_putbaraijlist(const std::shared_ptr<monty::ndarray<int,1>> subi,
                          const std::shared_ptr<monty::ndarray<int,1>> subj,
                          std::shared_ptr<monty::ndarray<long long,1>> matidx);
  
  void task_putbarc(const std::shared_ptr<monty::ndarray<int,1>> subj,
                    const std::shared_ptr<monty::ndarray<int,1>> subl,
                    const std::shared_ptr<monty::ndarray<int,1>> subk,
                    const std::shared_ptr<monty::ndarray<double,1>> val);
  
  std::shared_ptr<monty::ndarray<long long,1>> task_appendsymmatlist (const std::shared_ptr<monty::ndarray<int,1>>       & dim, 
                                                                      const std::shared_ptr<monty::ndarray<long long,1>> & nz, 
                                                                      const std::shared_ptr<monty::ndarray<int,1>>       & subk, 
                                                                      const std::shared_ptr<monty::ndarray<int,1>>       & subl, 
                                                                      const std::shared_ptr<monty::ndarray<double,1>>    & val);
  int  task_barvar_dim(int j);
  void task_putbaraij (int i, int j, int k);
  void task_putbaraij (int i, int j, const std::shared_ptr<monty::ndarray<int,1>> & k);
  void task_putbarcj  (int j, int k);
  void task_putbarcj  (int j,        const std::shared_ptr<monty::ndarray<int,1>> & k);
  int  task_barvardim (int index);

  int task_append_var(int num);
  int task_append_con(int num);

  void task_append_zerocones (int numcone);
  void task_clear_cones   ( const std::shared_ptr<monty::ndarray<int,1>> & idxs );
  void task_put_zerocones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs );
  void task_put_quadcones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs );
  void task_put_rquadcones( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs );
  void task_put_pexpcones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs );
  void task_put_ppowcones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs, const std::shared_ptr<monty::ndarray<double,1>> & alpha );
  void task_put_dexpcones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs );
  void task_put_dpowcones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs, const std::shared_ptr<monty::ndarray<double,1>> & alpha );

  void task_putarowlist(
    const std::shared_ptr<monty::ndarray<int,1>>       & idxs, 
    const std::shared_ptr<monty::ndarray<long long,1>> & ptrb, 
    const std::shared_ptr<monty::ndarray<int,1>>       & subj, 
    const std::shared_ptr<monty::ndarray<double,1>>    & cof);
  void task_putaijlist(
    const std::shared_ptr<monty::ndarray<int,1>>       & subi, 
    const std::shared_ptr<monty::ndarray<int,1>>       & subj, 
    const std::shared_ptr<monty::ndarray<double,1>>    & cof, 
    long long                           num);

  void task_setnumvar(int num);
  void task_cleanup(int oldnum, int oldnumcon, int oldnumcone, int oldnumbarvar);
  void task_solve();

  void task_putobjective( 
    bool                             maximize,
    const std::shared_ptr<monty::ndarray<int,1>>    & subj    ,
    const std::shared_ptr<monty::ndarray<double,1>> & cof     ,
    double                           cfix    );

  void task_putclist(   
    const std::shared_ptr<monty::ndarray<int,1>>    & subj,
    const std::shared_ptr<monty::ndarray<double,1>> & cof);


  void task_putobjectivename(const std::string & name);

  void task_write(const std::string & filename);
  void task_dump (const std::string & filename);

  MSKtask_t task_get();
  MSKtask_t __mosek_2fusion_2BaseModel__task_get();
  
  void dispose();

  void task_putxx_slice(SolutionType which, int first, int last, std::shared_ptr<monty::ndarray<double,1>> & xx);

  static void env_syeig (int n, std::shared_ptr<monty::ndarray<double,1>> & a, std::shared_ptr<monty::ndarray<double,1>> & w);
  static void env_potrf (int n, std::shared_ptr<monty::ndarray<double,1>> & a);                        
  static void env_syevd (int n, std::shared_ptr<monty::ndarray<double,1>> & a, std::shared_ptr<monty::ndarray<double,1>> & w);

  static void env_putlicensecode(std::shared_ptr<monty::ndarray<int,1>> code);
  static void env_putlicensepath(const std::string &licfile);
  static void env_putlicensewait(int wait);

  static std::string env_getversion();

  void convertSolutionStatus(MSKsoltypee soltype, p_SolutionStruct * sol, MSKsolstae status, MSKprostae prosta);


};

// End of file 'src/fusion/cxx/BaseModel_p.h'
struct p_Model : public ::mosek::fusion::p_BaseModel
{
Model * _pubthis;
static mosek::fusion::p_Model* _get_impl(mosek::fusion::Model * _inst){ return static_cast< mosek::fusion::p_Model* >(mosek::fusion::p_BaseModel::_get_impl(_inst)); }
static mosek::fusion::p_Model * _get_impl(mosek::fusion::Model::t _inst) { return _get_impl(_inst.get()); }
p_Model(Model * _pubthis);
virtual ~p_Model() { /* std::cout << "~p_Model" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::WorkStack > xs{};monty::rc_ptr< ::mosek::fusion::WorkStack > ws{};monty::rc_ptr< ::mosek::fusion::WorkStack > rs{};monty::rc_ptr< ::mosek::fusion::Utils::StringIntMap > con_map{};std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::ModelConstraint >,1 > > cons{};std::shared_ptr< monty::ndarray< int,1 > > natconmap_type{};std::shared_ptr< monty::ndarray< double,1 > > natconmap_ub{};std::shared_ptr< monty::ndarray< double,1 > > natconmap_lb{};std::shared_ptr< monty::ndarray< double,1 > > natconmap_efix{};std::shared_ptr< monty::ndarray< int,1 > > natconmap_idx{};std::shared_ptr< monty::ndarray< long long,1 > > natconmap_slackidx{};std::shared_ptr< monty::ndarray< int,1 > > natconmap_blockid{};monty::rc_ptr< ::mosek::fusion::LinkedBlocks > natconmap{};std::shared_ptr< monty::ndarray< bool,1 > > initsol_xx_flag{};std::shared_ptr< monty::ndarray< double,1 > > initsol_xx{};monty::rc_ptr< ::mosek::fusion::Utils::StringIntMap > var_map{};std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::ModelVariable >,1 > > barvars{};std::shared_ptr< monty::ndarray< int,1 > > natbarvarmap_ptr{};std::shared_ptr< monty::ndarray< int,1 > > natbarvarmap_num{};int natbarvarmap_nblock{};std::shared_ptr< monty::ndarray< int,1 > > natbarvar_dim{};std::shared_ptr< monty::ndarray< long long,1 > > natbarvar_ptr{};int natbarvar_numbarvarelm{};std::shared_ptr< monty::ndarray< int,1 > > natbarvar_j{};std::shared_ptr< monty::ndarray< int,1 > > natbarvar_i{};std::shared_ptr< monty::ndarray< int,1 > > natbarvar_idx{};std::shared_ptr< monty::ndarray< int,1 > > natvarmap_type{};std::shared_ptr< monty::ndarray< int,1 > > natconemap_dim{};monty::rc_ptr< ::mosek::fusion::LinkedBlocks > natconemap{};std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::ModelVariable >,1 > > vars{};int bfixidx{};std::shared_ptr< monty::ndarray< int,1 > > natvarmap_idx{};std::shared_ptr< monty::ndarray< int,1 > > natvarmap_blockid{};monty::rc_ptr< ::mosek::fusion::LinkedBlocks > natvarmap{};mosek::fusion::SolutionType solutionptr{};mosek::fusion::AccSolutionStatus acceptable_sol{};std::string model_name{};virtual void destroy();
static Model::t _new_Model(monty::rc_ptr< ::mosek::fusion::Model > _464);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _464);
static Model::t _new_Model(const std::string &  _469);
void _initialize(const std::string &  _469);
static Model::t _new_Model();
void _initialize();
virtual monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > __mosek_2fusion_2Model__formstConstr(monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > _472,std::shared_ptr< monty::ndarray< int,1 > > _473,std::shared_ptr< monty::ndarray< int,1 > > _474) ;
virtual void connames(std::shared_ptr< monty::ndarray< int,1 > > _475,const std::string &  _476,std::shared_ptr< monty::ndarray< int,1 > > _477,std::shared_ptr< monty::ndarray< long long,1 > > _478) ;
virtual void varnames(std::shared_ptr< monty::ndarray< int,1 > > _479,const std::string &  _480,std::shared_ptr< monty::ndarray< int,1 > > _481,std::shared_ptr< monty::ndarray< long long,1 > > _482) ;
virtual void varname(int _483,const std::string &  _484) ;
virtual void natbarvarmap_get(int _485,std::shared_ptr< monty::ndarray< int,1 > > _486) ;
virtual void natbarvar_get(int _490,std::shared_ptr< monty::ndarray< long long,1 > > _491) ;
virtual int natbarvarmap_alloc(int _498,int _499) ;
virtual int natvarmap_alloc(int _517) ;
virtual int natconmap_alloc(int _527) ;
virtual int natconemap_alloc(int _537) ;
virtual void make_continuous(std::shared_ptr< monty::ndarray< long long,1 > > _540) ;
virtual void make_integer(std::shared_ptr< monty::ndarray< long long,1 > > _546) ;
static  void putlicensewait(bool _552);
static  void putlicensepath(const std::string &  _553);
static  void putlicensecode(std::shared_ptr< monty::ndarray< int,1 > > _554);
virtual /* override */ void dispose() ;
virtual void nativeVarToStr(int _557,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > _558) ;
virtual MSKtask_t __mosek_2fusion_2Model__getTask() ;
virtual void getConstraintValues(bool _559,std::shared_ptr< monty::ndarray< int,1 > > _560,std::shared_ptr< monty::ndarray< double,1 > > _561,int _562) ;
virtual void getVariableDuals(bool _570,std::shared_ptr< monty::ndarray< long long,1 > > _571,std::shared_ptr< monty::ndarray< double,1 > > _572,int _573) ;
virtual void getVariableValues(bool _579,std::shared_ptr< monty::ndarray< long long,1 > > _580,std::shared_ptr< monty::ndarray< double,1 > > _581,int _582) ;
virtual void setVariableValues(bool _588,std::shared_ptr< monty::ndarray< long long,1 > > _589,std::shared_ptr< monty::ndarray< double,1 > > _590) ;
virtual void flushNames() ;
virtual void writeTask(const std::string &  _600) ;
virtual long long getSolverLIntInfo(const std::string &  _601) ;
virtual int getSolverIntInfo(const std::string &  _602) ;
virtual double getSolverDoubleInfo(const std::string &  _603) ;
virtual void setCallbackHandler(mosek::cbhandler_t  _604) ;
virtual void setDataCallbackHandler(mosek::datacbhandler_t  _605) ;
virtual void setLogHandler(mosek::msghandler_t  _606) ;
virtual void setSolverParam(const std::string &  _607,double _608) ;
virtual void setSolverParam(const std::string &  _609,int _610) ;
virtual void setSolverParam(const std::string &  _611,const std::string &  _612) ;
virtual void breakSolver() ;
virtual void solve() ;
virtual void flushSolutions() ;
virtual void flush_initsol(mosek::fusion::SolutionType _613) ;
virtual mosek::fusion::SolutionStatus getDualSolutionStatus() ;
virtual mosek::fusion::ProblemStatus getProblemStatus() ;
virtual mosek::fusion::SolutionStatus getPrimalSolutionStatus() ;
virtual double dualObjValue() ;
virtual double primalObjValue() ;
virtual monty::rc_ptr< ::mosek::fusion::SolutionStruct > __mosek_2fusion_2Model__get_sol_cache(mosek::fusion::SolutionType _620,bool _621,bool _622) ;
virtual monty::rc_ptr< ::mosek::fusion::SolutionStruct > __mosek_2fusion_2Model__get_sol_cache(mosek::fusion::SolutionType _628,bool _629) ;
virtual void setSolution_xx(std::shared_ptr< monty::ndarray< int,1 > > _630,std::shared_ptr< monty::ndarray< double,1 > > _631) ;
virtual void ensure_initsol_xx() ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_bars(mosek::fusion::SolutionType _638) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_barx(mosek::fusion::SolutionType _639) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_y(mosek::fusion::SolutionType _640) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_xc(mosek::fusion::SolutionType _641) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_snx(mosek::fusion::SolutionType _642) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_suc(mosek::fusion::SolutionType _643) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_slc(mosek::fusion::SolutionType _644) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_sux(mosek::fusion::SolutionType _645) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_slx(mosek::fusion::SolutionType _646) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_yx(mosek::fusion::SolutionType _647) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > getSolution_xx(mosek::fusion::SolutionType _648) ;
virtual void selectedSolution(mosek::fusion::SolutionType _649) ;
virtual mosek::fusion::AccSolutionStatus getAcceptedSolutionStatus() ;
virtual void acceptedSolutionStatus(mosek::fusion::AccSolutionStatus _650) ;
virtual mosek::fusion::ProblemStatus getProblemStatus(mosek::fusion::SolutionType _651) ;
virtual mosek::fusion::SolutionStatus getDualSolutionStatus(mosek::fusion::SolutionType _653) ;
virtual mosek::fusion::SolutionStatus getPrimalSolutionStatus(mosek::fusion::SolutionType _654) ;
virtual mosek::fusion::SolutionStatus getSolutionStatus(mosek::fusion::SolutionType _655,bool _656) ;
virtual void update(std::shared_ptr< monty::ndarray< int,1 > > _659,monty::rc_ptr< ::mosek::fusion::Expression > _660) ;
virtual void update(std::shared_ptr< monty::ndarray< int,1 > > _693,monty::rc_ptr< ::mosek::fusion::Expression > _694,std::shared_ptr< monty::ndarray< int,1 > > _695,bool _696) ;
virtual void updateObjective(monty::rc_ptr< ::mosek::fusion::Expression > _726,monty::rc_ptr< ::mosek::fusion::Variable > _727) ;
virtual void objective_(const std::string &  _761,mosek::fusion::ObjectiveSense _762,monty::rc_ptr< ::mosek::fusion::Expression > _763) ;
virtual void objective(double _792) ;
virtual void objective(mosek::fusion::ObjectiveSense _793,double _794) ;
virtual void objective(mosek::fusion::ObjectiveSense _795,monty::rc_ptr< ::mosek::fusion::Expression > _796) ;
virtual void objective(const std::string &  _797,double _798) ;
virtual void objective(const std::string &  _799,mosek::fusion::ObjectiveSense _800,double _801) ;
virtual void objective(const std::string &  _802,mosek::fusion::ObjectiveSense _803,monty::rc_ptr< ::mosek::fusion::Expression > _804) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(monty::rc_ptr< ::mosek::fusion::Expression > _805,monty::rc_ptr< ::mosek::fusion::ConeDomain > _806) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(const std::string &  _807,monty::rc_ptr< ::mosek::fusion::Expression > _808,monty::rc_ptr< ::mosek::fusion::ConeDomain > _809) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(monty::rc_ptr< ::mosek::fusion::Expression > _810,monty::rc_ptr< ::mosek::fusion::RangeDomain > _811) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(const std::string &  _812,monty::rc_ptr< ::mosek::fusion::Expression > _813,monty::rc_ptr< ::mosek::fusion::RangeDomain > _814) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(monty::rc_ptr< ::mosek::fusion::Expression > _815,monty::rc_ptr< ::mosek::fusion::LinearDomain > _816) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(const std::string &  _817,monty::rc_ptr< ::mosek::fusion::Expression > _818,monty::rc_ptr< ::mosek::fusion::LinearDomain > _819) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(monty::rc_ptr< ::mosek::fusion::Expression > _820,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _821) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(const std::string &  _822,monty::rc_ptr< ::mosek::fusion::Expression > _823,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _824) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(monty::rc_ptr< ::mosek::fusion::Expression > _825,monty::rc_ptr< ::mosek::fusion::PSDDomain > _826) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(const std::string &  _827,monty::rc_ptr< ::mosek::fusion::Expression > _828,monty::rc_ptr< ::mosek::fusion::PSDDomain > _829) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _830) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int _831,int _832,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _833) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int _834,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _835) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _836,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _837) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _838,int _839,int _840,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _841) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _842,int _843,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _844) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _845,std::shared_ptr< monty::ndarray< int,1 > > _846,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _847) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(monty::rc_ptr< ::mosek::fusion::PSDDomain > _848) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int _849,int _850,monty::rc_ptr< ::mosek::fusion::PSDDomain > _851) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int _852,monty::rc_ptr< ::mosek::fusion::PSDDomain > _853) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _854,monty::rc_ptr< ::mosek::fusion::PSDDomain > _855) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _856,int _857,int _858,monty::rc_ptr< ::mosek::fusion::PSDDomain > _859) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _860,int _861,monty::rc_ptr< ::mosek::fusion::PSDDomain > _862) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _863,std::shared_ptr< monty::ndarray< int,1 > > _864,monty::rc_ptr< ::mosek::fusion::PSDDomain > _865) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(monty::rc_ptr< ::mosek::fusion::ConeDomain > _866) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(monty::rc_ptr< ::mosek::fusion::RangeDomain > _867) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(monty::rc_ptr< ::mosek::fusion::LinearDomain > _868) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(std::shared_ptr< monty::ndarray< int,1 > > _869,monty::rc_ptr< ::mosek::fusion::ConeDomain > _870) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(std::shared_ptr< monty::ndarray< int,1 > > _871,monty::rc_ptr< ::mosek::fusion::RangeDomain > _872) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(std::shared_ptr< monty::ndarray< int,1 > > _873,monty::rc_ptr< ::mosek::fusion::LinearDomain > _874) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(std::shared_ptr< monty::ndarray< int,1 > > _875) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int _876,monty::rc_ptr< ::mosek::fusion::ConeDomain > _877) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int _878,monty::rc_ptr< ::mosek::fusion::RangeDomain > _879) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int _880,monty::rc_ptr< ::mosek::fusion::LinearDomain > _881) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int _882) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable() ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _883,monty::rc_ptr< ::mosek::fusion::ConeDomain > _884) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _885,monty::rc_ptr< ::mosek::fusion::RangeDomain > _886) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _887,monty::rc_ptr< ::mosek::fusion::LinearDomain > _888) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _889,std::shared_ptr< monty::ndarray< int,1 > > _890,monty::rc_ptr< ::mosek::fusion::ConeDomain > _891) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _892,std::shared_ptr< monty::ndarray< int,1 > > _893,monty::rc_ptr< ::mosek::fusion::RangeDomain > _894) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _895,std::shared_ptr< monty::ndarray< int,1 > > _896,monty::rc_ptr< ::mosek::fusion::LinearDomain > _897) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _898,std::shared_ptr< monty::ndarray< int,1 > > _899) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _900,int _901,monty::rc_ptr< ::mosek::fusion::ConeDomain > _902) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _903,int _904,monty::rc_ptr< ::mosek::fusion::RangeDomain > _905) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _906,int _907,monty::rc_ptr< ::mosek::fusion::LinearDomain > _908) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _909,int _910) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  _911) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__ranged_variable(const std::string &  _912,std::shared_ptr< monty::ndarray< int,1 > > _913,monty::rc_ptr< ::mosek::fusion::RangeDomain > _914) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable_(const std::string &  _935,std::shared_ptr< monty::ndarray< int,1 > > _936,monty::rc_ptr< ::mosek::fusion::ConeDomain > _937) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable_(const std::string &  _965,std::shared_ptr< monty::ndarray< int,1 > > _966,monty::rc_ptr< ::mosek::fusion::LinearDomain > _967) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable_(const std::string &  _1009,std::shared_ptr< monty::ndarray< int,1 > > _1010,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _1011) ;
virtual monty::rc_ptr< ::mosek::fusion::SymmetricVariable > __mosek_2fusion_2Model__variable_(const std::string &  _1033,std::shared_ptr< monty::ndarray< int,1 > > _1034,monty::rc_ptr< ::mosek::fusion::PSDDomain > _1035) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint_(const std::string &  _1050,monty::rc_ptr< ::mosek::fusion::Expression > _1051,monty::rc_ptr< ::mosek::fusion::RangeDomain > _1052) ;
virtual void update_bfix(std::shared_ptr< monty::ndarray< int,1 > > _1090,std::shared_ptr< monty::ndarray< double,1 > > _1091) ;
virtual void putarows(std::shared_ptr< monty::ndarray< int,1 > > _1093,monty::rc_ptr< ::mosek::fusion::WorkStack > _1094,int _1095,int _1096,int _1097,int _1098,int _1099,int _1100,std::shared_ptr< monty::ndarray< int,1 > > _1101) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint_(const std::string &  _1142,monty::rc_ptr< ::mosek::fusion::Expression > _1143,monty::rc_ptr< ::mosek::fusion::PSDDomain > _1144) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint_(const std::string &  _1209,monty::rc_ptr< ::mosek::fusion::Expression > _1210,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > _1211) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint_(const std::string &  _1261,monty::rc_ptr< ::mosek::fusion::Expression > _1262,monty::rc_ptr< ::mosek::fusion::ConeDomain > _1263) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint_(const std::string &  _1310,monty::rc_ptr< ::mosek::fusion::Expression > _1311,monty::rc_ptr< ::mosek::fusion::LinearDomain > _1312) ;
static  std::string getVersion();
virtual bool hasConstraint(const std::string &  _1357) ;
virtual bool hasVariable(const std::string &  _1358) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__getConstraint(int _1359) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__getConstraint(const std::string &  _1360) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__getVariable(int _1361) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__getVariable(const std::string &  _1362) ;
virtual std::string getName() ;
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Model__clone() ;
}; // struct Model;

// mosek.fusion.Debug from file 'src/fusion/cxx/Debug_p.h'
// namespace mosek::fusion
struct p_Debug
{
  Debug * _pubthis;

  p_Debug(Debug * _pubthis) : _pubthis(_pubthis) {}

  static Debug::t o ()                 { return monty::rc_ptr<Debug>(new Debug()); }
  Debug::t p (const std::string & val) { std::cout << val; return Debug::t(_pubthis); }
  Debug::t p (      int val)           { std::cout << val; return Debug::t(_pubthis); }
  Debug::t p (long long val)           { std::cout << val; return Debug::t(_pubthis); }
  Debug::t p (   double val)           { std::cout << val; return Debug::t(_pubthis); }
  Debug::t p (     bool val)           { std::cout << val; return Debug::t(_pubthis); }
  Debug::t lf()                        { std::cout << std::endl; return Debug::t(_pubthis); }

  static p_Debug * _get_impl(Debug * _inst) { return _inst->ptr; }

  template<typename T>
  Debug::t p(const std::shared_ptr<monty::ndarray<T,1>> & val)
  {
    if (val->size() > 0)
    {
      std::cout << (*val)[0];
      for (int i = 1; i < val->size(); ++i)
        std::cout << "," << (*val)[i];
    }
    return Debug::t(_pubthis);
  }

  Debug::t __mosek_2fusion_2Debug__p (const std::string & val) { std::cout << val; return Debug::t(_pubthis); }

  template<class C>
  Debug::t __mosek_2fusion_2Debug__p (C val) { std::cout << val; return Debug::t(_pubthis); }
  Debug::t __mosek_2fusion_2Debug__lf()      { std::cout << std::endl; return Debug::t(_pubthis); }
  
};
// End of file 'src/fusion/cxx/Debug_p.h'
struct p_Sort
{
Sort * _pubthis;
static mosek::fusion::p_Sort* _get_impl(mosek::fusion::Sort * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_Sort * _get_impl(mosek::fusion::Sort::t _inst) { return _get_impl(_inst.get()); }
p_Sort(Sort * _pubthis);
virtual ~p_Sort() { /* std::cout << "~p_Sort" << std::endl;*/ };
virtual void destroy();
static  void argTransposeSort(std::shared_ptr< monty::ndarray< long long,1 > > _151,std::shared_ptr< monty::ndarray< long long,1 > > _152,int _153,int _154,int _155,std::shared_ptr< monty::ndarray< long long,1 > > _156);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _164,std::shared_ptr< monty::ndarray< long long,1 > > _165);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _166,std::shared_ptr< monty::ndarray< int,1 > > _167);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _168,std::shared_ptr< monty::ndarray< long long,1 > > _169,std::shared_ptr< monty::ndarray< long long,1 > > _170);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _171,std::shared_ptr< monty::ndarray< int,1 > > _172,std::shared_ptr< monty::ndarray< int,1 > > _173);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _174,std::shared_ptr< monty::ndarray< long long,1 > > _175,long long _176,long long _177);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _178,std::shared_ptr< monty::ndarray< int,1 > > _179,long long _180,long long _181);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _182,std::shared_ptr< monty::ndarray< long long,1 > > _183,std::shared_ptr< monty::ndarray< long long,1 > > _184,long long _185,long long _186);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _187,std::shared_ptr< monty::ndarray< int,1 > > _188,std::shared_ptr< monty::ndarray< int,1 > > _189,long long _190,long long _191);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _192,std::shared_ptr< monty::ndarray< long long,1 > > _193,long long _194,long long _195,bool _196);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _199,std::shared_ptr< monty::ndarray< int,1 > > _200,long long _201,long long _202,bool _203);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _206,std::shared_ptr< monty::ndarray< long long,1 > > _207,std::shared_ptr< monty::ndarray< long long,1 > > _208,long long _209,long long _210,bool _211);
static  void argsort(std::shared_ptr< monty::ndarray< long long,1 > > _214,std::shared_ptr< monty::ndarray< int,1 > > _215,std::shared_ptr< monty::ndarray< int,1 > > _216,long long _217,long long _218,bool _219);
static  void argbucketsort(std::shared_ptr< monty::ndarray< long long,1 > > _222,std::shared_ptr< monty::ndarray< long long,1 > > _223,long long _224,long long _225,long long _226,long long _227);
static  void argbucketsort(std::shared_ptr< monty::ndarray< long long,1 > > _228,std::shared_ptr< monty::ndarray< int,1 > > _229,long long _230,long long _231,int _232,int _233);
static  void getminmax(std::shared_ptr< monty::ndarray< long long,1 > > _234,std::shared_ptr< monty::ndarray< long long,1 > > _235,std::shared_ptr< monty::ndarray< long long,1 > > _236,long long _237,long long _238,std::shared_ptr< monty::ndarray< long long,1 > > _239);
static  void getminmax(std::shared_ptr< monty::ndarray< long long,1 > > _242,std::shared_ptr< monty::ndarray< int,1 > > _243,std::shared_ptr< monty::ndarray< int,1 > > _244,long long _245,long long _246,std::shared_ptr< monty::ndarray< int,1 > > _247);
static  bool issorted(std::shared_ptr< monty::ndarray< long long,1 > > _250,std::shared_ptr< monty::ndarray< long long,1 > > _251,long long _252,long long _253,bool _254);
static  bool issorted(std::shared_ptr< monty::ndarray< long long,1 > > _256,std::shared_ptr< monty::ndarray< int,1 > > _257,long long _258,long long _259,bool _260);
static  bool issorted(std::shared_ptr< monty::ndarray< long long,1 > > _262,std::shared_ptr< monty::ndarray< long long,1 > > _263,std::shared_ptr< monty::ndarray< long long,1 > > _264,long long _265,long long _266,bool _267);
static  bool issorted(std::shared_ptr< monty::ndarray< long long,1 > > _269,std::shared_ptr< monty::ndarray< int,1 > > _270,std::shared_ptr< monty::ndarray< int,1 > > _271,long long _272,long long _273,bool _274);
}; // struct Sort;

struct p_IndexCounter
{
IndexCounter * _pubthis;
static mosek::fusion::p_IndexCounter* _get_impl(mosek::fusion::IndexCounter * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_IndexCounter * _get_impl(mosek::fusion::IndexCounter::t _inst) { return _get_impl(_inst.get()); }
p_IndexCounter(IndexCounter * _pubthis);
virtual ~p_IndexCounter() { /* std::cout << "~p_IndexCounter" << std::endl;*/ };
long long start{};std::shared_ptr< monty::ndarray< int,1 > > dims{};std::shared_ptr< monty::ndarray< long long,1 > > strides{};std::shared_ptr< monty::ndarray< long long,1 > > st{};std::shared_ptr< monty::ndarray< int,1 > > ii{};int n{};virtual void destroy();
static IndexCounter::t _new_IndexCounter(std::shared_ptr< monty::ndarray< int,1 > > _276);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _276);
static IndexCounter::t _new_IndexCounter(long long _278,std::shared_ptr< monty::ndarray< int,1 > > _279,std::shared_ptr< monty::ndarray< int,1 > > _280);
void _initialize(long long _278,std::shared_ptr< monty::ndarray< int,1 > > _279,std::shared_ptr< monty::ndarray< int,1 > > _280);
static IndexCounter::t _new_IndexCounter(long long _283,std::shared_ptr< monty::ndarray< int,1 > > _284,std::shared_ptr< monty::ndarray< long long,1 > > _285);
void _initialize(long long _283,std::shared_ptr< monty::ndarray< int,1 > > _284,std::shared_ptr< monty::ndarray< long long,1 > > _285);
virtual bool atEnd() ;
virtual std::shared_ptr< monty::ndarray< int,1 > > getIndex() ;
virtual long long next() ;
virtual long long get() ;
virtual void inc() ;
virtual void reset() ;
}; // struct IndexCounter;

struct p_CommonTools
{
CommonTools * _pubthis;
static mosek::fusion::p_CommonTools* _get_impl(mosek::fusion::CommonTools * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_CommonTools * _get_impl(mosek::fusion::CommonTools::t _inst) { return _get_impl(_inst.get()); }
p_CommonTools(CommonTools * _pubthis);
virtual ~p_CommonTools() { /* std::cout << "~p_CommonTools" << std::endl;*/ };
virtual void destroy();
static  std::shared_ptr< monty::ndarray< long long,1 > > resize(std::shared_ptr< monty::ndarray< long long,1 > > _291,int _292);
static  std::shared_ptr< monty::ndarray< int,1 > > resize(std::shared_ptr< monty::ndarray< int,1 > > _294,int _295);
static  std::shared_ptr< monty::ndarray< double,1 > > resize(std::shared_ptr< monty::ndarray< double,1 > > _297,int _298);
static  int binarySearch(std::shared_ptr< monty::ndarray< int,1 > > _300,int _301);
static  int binarySearch(std::shared_ptr< monty::ndarray< long long,1 > > _305,long long _306);
static  int binarySearchR(std::shared_ptr< monty::ndarray< long long,1 > > _308,long long _309);
static  int binarySearchL(std::shared_ptr< monty::ndarray< long long,1 > > _313,long long _314);
static  void ndIncr(std::shared_ptr< monty::ndarray< int,1 > > _318,std::shared_ptr< monty::ndarray< int,1 > > _319,std::shared_ptr< monty::ndarray< int,1 > > _320);
static  void transposeTriplets(std::shared_ptr< monty::ndarray< int,1 > > _322,std::shared_ptr< monty::ndarray< int,1 > > _323,std::shared_ptr< monty::ndarray< double,1 > > _324,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< long long,1 > >,1 > > _325,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< long long,1 > >,1 > > _326,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< double,1 > >,1 > > _327,long long _328,int _329,int _330);
static  void transposeTriplets(std::shared_ptr< monty::ndarray< int,1 > > _343,std::shared_ptr< monty::ndarray< int,1 > > _344,std::shared_ptr< monty::ndarray< double,1 > > _345,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< int,1 > >,1 > > _346,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< int,1 > >,1 > > _347,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< double,1 > >,1 > > _348,long long _349,int _350,int _351);
static  void tripletSort(std::shared_ptr< monty::ndarray< int,1 > > _364,std::shared_ptr< monty::ndarray< int,1 > > _365,std::shared_ptr< monty::ndarray< double,1 > > _366,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< int,1 > >,1 > > _367,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< int,1 > >,1 > > _368,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< double,1 > >,1 > > _369,long long _370,int _371,int _372);
static  void argMSort(std::shared_ptr< monty::ndarray< int,1 > > _398,std::shared_ptr< monty::ndarray< int,1 > > _399);
static  void mergeInto(std::shared_ptr< monty::ndarray< int,1 > > _404,std::shared_ptr< monty::ndarray< int,1 > > _405,std::shared_ptr< monty::ndarray< int,1 > > _406,int _407,int _408,int _409);
static  void argQsort(std::shared_ptr< monty::ndarray< long long,1 > > _415,std::shared_ptr< monty::ndarray< long long,1 > > _416,std::shared_ptr< monty::ndarray< long long,1 > > _417,long long _418,long long _419);
static  void argQsort(std::shared_ptr< monty::ndarray< long long,1 > > _420,std::shared_ptr< monty::ndarray< int,1 > > _421,std::shared_ptr< monty::ndarray< int,1 > > _422,long long _423,long long _424);
}; // struct CommonTools;

struct p_SolutionStruct
{
SolutionStruct * _pubthis;
static mosek::fusion::p_SolutionStruct* _get_impl(mosek::fusion::SolutionStruct * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_SolutionStruct * _get_impl(mosek::fusion::SolutionStruct::t _inst) { return _get_impl(_inst.get()); }
p_SolutionStruct(SolutionStruct * _pubthis);
virtual ~p_SolutionStruct() { /* std::cout << "~p_SolutionStruct" << std::endl;*/ };
std::shared_ptr< monty::ndarray< double,1 > > yx{};std::shared_ptr< monty::ndarray< double,1 > > snx{};std::shared_ptr< monty::ndarray< double,1 > > sux{};std::shared_ptr< monty::ndarray< double,1 > > slx{};std::shared_ptr< monty::ndarray< double,1 > > bars{};std::shared_ptr< monty::ndarray< double,1 > > barx{};std::shared_ptr< monty::ndarray< double,1 > > y{};std::shared_ptr< monty::ndarray< double,1 > > suc{};std::shared_ptr< monty::ndarray< double,1 > > slc{};std::shared_ptr< monty::ndarray< double,1 > > xx{};std::shared_ptr< monty::ndarray< double,1 > > xc{};double dobj{};double pobj{};mosek::fusion::ProblemStatus probstatus{};mosek::fusion::SolutionStatus dstatus{};mosek::fusion::SolutionStatus pstatus{};int sol_numbarvar{};int sol_numcone{};int sol_numvar{};int sol_numcon{};virtual void destroy();
static SolutionStruct::t _new_SolutionStruct(int _425,int _426,int _427,int _428);
void _initialize(int _425,int _426,int _427,int _428);
static SolutionStruct::t _new_SolutionStruct(monty::rc_ptr< ::mosek::fusion::SolutionStruct > _429);
void _initialize(monty::rc_ptr< ::mosek::fusion::SolutionStruct > _429);
virtual monty::rc_ptr< ::mosek::fusion::SolutionStruct > __mosek_2fusion_2SolutionStruct__clone() ;
virtual void resize(int _430,int _431,int _432,int _433) ;
virtual bool isDualAcceptable(mosek::fusion::AccSolutionStatus _453) ;
virtual bool isPrimalAcceptable(mosek::fusion::AccSolutionStatus _454) ;
virtual bool isAcceptable(mosek::fusion::SolutionStatus _455,mosek::fusion::AccSolutionStatus _456) ;
}; // struct SolutionStruct;

struct p_ConNZStruct
{
ConNZStruct * _pubthis;
static mosek::fusion::p_ConNZStruct* _get_impl(mosek::fusion::ConNZStruct * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_ConNZStruct * _get_impl(mosek::fusion::ConNZStruct::t _inst) { return _get_impl(_inst.get()); }
p_ConNZStruct(ConNZStruct * _pubthis);
virtual ~p_ConNZStruct() { /* std::cout << "~p_ConNZStruct" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > barmidx{};std::shared_ptr< monty::ndarray< int,1 > > barsubj{};std::shared_ptr< monty::ndarray< int,1 > > barsubi{};std::shared_ptr< monty::ndarray< double,1 > > bfix{};std::shared_ptr< monty::ndarray< double,1 > > cof{};std::shared_ptr< monty::ndarray< int,1 > > subj{};std::shared_ptr< monty::ndarray< long long,1 > > ptrb{};virtual void destroy();
static ConNZStruct::t _new_ConNZStruct(std::shared_ptr< monty::ndarray< long long,1 > > _457,std::shared_ptr< monty::ndarray< int,1 > > _458,std::shared_ptr< monty::ndarray< double,1 > > _459,std::shared_ptr< monty::ndarray< double,1 > > _460,std::shared_ptr< monty::ndarray< int,1 > > _461,std::shared_ptr< monty::ndarray< int,1 > > _462,std::shared_ptr< monty::ndarray< int,1 > > _463);
void _initialize(std::shared_ptr< monty::ndarray< long long,1 > > _457,std::shared_ptr< monty::ndarray< int,1 > > _458,std::shared_ptr< monty::ndarray< double,1 > > _459,std::shared_ptr< monty::ndarray< double,1 > > _460,std::shared_ptr< monty::ndarray< int,1 > > _461,std::shared_ptr< monty::ndarray< int,1 > > _462,std::shared_ptr< monty::ndarray< int,1 > > _463);
}; // struct ConNZStruct;

struct p_BaseVariable : public /*implements*/ virtual ::mosek::fusion::Variable
{
BaseVariable * _pubthis;
static mosek::fusion::p_BaseVariable* _get_impl(mosek::fusion::BaseVariable * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_BaseVariable * _get_impl(mosek::fusion::BaseVariable::t _inst) { return _get_impl(_inst.get()); }
p_BaseVariable(BaseVariable * _pubthis);
virtual ~p_BaseVariable() { /* std::cout << "~p_BaseVariable" << std::endl;*/ };
std::shared_ptr< monty::ndarray< long long,1 > > sparsity{};std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs{};monty::rc_ptr< ::mosek::fusion::Model > model{};std::shared_ptr< monty::ndarray< int,1 > > shape{};virtual void destroy();
static BaseVariable::t _new_BaseVariable(monty::rc_ptr< ::mosek::fusion::BaseVariable > _1604,monty::rc_ptr< ::mosek::fusion::Model > _1605);
void _initialize(monty::rc_ptr< ::mosek::fusion::BaseVariable > _1604,monty::rc_ptr< ::mosek::fusion::Model > _1605);
static BaseVariable::t _new_BaseVariable(monty::rc_ptr< ::mosek::fusion::Model > _1606,std::shared_ptr< monty::ndarray< int,1 > > _1607,std::shared_ptr< monty::ndarray< long long,1 > > _1608,std::shared_ptr< monty::ndarray< long long,1 > > _1609);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _1606,std::shared_ptr< monty::ndarray< int,1 > > _1607,std::shared_ptr< monty::ndarray< long long,1 > > _1608,std::shared_ptr< monty::ndarray< long long,1 > > _1609);
virtual /* override */ std::string toString() ;
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2BaseVariable__eval() ;
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expression__eval() { return __mosek_2fusion_2BaseVariable__eval(); }
virtual void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _1612,monty::rc_ptr< ::mosek::fusion::WorkStack > _1613,monty::rc_ptr< ::mosek::fusion::WorkStack > _1614) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__reshape(int _1634,int _1635,int _1636) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int _1634,int _1635,int _1636) { return __mosek_2fusion_2BaseVariable__reshape(_1634,_1635,_1636); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__reshape(int _1637,int _1638) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int _1637,int _1638) { return __mosek_2fusion_2BaseVariable__reshape(_1637,_1638); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__reshape(int _1639) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int _1639) { return __mosek_2fusion_2BaseVariable__reshape(_1639); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__reshape(std::shared_ptr< monty::ndarray< int,1 > > _1640) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(std::shared_ptr< monty::ndarray< int,1 > > _1640) { return __mosek_2fusion_2BaseVariable__reshape(_1640); }
virtual void setLevel(std::shared_ptr< monty::ndarray< double,1 > > _1644) ;
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2BaseVariable__getModel() ;
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Variable__getModel() { return __mosek_2fusion_2BaseVariable__getModel(); }
virtual int getND() ;
virtual int getDim(int _1647) ;
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() ;
virtual long long getSize() ;
virtual std::shared_ptr< monty::ndarray< double,1 > > dual() ;
virtual std::shared_ptr< monty::ndarray< double,1 > > level() ;
virtual void makeContinuous() ;
virtual void makeInteger() ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__transpose() ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__transpose() { return __mosek_2fusion_2BaseVariable__transpose(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(int _1668,int _1669,int _1670) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int _1668,int _1669,int _1670) { return __mosek_2fusion_2BaseVariable__index(_1668,_1669,_1670); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(int _1671,int _1672) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int _1671,int _1672) { return __mosek_2fusion_2BaseVariable__index(_1671,_1672); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(std::shared_ptr< monty::ndarray< int,1 > > _1673) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(std::shared_ptr< monty::ndarray< int,1 > > _1673) { return __mosek_2fusion_2BaseVariable__index(_1673); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(int _1676) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int _1676) { return __mosek_2fusion_2BaseVariable__index(_1676); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__pick(std::shared_ptr< monty::ndarray< int,1 > > _1677,std::shared_ptr< monty::ndarray< int,1 > > _1678,std::shared_ptr< monty::ndarray< int,1 > > _1679) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > _1677,std::shared_ptr< monty::ndarray< int,1 > > _1678,std::shared_ptr< monty::ndarray< int,1 > > _1679) { return __mosek_2fusion_2BaseVariable__pick(_1677,_1678,_1679); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__pick(std::shared_ptr< monty::ndarray< int,1 > > _1682,std::shared_ptr< monty::ndarray< int,1 > > _1683) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > _1682,std::shared_ptr< monty::ndarray< int,1 > > _1683) { return __mosek_2fusion_2BaseVariable__pick(_1682,_1683); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__pick(std::shared_ptr< monty::ndarray< int,2 > > _1686) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,2 > > _1686) { return __mosek_2fusion_2BaseVariable__pick(_1686); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__pick(std::shared_ptr< monty::ndarray< int,1 > > _1708) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > _1708) { return __mosek_2fusion_2BaseVariable__pick(_1708); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__antidiag(int _1719) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag(int _1719) { return __mosek_2fusion_2BaseVariable__antidiag(_1719); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__antidiag() ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag() { return __mosek_2fusion_2BaseVariable__antidiag(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__diag(int _1720) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag(int _1720) { return __mosek_2fusion_2BaseVariable__diag(_1720); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__diag() ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag() { return __mosek_2fusion_2BaseVariable__diag(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__general_diag(std::shared_ptr< monty::ndarray< int,1 > > _1721,std::shared_ptr< monty::ndarray< int,1 > > _1722,int _1723) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__slice(std::shared_ptr< monty::ndarray< int,1 > > _1744,std::shared_ptr< monty::ndarray< int,1 > > _1745) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(std::shared_ptr< monty::ndarray< int,1 > > _1744,std::shared_ptr< monty::ndarray< int,1 > > _1745) { return __mosek_2fusion_2BaseVariable__slice(_1744,_1745); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__slice(int _1779,int _1780) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(int _1779,int _1780) { return __mosek_2fusion_2BaseVariable__slice(_1779,_1780); }
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseVariable__asExpr() ;
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Variable__asExpr() { return __mosek_2fusion_2BaseVariable__asExpr(); }
virtual int inst(int _1789,std::shared_ptr< monty::ndarray< long long,1 > > _1790,int _1791,std::shared_ptr< monty::ndarray< long long,1 > > _1792) ;
virtual int numInst() ;
virtual void inst(int _1797,std::shared_ptr< monty::ndarray< long long,1 > > _1798) ;
virtual void set_values(std::shared_ptr< monty::ndarray< double,1 > > _1805,bool _1806) ;
virtual void dual_lu(int _1811,std::shared_ptr< monty::ndarray< double,1 > > _1812,bool _1813) ;
virtual void values(int _1816,std::shared_ptr< monty::ndarray< double,1 > > _1817,bool _1818) ;
virtual void make_continuous() ;
virtual void make_integer() ;
}; // struct BaseVariable;

struct p_SliceVariable : public ::mosek::fusion::p_BaseVariable
{
SliceVariable * _pubthis;
static mosek::fusion::p_SliceVariable* _get_impl(mosek::fusion::SliceVariable * _inst){ return static_cast< mosek::fusion::p_SliceVariable* >(mosek::fusion::p_BaseVariable::_get_impl(_inst)); }
static mosek::fusion::p_SliceVariable * _get_impl(mosek::fusion::SliceVariable::t _inst) { return _get_impl(_inst.get()); }
p_SliceVariable(SliceVariable * _pubthis);
virtual ~p_SliceVariable() { /* std::cout << "~p_SliceVariable" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > shape{};std::shared_ptr< monty::ndarray< long long,1 > > sparsity{};std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs{};virtual void destroy();
static SliceVariable::t _new_SliceVariable(monty::rc_ptr< ::mosek::fusion::Model > _1364,std::shared_ptr< monty::ndarray< int,1 > > _1365,std::shared_ptr< monty::ndarray< long long,1 > > _1366,std::shared_ptr< monty::ndarray< long long,1 > > _1367);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _1364,std::shared_ptr< monty::ndarray< int,1 > > _1365,std::shared_ptr< monty::ndarray< long long,1 > > _1366,std::shared_ptr< monty::ndarray< long long,1 > > _1367);
}; // struct SliceVariable;

struct p_ModelVariable : public ::mosek::fusion::p_BaseVariable
{
ModelVariable * _pubthis;
static mosek::fusion::p_ModelVariable* _get_impl(mosek::fusion::ModelVariable * _inst){ return static_cast< mosek::fusion::p_ModelVariable* >(mosek::fusion::p_BaseVariable::_get_impl(_inst)); }
static mosek::fusion::p_ModelVariable * _get_impl(mosek::fusion::ModelVariable::t _inst) { return _get_impl(_inst.get()); }
p_ModelVariable(ModelVariable * _pubthis);
virtual ~p_ModelVariable() { /* std::cout << "~p_ModelVariable" << std::endl;*/ };
std::shared_ptr< monty::ndarray< long long,1 > > sparsity{};std::shared_ptr< monty::ndarray< int,1 > > shape{};std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs{};long long varid{};std::string name{};virtual void destroy();
static ModelVariable::t _new_ModelVariable(monty::rc_ptr< ::mosek::fusion::ModelVariable > _1567,monty::rc_ptr< ::mosek::fusion::Model > _1568);
void _initialize(monty::rc_ptr< ::mosek::fusion::ModelVariable > _1567,monty::rc_ptr< ::mosek::fusion::Model > _1568);
static ModelVariable::t _new_ModelVariable(monty::rc_ptr< ::mosek::fusion::Model > _1569,const std::string &  _1570,std::shared_ptr< monty::ndarray< int,1 > > _1571,long long _1572,std::shared_ptr< monty::ndarray< long long,1 > > _1573,std::shared_ptr< monty::ndarray< long long,1 > > _1574);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _1569,const std::string &  _1570,std::shared_ptr< monty::ndarray< int,1 > > _1571,long long _1572,std::shared_ptr< monty::ndarray< long long,1 > > _1573,std::shared_ptr< monty::ndarray< long long,1 > > _1574);
virtual void flushNames() { throw monty::AbstractClassError("Call to abstract method"); }
virtual void elementName(long long _1575,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > _1576) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2ModelVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1577) { throw monty::AbstractClassError("Call to abstract method"); }
}; // struct ModelVariable;

struct p_SymRangedVariable : public ::mosek::fusion::p_ModelVariable, public /*implements*/ virtual ::mosek::fusion::SymmetricVariable
{
SymRangedVariable * _pubthis;
static mosek::fusion::p_SymRangedVariable* _get_impl(mosek::fusion::SymRangedVariable * _inst){ return static_cast< mosek::fusion::p_SymRangedVariable* >(mosek::fusion::p_ModelVariable::_get_impl(_inst)); }
static mosek::fusion::p_SymRangedVariable * _get_impl(mosek::fusion::SymRangedVariable::t _inst) { return _get_impl(_inst.get()); }
p_SymRangedVariable(SymRangedVariable * _pubthis);
virtual ~p_SymRangedVariable() { /* std::cout << "~p_SymRangedVariable" << std::endl;*/ };
int dim{};std::shared_ptr< monty::ndarray< long long,1 > > sparsity{};std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};bool names_flushed{};std::string name{};virtual void destroy();
static SymRangedVariable::t _new_SymRangedVariable(monty::rc_ptr< ::mosek::fusion::SymRangedVariable > _1368,monty::rc_ptr< ::mosek::fusion::Model > _1369);
void _initialize(monty::rc_ptr< ::mosek::fusion::SymRangedVariable > _1368,monty::rc_ptr< ::mosek::fusion::Model > _1369);
static SymRangedVariable::t _new_SymRangedVariable(monty::rc_ptr< ::mosek::fusion::Model > _1370,const std::string &  _1371,long long _1372,int _1373,std::shared_ptr< monty::ndarray< long long,1 > > _1374,std::shared_ptr< monty::ndarray< int,1 > > _1375);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _1370,const std::string &  _1371,long long _1372,int _1373,std::shared_ptr< monty::ndarray< long long,1 > > _1374,std::shared_ptr< monty::ndarray< int,1 > > _1375);
virtual void dual_u(int _1376,std::shared_ptr< monty::ndarray< double,1 > > _1377) ;
virtual void dual_l(int _1378,std::shared_ptr< monty::ndarray< double,1 > > _1379) ;
virtual /* override */ void flushNames() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2SymRangedVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1383) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2ModelVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1383) { return __mosek_2fusion_2SymRangedVariable__clone(_1383); }
static  std::shared_ptr< monty::ndarray< long long,1 > > mirror_idxs(int _1384,std::shared_ptr< monty::ndarray< long long,1 > > _1385,std::shared_ptr< monty::ndarray< int,1 > > _1386);
static  std::shared_ptr< monty::ndarray< long long,1 > > mirror_sp(int _1402,std::shared_ptr< monty::ndarray< long long,1 > > _1403);
}; // struct SymRangedVariable;

struct p_RangedVariable : public ::mosek::fusion::p_ModelVariable
{
RangedVariable * _pubthis;
static mosek::fusion::p_RangedVariable* _get_impl(mosek::fusion::RangedVariable * _inst){ return static_cast< mosek::fusion::p_RangedVariable* >(mosek::fusion::p_ModelVariable::_get_impl(_inst)); }
static mosek::fusion::p_RangedVariable * _get_impl(mosek::fusion::RangedVariable::t _inst) { return _get_impl(_inst.get()); }
p_RangedVariable(RangedVariable * _pubthis);
virtual ~p_RangedVariable() { /* std::cout << "~p_RangedVariable" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > shape{};std::string name{};bool names_flushed{};std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};std::shared_ptr< monty::ndarray< long long,1 > > sparsity{};virtual void destroy();
static RangedVariable::t _new_RangedVariable(monty::rc_ptr< ::mosek::fusion::RangedVariable > _1414,monty::rc_ptr< ::mosek::fusion::Model > _1415);
void _initialize(monty::rc_ptr< ::mosek::fusion::RangedVariable > _1414,monty::rc_ptr< ::mosek::fusion::Model > _1415);
static RangedVariable::t _new_RangedVariable(monty::rc_ptr< ::mosek::fusion::Model > _1416,const std::string &  _1417,long long _1418,std::shared_ptr< monty::ndarray< int,1 > > _1419,std::shared_ptr< monty::ndarray< long long,1 > > _1420,std::shared_ptr< monty::ndarray< int,1 > > _1421);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _1416,const std::string &  _1417,long long _1418,std::shared_ptr< monty::ndarray< int,1 > > _1419,std::shared_ptr< monty::ndarray< long long,1 > > _1420,std::shared_ptr< monty::ndarray< int,1 > > _1421);
virtual monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > __mosek_2fusion_2RangedVariable__elementDesc(long long _1422,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > _1423) ;
virtual /* override */ void flushNames() ;
virtual void dual_u(int _1427,std::shared_ptr< monty::ndarray< double,1 > > _1428) ;
virtual void dual_l(int _1429,std::shared_ptr< monty::ndarray< double,1 > > _1430) ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2RangedVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1431) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2ModelVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1431) { return __mosek_2fusion_2RangedVariable__clone(_1431); }
static  std::shared_ptr< monty::ndarray< long long,1 > > globalNativeIndexes(std::shared_ptr< monty::ndarray< int,1 > > _1432);
}; // struct RangedVariable;

struct p_LinearPSDVariable : public ::mosek::fusion::p_ModelVariable
{
LinearPSDVariable * _pubthis;
static mosek::fusion::p_LinearPSDVariable* _get_impl(mosek::fusion::LinearPSDVariable * _inst){ return static_cast< mosek::fusion::p_LinearPSDVariable* >(mosek::fusion::p_ModelVariable::_get_impl(_inst)); }
static mosek::fusion::p_LinearPSDVariable * _get_impl(mosek::fusion::LinearPSDVariable::t _inst) { return _get_impl(_inst.get()); }
p_LinearPSDVariable(LinearPSDVariable * _pubthis);
virtual ~p_LinearPSDVariable() { /* std::cout << "~p_LinearPSDVariable" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > shape{};std::string name{};int varid{};std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs{};int conedim{};virtual void destroy();
static LinearPSDVariable::t _new_LinearPSDVariable(monty::rc_ptr< ::mosek::fusion::LinearPSDVariable > _1435,monty::rc_ptr< ::mosek::fusion::Model > _1436);
void _initialize(monty::rc_ptr< ::mosek::fusion::LinearPSDVariable > _1435,monty::rc_ptr< ::mosek::fusion::Model > _1436);
static LinearPSDVariable::t _new_LinearPSDVariable(monty::rc_ptr< ::mosek::fusion::Model > _1437,const std::string &  _1438,int _1439,std::shared_ptr< monty::ndarray< int,1 > > _1440,int _1441,std::shared_ptr< monty::ndarray< long long,1 > > _1442);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _1437,const std::string &  _1438,int _1439,std::shared_ptr< monty::ndarray< int,1 > > _1440,int _1441,std::shared_ptr< monty::ndarray< long long,1 > > _1442);
virtual /* override */ void flushNames() ;
virtual /* override */ std::string toString() ;
virtual void make_continuous(std::shared_ptr< monty::ndarray< long long,1 > > _1445) ;
virtual void make_integer(std::shared_ptr< monty::ndarray< long long,1 > > _1446) ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2LinearPSDVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1447) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2ModelVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1447) { return __mosek_2fusion_2LinearPSDVariable__clone(_1447); }
static  std::shared_ptr< monty::ndarray< long long,1 > > globalNativeIndexes(std::shared_ptr< monty::ndarray< long long,1 > > _1448);
}; // struct LinearPSDVariable;

struct p_PSDVariable : public ::mosek::fusion::p_ModelVariable, public /*implements*/ virtual ::mosek::fusion::SymmetricVariable
{
PSDVariable * _pubthis;
static mosek::fusion::p_PSDVariable* _get_impl(mosek::fusion::PSDVariable * _inst){ return static_cast< mosek::fusion::p_PSDVariable* >(mosek::fusion::p_ModelVariable::_get_impl(_inst)); }
static mosek::fusion::p_PSDVariable * _get_impl(mosek::fusion::PSDVariable::t _inst) { return _get_impl(_inst.get()); }
p_PSDVariable(PSDVariable * _pubthis);
virtual ~p_PSDVariable() { /* std::cout << "~p_PSDVariable" << std::endl;*/ };
int conedim2{};int conedim1{};std::shared_ptr< monty::ndarray< int,1 > > shape{};std::string name{};std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs{};int varid{};virtual void destroy();
static PSDVariable::t _new_PSDVariable(monty::rc_ptr< ::mosek::fusion::PSDVariable > _1450,monty::rc_ptr< ::mosek::fusion::Model > _1451);
void _initialize(monty::rc_ptr< ::mosek::fusion::PSDVariable > _1450,monty::rc_ptr< ::mosek::fusion::Model > _1451);
static PSDVariable::t _new_PSDVariable(monty::rc_ptr< ::mosek::fusion::Model > _1452,const std::string &  _1453,int _1454,std::shared_ptr< monty::ndarray< int,1 > > _1455,int _1456,int _1457,std::shared_ptr< monty::ndarray< long long,1 > > _1458);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _1452,const std::string &  _1453,int _1454,std::shared_ptr< monty::ndarray< int,1 > > _1455,int _1456,int _1457,std::shared_ptr< monty::ndarray< long long,1 > > _1458);
virtual /* override */ void flushNames() ;
virtual /* override */ std::string toString() ;
virtual monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > __mosek_2fusion_2PSDVariable__elementDesc(long long _1461,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > _1462) ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2PSDVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1463) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2ModelVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1463) { return __mosek_2fusion_2PSDVariable__clone(_1463); }
static  std::shared_ptr< monty::ndarray< long long,1 > > fullnativeidxs(std::shared_ptr< monty::ndarray< int,1 > > _1464,int _1465,int _1466,std::shared_ptr< monty::ndarray< long long,1 > > _1467);
}; // struct PSDVariable;

struct p_SymLinearVariable : public ::mosek::fusion::p_ModelVariable, public /*implements*/ virtual ::mosek::fusion::SymmetricVariable
{
SymLinearVariable * _pubthis;
static mosek::fusion::p_SymLinearVariable* _get_impl(mosek::fusion::SymLinearVariable * _inst){ return static_cast< mosek::fusion::p_SymLinearVariable* >(mosek::fusion::p_ModelVariable::_get_impl(_inst)); }
static mosek::fusion::p_SymLinearVariable * _get_impl(mosek::fusion::SymLinearVariable::t _inst) { return _get_impl(_inst.get()); }
p_SymLinearVariable(SymLinearVariable * _pubthis);
virtual ~p_SymLinearVariable() { /* std::cout << "~p_SymLinearVariable" << std::endl;*/ };
int dim{};std::shared_ptr< monty::ndarray< long long,1 > > sparsity{};std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};bool names_flushed{};std::string name{};virtual void destroy();
static SymLinearVariable::t _new_SymLinearVariable(monty::rc_ptr< ::mosek::fusion::SymLinearVariable > _1492,monty::rc_ptr< ::mosek::fusion::Model > _1493);
void _initialize(monty::rc_ptr< ::mosek::fusion::SymLinearVariable > _1492,monty::rc_ptr< ::mosek::fusion::Model > _1493);
static SymLinearVariable::t _new_SymLinearVariable(monty::rc_ptr< ::mosek::fusion::Model > _1494,const std::string &  _1495,long long _1496,int _1497,std::shared_ptr< monty::ndarray< long long,1 > > _1498,std::shared_ptr< monty::ndarray< int,1 > > _1499);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _1494,const std::string &  _1495,long long _1496,int _1497,std::shared_ptr< monty::ndarray< long long,1 > > _1498,std::shared_ptr< monty::ndarray< int,1 > > _1499);
virtual /* override */ void flushNames() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2SymLinearVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1503) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2ModelVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1503) { return __mosek_2fusion_2SymLinearVariable__clone(_1503); }
static  std::shared_ptr< monty::ndarray< long long,1 > > mirror_idxs(int _1504,std::shared_ptr< monty::ndarray< long long,1 > > _1505,std::shared_ptr< monty::ndarray< int,1 > > _1506);
static  std::shared_ptr< monty::ndarray< long long,1 > > mirror_sp(int _1522,std::shared_ptr< monty::ndarray< long long,1 > > _1523);
}; // struct SymLinearVariable;

struct p_LinearVariable : public ::mosek::fusion::p_ModelVariable
{
LinearVariable * _pubthis;
static mosek::fusion::p_LinearVariable* _get_impl(mosek::fusion::LinearVariable * _inst){ return static_cast< mosek::fusion::p_LinearVariable* >(mosek::fusion::p_ModelVariable::_get_impl(_inst)); }
static mosek::fusion::p_LinearVariable * _get_impl(mosek::fusion::LinearVariable::t _inst) { return _get_impl(_inst.get()); }
p_LinearVariable(LinearVariable * _pubthis);
virtual ~p_LinearVariable() { /* std::cout << "~p_LinearVariable" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > shape{};std::shared_ptr< monty::ndarray< long long,1 > > sparsity{};std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};bool names_flushed{};std::string name{};virtual void destroy();
static LinearVariable::t _new_LinearVariable(monty::rc_ptr< ::mosek::fusion::LinearVariable > _1534,monty::rc_ptr< ::mosek::fusion::Model > _1535);
void _initialize(monty::rc_ptr< ::mosek::fusion::LinearVariable > _1534,monty::rc_ptr< ::mosek::fusion::Model > _1535);
static LinearVariable::t _new_LinearVariable(monty::rc_ptr< ::mosek::fusion::Model > _1536,const std::string &  _1537,long long _1538,std::shared_ptr< monty::ndarray< int,1 > > _1539,std::shared_ptr< monty::ndarray< long long,1 > > _1540,std::shared_ptr< monty::ndarray< int,1 > > _1541);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _1536,const std::string &  _1537,long long _1538,std::shared_ptr< monty::ndarray< int,1 > > _1539,std::shared_ptr< monty::ndarray< long long,1 > > _1540,std::shared_ptr< monty::ndarray< int,1 > > _1541);
virtual /* override */ std::string toString() ;
virtual /* override */ void flushNames() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2LinearVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1547) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2ModelVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1547) { return __mosek_2fusion_2LinearVariable__clone(_1547); }
static  std::shared_ptr< monty::ndarray< long long,1 > > globalNativeIndexes(std::shared_ptr< monty::ndarray< int,1 > > _1548);
}; // struct LinearVariable;

struct p_ConicVariable : public ::mosek::fusion::p_ModelVariable
{
ConicVariable * _pubthis;
static mosek::fusion::p_ConicVariable* _get_impl(mosek::fusion::ConicVariable * _inst){ return static_cast< mosek::fusion::p_ConicVariable* >(mosek::fusion::p_ModelVariable::_get_impl(_inst)); }
static mosek::fusion::p_ConicVariable * _get_impl(mosek::fusion::ConicVariable::t _inst) { return _get_impl(_inst.get()); }
p_ConicVariable(ConicVariable * _pubthis);
virtual ~p_ConicVariable() { /* std::cout << "~p_ConicVariable" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};std::shared_ptr< monty::ndarray< int,1 > > shape{};std::string name{};bool names_flushed{};int varid{};virtual void destroy();
static ConicVariable::t _new_ConicVariable(monty::rc_ptr< ::mosek::fusion::ConicVariable > _1551,monty::rc_ptr< ::mosek::fusion::Model > _1552);
void _initialize(monty::rc_ptr< ::mosek::fusion::ConicVariable > _1551,monty::rc_ptr< ::mosek::fusion::Model > _1552);
static ConicVariable::t _new_ConicVariable(monty::rc_ptr< ::mosek::fusion::Model > _1553,const std::string &  _1554,int _1555,std::shared_ptr< monty::ndarray< int,1 > > _1556,std::shared_ptr< monty::ndarray< int,1 > > _1557);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _1553,const std::string &  _1554,int _1555,std::shared_ptr< monty::ndarray< int,1 > > _1556,std::shared_ptr< monty::ndarray< int,1 > > _1557);
virtual /* override */ std::string toString() ;
virtual /* override */ void flushNames() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2ConicVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1563) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelVariable > __mosek_2fusion_2ModelVariable__clone(monty::rc_ptr< ::mosek::fusion::Model > _1563) { return __mosek_2fusion_2ConicVariable__clone(_1563); }
static  std::shared_ptr< monty::ndarray< long long,1 > > globalNativeIndexes(std::shared_ptr< monty::ndarray< int,1 > > _1564);
}; // struct ConicVariable;

struct p_NilVariable : public ::mosek::fusion::p_BaseVariable, public /*implements*/ virtual ::mosek::fusion::SymmetricVariable
{
NilVariable * _pubthis;
static mosek::fusion::p_NilVariable* _get_impl(mosek::fusion::NilVariable * _inst){ return static_cast< mosek::fusion::p_NilVariable* >(mosek::fusion::p_BaseVariable::_get_impl(_inst)); }
static mosek::fusion::p_NilVariable * _get_impl(mosek::fusion::NilVariable::t _inst) { return _get_impl(_inst.get()); }
p_NilVariable(NilVariable * _pubthis);
virtual ~p_NilVariable() { /* std::cout << "~p_NilVariable" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > shape{};virtual void destroy();
static NilVariable::t _new_NilVariable(std::shared_ptr< monty::ndarray< int,1 > > _1578);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _1578);
static NilVariable::t _new_NilVariable();
void _initialize();
virtual void flushNames() ;
virtual monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > __mosek_2fusion_2NilVariable__elementDesc(long long _1580,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > _1581) ;
virtual void elementName(long long _1582,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > _1583) ;
virtual /* override */ int numInst() ;
virtual int inst(int _1584,std::shared_ptr< monty::ndarray< long long,1 > > _1585,std::shared_ptr< monty::ndarray< long long,1 > > _1586) ;
virtual /* override */ void inst(int _1587,std::shared_ptr< monty::ndarray< long long,1 > > _1588) ;
virtual /* override */ void set_values(std::shared_ptr< monty::ndarray< double,1 > > _1589,bool _1590) ;
virtual /* override */ void values(int _1591,std::shared_ptr< monty::ndarray< double,1 > > _1592,bool _1593) ;
virtual /* override */ void make_continuous() ;
virtual /* override */ void make_integer() ;
virtual /* override */ std::string toString() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2NilVariable__index(std::shared_ptr< monty::ndarray< int,1 > > _1594) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(std::shared_ptr< monty::ndarray< int,1 > > _1594) { return __mosek_2fusion_2NilVariable__index(_1594); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(std::shared_ptr< monty::ndarray< int,1 > > _1594) { return __mosek_2fusion_2NilVariable__index(_1594); }
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2NilVariable__index(int _1596) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(int _1596) { return __mosek_2fusion_2NilVariable__index(_1596); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int _1596) { return __mosek_2fusion_2NilVariable__index(_1596); }
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2NilVariable__slice(std::shared_ptr< monty::ndarray< int,1 > > _1598,std::shared_ptr< monty::ndarray< int,1 > > _1599) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__slice(std::shared_ptr< monty::ndarray< int,1 > > _1598,std::shared_ptr< monty::ndarray< int,1 > > _1599) { return __mosek_2fusion_2NilVariable__slice(_1598,_1599); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(std::shared_ptr< monty::ndarray< int,1 > > _1598,std::shared_ptr< monty::ndarray< int,1 > > _1599) { return __mosek_2fusion_2NilVariable__slice(_1598,_1599); }
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2NilVariable__slice(int _1602,int _1603) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__slice(int _1602,int _1603) { return __mosek_2fusion_2NilVariable__slice(_1602,_1603); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(int _1602,int _1603) { return __mosek_2fusion_2NilVariable__slice(_1602,_1603); }
}; // struct NilVariable;

struct p_Var
{
Var * _pubthis;
static mosek::fusion::p_Var* _get_impl(mosek::fusion::Var * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_Var * _get_impl(mosek::fusion::Var::t _inst) { return _get_impl(_inst.get()); }
p_Var(Var * _pubthis);
virtual ~p_Var() { /* std::cout << "~p_Var" << std::endl;*/ };
virtual void destroy();
static  monty::rc_ptr< ::mosek::fusion::Variable > empty(std::shared_ptr< monty::ndarray< int,1 > > _1860);
static  monty::rc_ptr< ::mosek::fusion::Variable > compress(monty::rc_ptr< ::mosek::fusion::Variable > _1862);
static  monty::rc_ptr< ::mosek::fusion::Variable > reshape(monty::rc_ptr< ::mosek::fusion::Variable > _1870,int _1871);
static  monty::rc_ptr< ::mosek::fusion::Variable > reshape(monty::rc_ptr< ::mosek::fusion::Variable > _1872,int _1873,int _1874);
static  monty::rc_ptr< ::mosek::fusion::Variable > flatten(monty::rc_ptr< ::mosek::fusion::Variable > _1875);
static  monty::rc_ptr< ::mosek::fusion::Variable > reshape(monty::rc_ptr< ::mosek::fusion::Variable > _1876,std::shared_ptr< monty::ndarray< int,1 > > _1877);
static  monty::rc_ptr< ::mosek::fusion::Variable > index_permute_(monty::rc_ptr< ::mosek::fusion::Variable > _1878,std::shared_ptr< monty::ndarray< int,1 > > _1879);
static  monty::rc_ptr< ::mosek::fusion::Variable > hrepeat(monty::rc_ptr< ::mosek::fusion::Variable > _1908,int _1909);
static  monty::rc_ptr< ::mosek::fusion::Variable > vrepeat(monty::rc_ptr< ::mosek::fusion::Variable > _1910,int _1911);
static  monty::rc_ptr< ::mosek::fusion::Variable > repeat(monty::rc_ptr< ::mosek::fusion::Variable > _1912,int _1913);
static  monty::rc_ptr< ::mosek::fusion::Variable > repeat(monty::rc_ptr< ::mosek::fusion::Variable > _1914,int _1915,int _1916);
static  monty::rc_ptr< ::mosek::fusion::Variable > drepeat(monty::rc_ptr< ::mosek::fusion::Variable > _1917,int _1918,int _1919);
static  monty::rc_ptr< ::mosek::fusion::Variable > stack(std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > >,1 > > _1983);
static  monty::rc_ptr< ::mosek::fusion::Variable > vstack(monty::rc_ptr< ::mosek::fusion::Variable > _1985,monty::rc_ptr< ::mosek::fusion::Variable > _1986,monty::rc_ptr< ::mosek::fusion::Variable > _1987);
static  monty::rc_ptr< ::mosek::fusion::Variable > vstack(monty::rc_ptr< ::mosek::fusion::Variable > _1988,monty::rc_ptr< ::mosek::fusion::Variable > _1989);
static  monty::rc_ptr< ::mosek::fusion::Variable > vstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > _1990);
static  monty::rc_ptr< ::mosek::fusion::Variable > hstack(monty::rc_ptr< ::mosek::fusion::Variable > _1991,monty::rc_ptr< ::mosek::fusion::Variable > _1992,monty::rc_ptr< ::mosek::fusion::Variable > _1993);
static  monty::rc_ptr< ::mosek::fusion::Variable > hstack(monty::rc_ptr< ::mosek::fusion::Variable > _1994,monty::rc_ptr< ::mosek::fusion::Variable > _1995);
static  monty::rc_ptr< ::mosek::fusion::Variable > hstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > _1996);
static  monty::rc_ptr< ::mosek::fusion::Variable > stack(monty::rc_ptr< ::mosek::fusion::Variable > _1997,monty::rc_ptr< ::mosek::fusion::Variable > _1998,monty::rc_ptr< ::mosek::fusion::Variable > _1999,int _2000);
static  monty::rc_ptr< ::mosek::fusion::Variable > stack(monty::rc_ptr< ::mosek::fusion::Variable > _2001,monty::rc_ptr< ::mosek::fusion::Variable > _2002,int _2003);
static  monty::rc_ptr< ::mosek::fusion::Variable > stack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > _2004,int _2005);
static  monty::rc_ptr< ::mosek::fusion::Variable > promote(monty::rc_ptr< ::mosek::fusion::Variable > _2008,int _2009);
static  monty::rc_ptr< ::mosek::fusion::Variable > dstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > _2014,int _2015);
}; // struct Var;

struct p_ConstraintCache
{
ConstraintCache * _pubthis;
static mosek::fusion::p_ConstraintCache* _get_impl(mosek::fusion::ConstraintCache * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_ConstraintCache * _get_impl(mosek::fusion::ConstraintCache::t _inst) { return _get_impl(_inst.get()); }
p_ConstraintCache(ConstraintCache * _pubthis);
virtual ~p_ConstraintCache() { /* std::cout << "~p_ConstraintCache" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > barmatidx{};std::shared_ptr< monty::ndarray< int,1 > > barsubj{};std::shared_ptr< monty::ndarray< int,1 > > barsubi{};long long nbarnz{};long long nunordered{};std::shared_ptr< monty::ndarray< int,1 > > buffer_subi{};std::shared_ptr< monty::ndarray< int,1 > > buffer_subj{};std::shared_ptr< monty::ndarray< double,1 > > buffer_cof{};std::shared_ptr< monty::ndarray< double,1 > > bfix{};std::shared_ptr< monty::ndarray< double,1 > > cof{};std::shared_ptr< monty::ndarray< int,1 > > subi{};std::shared_ptr< monty::ndarray< int,1 > > subj{};long long nnz{};int nrows{};virtual void destroy();
static ConstraintCache::t _new_ConstraintCache(monty::rc_ptr< ::mosek::fusion::ConstraintCache > _2138);
void _initialize(monty::rc_ptr< ::mosek::fusion::ConstraintCache > _2138);
static ConstraintCache::t _new_ConstraintCache(std::shared_ptr< monty::ndarray< long long,1 > > _2139,std::shared_ptr< monty::ndarray< double,1 > > _2140,std::shared_ptr< monty::ndarray< int,1 > > _2141,std::shared_ptr< monty::ndarray< double,1 > > _2142,std::shared_ptr< monty::ndarray< int,1 > > _2143,std::shared_ptr< monty::ndarray< int,1 > > _2144,std::shared_ptr< monty::ndarray< int,1 > > _2145);
void _initialize(std::shared_ptr< monty::ndarray< long long,1 > > _2139,std::shared_ptr< monty::ndarray< double,1 > > _2140,std::shared_ptr< monty::ndarray< int,1 > > _2141,std::shared_ptr< monty::ndarray< double,1 > > _2142,std::shared_ptr< monty::ndarray< int,1 > > _2143,std::shared_ptr< monty::ndarray< int,1 > > _2144,std::shared_ptr< monty::ndarray< int,1 > > _2145);
virtual void unchecked_add_fx(std::shared_ptr< monty::ndarray< double,1 > > _2148) ;
virtual long long order_barentries() ;
virtual void add_bar(std::shared_ptr< monty::ndarray< int,1 > > _2158,std::shared_ptr< monty::ndarray< int,1 > > _2159,std::shared_ptr< monty::ndarray< int,1 > > _2160) ;
virtual void unchecked_add_l(std::shared_ptr< monty::ndarray< long long,1 > > _2166,std::shared_ptr< monty::ndarray< int,1 > > _2167,std::shared_ptr< monty::ndarray< double,1 > > _2168,std::shared_ptr< monty::ndarray< double,1 > > _2169) ;
virtual void add(std::shared_ptr< monty::ndarray< long long,1 > > _2178,std::shared_ptr< monty::ndarray< int,1 > > _2179,std::shared_ptr< monty::ndarray< double,1 > > _2180,std::shared_ptr< monty::ndarray< double,1 > > _2181) ;
virtual long long flush(std::shared_ptr< monty::ndarray< int,1 > > _2182,std::shared_ptr< monty::ndarray< int,1 > > _2183,std::shared_ptr< monty::ndarray< double,1 > > _2184,std::shared_ptr< monty::ndarray< double,1 > > _2185) ;
virtual long long numUnsorted() ;
virtual monty::rc_ptr< ::mosek::fusion::ConstraintCache > __mosek_2fusion_2ConstraintCache__clone() ;
}; // struct ConstraintCache;

struct p_Constraint
{
Constraint * _pubthis;
static mosek::fusion::p_Constraint* _get_impl(mosek::fusion::Constraint * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_Constraint * _get_impl(mosek::fusion::Constraint::t _inst) { return _get_impl(_inst.get()); }
p_Constraint(Constraint * _pubthis);
virtual ~p_Constraint() { /* std::cout << "~p_Constraint" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};std::shared_ptr< monty::ndarray< int,1 > > shape{};monty::rc_ptr< ::mosek::fusion::Model > model{};virtual void destroy();
static Constraint::t _new_Constraint(monty::rc_ptr< ::mosek::fusion::Constraint > _2259,monty::rc_ptr< ::mosek::fusion::Model > _2260);
void _initialize(monty::rc_ptr< ::mosek::fusion::Constraint > _2259,monty::rc_ptr< ::mosek::fusion::Model > _2260);
static Constraint::t _new_Constraint(monty::rc_ptr< ::mosek::fusion::Model > _2261,std::shared_ptr< monty::ndarray< int,1 > > _2262,std::shared_ptr< monty::ndarray< int,1 > > _2263);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _2261,std::shared_ptr< monty::ndarray< int,1 > > _2262,std::shared_ptr< monty::ndarray< int,1 > > _2263);
virtual /* override */ std::string toString() ;
virtual void toStringArray(std::shared_ptr< monty::ndarray< long long,1 > > _2264,long long _2265,std::shared_ptr< monty::ndarray< std::string,1 > > _2266) ;
virtual std::shared_ptr< monty::ndarray< double,1 > > dual() ;
virtual std::shared_ptr< monty::ndarray< double,1 > > level() ;
virtual void values(bool _2269,int _2270,std::shared_ptr< monty::ndarray< double,1 > > _2271) ;
virtual void update(std::shared_ptr< monty::ndarray< double,1 > > _2272) ;
virtual void update(monty::rc_ptr< ::mosek::fusion::Expression > _2273) ;
virtual void update(monty::rc_ptr< ::mosek::fusion::Expression > _2277,monty::rc_ptr< ::mosek::fusion::Variable > _2278,bool _2279) ;
virtual void update(monty::rc_ptr< ::mosek::fusion::Expression > _2298,monty::rc_ptr< ::mosek::fusion::Variable > _2299) ;
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Constraint__get_model() ;
virtual int get_nd() ;
virtual long long size() ;
static  monty::rc_ptr< ::mosek::fusion::Constraint > stack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Constraint >,1 > > _2302,int _2303);
static  monty::rc_ptr< ::mosek::fusion::Constraint > stack(monty::rc_ptr< ::mosek::fusion::Constraint > _2304,monty::rc_ptr< ::mosek::fusion::Constraint > _2305,monty::rc_ptr< ::mosek::fusion::Constraint > _2306,int _2307);
static  monty::rc_ptr< ::mosek::fusion::Constraint > stack(monty::rc_ptr< ::mosek::fusion::Constraint > _2308,monty::rc_ptr< ::mosek::fusion::Constraint > _2309,int _2310);
static  monty::rc_ptr< ::mosek::fusion::Constraint > hstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Constraint >,1 > > _2311);
static  monty::rc_ptr< ::mosek::fusion::Constraint > vstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Constraint >,1 > > _2312);
static  monty::rc_ptr< ::mosek::fusion::Constraint > hstack(monty::rc_ptr< ::mosek::fusion::Constraint > _2313,monty::rc_ptr< ::mosek::fusion::Constraint > _2314,monty::rc_ptr< ::mosek::fusion::Constraint > _2315);
static  monty::rc_ptr< ::mosek::fusion::Constraint > vstack(monty::rc_ptr< ::mosek::fusion::Constraint > _2316,monty::rc_ptr< ::mosek::fusion::Constraint > _2317,monty::rc_ptr< ::mosek::fusion::Constraint > _2318);
static  monty::rc_ptr< ::mosek::fusion::Constraint > hstack(monty::rc_ptr< ::mosek::fusion::Constraint > _2319,monty::rc_ptr< ::mosek::fusion::Constraint > _2320);
static  monty::rc_ptr< ::mosek::fusion::Constraint > vstack(monty::rc_ptr< ::mosek::fusion::Constraint > _2321,monty::rc_ptr< ::mosek::fusion::Constraint > _2322);
static  monty::rc_ptr< ::mosek::fusion::Constraint > dstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Constraint >,1 > > _2323,int _2324);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Constraint__index(std::shared_ptr< monty::ndarray< int,1 > > _2375) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Constraint__index(int _2382) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Constraint__slice(std::shared_ptr< monty::ndarray< int,1 > > _2383,std::shared_ptr< monty::ndarray< int,1 > > _2384) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Constraint__slice(int _2403,int _2404) ;
virtual int getND() ;
virtual int getSize() ;
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Constraint__getModel() ;
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() ;
}; // struct Constraint;

struct p_SliceConstraint : public ::mosek::fusion::p_Constraint
{
SliceConstraint * _pubthis;
static mosek::fusion::p_SliceConstraint* _get_impl(mosek::fusion::SliceConstraint * _inst){ return static_cast< mosek::fusion::p_SliceConstraint* >(mosek::fusion::p_Constraint::_get_impl(_inst)); }
static mosek::fusion::p_SliceConstraint * _get_impl(mosek::fusion::SliceConstraint::t _inst) { return _get_impl(_inst.get()); }
p_SliceConstraint(SliceConstraint * _pubthis);
virtual ~p_SliceConstraint() { /* std::cout << "~p_SliceConstraint" << std::endl;*/ };
virtual void destroy();
static SliceConstraint::t _new_SliceConstraint(monty::rc_ptr< ::mosek::fusion::Model > _2208,std::shared_ptr< monty::ndarray< int,1 > > _2209,std::shared_ptr< monty::ndarray< int,1 > > _2210);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _2208,std::shared_ptr< monty::ndarray< int,1 > > _2209,std::shared_ptr< monty::ndarray< int,1 > > _2210);
virtual /* override */ std::string toString() ;
}; // struct SliceConstraint;

struct p_ModelConstraint : public ::mosek::fusion::p_Constraint
{
ModelConstraint * _pubthis;
static mosek::fusion::p_ModelConstraint* _get_impl(mosek::fusion::ModelConstraint * _inst){ return static_cast< mosek::fusion::p_ModelConstraint* >(mosek::fusion::p_Constraint::_get_impl(_inst)); }
static mosek::fusion::p_ModelConstraint * _get_impl(mosek::fusion::ModelConstraint::t _inst) { return _get_impl(_inst.get()); }
p_ModelConstraint(ModelConstraint * _pubthis);
virtual ~p_ModelConstraint() { /* std::cout << "~p_ModelConstraint" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > shape{};std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};bool names_flushed{};std::string name{};virtual void destroy();
static ModelConstraint::t _new_ModelConstraint(monty::rc_ptr< ::mosek::fusion::ModelConstraint > _2248,monty::rc_ptr< ::mosek::fusion::Model > _2249);
void _initialize(monty::rc_ptr< ::mosek::fusion::ModelConstraint > _2248,monty::rc_ptr< ::mosek::fusion::Model > _2249);
static ModelConstraint::t _new_ModelConstraint(monty::rc_ptr< ::mosek::fusion::Model > _2250,const std::string &  _2251,std::shared_ptr< monty::ndarray< int,1 > > _2252,std::shared_ptr< monty::ndarray< int,1 > > _2253);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _2250,const std::string &  _2251,std::shared_ptr< monty::ndarray< int,1 > > _2252,std::shared_ptr< monty::ndarray< int,1 > > _2253);
virtual /* override */ std::string toString() ;
virtual void flushNames() ;
virtual monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2ModelConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2258) { throw monty::AbstractClassError("Call to abstract method"); }
}; // struct ModelConstraint;

struct p_LinearPSDConstraint : public ::mosek::fusion::p_ModelConstraint
{
LinearPSDConstraint * _pubthis;
static mosek::fusion::p_LinearPSDConstraint* _get_impl(mosek::fusion::LinearPSDConstraint * _inst){ return static_cast< mosek::fusion::p_LinearPSDConstraint* >(mosek::fusion::p_ModelConstraint::_get_impl(_inst)); }
static mosek::fusion::p_LinearPSDConstraint * _get_impl(mosek::fusion::LinearPSDConstraint::t _inst) { return _get_impl(_inst.get()); }
p_LinearPSDConstraint(LinearPSDConstraint * _pubthis);
virtual ~p_LinearPSDConstraint() { /* std::cout << "~p_LinearPSDConstraint" << std::endl;*/ };
int conedim{};std::shared_ptr< monty::ndarray< int,1 > > shape{};int conid{};std::shared_ptr< monty::ndarray< long long,1 > > slackidxs{};std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};virtual void destroy();
static LinearPSDConstraint::t _new_LinearPSDConstraint(monty::rc_ptr< ::mosek::fusion::LinearPSDConstraint > _2084,monty::rc_ptr< ::mosek::fusion::Model > _2085);
void _initialize(monty::rc_ptr< ::mosek::fusion::LinearPSDConstraint > _2084,monty::rc_ptr< ::mosek::fusion::Model > _2085);
static LinearPSDConstraint::t _new_LinearPSDConstraint(monty::rc_ptr< ::mosek::fusion::Model > _2086,const std::string &  _2087,int _2088,std::shared_ptr< monty::ndarray< int,1 > > _2089,int _2090,std::shared_ptr< monty::ndarray< int,1 > > _2091,std::shared_ptr< monty::ndarray< long long,1 > > _2092);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _2086,const std::string &  _2087,int _2088,std::shared_ptr< monty::ndarray< int,1 > > _2089,int _2090,std::shared_ptr< monty::ndarray< int,1 > > _2091,std::shared_ptr< monty::ndarray< long long,1 > > _2092);
virtual void domainToString(long long _2093,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > _2094) ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2LinearPSDConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2098) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2ModelConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2098) { return __mosek_2fusion_2LinearPSDConstraint__clone(_2098); }
}; // struct LinearPSDConstraint;

struct p_PSDConstraint : public ::mosek::fusion::p_ModelConstraint
{
PSDConstraint * _pubthis;
static mosek::fusion::p_PSDConstraint* _get_impl(mosek::fusion::PSDConstraint * _inst){ return static_cast< mosek::fusion::p_PSDConstraint* >(mosek::fusion::p_ModelConstraint::_get_impl(_inst)); }
static mosek::fusion::p_PSDConstraint * _get_impl(mosek::fusion::PSDConstraint::t _inst) { return _get_impl(_inst.get()); }
p_PSDConstraint(PSDConstraint * _pubthis);
virtual ~p_PSDConstraint() { /* std::cout << "~p_PSDConstraint" << std::endl;*/ };
bool names_flushed{};int conedim1{};int conedim0{};std::shared_ptr< monty::ndarray< int,1 > > shape{};std::string name{};std::shared_ptr< monty::ndarray< long long,1 > > slackidxs{};std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};int conid{};virtual void destroy();
static PSDConstraint::t _new_PSDConstraint(monty::rc_ptr< ::mosek::fusion::PSDConstraint > _2099,monty::rc_ptr< ::mosek::fusion::Model > _2100);
void _initialize(monty::rc_ptr< ::mosek::fusion::PSDConstraint > _2099,monty::rc_ptr< ::mosek::fusion::Model > _2100);
static PSDConstraint::t _new_PSDConstraint(monty::rc_ptr< ::mosek::fusion::Model > _2101,const std::string &  _2102,int _2103,std::shared_ptr< monty::ndarray< int,1 > > _2104,int _2105,int _2106,std::shared_ptr< monty::ndarray< long long,1 > > _2107,std::shared_ptr< monty::ndarray< int,1 > > _2108);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _2101,const std::string &  _2102,int _2103,std::shared_ptr< monty::ndarray< int,1 > > _2104,int _2105,int _2106,std::shared_ptr< monty::ndarray< long long,1 > > _2107,std::shared_ptr< monty::ndarray< int,1 > > _2108);
virtual /* override */ std::string toString() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2PSDConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2109) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2ModelConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2109) { return __mosek_2fusion_2PSDConstraint__clone(_2109); }
static  std::shared_ptr< monty::ndarray< int,1 > > computenidxs(std::shared_ptr< monty::ndarray< int,1 > > _2110,int _2111,int _2112,std::shared_ptr< monty::ndarray< int,1 > > _2113);
}; // struct PSDConstraint;

struct p_RangedConstraint : public ::mosek::fusion::p_ModelConstraint
{
RangedConstraint * _pubthis;
static mosek::fusion::p_RangedConstraint* _get_impl(mosek::fusion::RangedConstraint * _inst){ return static_cast< mosek::fusion::p_RangedConstraint* >(mosek::fusion::p_ModelConstraint::_get_impl(_inst)); }
static mosek::fusion::p_RangedConstraint * _get_impl(mosek::fusion::RangedConstraint::t _inst) { return _get_impl(_inst.get()); }
p_RangedConstraint(RangedConstraint * _pubthis);
virtual ~p_RangedConstraint() { /* std::cout << "~p_RangedConstraint" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};std::shared_ptr< monty::ndarray< int,1 > > shape{};virtual void destroy();
static RangedConstraint::t _new_RangedConstraint(monty::rc_ptr< ::mosek::fusion::RangedConstraint > _2212,monty::rc_ptr< ::mosek::fusion::Model > _2213);
void _initialize(monty::rc_ptr< ::mosek::fusion::RangedConstraint > _2212,monty::rc_ptr< ::mosek::fusion::Model > _2213);
static RangedConstraint::t _new_RangedConstraint(monty::rc_ptr< ::mosek::fusion::Model > _2214,const std::string &  _2215,std::shared_ptr< monty::ndarray< int,1 > > _2216,std::shared_ptr< monty::ndarray< int,1 > > _2217);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _2214,const std::string &  _2215,std::shared_ptr< monty::ndarray< int,1 > > _2216,std::shared_ptr< monty::ndarray< int,1 > > _2217);
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2RangedConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2218) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2ModelConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2218) { return __mosek_2fusion_2RangedConstraint__clone(_2218); }
}; // struct RangedConstraint;

struct p_ConicConstraint : public ::mosek::fusion::p_ModelConstraint
{
ConicConstraint * _pubthis;
static mosek::fusion::p_ConicConstraint* _get_impl(mosek::fusion::ConicConstraint * _inst){ return static_cast< mosek::fusion::p_ConicConstraint* >(mosek::fusion::p_ModelConstraint::_get_impl(_inst)); }
static mosek::fusion::p_ConicConstraint * _get_impl(mosek::fusion::ConicConstraint::t _inst) { return _get_impl(_inst.get()); }
p_ConicConstraint(ConicConstraint * _pubthis);
virtual ~p_ConicConstraint() { /* std::cout << "~p_ConicConstraint" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > nativeslack{};std::shared_ptr< monty::ndarray< int,1 > > nativeidxs{};bool names_flushed{};std::string name{};std::shared_ptr< monty::ndarray< int,1 > > shape{};monty::rc_ptr< ::mosek::fusion::ConeDomain > dom{};int conid{};virtual void destroy();
static ConicConstraint::t _new_ConicConstraint(monty::rc_ptr< ::mosek::fusion::ConicConstraint > _2219,monty::rc_ptr< ::mosek::fusion::Model > _2220);
void _initialize(monty::rc_ptr< ::mosek::fusion::ConicConstraint > _2219,monty::rc_ptr< ::mosek::fusion::Model > _2220);
static ConicConstraint::t _new_ConicConstraint(monty::rc_ptr< ::mosek::fusion::Model > _2221,const std::string &  _2222,monty::rc_ptr< ::mosek::fusion::ConeDomain > _2223,std::shared_ptr< monty::ndarray< int,1 > > _2224,int _2225,std::shared_ptr< monty::ndarray< int,1 > > _2226,std::shared_ptr< monty::ndarray< int,1 > > _2227);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _2221,const std::string &  _2222,monty::rc_ptr< ::mosek::fusion::ConeDomain > _2223,std::shared_ptr< monty::ndarray< int,1 > > _2224,int _2225,std::shared_ptr< monty::ndarray< int,1 > > _2226,std::shared_ptr< monty::ndarray< int,1 > > _2227);
virtual /* override */ void flushNames() ;
virtual /* override */ std::string toString() ;
virtual void domainToString(long long _2234,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > _2235) ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2ConicConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2236) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2ModelConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2236) { return __mosek_2fusion_2ConicConstraint__clone(_2236); }
}; // struct ConicConstraint;

struct p_LinearConstraint : public ::mosek::fusion::p_ModelConstraint
{
LinearConstraint * _pubthis;
static mosek::fusion::p_LinearConstraint* _get_impl(mosek::fusion::LinearConstraint * _inst){ return static_cast< mosek::fusion::p_LinearConstraint* >(mosek::fusion::p_ModelConstraint::_get_impl(_inst)); }
static mosek::fusion::p_LinearConstraint * _get_impl(mosek::fusion::LinearConstraint::t _inst) { return _get_impl(_inst.get()); }
p_LinearConstraint(LinearConstraint * _pubthis);
virtual ~p_LinearConstraint() { /* std::cout << "~p_LinearConstraint" << std::endl;*/ };
std::string name{};int conid{};virtual void destroy();
static LinearConstraint::t _new_LinearConstraint(monty::rc_ptr< ::mosek::fusion::LinearConstraint > _2237,monty::rc_ptr< ::mosek::fusion::Model > _2238);
void _initialize(monty::rc_ptr< ::mosek::fusion::LinearConstraint > _2237,monty::rc_ptr< ::mosek::fusion::Model > _2238);
static LinearConstraint::t _new_LinearConstraint(monty::rc_ptr< ::mosek::fusion::Model > _2239,const std::string &  _2240,int _2241,std::shared_ptr< monty::ndarray< int,1 > > _2242,std::shared_ptr< monty::ndarray< int,1 > > _2243);
void _initialize(monty::rc_ptr< ::mosek::fusion::Model > _2239,const std::string &  _2240,int _2241,std::shared_ptr< monty::ndarray< int,1 > > _2242,std::shared_ptr< monty::ndarray< int,1 > > _2243);
virtual /* override */ std::string toString() ;
virtual void domainToString(long long _2245,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > _2246) ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2LinearConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2247) ;
virtual monty::rc_ptr< ::mosek::fusion::ModelConstraint > __mosek_2fusion_2ModelConstraint__clone(monty::rc_ptr< ::mosek::fusion::Model > _2247) { return __mosek_2fusion_2LinearConstraint__clone(_2247); }
}; // struct LinearConstraint;

struct p_Set
{
Set * _pubthis;
static mosek::fusion::p_Set* _get_impl(mosek::fusion::Set * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_Set * _get_impl(mosek::fusion::Set::t _inst) { return _get_impl(_inst.get()); }
p_Set(Set * _pubthis);
virtual ~p_Set() { /* std::cout << "~p_Set" << std::endl;*/ };
virtual void destroy();
static  long long size(std::shared_ptr< monty::ndarray< int,1 > > _2409);
static  bool match(std::shared_ptr< monty::ndarray< int,1 > > _2412,std::shared_ptr< monty::ndarray< int,1 > > _2413);
static  long long linearidx(std::shared_ptr< monty::ndarray< int,1 > > _2415,std::shared_ptr< monty::ndarray< int,1 > > _2416);
static  std::shared_ptr< monty::ndarray< int,1 > > idxtokey(std::shared_ptr< monty::ndarray< int,1 > > _2419,long long _2420);
static  void idxtokey(std::shared_ptr< monty::ndarray< int,1 > > _2422,long long _2423,std::shared_ptr< monty::ndarray< int,1 > > _2424);
static  std::string indexToString(std::shared_ptr< monty::ndarray< int,1 > > _2428,long long _2429);
static  std::string keyToString(std::shared_ptr< monty::ndarray< int,1 > > _2436);
static  void indexToKey(std::shared_ptr< monty::ndarray< int,1 > > _2439,long long _2440,std::shared_ptr< monty::ndarray< int,1 > > _2441);
static  std::shared_ptr< monty::ndarray< long long,1 > > strides(std::shared_ptr< monty::ndarray< int,1 > > _2445);
static  std::shared_ptr< monty::ndarray< int,1 > > make(std::shared_ptr< monty::ndarray< int,1 > > _2449,std::shared_ptr< monty::ndarray< int,1 > > _2450);
static  std::shared_ptr< monty::ndarray< int,1 > > make(std::shared_ptr< monty::ndarray< int,1 > > _2454);
static  std::shared_ptr< monty::ndarray< int,1 > > make(int _2456,int _2457,int _2458);
static  std::shared_ptr< monty::ndarray< int,1 > > make(int _2459,int _2460);
static  std::shared_ptr< monty::ndarray< int,1 > > make(int _2461);
static  std::shared_ptr< monty::ndarray< int,1 > > scalar();
static  std::shared_ptr< monty::ndarray< int,1 > > make(std::shared_ptr< monty::ndarray< std::string,1 > > _2462);
}; // struct Set;

struct p_ConeDomain
{
ConeDomain * _pubthis;
static mosek::fusion::p_ConeDomain* _get_impl(mosek::fusion::ConeDomain * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_ConeDomain * _get_impl(mosek::fusion::ConeDomain::t _inst) { return _get_impl(_inst.get()); }
p_ConeDomain(ConeDomain * _pubthis);
virtual ~p_ConeDomain() { /* std::cout << "~p_ConeDomain" << std::endl;*/ };
double alpha{};std::shared_ptr< monty::ndarray< int,1 > > shape{};bool int_flag{};bool axisset{};int axisidx{};mosek::fusion::QConeKey key{};virtual void destroy();
static ConeDomain::t _new_ConeDomain(mosek::fusion::QConeKey _2463,double _2464,std::shared_ptr< monty::ndarray< int,1 > > _2465);
void _initialize(mosek::fusion::QConeKey _2463,double _2464,std::shared_ptr< monty::ndarray< int,1 > > _2465);
static ConeDomain::t _new_ConeDomain(mosek::fusion::QConeKey _2466,std::shared_ptr< monty::ndarray< int,1 > > _2467);
void _initialize(mosek::fusion::QConeKey _2466,std::shared_ptr< monty::ndarray< int,1 > > _2467);
virtual bool match_shape(std::shared_ptr< monty::ndarray< int,1 > > _2468) ;
virtual monty::rc_ptr< ::mosek::fusion::ConeDomain > __mosek_2fusion_2ConeDomain__integral() ;
virtual bool axisIsSet() ;
virtual int getAxis() ;
virtual monty::rc_ptr< ::mosek::fusion::ConeDomain > __mosek_2fusion_2ConeDomain__axis(int _2469) ;
}; // struct ConeDomain;

struct p_LinPSDDomain
{
LinPSDDomain * _pubthis;
static mosek::fusion::p_LinPSDDomain* _get_impl(mosek::fusion::LinPSDDomain * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_LinPSDDomain * _get_impl(mosek::fusion::LinPSDDomain::t _inst) { return _get_impl(_inst.get()); }
p_LinPSDDomain(LinPSDDomain * _pubthis);
virtual ~p_LinPSDDomain() { /* std::cout << "~p_LinPSDDomain" << std::endl;*/ };
int conedim{};std::shared_ptr< monty::ndarray< int,1 > > shape{};virtual void destroy();
static LinPSDDomain::t _new_LinPSDDomain(std::shared_ptr< monty::ndarray< int,1 > > _2470,int _2471);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _2470,int _2471);
static LinPSDDomain::t _new_LinPSDDomain(std::shared_ptr< monty::ndarray< int,1 > > _2472);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _2472);
static LinPSDDomain::t _new_LinPSDDomain();
void _initialize();
}; // struct LinPSDDomain;

struct p_PSDDomain
{
PSDDomain * _pubthis;
static mosek::fusion::p_PSDDomain* _get_impl(mosek::fusion::PSDDomain * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_PSDDomain * _get_impl(mosek::fusion::PSDDomain::t _inst) { return _get_impl(_inst.get()); }
p_PSDDomain(PSDDomain * _pubthis);
virtual ~p_PSDDomain() { /* std::cout << "~p_PSDDomain" << std::endl;*/ };
bool axisIsSet{};int conedim2{};int conedim1{};mosek::fusion::PSDKey key{};std::shared_ptr< monty::ndarray< int,1 > > shape{};virtual void destroy();
static PSDDomain::t _new_PSDDomain(mosek::fusion::PSDKey _2473,std::shared_ptr< monty::ndarray< int,1 > > _2474,int _2475,int _2476);
void _initialize(mosek::fusion::PSDKey _2473,std::shared_ptr< monty::ndarray< int,1 > > _2474,int _2475,int _2476);
static PSDDomain::t _new_PSDDomain(mosek::fusion::PSDKey _2478,std::shared_ptr< monty::ndarray< int,1 > > _2479);
void _initialize(mosek::fusion::PSDKey _2478,std::shared_ptr< monty::ndarray< int,1 > > _2479);
static PSDDomain::t _new_PSDDomain(mosek::fusion::PSDKey _2480);
void _initialize(mosek::fusion::PSDKey _2480);
virtual monty::rc_ptr< ::mosek::fusion::PSDDomain > __mosek_2fusion_2PSDDomain__axis(int _2481,int _2482) ;
}; // struct PSDDomain;

struct p_RangeDomain
{
RangeDomain * _pubthis;
static mosek::fusion::p_RangeDomain* _get_impl(mosek::fusion::RangeDomain * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_RangeDomain * _get_impl(mosek::fusion::RangeDomain::t _inst) { return _get_impl(_inst.get()); }
p_RangeDomain(RangeDomain * _pubthis);
virtual ~p_RangeDomain() { /* std::cout << "~p_RangeDomain" << std::endl;*/ };
bool cardinal_flag{};bool scalable{};std::shared_ptr< monty::ndarray< double,1 > > ub{};std::shared_ptr< monty::ndarray< double,1 > > lb{};std::shared_ptr< monty::ndarray< int,2 > > sparsity{};bool empty{};std::shared_ptr< monty::ndarray< int,1 > > shape{};virtual void destroy();
static RangeDomain::t _new_RangeDomain(bool _2484,std::shared_ptr< monty::ndarray< double,1 > > _2485,std::shared_ptr< monty::ndarray< double,1 > > _2486,std::shared_ptr< monty::ndarray< int,1 > > _2487);
void _initialize(bool _2484,std::shared_ptr< monty::ndarray< double,1 > > _2485,std::shared_ptr< monty::ndarray< double,1 > > _2486,std::shared_ptr< monty::ndarray< int,1 > > _2487);
static RangeDomain::t _new_RangeDomain(bool _2488,std::shared_ptr< monty::ndarray< double,1 > > _2489,std::shared_ptr< monty::ndarray< double,1 > > _2490,std::shared_ptr< monty::ndarray< int,1 > > _2491,std::shared_ptr< monty::ndarray< int,2 > > _2492);
void _initialize(bool _2488,std::shared_ptr< monty::ndarray< double,1 > > _2489,std::shared_ptr< monty::ndarray< double,1 > > _2490,std::shared_ptr< monty::ndarray< int,1 > > _2491,std::shared_ptr< monty::ndarray< int,2 > > _2492);
static RangeDomain::t _new_RangeDomain(bool _2493,std::shared_ptr< monty::ndarray< double,1 > > _2494,std::shared_ptr< monty::ndarray< double,1 > > _2495,std::shared_ptr< monty::ndarray< int,1 > > _2496,std::shared_ptr< monty::ndarray< int,2 > > _2497,int _2498);
void _initialize(bool _2493,std::shared_ptr< monty::ndarray< double,1 > > _2494,std::shared_ptr< monty::ndarray< double,1 > > _2495,std::shared_ptr< monty::ndarray< int,1 > > _2496,std::shared_ptr< monty::ndarray< int,2 > > _2497,int _2498);
static RangeDomain::t _new_RangeDomain(monty::rc_ptr< ::mosek::fusion::RangeDomain > _2499);
void _initialize(monty::rc_ptr< ::mosek::fusion::RangeDomain > _2499);
virtual monty::rc_ptr< ::mosek::fusion::SymmetricRangeDomain > __mosek_2fusion_2RangeDomain__symmetric() ;
virtual monty::rc_ptr< ::mosek::fusion::RangeDomain > __mosek_2fusion_2RangeDomain__sparse(std::shared_ptr< monty::ndarray< int,2 > > _2500) ;
virtual monty::rc_ptr< ::mosek::fusion::RangeDomain > __mosek_2fusion_2RangeDomain__sparse(std::shared_ptr< monty::ndarray< int,1 > > _2503) ;
virtual monty::rc_ptr< ::mosek::fusion::RangeDomain > __mosek_2fusion_2RangeDomain__sparse() ;
virtual monty::rc_ptr< ::mosek::fusion::RangeDomain > __mosek_2fusion_2RangeDomain__integral() ;
virtual monty::rc_ptr< ::mosek::fusion::RangeDomain > __mosek_2fusion_2RangeDomain__withShape(std::shared_ptr< monty::ndarray< int,1 > > _2505) ;
virtual bool match_shape(std::shared_ptr< monty::ndarray< int,1 > > _2506) ;
}; // struct RangeDomain;

struct p_SymmetricRangeDomain : public ::mosek::fusion::p_RangeDomain
{
SymmetricRangeDomain * _pubthis;
static mosek::fusion::p_SymmetricRangeDomain* _get_impl(mosek::fusion::SymmetricRangeDomain * _inst){ return static_cast< mosek::fusion::p_SymmetricRangeDomain* >(mosek::fusion::p_RangeDomain::_get_impl(_inst)); }
static mosek::fusion::p_SymmetricRangeDomain * _get_impl(mosek::fusion::SymmetricRangeDomain::t _inst) { return _get_impl(_inst.get()); }
p_SymmetricRangeDomain(SymmetricRangeDomain * _pubthis);
virtual ~p_SymmetricRangeDomain() { /* std::cout << "~p_SymmetricRangeDomain" << std::endl;*/ };
int dim{};virtual void destroy();
static SymmetricRangeDomain::t _new_SymmetricRangeDomain(monty::rc_ptr< ::mosek::fusion::RangeDomain > _2483);
void _initialize(monty::rc_ptr< ::mosek::fusion::RangeDomain > _2483);
}; // struct SymmetricRangeDomain;

struct p_SymmetricLinearDomain
{
SymmetricLinearDomain * _pubthis;
static mosek::fusion::p_SymmetricLinearDomain* _get_impl(mosek::fusion::SymmetricLinearDomain * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_SymmetricLinearDomain * _get_impl(mosek::fusion::SymmetricLinearDomain::t _inst) { return _get_impl(_inst.get()); }
p_SymmetricLinearDomain(SymmetricLinearDomain * _pubthis);
virtual ~p_SymmetricLinearDomain() { /* std::cout << "~p_SymmetricLinearDomain" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,2 > > sparsity{};bool cardinal_flag{};mosek::fusion::RelationKey key{};std::shared_ptr< monty::ndarray< int,1 > > shape{};monty::rc_ptr< ::mosek::fusion::LinearDomain > dom{};int dim{};virtual void destroy();
static SymmetricLinearDomain::t _new_SymmetricLinearDomain(monty::rc_ptr< ::mosek::fusion::LinearDomain > _2508);
void _initialize(monty::rc_ptr< ::mosek::fusion::LinearDomain > _2508);
virtual monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > __mosek_2fusion_2SymmetricLinearDomain__sparse(std::shared_ptr< monty::ndarray< int,2 > > _2509) ;
virtual monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > __mosek_2fusion_2SymmetricLinearDomain__sparse(std::shared_ptr< monty::ndarray< int,1 > > _2512) ;
virtual monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > __mosek_2fusion_2SymmetricLinearDomain__integral() ;
virtual bool match_shape(std::shared_ptr< monty::ndarray< int,1 > > _2514) ;
}; // struct SymmetricLinearDomain;

struct p_LinearDomain
{
LinearDomain * _pubthis;
static mosek::fusion::p_LinearDomain* _get_impl(mosek::fusion::LinearDomain * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_LinearDomain * _get_impl(mosek::fusion::LinearDomain::t _inst) { return _get_impl(_inst.get()); }
p_LinearDomain(LinearDomain * _pubthis);
virtual ~p_LinearDomain() { /* std::cout << "~p_LinearDomain" << std::endl;*/ };
bool empty{};bool scalable{};std::shared_ptr< monty::ndarray< int,2 > > sparsity{};bool cardinal_flag{};mosek::fusion::RelationKey key{};std::shared_ptr< monty::ndarray< double,1 > > bnd{};std::shared_ptr< monty::ndarray< int,1 > > shape{};virtual void destroy();
static LinearDomain::t _new_LinearDomain(mosek::fusion::RelationKey _2516,bool _2517,std::shared_ptr< monty::ndarray< double,1 > > _2518,std::shared_ptr< monty::ndarray< int,1 > > _2519);
void _initialize(mosek::fusion::RelationKey _2516,bool _2517,std::shared_ptr< monty::ndarray< double,1 > > _2518,std::shared_ptr< monty::ndarray< int,1 > > _2519);
static LinearDomain::t _new_LinearDomain(mosek::fusion::RelationKey _2520,bool _2521,std::shared_ptr< monty::ndarray< double,1 > > _2522,std::shared_ptr< monty::ndarray< int,1 > > _2523,std::shared_ptr< monty::ndarray< int,2 > > _2524,int _2525);
void _initialize(mosek::fusion::RelationKey _2520,bool _2521,std::shared_ptr< monty::ndarray< double,1 > > _2522,std::shared_ptr< monty::ndarray< int,1 > > _2523,std::shared_ptr< monty::ndarray< int,2 > > _2524,int _2525);
static LinearDomain::t _new_LinearDomain(monty::rc_ptr< ::mosek::fusion::LinearDomain > _2526);
void _initialize(monty::rc_ptr< ::mosek::fusion::LinearDomain > _2526);
virtual monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > __mosek_2fusion_2LinearDomain__symmetric() ;
virtual monty::rc_ptr< ::mosek::fusion::LinearDomain > __mosek_2fusion_2LinearDomain__sparse(std::shared_ptr< monty::ndarray< int,2 > > _2527) ;
virtual monty::rc_ptr< ::mosek::fusion::LinearDomain > __mosek_2fusion_2LinearDomain__sparse(std::shared_ptr< monty::ndarray< int,1 > > _2530) ;
virtual monty::rc_ptr< ::mosek::fusion::LinearDomain > __mosek_2fusion_2LinearDomain__sparse() ;
virtual monty::rc_ptr< ::mosek::fusion::LinearDomain > __mosek_2fusion_2LinearDomain__integral() ;
virtual monty::rc_ptr< ::mosek::fusion::LinearDomain > __mosek_2fusion_2LinearDomain__withShape(std::shared_ptr< monty::ndarray< int,1 > > _2532) ;
virtual bool match_shape(std::shared_ptr< monty::ndarray< int,1 > > _2533) ;
}; // struct LinearDomain;

struct p_Domain
{
Domain * _pubthis;
static mosek::fusion::p_Domain* _get_impl(mosek::fusion::Domain * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_Domain * _get_impl(mosek::fusion::Domain::t _inst) { return _get_impl(_inst.get()); }
p_Domain(Domain * _pubthis);
virtual ~p_Domain() { /* std::cout << "~p_Domain" << std::endl;*/ };
virtual void destroy();
static  long long dimsize(std::shared_ptr< monty::ndarray< int,1 > > _2535);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > mkLinearDomain(mosek::fusion::RelationKey _2538,monty::rc_ptr< ::mosek::fusion::Matrix > _2539);
static  long long prod(std::shared_ptr< monty::ndarray< int,1 > > _2545);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(bool _2548,std::shared_ptr< monty::ndarray< double,1 > > _2549,std::shared_ptr< monty::ndarray< double,1 > > _2550,std::shared_ptr< monty::ndarray< int,2 > > _2551,std::shared_ptr< monty::ndarray< int,1 > > _2552);
static  monty::rc_ptr< ::mosek::fusion::SymmetricRangeDomain > symmetric(monty::rc_ptr< ::mosek::fusion::RangeDomain > _2554);
static  monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > symmetric(monty::rc_ptr< ::mosek::fusion::LinearDomain > _2555);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > sparse(monty::rc_ptr< ::mosek::fusion::RangeDomain > _2556,std::shared_ptr< monty::ndarray< int,2 > > _2557);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > sparse(monty::rc_ptr< ::mosek::fusion::RangeDomain > _2558,std::shared_ptr< monty::ndarray< int,1 > > _2559);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > sparse(monty::rc_ptr< ::mosek::fusion::LinearDomain > _2560,std::shared_ptr< monty::ndarray< int,2 > > _2561);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > sparse(monty::rc_ptr< ::mosek::fusion::LinearDomain > _2562,std::shared_ptr< monty::ndarray< int,1 > > _2563);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > integral(monty::rc_ptr< ::mosek::fusion::RangeDomain > _2564);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > integral(monty::rc_ptr< ::mosek::fusion::LinearDomain > _2565);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > integral(monty::rc_ptr< ::mosek::fusion::ConeDomain > _2566);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > axis(monty::rc_ptr< ::mosek::fusion::ConeDomain > _2567,int _2568);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inDPowerCone(double _2569,std::shared_ptr< monty::ndarray< int,1 > > _2570);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inDPowerCone(double _2572,int _2573);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inDPowerCone(double _2574);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inPPowerCone(double _2575,std::shared_ptr< monty::ndarray< int,1 > > _2576);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inPPowerCone(double _2578,int _2579);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inPPowerCone(double _2580);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inDExpCone(std::shared_ptr< monty::ndarray< int,1 > > _2581);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inDExpCone(int _2583);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inDExpCone();
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inPExpCone(std::shared_ptr< monty::ndarray< int,1 > > _2584);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inPExpCone(int _2586);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inPExpCone();
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inRotatedQCone(std::shared_ptr< monty::ndarray< int,1 > > _2587);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inRotatedQCone(int _2589,int _2590);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inRotatedQCone(int _2591);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inRotatedQCone();
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inQCone(std::shared_ptr< monty::ndarray< int,1 > > _2592);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inQCone(int _2594,int _2595);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inQCone(int _2596);
static  monty::rc_ptr< ::mosek::fusion::ConeDomain > inQCone();
static  monty::rc_ptr< ::mosek::fusion::LinPSDDomain > isLinPSD(int _2597,int _2598);
static  monty::rc_ptr< ::mosek::fusion::LinPSDDomain > isLinPSD(int _2599);
static  monty::rc_ptr< ::mosek::fusion::LinPSDDomain > isLinPSD();
static  monty::rc_ptr< ::mosek::fusion::PSDDomain > isTrilPSD(int _2600,int _2601);
static  monty::rc_ptr< ::mosek::fusion::PSDDomain > isTrilPSD(int _2602);
static  monty::rc_ptr< ::mosek::fusion::PSDDomain > isTrilPSD();
static  monty::rc_ptr< ::mosek::fusion::PSDDomain > inPSDCone(int _2603,int _2604);
static  monty::rc_ptr< ::mosek::fusion::PSDDomain > inPSDCone(int _2605);
static  monty::rc_ptr< ::mosek::fusion::PSDDomain > inPSDCone();
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > binary();
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > binary(std::shared_ptr< monty::ndarray< int,1 > > _2606);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > binary(int _2607,int _2608);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > binary(int _2609);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(std::shared_ptr< monty::ndarray< double,1 > > _2610,std::shared_ptr< monty::ndarray< double,1 > > _2611,std::shared_ptr< monty::ndarray< int,1 > > _2612);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(std::shared_ptr< monty::ndarray< double,1 > > _2613,double _2614,std::shared_ptr< monty::ndarray< int,1 > > _2615);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(double _2617,std::shared_ptr< monty::ndarray< double,1 > > _2618,std::shared_ptr< monty::ndarray< int,1 > > _2619);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(double _2621,double _2622,std::shared_ptr< monty::ndarray< int,1 > > _2623);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(std::shared_ptr< monty::ndarray< double,1 > > _2624,std::shared_ptr< monty::ndarray< double,1 > > _2625);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(std::shared_ptr< monty::ndarray< double,1 > > _2626,double _2627);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(double _2629,std::shared_ptr< monty::ndarray< double,1 > > _2630);
static  monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(double _2632,double _2633);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(monty::rc_ptr< ::mosek::fusion::Matrix > _2634);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(std::shared_ptr< monty::ndarray< double,1 > > _2635,std::shared_ptr< monty::ndarray< int,1 > > _2636);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(std::shared_ptr< monty::ndarray< double,2 > > _2637);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(std::shared_ptr< monty::ndarray< double,1 > > _2640);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(double _2641,std::shared_ptr< monty::ndarray< int,1 > > _2642);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(double _2644,int _2645,int _2646);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(double _2648,int _2649);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(double _2651);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(monty::rc_ptr< ::mosek::fusion::Matrix > _2652);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(std::shared_ptr< monty::ndarray< double,1 > > _2653,std::shared_ptr< monty::ndarray< int,1 > > _2654);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(std::shared_ptr< monty::ndarray< double,2 > > _2655);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(std::shared_ptr< monty::ndarray< double,1 > > _2658);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(double _2659,std::shared_ptr< monty::ndarray< int,1 > > _2660);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(double _2661,int _2662,int _2663);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(double _2664,int _2665);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(double _2666);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(monty::rc_ptr< ::mosek::fusion::Matrix > _2667);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(std::shared_ptr< monty::ndarray< double,1 > > _2668,std::shared_ptr< monty::ndarray< int,1 > > _2669);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(std::shared_ptr< monty::ndarray< double,2 > > _2670);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(std::shared_ptr< monty::ndarray< double,1 > > _2673);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(double _2674,std::shared_ptr< monty::ndarray< int,1 > > _2675);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(double _2676,int _2677,int _2678);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(double _2679,int _2680);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(double _2681);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > unbounded(std::shared_ptr< monty::ndarray< int,1 > > _2682);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > unbounded(int _2684,int _2685);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > unbounded(int _2686);
static  monty::rc_ptr< ::mosek::fusion::LinearDomain > unbounded();
}; // struct Domain;

struct p_BaseExpression : public /*implements*/ virtual ::mosek::fusion::Expression
{
BaseExpression * _pubthis;
static mosek::fusion::p_BaseExpression* _get_impl(mosek::fusion::BaseExpression * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_BaseExpression * _get_impl(mosek::fusion::BaseExpression::t _inst) { return _get_impl(_inst.get()); }
p_BaseExpression(BaseExpression * _pubthis);
virtual ~p_BaseExpression() { /* std::cout << "~p_BaseExpression" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > shape{};virtual void destroy();
static BaseExpression::t _new_BaseExpression(std::shared_ptr< monty::ndarray< int,1 > > _4861);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _4861);
virtual /* override */ std::string toString() ;
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2BaseExpression__eval() ;
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expression__eval() { return __mosek_2fusion_2BaseExpression__eval(); }
static  void storeexpr(monty::rc_ptr< ::mosek::fusion::WorkStack > _4877,std::shared_ptr< monty::ndarray< int,1 > > _4878,std::shared_ptr< monty::ndarray< int,1 > > _4879,std::shared_ptr< monty::ndarray< long long,1 > > _4880,std::shared_ptr< monty::ndarray< long long,1 > > _4881,std::shared_ptr< monty::ndarray< double,1 > > _4882,std::shared_ptr< monty::ndarray< double,1 > > _4883);
virtual void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4894,monty::rc_ptr< ::mosek::fusion::WorkStack > _4895,monty::rc_ptr< ::mosek::fusion::WorkStack > _4896) { throw monty::AbstractClassError("Call to abstract method"); }
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__pick(std::shared_ptr< monty::ndarray< int,2 > > _4897) ;
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__pick(std::shared_ptr< monty::ndarray< int,2 > > _4897) { return __mosek_2fusion_2BaseExpression__pick(_4897); }
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__pick(std::shared_ptr< monty::ndarray< int,1 > > _4898) ;
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__pick(std::shared_ptr< monty::ndarray< int,1 > > _4898) { return __mosek_2fusion_2BaseExpression__pick(_4898); }
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__index(std::shared_ptr< monty::ndarray< int,1 > > _4901) ;
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__index(std::shared_ptr< monty::ndarray< int,1 > > _4901) { return __mosek_2fusion_2BaseExpression__index(_4901); }
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__index(int _4904) ;
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__index(int _4904) { return __mosek_2fusion_2BaseExpression__index(_4904); }
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__slice(std::shared_ptr< monty::ndarray< int,1 > > _4906,std::shared_ptr< monty::ndarray< int,1 > > _4907) ;
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__slice(std::shared_ptr< monty::ndarray< int,1 > > _4906,std::shared_ptr< monty::ndarray< int,1 > > _4907) { return __mosek_2fusion_2BaseExpression__slice(_4906,_4907); }
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__slice(int _4908,int _4909) ;
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__slice(int _4908,int _4909) { return __mosek_2fusion_2BaseExpression__slice(_4908,_4909); }
virtual long long getSize() ;
virtual int getND() ;
virtual int getDim(int _4910) ;
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() ;
}; // struct BaseExpression;

struct p_ExprConst : public ::mosek::fusion::p_BaseExpression
{
ExprConst * _pubthis;
static mosek::fusion::p_ExprConst* _get_impl(mosek::fusion::ExprConst * _inst){ return static_cast< mosek::fusion::p_ExprConst* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprConst * _get_impl(mosek::fusion::ExprConst::t _inst) { return _get_impl(_inst.get()); }
p_ExprConst(ExprConst * _pubthis);
virtual ~p_ExprConst() { /* std::cout << "~p_ExprConst" << std::endl;*/ };
std::shared_ptr< monty::ndarray< long long,1 > > sparsity{};std::shared_ptr< monty::ndarray< double,1 > > bfix{};virtual void destroy();
static ExprConst::t _new_ExprConst(std::shared_ptr< monty::ndarray< int,1 > > _2687,std::shared_ptr< monty::ndarray< long long,1 > > _2688,std::shared_ptr< monty::ndarray< double,1 > > _2689);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _2687,std::shared_ptr< monty::ndarray< long long,1 > > _2688,std::shared_ptr< monty::ndarray< double,1 > > _2689);
static ExprConst::t _new_ExprConst(std::shared_ptr< monty::ndarray< int,1 > > _2690,std::shared_ptr< monty::ndarray< long long,1 > > _2691,double _2692);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _2690,std::shared_ptr< monty::ndarray< long long,1 > > _2691,double _2692);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _2695,monty::rc_ptr< ::mosek::fusion::WorkStack > _2696,monty::rc_ptr< ::mosek::fusion::WorkStack > _2697) ;
static  void validate(std::shared_ptr< monty::ndarray< int,1 > > _2711,std::shared_ptr< monty::ndarray< double,1 > > _2712,std::shared_ptr< monty::ndarray< long long,1 > > _2713);
virtual /* override */ std::string toString() ;
}; // struct ExprConst;

struct p_ExprPick : public ::mosek::fusion::p_BaseExpression
{
ExprPick * _pubthis;
static mosek::fusion::p_ExprPick* _get_impl(mosek::fusion::ExprPick * _inst){ return static_cast< mosek::fusion::p_ExprPick* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprPick * _get_impl(mosek::fusion::ExprPick::t _inst) { return _get_impl(_inst.get()); }
p_ExprPick(ExprPick * _pubthis);
virtual ~p_ExprPick() { /* std::cout << "~p_ExprPick" << std::endl;*/ };
std::shared_ptr< monty::ndarray< long long,1 > > idxs{};monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprPick::t _new_ExprPick(monty::rc_ptr< ::mosek::fusion::Expression > _2717,std::shared_ptr< monty::ndarray< int,2 > > _2718);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _2717,std::shared_ptr< monty::ndarray< int,2 > > _2718);
static ExprPick::t _new_ExprPick(monty::rc_ptr< ::mosek::fusion::Expression > _2730,std::shared_ptr< monty::ndarray< long long,1 > > _2731);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _2730,std::shared_ptr< monty::ndarray< long long,1 > > _2731);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _2736,monty::rc_ptr< ::mosek::fusion::WorkStack > _2737,monty::rc_ptr< ::mosek::fusion::WorkStack > _2738) ;
virtual /* override */ std::string toString() ;
}; // struct ExprPick;

struct p_ExprSlice : public ::mosek::fusion::p_BaseExpression
{
ExprSlice * _pubthis;
static mosek::fusion::p_ExprSlice* _get_impl(mosek::fusion::ExprSlice * _inst){ return static_cast< mosek::fusion::p_ExprSlice* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprSlice * _get_impl(mosek::fusion::ExprSlice::t _inst) { return _get_impl(_inst.get()); }
p_ExprSlice(ExprSlice * _pubthis);
virtual ~p_ExprSlice() { /* std::cout << "~p_ExprSlice" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > last{};std::shared_ptr< monty::ndarray< int,1 > > first{};monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprSlice::t _new_ExprSlice(monty::rc_ptr< ::mosek::fusion::Expression > _2790,std::shared_ptr< monty::ndarray< int,1 > > _2791,std::shared_ptr< monty::ndarray< int,1 > > _2792);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _2790,std::shared_ptr< monty::ndarray< int,1 > > _2791,std::shared_ptr< monty::ndarray< int,1 > > _2792);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _2793,monty::rc_ptr< ::mosek::fusion::WorkStack > _2794,monty::rc_ptr< ::mosek::fusion::WorkStack > _2795) ;
static  std::shared_ptr< monty::ndarray< int,1 > > makeShape(std::shared_ptr< monty::ndarray< int,1 > > _2849,std::shared_ptr< monty::ndarray< int,1 > > _2850,std::shared_ptr< monty::ndarray< int,1 > > _2851);
virtual /* override */ std::string toString() ;
}; // struct ExprSlice;

struct p_ExprPermuteDims : public ::mosek::fusion::p_BaseExpression
{
ExprPermuteDims * _pubthis;
static mosek::fusion::p_ExprPermuteDims* _get_impl(mosek::fusion::ExprPermuteDims * _inst){ return static_cast< mosek::fusion::p_ExprPermuteDims* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprPermuteDims * _get_impl(mosek::fusion::ExprPermuteDims::t _inst) { return _get_impl(_inst.get()); }
p_ExprPermuteDims(ExprPermuteDims * _pubthis);
virtual ~p_ExprPermuteDims() { /* std::cout << "~p_ExprPermuteDims" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > dperm{};monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprPermuteDims::t _new_ExprPermuteDims(std::shared_ptr< monty::ndarray< int,1 > > _2856,monty::rc_ptr< ::mosek::fusion::Expression > _2857);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _2856,monty::rc_ptr< ::mosek::fusion::Expression > _2857);
static ExprPermuteDims::t _new_ExprPermuteDims(std::shared_ptr< monty::ndarray< int,1 > > _2863,monty::rc_ptr< ::mosek::fusion::Expression > _2864,int _2865);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _2863,monty::rc_ptr< ::mosek::fusion::Expression > _2864,int _2865);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _2866,monty::rc_ptr< ::mosek::fusion::WorkStack > _2867,monty::rc_ptr< ::mosek::fusion::WorkStack > _2868) ;
static  std::shared_ptr< monty::ndarray< int,1 > > computeshape(std::shared_ptr< monty::ndarray< int,1 > > _2920,std::shared_ptr< monty::ndarray< int,1 > > _2921);
}; // struct ExprPermuteDims;

struct p_ExprTranspose : public ::mosek::fusion::p_BaseExpression
{
ExprTranspose * _pubthis;
static mosek::fusion::p_ExprTranspose* _get_impl(mosek::fusion::ExprTranspose * _inst){ return static_cast< mosek::fusion::p_ExprTranspose* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprTranspose * _get_impl(mosek::fusion::ExprTranspose::t _inst) { return _get_impl(_inst.get()); }
p_ExprTranspose(ExprTranspose * _pubthis);
virtual ~p_ExprTranspose() { /* std::cout << "~p_ExprTranspose" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprTranspose::t _new_ExprTranspose(monty::rc_ptr< ::mosek::fusion::Expression > _2923);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _2923);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _2924,monty::rc_ptr< ::mosek::fusion::WorkStack > _2925,monty::rc_ptr< ::mosek::fusion::WorkStack > _2926) ;
virtual /* override */ std::string toString() ;
static  std::shared_ptr< monty::ndarray< int,1 > > transposeShape(std::shared_ptr< monty::ndarray< int,1 > > _2970);
}; // struct ExprTranspose;

struct p_ExprStack : public ::mosek::fusion::p_BaseExpression
{
ExprStack * _pubthis;
static mosek::fusion::p_ExprStack* _get_impl(mosek::fusion::ExprStack * _inst){ return static_cast< mosek::fusion::p_ExprStack* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprStack * _get_impl(mosek::fusion::ExprStack::t _inst) { return _get_impl(_inst.get()); }
p_ExprStack(ExprStack * _pubthis);
virtual ~p_ExprStack() { /* std::cout << "~p_ExprStack" << std::endl;*/ };
int dim{};std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > exprs{};virtual void destroy();
static ExprStack::t _new_ExprStack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _2971,int _2972);
void _initialize(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _2971,int _2972);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _2974,monty::rc_ptr< ::mosek::fusion::WorkStack > _2975,monty::rc_ptr< ::mosek::fusion::WorkStack > _2976) ;
static  std::shared_ptr< monty::ndarray< int,1 > > getshape(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _3124,int _3125);
virtual /* override */ std::string toString() ;
}; // struct ExprStack;

struct p_ExprInner : public ::mosek::fusion::p_BaseExpression
{
ExprInner * _pubthis;
static mosek::fusion::p_ExprInner* _get_impl(mosek::fusion::ExprInner * _inst){ return static_cast< mosek::fusion::p_ExprInner* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprInner * _get_impl(mosek::fusion::ExprInner::t _inst) { return _get_impl(_inst.get()); }
p_ExprInner(ExprInner * _pubthis);
virtual ~p_ExprInner() { /* std::cout << "~p_ExprInner" << std::endl;*/ };
std::shared_ptr< monty::ndarray< double,1 > > vcof{};std::shared_ptr< monty::ndarray< long long,1 > > vsub{};monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprInner::t _new_ExprInner(monty::rc_ptr< ::mosek::fusion::Expression > _3139,std::shared_ptr< monty::ndarray< long long,1 > > _3140,std::shared_ptr< monty::ndarray< double,1 > > _3141);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _3139,std::shared_ptr< monty::ndarray< long long,1 > > _3140,std::shared_ptr< monty::ndarray< double,1 > > _3141);
static ExprInner::t _new_ExprInner(monty::rc_ptr< ::mosek::fusion::Expression > _3147,std::shared_ptr< monty::ndarray< double,1 > > _3148);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _3147,std::shared_ptr< monty::ndarray< double,1 > > _3148);
static ExprInner::t _new_ExprInner(monty::rc_ptr< ::mosek::fusion::Expression > _3150,std::shared_ptr< monty::ndarray< int,2 > > _3151,std::shared_ptr< monty::ndarray< double,1 > > _3152);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _3150,std::shared_ptr< monty::ndarray< int,2 > > _3151,std::shared_ptr< monty::ndarray< double,1 > > _3152);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _3153,monty::rc_ptr< ::mosek::fusion::WorkStack > _3154,monty::rc_ptr< ::mosek::fusion::WorkStack > _3155) ;
static  std::shared_ptr< monty::ndarray< long long,1 > > range(int _3191);
static  std::shared_ptr< monty::ndarray< long long,1 > > convert(std::shared_ptr< monty::ndarray< int,1 > > _3193,std::shared_ptr< monty::ndarray< int,2 > > _3194);
virtual /* override */ std::string toString() ;
}; // struct ExprInner;

struct p_ExprMulDiagRight : public ::mosek::fusion::p_BaseExpression
{
ExprMulDiagRight * _pubthis;
static mosek::fusion::p_ExprMulDiagRight* _get_impl(mosek::fusion::ExprMulDiagRight * _inst){ return static_cast< mosek::fusion::p_ExprMulDiagRight* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprMulDiagRight * _get_impl(mosek::fusion::ExprMulDiagRight::t _inst) { return _get_impl(_inst.get()); }
p_ExprMulDiagRight(ExprMulDiagRight * _pubthis);
virtual ~p_ExprMulDiagRight() { /* std::cout << "~p_ExprMulDiagRight" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > expr{};std::shared_ptr< monty::ndarray< double,1 > > mval{};std::shared_ptr< monty::ndarray< int,1 > > msubj{};std::shared_ptr< monty::ndarray< int,1 > > msubi{};int mdim1{};int mdim0{};virtual void destroy();
static ExprMulDiagRight::t _new_ExprMulDiagRight(int _3201,int _3202,std::shared_ptr< monty::ndarray< int,1 > > _3203,std::shared_ptr< monty::ndarray< int,1 > > _3204,std::shared_ptr< monty::ndarray< double,1 > > _3205,monty::rc_ptr< ::mosek::fusion::Expression > _3206,int _3207);
void _initialize(int _3201,int _3202,std::shared_ptr< monty::ndarray< int,1 > > _3203,std::shared_ptr< monty::ndarray< int,1 > > _3204,std::shared_ptr< monty::ndarray< double,1 > > _3205,monty::rc_ptr< ::mosek::fusion::Expression > _3206,int _3207);
static ExprMulDiagRight::t _new_ExprMulDiagRight(int _3208,int _3209,std::shared_ptr< monty::ndarray< int,1 > > _3210,std::shared_ptr< monty::ndarray< int,1 > > _3211,std::shared_ptr< monty::ndarray< double,1 > > _3212,monty::rc_ptr< ::mosek::fusion::Expression > _3213);
void _initialize(int _3208,int _3209,std::shared_ptr< monty::ndarray< int,1 > > _3210,std::shared_ptr< monty::ndarray< int,1 > > _3211,std::shared_ptr< monty::ndarray< double,1 > > _3212,monty::rc_ptr< ::mosek::fusion::Expression > _3213);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _3214,monty::rc_ptr< ::mosek::fusion::WorkStack > _3215,monty::rc_ptr< ::mosek::fusion::WorkStack > _3216) ;
static  int validate(int _3308,int _3309,std::shared_ptr< monty::ndarray< int,1 > > _3310,std::shared_ptr< monty::ndarray< int,1 > > _3311,std::shared_ptr< monty::ndarray< double,1 > > _3312,monty::rc_ptr< ::mosek::fusion::Expression > _3313);
virtual /* override */ std::string toString() ;
}; // struct ExprMulDiagRight;

struct p_ExprMulDiagLeft : public ::mosek::fusion::p_BaseExpression
{
ExprMulDiagLeft * _pubthis;
static mosek::fusion::p_ExprMulDiagLeft* _get_impl(mosek::fusion::ExprMulDiagLeft * _inst){ return static_cast< mosek::fusion::p_ExprMulDiagLeft* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprMulDiagLeft * _get_impl(mosek::fusion::ExprMulDiagLeft::t _inst) { return _get_impl(_inst.get()); }
p_ExprMulDiagLeft(ExprMulDiagLeft * _pubthis);
virtual ~p_ExprMulDiagLeft() { /* std::cout << "~p_ExprMulDiagLeft" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > expr{};std::shared_ptr< monty::ndarray< double,1 > > mval{};std::shared_ptr< monty::ndarray< int,1 > > msubj{};std::shared_ptr< monty::ndarray< int,1 > > msubi{};int mdim1{};int mdim0{};virtual void destroy();
static ExprMulDiagLeft::t _new_ExprMulDiagLeft(int _3322,int _3323,std::shared_ptr< monty::ndarray< int,1 > > _3324,std::shared_ptr< monty::ndarray< int,1 > > _3325,std::shared_ptr< monty::ndarray< double,1 > > _3326,monty::rc_ptr< ::mosek::fusion::Expression > _3327,int _3328);
void _initialize(int _3322,int _3323,std::shared_ptr< monty::ndarray< int,1 > > _3324,std::shared_ptr< monty::ndarray< int,1 > > _3325,std::shared_ptr< monty::ndarray< double,1 > > _3326,monty::rc_ptr< ::mosek::fusion::Expression > _3327,int _3328);
static ExprMulDiagLeft::t _new_ExprMulDiagLeft(int _3329,int _3330,std::shared_ptr< monty::ndarray< int,1 > > _3331,std::shared_ptr< monty::ndarray< int,1 > > _3332,std::shared_ptr< monty::ndarray< double,1 > > _3333,monty::rc_ptr< ::mosek::fusion::Expression > _3334);
void _initialize(int _3329,int _3330,std::shared_ptr< monty::ndarray< int,1 > > _3331,std::shared_ptr< monty::ndarray< int,1 > > _3332,std::shared_ptr< monty::ndarray< double,1 > > _3333,monty::rc_ptr< ::mosek::fusion::Expression > _3334);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _3335,monty::rc_ptr< ::mosek::fusion::WorkStack > _3336,monty::rc_ptr< ::mosek::fusion::WorkStack > _3337) ;
static  int validate(int _3422,int _3423,std::shared_ptr< monty::ndarray< int,1 > > _3424,std::shared_ptr< monty::ndarray< int,1 > > _3425,std::shared_ptr< monty::ndarray< double,1 > > _3426,monty::rc_ptr< ::mosek::fusion::Expression > _3427);
virtual /* override */ std::string toString() ;
}; // struct ExprMulDiagLeft;

struct p_ExprMulElement : public ::mosek::fusion::p_BaseExpression
{
ExprMulElement * _pubthis;
static mosek::fusion::p_ExprMulElement* _get_impl(mosek::fusion::ExprMulElement * _inst){ return static_cast< mosek::fusion::p_ExprMulElement* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprMulElement * _get_impl(mosek::fusion::ExprMulElement::t _inst) { return _get_impl(_inst.get()); }
p_ExprMulElement(ExprMulElement * _pubthis);
virtual ~p_ExprMulElement() { /* std::cout << "~p_ExprMulElement" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > expr{};std::shared_ptr< monty::ndarray< long long,1 > > msp{};std::shared_ptr< monty::ndarray< double,1 > > mcof{};virtual void destroy();
static ExprMulElement::t _new_ExprMulElement(std::shared_ptr< monty::ndarray< double,1 > > _3436,std::shared_ptr< monty::ndarray< long long,1 > > _3437,monty::rc_ptr< ::mosek::fusion::Expression > _3438);
void _initialize(std::shared_ptr< monty::ndarray< double,1 > > _3436,std::shared_ptr< monty::ndarray< long long,1 > > _3437,monty::rc_ptr< ::mosek::fusion::Expression > _3438);
static ExprMulElement::t _new_ExprMulElement(std::shared_ptr< monty::ndarray< double,1 > > _3445,std::shared_ptr< monty::ndarray< long long,1 > > _3446,monty::rc_ptr< ::mosek::fusion::Expression > _3447,int _3448);
void _initialize(std::shared_ptr< monty::ndarray< double,1 > > _3445,std::shared_ptr< monty::ndarray< long long,1 > > _3446,monty::rc_ptr< ::mosek::fusion::Expression > _3447,int _3448);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _3449,monty::rc_ptr< ::mosek::fusion::WorkStack > _3450,monty::rc_ptr< ::mosek::fusion::WorkStack > _3451) ;
virtual /* override */ std::string toString() ;
}; // struct ExprMulElement;

struct p_ExprMulScalarConst : public ::mosek::fusion::p_BaseExpression
{
ExprMulScalarConst * _pubthis;
static mosek::fusion::p_ExprMulScalarConst* _get_impl(mosek::fusion::ExprMulScalarConst * _inst){ return static_cast< mosek::fusion::p_ExprMulScalarConst* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprMulScalarConst * _get_impl(mosek::fusion::ExprMulScalarConst::t _inst) { return _get_impl(_inst.get()); }
p_ExprMulScalarConst(ExprMulScalarConst * _pubthis);
virtual ~p_ExprMulScalarConst() { /* std::cout << "~p_ExprMulScalarConst" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > expr{};double c{};virtual void destroy();
static ExprMulScalarConst::t _new_ExprMulScalarConst(double _3498,monty::rc_ptr< ::mosek::fusion::Expression > _3499);
void _initialize(double _3498,monty::rc_ptr< ::mosek::fusion::Expression > _3499);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _3500,monty::rc_ptr< ::mosek::fusion::WorkStack > _3501,monty::rc_ptr< ::mosek::fusion::WorkStack > _3502) ;
virtual /* override */ std::string toString() ;
}; // struct ExprMulScalarConst;

struct p_ExprScalarMul : public ::mosek::fusion::p_BaseExpression
{
ExprScalarMul * _pubthis;
static mosek::fusion::p_ExprScalarMul* _get_impl(mosek::fusion::ExprScalarMul * _inst){ return static_cast< mosek::fusion::p_ExprScalarMul* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprScalarMul * _get_impl(mosek::fusion::ExprScalarMul::t _inst) { return _get_impl(_inst.get()); }
p_ExprScalarMul(ExprScalarMul * _pubthis);
virtual ~p_ExprScalarMul() { /* std::cout << "~p_ExprScalarMul" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > expr{};std::shared_ptr< monty::ndarray< double,1 > > mval{};std::shared_ptr< monty::ndarray< int,1 > > msubj{};std::shared_ptr< monty::ndarray< int,1 > > msubi{};int mdim1{};int mdim0{};virtual void destroy();
static ExprScalarMul::t _new_ExprScalarMul(int _3510,int _3511,std::shared_ptr< monty::ndarray< int,1 > > _3512,std::shared_ptr< monty::ndarray< int,1 > > _3513,std::shared_ptr< monty::ndarray< double,1 > > _3514,monty::rc_ptr< ::mosek::fusion::Expression > _3515,int _3516);
void _initialize(int _3510,int _3511,std::shared_ptr< monty::ndarray< int,1 > > _3512,std::shared_ptr< monty::ndarray< int,1 > > _3513,std::shared_ptr< monty::ndarray< double,1 > > _3514,monty::rc_ptr< ::mosek::fusion::Expression > _3515,int _3516);
static ExprScalarMul::t _new_ExprScalarMul(int _3517,int _3518,std::shared_ptr< monty::ndarray< int,1 > > _3519,std::shared_ptr< monty::ndarray< int,1 > > _3520,std::shared_ptr< monty::ndarray< double,1 > > _3521,monty::rc_ptr< ::mosek::fusion::Expression > _3522);
void _initialize(int _3517,int _3518,std::shared_ptr< monty::ndarray< int,1 > > _3519,std::shared_ptr< monty::ndarray< int,1 > > _3520,std::shared_ptr< monty::ndarray< double,1 > > _3521,monty::rc_ptr< ::mosek::fusion::Expression > _3522);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _3523,monty::rc_ptr< ::mosek::fusion::WorkStack > _3524,monty::rc_ptr< ::mosek::fusion::WorkStack > _3525) ;
static  int validate(int _3550,int _3551,std::shared_ptr< monty::ndarray< int,1 > > _3552,std::shared_ptr< monty::ndarray< int,1 > > _3553,std::shared_ptr< monty::ndarray< double,1 > > _3554,monty::rc_ptr< ::mosek::fusion::Expression > _3555);
virtual /* override */ std::string toString() ;
}; // struct ExprScalarMul;

struct p_ExprMulRight : public ::mosek::fusion::p_BaseExpression
{
ExprMulRight * _pubthis;
static mosek::fusion::p_ExprMulRight* _get_impl(mosek::fusion::ExprMulRight * _inst){ return static_cast< mosek::fusion::p_ExprMulRight* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprMulRight * _get_impl(mosek::fusion::ExprMulRight::t _inst) { return _get_impl(_inst.get()); }
p_ExprMulRight(ExprMulRight * _pubthis);
virtual ~p_ExprMulRight() { /* std::cout << "~p_ExprMulRight" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > expr{};std::shared_ptr< monty::ndarray< double,1 > > mval{};std::shared_ptr< monty::ndarray< int,1 > > msubj{};std::shared_ptr< monty::ndarray< int,1 > > msubi{};int mdim1{};int mdim0{};virtual void destroy();
static ExprMulRight::t _new_ExprMulRight(int _3562,int _3563,std::shared_ptr< monty::ndarray< int,1 > > _3564,std::shared_ptr< monty::ndarray< int,1 > > _3565,std::shared_ptr< monty::ndarray< double,1 > > _3566,monty::rc_ptr< ::mosek::fusion::Expression > _3567,int _3568);
void _initialize(int _3562,int _3563,std::shared_ptr< monty::ndarray< int,1 > > _3564,std::shared_ptr< monty::ndarray< int,1 > > _3565,std::shared_ptr< monty::ndarray< double,1 > > _3566,monty::rc_ptr< ::mosek::fusion::Expression > _3567,int _3568);
static ExprMulRight::t _new_ExprMulRight(int _3569,int _3570,std::shared_ptr< monty::ndarray< int,1 > > _3571,std::shared_ptr< monty::ndarray< int,1 > > _3572,std::shared_ptr< monty::ndarray< double,1 > > _3573,monty::rc_ptr< ::mosek::fusion::Expression > _3574);
void _initialize(int _3569,int _3570,std::shared_ptr< monty::ndarray< int,1 > > _3571,std::shared_ptr< monty::ndarray< int,1 > > _3572,std::shared_ptr< monty::ndarray< double,1 > > _3573,monty::rc_ptr< ::mosek::fusion::Expression > _3574);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _3575,monty::rc_ptr< ::mosek::fusion::WorkStack > _3576,monty::rc_ptr< ::mosek::fusion::WorkStack > _3577) ;
static  std::shared_ptr< monty::ndarray< int,1 > > computeshape(int _3726,std::shared_ptr< monty::ndarray< int,1 > > _3727);
static  int validate(int _3728,int _3729,std::shared_ptr< monty::ndarray< int,1 > > _3730,std::shared_ptr< monty::ndarray< int,1 > > _3731,std::shared_ptr< monty::ndarray< double,1 > > _3732,monty::rc_ptr< ::mosek::fusion::Expression > _3733);
virtual /* override */ std::string toString() ;
}; // struct ExprMulRight;

struct p_ExprMulLeft : public ::mosek::fusion::p_BaseExpression
{
ExprMulLeft * _pubthis;
static mosek::fusion::p_ExprMulLeft* _get_impl(mosek::fusion::ExprMulLeft * _inst){ return static_cast< mosek::fusion::p_ExprMulLeft* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprMulLeft * _get_impl(mosek::fusion::ExprMulLeft::t _inst) { return _get_impl(_inst.get()); }
p_ExprMulLeft(ExprMulLeft * _pubthis);
virtual ~p_ExprMulLeft() { /* std::cout << "~p_ExprMulLeft" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > expr{};std::shared_ptr< monty::ndarray< double,1 > > mval{};std::shared_ptr< monty::ndarray< int,1 > > msubj{};std::shared_ptr< monty::ndarray< int,1 > > msubi{};int mdim1{};int mdim0{};virtual void destroy();
static ExprMulLeft::t _new_ExprMulLeft(int _3742,int _3743,std::shared_ptr< monty::ndarray< int,1 > > _3744,std::shared_ptr< monty::ndarray< int,1 > > _3745,std::shared_ptr< monty::ndarray< double,1 > > _3746,monty::rc_ptr< ::mosek::fusion::Expression > _3747,int _3748);
void _initialize(int _3742,int _3743,std::shared_ptr< monty::ndarray< int,1 > > _3744,std::shared_ptr< monty::ndarray< int,1 > > _3745,std::shared_ptr< monty::ndarray< double,1 > > _3746,monty::rc_ptr< ::mosek::fusion::Expression > _3747,int _3748);
static ExprMulLeft::t _new_ExprMulLeft(int _3749,int _3750,std::shared_ptr< monty::ndarray< int,1 > > _3751,std::shared_ptr< monty::ndarray< int,1 > > _3752,std::shared_ptr< monty::ndarray< double,1 > > _3753,monty::rc_ptr< ::mosek::fusion::Expression > _3754);
void _initialize(int _3749,int _3750,std::shared_ptr< monty::ndarray< int,1 > > _3751,std::shared_ptr< monty::ndarray< int,1 > > _3752,std::shared_ptr< monty::ndarray< double,1 > > _3753,monty::rc_ptr< ::mosek::fusion::Expression > _3754);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _3755,monty::rc_ptr< ::mosek::fusion::WorkStack > _3756,monty::rc_ptr< ::mosek::fusion::WorkStack > _3757) ;
static  std::shared_ptr< monty::ndarray< int,1 > > computeshape(int _3855,int _3856,std::shared_ptr< monty::ndarray< int,1 > > _3857);
static  int validate(int _3858,int _3859,std::shared_ptr< monty::ndarray< int,1 > > _3860,std::shared_ptr< monty::ndarray< int,1 > > _3861,std::shared_ptr< monty::ndarray< double,1 > > _3862,monty::rc_ptr< ::mosek::fusion::Expression > _3863);
virtual /* override */ std::string toString() ;
}; // struct ExprMulLeft;

struct p_ExprMulVar : public ::mosek::fusion::p_BaseExpression
{
ExprMulVar * _pubthis;
static mosek::fusion::p_ExprMulVar* _get_impl(mosek::fusion::ExprMulVar * _inst){ return static_cast< mosek::fusion::p_ExprMulVar* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprMulVar * _get_impl(mosek::fusion::ExprMulVar::t _inst) { return _get_impl(_inst.get()); }
p_ExprMulVar(ExprMulVar * _pubthis);
virtual ~p_ExprMulVar() { /* std::cout << "~p_ExprMulVar" << std::endl;*/ };
bool left{};monty::rc_ptr< ::mosek::fusion::Variable > x{};std::shared_ptr< monty::ndarray< double,1 > > mcof{};std::shared_ptr< monty::ndarray< int,1 > > msubj{};std::shared_ptr< monty::ndarray< int,1 > > msubi{};int mdimj{};int mdimi{};virtual void destroy();
static ExprMulVar::t _new_ExprMulVar(bool _3871,int _3872,int _3873,std::shared_ptr< monty::ndarray< int,1 > > _3874,std::shared_ptr< monty::ndarray< int,1 > > _3875,std::shared_ptr< monty::ndarray< double,1 > > _3876,monty::rc_ptr< ::mosek::fusion::Variable > _3877);
void _initialize(bool _3871,int _3872,int _3873,std::shared_ptr< monty::ndarray< int,1 > > _3874,std::shared_ptr< monty::ndarray< int,1 > > _3875,std::shared_ptr< monty::ndarray< double,1 > > _3876,monty::rc_ptr< ::mosek::fusion::Variable > _3877);
static ExprMulVar::t _new_ExprMulVar(bool _3880,int _3881,int _3882,std::shared_ptr< monty::ndarray< int,1 > > _3883,std::shared_ptr< monty::ndarray< int,1 > > _3884,std::shared_ptr< monty::ndarray< double,1 > > _3885,monty::rc_ptr< ::mosek::fusion::Variable > _3886,int _3887);
void _initialize(bool _3880,int _3881,int _3882,std::shared_ptr< monty::ndarray< int,1 > > _3883,std::shared_ptr< monty::ndarray< int,1 > > _3884,std::shared_ptr< monty::ndarray< double,1 > > _3885,monty::rc_ptr< ::mosek::fusion::Variable > _3886,int _3887);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _3888,monty::rc_ptr< ::mosek::fusion::WorkStack > _3889,monty::rc_ptr< ::mosek::fusion::WorkStack > _3890) ;
virtual void eval_right(monty::rc_ptr< ::mosek::fusion::WorkStack > _3891,monty::rc_ptr< ::mosek::fusion::WorkStack > _3892,monty::rc_ptr< ::mosek::fusion::WorkStack > _3893) ;
virtual void eval_left(monty::rc_ptr< ::mosek::fusion::WorkStack > _3999,monty::rc_ptr< ::mosek::fusion::WorkStack > _4000,monty::rc_ptr< ::mosek::fusion::WorkStack > _4001) ;
virtual void validate(int _4076,int _4077,std::shared_ptr< monty::ndarray< int,1 > > _4078,std::shared_ptr< monty::ndarray< int,1 > > _4079,std::shared_ptr< monty::ndarray< double,1 > > _4080) ;
static  std::shared_ptr< monty::ndarray< int,1 > > resshape(int _4084,int _4085,std::shared_ptr< monty::ndarray< int,1 > > _4086,bool _4087);
virtual /* override */ std::string toString() ;
}; // struct ExprMulVar;

struct p_ExprMulScalarVar : public ::mosek::fusion::p_BaseExpression
{
ExprMulScalarVar * _pubthis;
static mosek::fusion::p_ExprMulScalarVar* _get_impl(mosek::fusion::ExprMulScalarVar * _inst){ return static_cast< mosek::fusion::p_ExprMulScalarVar* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprMulScalarVar * _get_impl(mosek::fusion::ExprMulScalarVar::t _inst) { return _get_impl(_inst.get()); }
p_ExprMulScalarVar(ExprMulScalarVar * _pubthis);
virtual ~p_ExprMulScalarVar() { /* std::cout << "~p_ExprMulScalarVar" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Variable > x{};std::shared_ptr< monty::ndarray< double,1 > > mcof{};std::shared_ptr< monty::ndarray< int,1 > > msubj{};std::shared_ptr< monty::ndarray< int,1 > > msubi{};int mdimj{};int mdimi{};virtual void destroy();
static ExprMulScalarVar::t _new_ExprMulScalarVar(int _4088,int _4089,std::shared_ptr< monty::ndarray< int,1 > > _4090,std::shared_ptr< monty::ndarray< int,1 > > _4091,std::shared_ptr< monty::ndarray< double,1 > > _4092,monty::rc_ptr< ::mosek::fusion::Variable > _4093);
void _initialize(int _4088,int _4089,std::shared_ptr< monty::ndarray< int,1 > > _4090,std::shared_ptr< monty::ndarray< int,1 > > _4091,std::shared_ptr< monty::ndarray< double,1 > > _4092,monty::rc_ptr< ::mosek::fusion::Variable > _4093);
static ExprMulScalarVar::t _new_ExprMulScalarVar(int _4098,int _4099,std::shared_ptr< monty::ndarray< int,1 > > _4100,std::shared_ptr< monty::ndarray< int,1 > > _4101,std::shared_ptr< monty::ndarray< double,1 > > _4102,monty::rc_ptr< ::mosek::fusion::Variable > _4103,int _4104);
void _initialize(int _4098,int _4099,std::shared_ptr< monty::ndarray< int,1 > > _4100,std::shared_ptr< monty::ndarray< int,1 > > _4101,std::shared_ptr< monty::ndarray< double,1 > > _4102,monty::rc_ptr< ::mosek::fusion::Variable > _4103,int _4104);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4105,monty::rc_ptr< ::mosek::fusion::WorkStack > _4106,monty::rc_ptr< ::mosek::fusion::WorkStack > _4107) ;
virtual /* override */ std::string toString() ;
}; // struct ExprMulScalarVar;

struct p_ExprMulVarScalarConst : public ::mosek::fusion::p_BaseExpression
{
ExprMulVarScalarConst * _pubthis;
static mosek::fusion::p_ExprMulVarScalarConst* _get_impl(mosek::fusion::ExprMulVarScalarConst * _inst){ return static_cast< mosek::fusion::p_ExprMulVarScalarConst* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprMulVarScalarConst * _get_impl(mosek::fusion::ExprMulVarScalarConst::t _inst) { return _get_impl(_inst.get()); }
p_ExprMulVarScalarConst(ExprMulVarScalarConst * _pubthis);
virtual ~p_ExprMulVarScalarConst() { /* std::cout << "~p_ExprMulVarScalarConst" << std::endl;*/ };
double c{};monty::rc_ptr< ::mosek::fusion::Variable > x{};virtual void destroy();
static ExprMulVarScalarConst::t _new_ExprMulVarScalarConst(monty::rc_ptr< ::mosek::fusion::Variable > _4126,double _4127);
void _initialize(monty::rc_ptr< ::mosek::fusion::Variable > _4126,double _4127);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4128,monty::rc_ptr< ::mosek::fusion::WorkStack > _4129,monty::rc_ptr< ::mosek::fusion::WorkStack > _4130) ;
virtual /* override */ std::string toString() ;
}; // struct ExprMulVarScalarConst;

struct p_ExprAdd : public ::mosek::fusion::p_BaseExpression
{
ExprAdd * _pubthis;
static mosek::fusion::p_ExprAdd* _get_impl(mosek::fusion::ExprAdd * _inst){ return static_cast< mosek::fusion::p_ExprAdd* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprAdd * _get_impl(mosek::fusion::ExprAdd::t _inst) { return _get_impl(_inst.get()); }
p_ExprAdd(ExprAdd * _pubthis);
virtual ~p_ExprAdd() { /* std::cout << "~p_ExprAdd" << std::endl;*/ };
double m2{};double m1{};monty::rc_ptr< ::mosek::fusion::Expression > e2{};monty::rc_ptr< ::mosek::fusion::Expression > e1{};virtual void destroy();
static ExprAdd::t _new_ExprAdd(monty::rc_ptr< ::mosek::fusion::Expression > _4148,monty::rc_ptr< ::mosek::fusion::Expression > _4149,double _4150,double _4151);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _4148,monty::rc_ptr< ::mosek::fusion::Expression > _4149,double _4150,double _4151);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4153,monty::rc_ptr< ::mosek::fusion::WorkStack > _4154,monty::rc_ptr< ::mosek::fusion::WorkStack > _4155) ;
virtual /* override */ std::string toString() ;
}; // struct ExprAdd;

struct p_ExprWSum : public ::mosek::fusion::p_BaseExpression
{
ExprWSum * _pubthis;
static mosek::fusion::p_ExprWSum* _get_impl(mosek::fusion::ExprWSum * _inst){ return static_cast< mosek::fusion::p_ExprWSum* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprWSum * _get_impl(mosek::fusion::ExprWSum::t _inst) { return _get_impl(_inst.get()); }
p_ExprWSum(ExprWSum * _pubthis);
virtual ~p_ExprWSum() { /* std::cout << "~p_ExprWSum" << std::endl;*/ };
std::shared_ptr< monty::ndarray< double,1 > > w{};std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > es{};virtual void destroy();
static ExprWSum::t _new_ExprWSum(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _4265,std::shared_ptr< monty::ndarray< double,1 > > _4266);
void _initialize(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _4265,std::shared_ptr< monty::ndarray< double,1 > > _4266);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4273,monty::rc_ptr< ::mosek::fusion::WorkStack > _4274,monty::rc_ptr< ::mosek::fusion::WorkStack > _4275) ;
virtual /* override */ std::string toString() ;
}; // struct ExprWSum;

struct p_ExprSumReduce : public ::mosek::fusion::p_BaseExpression
{
ExprSumReduce * _pubthis;
static mosek::fusion::p_ExprSumReduce* _get_impl(mosek::fusion::ExprSumReduce * _inst){ return static_cast< mosek::fusion::p_ExprSumReduce* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprSumReduce * _get_impl(mosek::fusion::ExprSumReduce::t _inst) { return _get_impl(_inst.get()); }
p_ExprSumReduce(ExprSumReduce * _pubthis);
virtual ~p_ExprSumReduce() { /* std::cout << "~p_ExprSumReduce" << std::endl;*/ };
int dim{};monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprSumReduce::t _new_ExprSumReduce(int _4342,monty::rc_ptr< ::mosek::fusion::Expression > _4343);
void _initialize(int _4342,monty::rc_ptr< ::mosek::fusion::Expression > _4343);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4345,monty::rc_ptr< ::mosek::fusion::WorkStack > _4346,monty::rc_ptr< ::mosek::fusion::WorkStack > _4347) ;
static  std::shared_ptr< monty::ndarray< int,1 > > computeShape(int _4451,std::shared_ptr< monty::ndarray< int,1 > > _4452);
virtual /* override */ std::string toString() ;
}; // struct ExprSumReduce;

struct p_ExprDenseTril : public ::mosek::fusion::p_BaseExpression
{
ExprDenseTril * _pubthis;
static mosek::fusion::p_ExprDenseTril* _get_impl(mosek::fusion::ExprDenseTril * _inst){ return static_cast< mosek::fusion::p_ExprDenseTril* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprDenseTril * _get_impl(mosek::fusion::ExprDenseTril::t _inst) { return _get_impl(_inst.get()); }
p_ExprDenseTril(ExprDenseTril * _pubthis);
virtual ~p_ExprDenseTril() { /* std::cout << "~p_ExprDenseTril" << std::endl;*/ };
int dim1{};int dim0{};monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprDenseTril::t _new_ExprDenseTril(int _4456,int _4457,monty::rc_ptr< ::mosek::fusion::Expression > _4458,int _4459);
void _initialize(int _4456,int _4457,monty::rc_ptr< ::mosek::fusion::Expression > _4458,int _4459);
static ExprDenseTril::t _new_ExprDenseTril(int _4460,int _4461,monty::rc_ptr< ::mosek::fusion::Expression > _4462);
void _initialize(int _4460,int _4461,monty::rc_ptr< ::mosek::fusion::Expression > _4462);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4464,monty::rc_ptr< ::mosek::fusion::WorkStack > _4465,monty::rc_ptr< ::mosek::fusion::WorkStack > _4466) ;
}; // struct ExprDenseTril;

struct p_ExprDense : public ::mosek::fusion::p_BaseExpression
{
ExprDense * _pubthis;
static mosek::fusion::p_ExprDense* _get_impl(mosek::fusion::ExprDense * _inst){ return static_cast< mosek::fusion::p_ExprDense* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprDense * _get_impl(mosek::fusion::ExprDense::t _inst) { return _get_impl(_inst.get()); }
p_ExprDense(ExprDense * _pubthis);
virtual ~p_ExprDense() { /* std::cout << "~p_ExprDense" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprDense::t _new_ExprDense(monty::rc_ptr< ::mosek::fusion::Expression > _4539);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _4539);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4540,monty::rc_ptr< ::mosek::fusion::WorkStack > _4541,monty::rc_ptr< ::mosek::fusion::WorkStack > _4542) ;
virtual /* override */ std::string toString() ;
}; // struct ExprDense;

struct p_ExprSymmetrize : public ::mosek::fusion::p_BaseExpression
{
ExprSymmetrize * _pubthis;
static mosek::fusion::p_ExprSymmetrize* _get_impl(mosek::fusion::ExprSymmetrize * _inst){ return static_cast< mosek::fusion::p_ExprSymmetrize* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprSymmetrize * _get_impl(mosek::fusion::ExprSymmetrize::t _inst) { return _get_impl(_inst.get()); }
p_ExprSymmetrize(ExprSymmetrize * _pubthis);
virtual ~p_ExprSymmetrize() { /* std::cout << "~p_ExprSymmetrize" << std::endl;*/ };
int dim1{};int dim0{};monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprSymmetrize::t _new_ExprSymmetrize(int _4565,int _4566,monty::rc_ptr< ::mosek::fusion::Expression > _4567,int _4568);
void _initialize(int _4565,int _4566,monty::rc_ptr< ::mosek::fusion::Expression > _4567,int _4568);
static ExprSymmetrize::t _new_ExprSymmetrize(int _4569,int _4570,monty::rc_ptr< ::mosek::fusion::Expression > _4571);
void _initialize(int _4569,int _4570,monty::rc_ptr< ::mosek::fusion::Expression > _4571);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4573,monty::rc_ptr< ::mosek::fusion::WorkStack > _4574,monty::rc_ptr< ::mosek::fusion::WorkStack > _4575) ;
}; // struct ExprSymmetrize;

struct p_ExprCompress : public ::mosek::fusion::p_BaseExpression
{
ExprCompress * _pubthis;
static mosek::fusion::p_ExprCompress* _get_impl(mosek::fusion::ExprCompress * _inst){ return static_cast< mosek::fusion::p_ExprCompress* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprCompress * _get_impl(mosek::fusion::ExprCompress::t _inst) { return _get_impl(_inst.get()); }
p_ExprCompress(ExprCompress * _pubthis);
virtual ~p_ExprCompress() { /* std::cout << "~p_ExprCompress" << std::endl;*/ };
double eps{};monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprCompress::t _new_ExprCompress(monty::rc_ptr< ::mosek::fusion::Expression > _4674);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _4674);
static ExprCompress::t _new_ExprCompress(monty::rc_ptr< ::mosek::fusion::Expression > _4675,double _4676);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _4675,double _4676);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4677,monty::rc_ptr< ::mosek::fusion::WorkStack > _4678,monty::rc_ptr< ::mosek::fusion::WorkStack > _4679) ;
static  void arg_sort(monty::rc_ptr< ::mosek::fusion::WorkStack > _4745,monty::rc_ptr< ::mosek::fusion::WorkStack > _4746,int _4747,int _4748,int _4749,int _4750,int _4751);
static  void merge_sort(int _4787,int _4788,int _4789,int _4790,int _4791,int _4792,std::shared_ptr< monty::ndarray< int,1 > > _4793,std::shared_ptr< monty::ndarray< long long,1 > > _4794);
virtual /* override */ std::string toString() ;
}; // struct ExprCompress;

struct p_ExprCondense : public ::mosek::fusion::p_BaseExpression
{
ExprCondense * _pubthis;
static mosek::fusion::p_ExprCondense* _get_impl(mosek::fusion::ExprCondense * _inst){ return static_cast< mosek::fusion::p_ExprCondense* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprCondense * _get_impl(mosek::fusion::ExprCondense::t _inst) { return _get_impl(_inst.get()); }
p_ExprCondense(ExprCondense * _pubthis);
virtual ~p_ExprCondense() { /* std::cout << "~p_ExprCondense" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > expr{};virtual void destroy();
static ExprCondense::t _new_ExprCondense(monty::rc_ptr< ::mosek::fusion::Expression > _4817);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _4817);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4818,monty::rc_ptr< ::mosek::fusion::WorkStack > _4819,monty::rc_ptr< ::mosek::fusion::WorkStack > _4820) ;
}; // struct ExprCondense;

struct p_ExprFromVar : public ::mosek::fusion::p_BaseExpression
{
ExprFromVar * _pubthis;
static mosek::fusion::p_ExprFromVar* _get_impl(mosek::fusion::ExprFromVar * _inst){ return static_cast< mosek::fusion::p_ExprFromVar* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprFromVar * _get_impl(mosek::fusion::ExprFromVar::t _inst) { return _get_impl(_inst.get()); }
p_ExprFromVar(ExprFromVar * _pubthis);
virtual ~p_ExprFromVar() { /* std::cout << "~p_ExprFromVar" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Variable > x{};virtual void destroy();
static ExprFromVar::t _new_ExprFromVar(monty::rc_ptr< ::mosek::fusion::Variable > _4827);
void _initialize(monty::rc_ptr< ::mosek::fusion::Variable > _4827);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4828,monty::rc_ptr< ::mosek::fusion::WorkStack > _4829,monty::rc_ptr< ::mosek::fusion::WorkStack > _4830) ;
virtual /* override */ std::string toString() ;
}; // struct ExprFromVar;

struct p_ExprReshape : public ::mosek::fusion::p_BaseExpression
{
ExprReshape * _pubthis;
static mosek::fusion::p_ExprReshape* _get_impl(mosek::fusion::ExprReshape * _inst){ return static_cast< mosek::fusion::p_ExprReshape* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_ExprReshape * _get_impl(mosek::fusion::ExprReshape::t _inst) { return _get_impl(_inst.get()); }
p_ExprReshape(ExprReshape * _pubthis);
virtual ~p_ExprReshape() { /* std::cout << "~p_ExprReshape" << std::endl;*/ };
monty::rc_ptr< ::mosek::fusion::Expression > e{};virtual void destroy();
static ExprReshape::t _new_ExprReshape(std::shared_ptr< monty::ndarray< int,1 > > _4848,monty::rc_ptr< ::mosek::fusion::Expression > _4849);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _4848,monty::rc_ptr< ::mosek::fusion::Expression > _4849);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _4851,monty::rc_ptr< ::mosek::fusion::WorkStack > _4852,monty::rc_ptr< ::mosek::fusion::WorkStack > _4853) ;
virtual /* override */ std::string toString() ;
}; // struct ExprReshape;

struct p_Expr : public ::mosek::fusion::p_BaseExpression
{
Expr * _pubthis;
static mosek::fusion::p_Expr* _get_impl(mosek::fusion::Expr * _inst){ return static_cast< mosek::fusion::p_Expr* >(mosek::fusion::p_BaseExpression::_get_impl(_inst)); }
static mosek::fusion::p_Expr * _get_impl(mosek::fusion::Expr::t _inst) { return _get_impl(_inst.get()); }
p_Expr(Expr * _pubthis);
virtual ~p_Expr() { /* std::cout << "~p_Expr" << std::endl;*/ };
std::shared_ptr< monty::ndarray< long long,1 > > inst{};std::shared_ptr< monty::ndarray< double,1 > > cof_v{};std::shared_ptr< monty::ndarray< long long,1 > > subj{};std::shared_ptr< monty::ndarray< long long,1 > > ptrb{};std::shared_ptr< monty::ndarray< double,1 > > bfix{};std::shared_ptr< monty::ndarray< int,1 > > shape{};virtual void destroy();
static Expr::t _new_Expr(std::shared_ptr< monty::ndarray< long long,1 > > _4985,std::shared_ptr< monty::ndarray< long long,1 > > _4986,std::shared_ptr< monty::ndarray< double,1 > > _4987,std::shared_ptr< monty::ndarray< double,1 > > _4988,std::shared_ptr< monty::ndarray< int,1 > > _4989,std::shared_ptr< monty::ndarray< long long,1 > > _4990);
void _initialize(std::shared_ptr< monty::ndarray< long long,1 > > _4985,std::shared_ptr< monty::ndarray< long long,1 > > _4986,std::shared_ptr< monty::ndarray< double,1 > > _4987,std::shared_ptr< monty::ndarray< double,1 > > _4988,std::shared_ptr< monty::ndarray< int,1 > > _4989,std::shared_ptr< monty::ndarray< long long,1 > > _4990);
static Expr::t _new_Expr(std::shared_ptr< monty::ndarray< long long,1 > > _5001,std::shared_ptr< monty::ndarray< long long,1 > > _5002,std::shared_ptr< monty::ndarray< double,1 > > _5003,std::shared_ptr< monty::ndarray< double,1 > > _5004,std::shared_ptr< monty::ndarray< int,1 > > _5005,std::shared_ptr< monty::ndarray< long long,1 > > _5006,int _5007);
void _initialize(std::shared_ptr< monty::ndarray< long long,1 > > _5001,std::shared_ptr< monty::ndarray< long long,1 > > _5002,std::shared_ptr< monty::ndarray< double,1 > > _5003,std::shared_ptr< monty::ndarray< double,1 > > _5004,std::shared_ptr< monty::ndarray< int,1 > > _5005,std::shared_ptr< monty::ndarray< long long,1 > > _5006,int _5007);
static Expr::t _new_Expr(monty::rc_ptr< ::mosek::fusion::Expression > _5008);
void _initialize(monty::rc_ptr< ::mosek::fusion::Expression > _5008);
virtual long long prod(std::shared_ptr< monty::ndarray< int,1 > > _5033) ;
static  std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > varstack(std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > >,1 > > _5036);
static  std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > varstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > _5039,std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > _5040);
static  monty::rc_ptr< ::mosek::fusion::Expression > condense(monty::rc_ptr< ::mosek::fusion::Expression > _5044);
static  monty::rc_ptr< ::mosek::fusion::Expression > flatten(monty::rc_ptr< ::mosek::fusion::Expression > _5045);
static  monty::rc_ptr< ::mosek::fusion::Expression > reshape(monty::rc_ptr< ::mosek::fusion::Expression > _5046,int _5047,int _5048);
static  monty::rc_ptr< ::mosek::fusion::Expression > reshape(monty::rc_ptr< ::mosek::fusion::Expression > _5049,int _5050);
static  monty::rc_ptr< ::mosek::fusion::Expression > reshape(monty::rc_ptr< ::mosek::fusion::Expression > _5051,std::shared_ptr< monty::ndarray< int,1 > > _5052);
virtual long long size() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expr__eval() ;
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2BaseExpression__eval() { return __mosek_2fusion_2Expr__eval(); }
static  monty::rc_ptr< ::mosek::fusion::Expression > zeros(std::shared_ptr< monty::ndarray< int,1 > > _5055);
static  monty::rc_ptr< ::mosek::fusion::Expression > zeros(int _5056);
static  monty::rc_ptr< ::mosek::fusion::Expression > ones();
static  monty::rc_ptr< ::mosek::fusion::Expression > ones(std::shared_ptr< monty::ndarray< int,1 > > _5057,std::shared_ptr< monty::ndarray< int,2 > > _5058);
static  monty::rc_ptr< ::mosek::fusion::Expression > ones(std::shared_ptr< monty::ndarray< int,1 > > _5059);
static  monty::rc_ptr< ::mosek::fusion::Expression > ones(int _5060);
static  monty::rc_ptr< ::mosek::fusion::Expression > constTerm(monty::rc_ptr< ::mosek::fusion::NDSparseArray > _5061);
static  monty::rc_ptr< ::mosek::fusion::Expression > constTerm(monty::rc_ptr< ::mosek::fusion::Matrix > _5062);
static  monty::rc_ptr< ::mosek::fusion::Expression > constTerm(double _5071);
static  monty::rc_ptr< ::mosek::fusion::Expression > constTerm(std::shared_ptr< monty::ndarray< int,1 > > _5072,std::shared_ptr< monty::ndarray< int,2 > > _5073,double _5074);
static  monty::rc_ptr< ::mosek::fusion::Expression > constTerm(std::shared_ptr< monty::ndarray< int,1 > > _5082,std::shared_ptr< monty::ndarray< int,2 > > _5083,std::shared_ptr< monty::ndarray< double,1 > > _5084);
static  monty::rc_ptr< ::mosek::fusion::Expression > constTerm(std::shared_ptr< monty::ndarray< int,1 > > _5092,double _5093);
static  monty::rc_ptr< ::mosek::fusion::Expression > constTerm(int _5094,double _5095);
static  monty::rc_ptr< ::mosek::fusion::Expression > constTerm(std::shared_ptr< monty::ndarray< double,2 > > _5097);
static  monty::rc_ptr< ::mosek::fusion::Expression > constTerm(std::shared_ptr< monty::ndarray< double,1 > > _5100);
virtual long long numNonzeros() ;
static  monty::rc_ptr< ::mosek::fusion::Expression > sum(monty::rc_ptr< ::mosek::fusion::Expression > _5101,int _5102);
static  monty::rc_ptr< ::mosek::fusion::Expression > sum(monty::rc_ptr< ::mosek::fusion::Expression > _5103);
static  monty::rc_ptr< ::mosek::fusion::Expression > neg(monty::rc_ptr< ::mosek::fusion::Expression > _5104);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(bool _5105,monty::rc_ptr< ::mosek::fusion::Matrix > _5106,monty::rc_ptr< ::mosek::fusion::Expression > _5107);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Variable > _5114,monty::rc_ptr< ::mosek::fusion::Matrix > _5115);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Matrix > _5116,monty::rc_ptr< ::mosek::fusion::Variable > _5117);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Expression > _5118,monty::rc_ptr< ::mosek::fusion::Matrix > _5119);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Matrix > _5120,monty::rc_ptr< ::mosek::fusion::Expression > _5121);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Variable > _5122,std::shared_ptr< monty::ndarray< double,2 > > _5123);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Expression > _5130,std::shared_ptr< monty::ndarray< double,2 > > _5131);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(std::shared_ptr< monty::ndarray< double,2 > > _5138,monty::rc_ptr< ::mosek::fusion::Variable > _5139);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(std::shared_ptr< monty::ndarray< double,2 > > _5146,monty::rc_ptr< ::mosek::fusion::Expression > _5147);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm_(monty::rc_ptr< ::mosek::fusion::Matrix > _5154,monty::rc_ptr< ::mosek::fusion::Expression > _5155);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm_(std::shared_ptr< monty::ndarray< double,1 > > _5164,monty::rc_ptr< ::mosek::fusion::Expression > _5165);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm_(monty::rc_ptr< ::mosek::fusion::NDSparseArray > _5167,monty::rc_ptr< ::mosek::fusion::Expression > _5168);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Expression > _5171,double _5172);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(double _5173,monty::rc_ptr< ::mosek::fusion::Expression > _5174);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Expression > _5175,std::shared_ptr< monty::ndarray< double,1 > > _5176);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(std::shared_ptr< monty::ndarray< double,1 > > _5177,monty::rc_ptr< ::mosek::fusion::Expression > _5178);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Expression > _5179,std::shared_ptr< monty::ndarray< double,2 > > _5180);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(std::shared_ptr< monty::ndarray< double,2 > > _5181,monty::rc_ptr< ::mosek::fusion::Expression > _5182);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Expression > _5183,monty::rc_ptr< ::mosek::fusion::Matrix > _5184);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Matrix > _5185,monty::rc_ptr< ::mosek::fusion::Expression > _5186);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(bool _5187,std::shared_ptr< monty::ndarray< double,1 > > _5188,monty::rc_ptr< ::mosek::fusion::Expression > _5189);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(bool _5204,std::shared_ptr< monty::ndarray< double,2 > > _5205,monty::rc_ptr< ::mosek::fusion::Expression > _5206);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(bool _5221,monty::rc_ptr< ::mosek::fusion::Matrix > _5222,monty::rc_ptr< ::mosek::fusion::Expression > _5223);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Variable > _5232,monty::rc_ptr< ::mosek::fusion::Matrix > _5233);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Matrix > _5239,monty::rc_ptr< ::mosek::fusion::Variable > _5240);
static  monty::rc_ptr< ::mosek::fusion::Expression > mul(bool _5246,int _5247,int _5248,std::shared_ptr< monty::ndarray< int,1 > > _5249,std::shared_ptr< monty::ndarray< int,1 > > _5250,std::shared_ptr< monty::ndarray< double,1 > > _5251,monty::rc_ptr< ::mosek::fusion::Variable > _5252);
static  monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::Expression > _5254,monty::rc_ptr< ::mosek::fusion::Matrix > _5255);
static  monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::Expression > _5263,std::shared_ptr< monty::ndarray< double,2 > > _5264);
static  monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::Expression > _5268,monty::rc_ptr< ::mosek::fusion::NDSparseArray > _5269);
static  monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::Expression > _5270,std::shared_ptr< monty::ndarray< double,1 > > _5271);
static  monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::Matrix > _5276,monty::rc_ptr< ::mosek::fusion::Expression > _5277);
static  monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::NDSparseArray > _5278,monty::rc_ptr< ::mosek::fusion::Expression > _5279);
static  monty::rc_ptr< ::mosek::fusion::Expression > dot(std::shared_ptr< monty::ndarray< double,2 > > _5280,monty::rc_ptr< ::mosek::fusion::Expression > _5281);
static  monty::rc_ptr< ::mosek::fusion::Expression > dot(std::shared_ptr< monty::ndarray< double,1 > > _5282,monty::rc_ptr< ::mosek::fusion::Expression > _5283);
static  monty::rc_ptr< ::mosek::fusion::Expression > outer(std::shared_ptr< monty::ndarray< double,1 > > _5284,monty::rc_ptr< ::mosek::fusion::Expression > _5285);
static  monty::rc_ptr< ::mosek::fusion::Expression > outer(monty::rc_ptr< ::mosek::fusion::Expression > _5289,std::shared_ptr< monty::ndarray< double,1 > > _5290);
static  monty::rc_ptr< ::mosek::fusion::Expression > outer(monty::rc_ptr< ::mosek::fusion::Matrix > _5293,monty::rc_ptr< ::mosek::fusion::Variable > _5294);
static  monty::rc_ptr< ::mosek::fusion::Expression > outer(monty::rc_ptr< ::mosek::fusion::Variable > _5301,monty::rc_ptr< ::mosek::fusion::Matrix > _5302);
static  monty::rc_ptr< ::mosek::fusion::Expression > outer(std::shared_ptr< monty::ndarray< double,1 > > _5309,monty::rc_ptr< ::mosek::fusion::Variable > _5310);
static  monty::rc_ptr< ::mosek::fusion::Expression > outer(monty::rc_ptr< ::mosek::fusion::Variable > _5312,std::shared_ptr< monty::ndarray< double,1 > > _5313);
static  monty::rc_ptr< ::mosek::fusion::Expression > outer_(int _5315,std::shared_ptr< monty::ndarray< long long,1 > > _5316,std::shared_ptr< monty::ndarray< long long,1 > > _5317,std::shared_ptr< monty::ndarray< double,1 > > _5318,std::shared_ptr< monty::ndarray< double,1 > > _5319,std::shared_ptr< monty::ndarray< long long,1 > > _5320,std::shared_ptr< monty::ndarray< double,1 > > _5321,std::shared_ptr< monty::ndarray< int,1 > > _5322,int _5323,bool _5324);
static  monty::rc_ptr< ::mosek::fusion::Expression > outer_(monty::rc_ptr< ::mosek::fusion::Variable > _5354,int _5355,std::shared_ptr< monty::ndarray< double,1 > > _5356,std::shared_ptr< monty::ndarray< int,1 > > _5357,int _5358,bool _5359);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > >,1 > > _5376);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(double _5382,double _5383,double _5384);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(double _5385,double _5386,monty::rc_ptr< ::mosek::fusion::Expression > _5387);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(double _5388,monty::rc_ptr< ::mosek::fusion::Expression > _5389,double _5390);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(double _5391,monty::rc_ptr< ::mosek::fusion::Expression > _5392,monty::rc_ptr< ::mosek::fusion::Expression > _5393);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > _5394,double _5395,double _5396);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > _5397,double _5398,monty::rc_ptr< ::mosek::fusion::Expression > _5399);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > _5400,monty::rc_ptr< ::mosek::fusion::Expression > _5401,double _5402);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > _5403,monty::rc_ptr< ::mosek::fusion::Expression > _5404,monty::rc_ptr< ::mosek::fusion::Expression > _5405);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(double _5406,monty::rc_ptr< ::mosek::fusion::Expression > _5407);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > _5408,double _5409);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > _5410,monty::rc_ptr< ::mosek::fusion::Expression > _5411);
static  monty::rc_ptr< ::mosek::fusion::Expression > vstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _5412);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > _5414,monty::rc_ptr< ::mosek::fusion::Expression > _5415,monty::rc_ptr< ::mosek::fusion::Expression > _5416);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > _5417,monty::rc_ptr< ::mosek::fusion::Expression > _5418,double _5419);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > _5420,double _5421,monty::rc_ptr< ::mosek::fusion::Expression > _5422);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > _5423,double _5424,double _5425);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(double _5426,monty::rc_ptr< ::mosek::fusion::Expression > _5427,monty::rc_ptr< ::mosek::fusion::Expression > _5428);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(double _5429,monty::rc_ptr< ::mosek::fusion::Expression > _5430,double _5431);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(double _5432,double _5433,monty::rc_ptr< ::mosek::fusion::Expression > _5434);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(double _5435,monty::rc_ptr< ::mosek::fusion::Expression > _5436);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > _5437,double _5438);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > _5439,monty::rc_ptr< ::mosek::fusion::Expression > _5440);
static  monty::rc_ptr< ::mosek::fusion::Expression > hstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _5441);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5443,monty::rc_ptr< ::mosek::fusion::Expression > _5444,monty::rc_ptr< ::mosek::fusion::Expression > _5445,monty::rc_ptr< ::mosek::fusion::Expression > _5446);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5447,monty::rc_ptr< ::mosek::fusion::Expression > _5448,monty::rc_ptr< ::mosek::fusion::Expression > _5449,double _5450);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5451,monty::rc_ptr< ::mosek::fusion::Expression > _5452,double _5453,monty::rc_ptr< ::mosek::fusion::Expression > _5454);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5455,monty::rc_ptr< ::mosek::fusion::Expression > _5456,double _5457,double _5458);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5459,double _5460,monty::rc_ptr< ::mosek::fusion::Expression > _5461,monty::rc_ptr< ::mosek::fusion::Expression > _5462);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5463,double _5464,monty::rc_ptr< ::mosek::fusion::Expression > _5465,double _5466);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5467,double _5468,double _5469,monty::rc_ptr< ::mosek::fusion::Expression > _5470);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5471,double _5472,monty::rc_ptr< ::mosek::fusion::Expression > _5473);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5474,monty::rc_ptr< ::mosek::fusion::Expression > _5475,double _5476);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5477,monty::rc_ptr< ::mosek::fusion::Expression > _5478,monty::rc_ptr< ::mosek::fusion::Expression > _5479);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack(int _5480,std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _5481);
static  monty::rc_ptr< ::mosek::fusion::Expression > stack_(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _5482,int _5483);
static  std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > promote(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _5484,int _5485);
static  monty::rc_ptr< ::mosek::fusion::Expression > repeat(monty::rc_ptr< ::mosek::fusion::Expression > _5498,int _5499,int _5500);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > _5502);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > _5504);
static  monty::rc_ptr< ::mosek::fusion::Expression > add_(monty::rc_ptr< ::mosek::fusion::Expression > _5537,double _5538,monty::rc_ptr< ::mosek::fusion::Expression > _5539,double _5540);
static  monty::rc_ptr< ::mosek::fusion::Expression > transpose(monty::rc_ptr< ::mosek::fusion::Expression > _5551);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::Matrix > _5552,monty::rc_ptr< ::mosek::fusion::Expression > _5553);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::NDSparseArray > _5554,monty::rc_ptr< ::mosek::fusion::Expression > _5555);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm(std::shared_ptr< monty::ndarray< double,2 > > _5556,monty::rc_ptr< ::mosek::fusion::Expression > _5557);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm(std::shared_ptr< monty::ndarray< double,1 > > _5558,monty::rc_ptr< ::mosek::fusion::Expression > _5559);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::Expression > _5560,monty::rc_ptr< ::mosek::fusion::Matrix > _5561);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::Expression > _5562,std::shared_ptr< monty::ndarray< double,2 > > _5563);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::Expression > _5564,std::shared_ptr< monty::ndarray< double,1 > > _5565);
static  monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::Expression > _5566,monty::rc_ptr< ::mosek::fusion::NDSparseArray > _5567);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::NDSparseArray > _5568,monty::rc_ptr< ::mosek::fusion::Expression > _5569);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > _5570,monty::rc_ptr< ::mosek::fusion::NDSparseArray > _5571);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Matrix > _5572,monty::rc_ptr< ::mosek::fusion::Expression > _5573);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > _5574,monty::rc_ptr< ::mosek::fusion::Matrix > _5575);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(double _5576,monty::rc_ptr< ::mosek::fusion::Expression > _5577);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > _5578,double _5579);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(std::shared_ptr< monty::ndarray< double,2 > > _5580,monty::rc_ptr< ::mosek::fusion::Expression > _5581);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(std::shared_ptr< monty::ndarray< double,1 > > _5582,monty::rc_ptr< ::mosek::fusion::Expression > _5583);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > _5584,std::shared_ptr< monty::ndarray< double,2 > > _5585);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > _5586,std::shared_ptr< monty::ndarray< double,1 > > _5587);
static  monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > _5588,monty::rc_ptr< ::mosek::fusion::Expression > _5589);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::NDSparseArray > _5590,monty::rc_ptr< ::mosek::fusion::Expression > _5591);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > _5592,monty::rc_ptr< ::mosek::fusion::NDSparseArray > _5593);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Matrix > _5594,monty::rc_ptr< ::mosek::fusion::Expression > _5595);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > _5596,monty::rc_ptr< ::mosek::fusion::Matrix > _5597);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(double _5598,monty::rc_ptr< ::mosek::fusion::Expression > _5599);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > _5600,double _5601);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(std::shared_ptr< monty::ndarray< double,2 > > _5602,monty::rc_ptr< ::mosek::fusion::Expression > _5603);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(std::shared_ptr< monty::ndarray< double,1 > > _5604,monty::rc_ptr< ::mosek::fusion::Expression > _5605);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > _5606,std::shared_ptr< monty::ndarray< double,2 > > _5607);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > _5608,std::shared_ptr< monty::ndarray< double,1 > > _5609);
static  monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > _5610,monty::rc_ptr< ::mosek::fusion::Expression > _5611);
virtual /* override */ int getND() ;
virtual /* override */ std::shared_ptr< monty::ndarray< int,1 > > getShape() ;
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > _5612,monty::rc_ptr< ::mosek::fusion::WorkStack > _5613,monty::rc_ptr< ::mosek::fusion::WorkStack > _5614) ;
static  void validateData(std::shared_ptr< monty::ndarray< long long,1 > > _5616,std::shared_ptr< monty::ndarray< long long,1 > > _5617,std::shared_ptr< monty::ndarray< double,1 > > _5618,std::shared_ptr< monty::ndarray< double,1 > > _5619,std::shared_ptr< monty::ndarray< int,1 > > _5620,std::shared_ptr< monty::ndarray< long long,1 > > _5621);
static  monty::rc_ptr< ::mosek::fusion::Model > extractModel(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > _5634);
}; // struct Expr;

struct p_WorkStack
{
WorkStack * _pubthis;
static mosek::fusion::p_WorkStack* _get_impl(mosek::fusion::WorkStack * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_WorkStack * _get_impl(mosek::fusion::WorkStack::t _inst) { return _get_impl(_inst.get()); }
p_WorkStack(WorkStack * _pubthis);
virtual ~p_WorkStack() { /* std::cout << "~p_WorkStack" << std::endl;*/ };
int cof_base{};int bfix_base{};int nidxs_base{};int sp_base{};int shape_base{};int ptr_base{};bool hassp{};int nelem{};int nnz{};int nd{};int pf64{};int pi64{};int pi32{};std::shared_ptr< monty::ndarray< double,1 > > f64{};std::shared_ptr< monty::ndarray< long long,1 > > i64{};std::shared_ptr< monty::ndarray< int,1 > > i32{};virtual void destroy();
static WorkStack::t _new_WorkStack();
void _initialize();
virtual bool peek_hassp() ;
virtual int peek_nnz() ;
virtual int peek_nelem() ;
virtual int peek_dim(int _4911) ;
virtual int peek_nd() ;
virtual void alloc_expr(int _4912,int _4913,int _4914,bool _4915) ;
virtual void move_expr(monty::rc_ptr< ::mosek::fusion::WorkStack > _4916) ;
virtual void peek_expr() ;
virtual void pop_expr() ;
virtual void ensure_sparsity() ;
virtual void clear() ;
virtual int allocf64(int _4931) ;
virtual int alloci64(int _4933) ;
virtual int alloci32(int _4935) ;
virtual void pushf64(double _4937) ;
virtual void pushi64(long long _4938) ;
virtual void pushi32(int _4939) ;
virtual void ensuref64(int _4940) ;
virtual void ensurei64(int _4943) ;
virtual void ensurei32(int _4946) ;
virtual int popf64(int _4949) ;
virtual int popi64(int _4950) ;
virtual int popi32(int _4951) ;
virtual void popf64(int _4952,std::shared_ptr< monty::ndarray< double,1 > > _4953,int _4954) ;
virtual void popi64(int _4955,std::shared_ptr< monty::ndarray< long long,1 > > _4956,int _4957) ;
virtual void popi32(int _4958,std::shared_ptr< monty::ndarray< int,1 > > _4959,int _4960) ;
virtual double popf64() ;
virtual long long popi64() ;
virtual int popi32() ;
virtual double peekf64() ;
virtual long long peeki64() ;
virtual int peeki32() ;
virtual double peekf64(int _4961) ;
virtual long long peeki64(int _4962) ;
virtual int peeki32(int _4963) ;
}; // struct WorkStack;

struct p_SymmetricExpr
{
SymmetricExpr * _pubthis;
static mosek::fusion::p_SymmetricExpr* _get_impl(mosek::fusion::SymmetricExpr * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_SymmetricExpr * _get_impl(mosek::fusion::SymmetricExpr::t _inst) { return _get_impl(_inst.get()); }
p_SymmetricExpr(SymmetricExpr * _pubthis);
virtual ~p_SymmetricExpr() { /* std::cout << "~p_SymmetricExpr" << std::endl;*/ };
std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > xs{};monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > b{};std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::SymmetricMatrix >,1 > > Ms{};int n{};virtual void destroy();
static SymmetricExpr::t _new_SymmetricExpr(int _4964,std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::SymmetricMatrix >,1 > > _4965,std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > _4966,monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > _4967);
void _initialize(int _4964,std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::SymmetricMatrix >,1 > > _4965,std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > _4966,monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > _4967);
static  monty::rc_ptr< ::mosek::fusion::SymmetricExpr > add(monty::rc_ptr< ::mosek::fusion::SymmetricExpr > _4968,monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > _4969);
static  monty::rc_ptr< ::mosek::fusion::SymmetricExpr > mul(monty::rc_ptr< ::mosek::fusion::SymmetricExpr > _4970,double _4971);
static  monty::rc_ptr< ::mosek::fusion::SymmetricExpr > add(monty::rc_ptr< ::mosek::fusion::SymmetricExpr > _4973,monty::rc_ptr< ::mosek::fusion::SymmetricExpr > _4974);
virtual /* override */ std::string toString() ;
}; // struct SymmetricExpr;

struct p_FlatExpr
{
FlatExpr * _pubthis;
static mosek::fusion::p_FlatExpr* _get_impl(mosek::fusion::FlatExpr * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_FlatExpr * _get_impl(mosek::fusion::FlatExpr::t _inst) { return _get_impl(_inst.get()); }
p_FlatExpr(FlatExpr * _pubthis);
virtual ~p_FlatExpr() { /* std::cout << "~p_FlatExpr" << std::endl;*/ };
std::shared_ptr< monty::ndarray< long long,1 > > inst{};std::shared_ptr< monty::ndarray< int,1 > > shape{};long long nnz{};std::shared_ptr< monty::ndarray< double,1 > > cof{};std::shared_ptr< monty::ndarray< long long,1 > > subj{};std::shared_ptr< monty::ndarray< long long,1 > > ptrb{};std::shared_ptr< monty::ndarray< double,1 > > bfix{};virtual void destroy();
static FlatExpr::t _new_FlatExpr(monty::rc_ptr< ::mosek::fusion::FlatExpr > _5647);
void _initialize(monty::rc_ptr< ::mosek::fusion::FlatExpr > _5647);
static FlatExpr::t _new_FlatExpr(std::shared_ptr< monty::ndarray< double,1 > > _5648,std::shared_ptr< monty::ndarray< long long,1 > > _5649,std::shared_ptr< monty::ndarray< long long,1 > > _5650,std::shared_ptr< monty::ndarray< double,1 > > _5651,std::shared_ptr< monty::ndarray< int,1 > > _5652,std::shared_ptr< monty::ndarray< long long,1 > > _5653);
void _initialize(std::shared_ptr< monty::ndarray< double,1 > > _5648,std::shared_ptr< monty::ndarray< long long,1 > > _5649,std::shared_ptr< monty::ndarray< long long,1 > > _5650,std::shared_ptr< monty::ndarray< double,1 > > _5651,std::shared_ptr< monty::ndarray< int,1 > > _5652,std::shared_ptr< monty::ndarray< long long,1 > > _5653);
virtual /* override */ std::string toString() ;
virtual int size() ;
}; // struct FlatExpr;

struct p_SymmetricMatrix
{
SymmetricMatrix * _pubthis;
static mosek::fusion::p_SymmetricMatrix* _get_impl(mosek::fusion::SymmetricMatrix * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_SymmetricMatrix * _get_impl(mosek::fusion::SymmetricMatrix::t _inst) { return _get_impl(_inst.get()); }
p_SymmetricMatrix(SymmetricMatrix * _pubthis);
virtual ~p_SymmetricMatrix() { /* std::cout << "~p_SymmetricMatrix" << std::endl;*/ };
int nnz{};double scale{};std::shared_ptr< monty::ndarray< double,1 > > vval{};std::shared_ptr< monty::ndarray< int,1 > > vsubj{};std::shared_ptr< monty::ndarray< int,1 > > vsubi{};std::shared_ptr< monty::ndarray< double,1 > > uval{};std::shared_ptr< monty::ndarray< int,1 > > usubj{};std::shared_ptr< monty::ndarray< int,1 > > usubi{};int d1{};int d0{};virtual void destroy();
static SymmetricMatrix::t _new_SymmetricMatrix(int _5655,int _5656,std::shared_ptr< monty::ndarray< int,1 > > _5657,std::shared_ptr< monty::ndarray< int,1 > > _5658,std::shared_ptr< monty::ndarray< double,1 > > _5659,std::shared_ptr< monty::ndarray< int,1 > > _5660,std::shared_ptr< monty::ndarray< int,1 > > _5661,std::shared_ptr< monty::ndarray< double,1 > > _5662,double _5663);
void _initialize(int _5655,int _5656,std::shared_ptr< monty::ndarray< int,1 > > _5657,std::shared_ptr< monty::ndarray< int,1 > > _5658,std::shared_ptr< monty::ndarray< double,1 > > _5659,std::shared_ptr< monty::ndarray< int,1 > > _5660,std::shared_ptr< monty::ndarray< int,1 > > _5661,std::shared_ptr< monty::ndarray< double,1 > > _5662,double _5663);
static  monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > rankOne(int _5664,std::shared_ptr< monty::ndarray< int,1 > > _5665,std::shared_ptr< monty::ndarray< double,1 > > _5666);
static  monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > rankOne(std::shared_ptr< monty::ndarray< double,1 > > _5674);
static  monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > antiDiag(std::shared_ptr< monty::ndarray< double,1 > > _5682);
static  monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > diag(std::shared_ptr< monty::ndarray< double,1 > > _5689);
virtual monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > __mosek_2fusion_2SymmetricMatrix__add(monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > _5695) ;
virtual monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > __mosek_2fusion_2SymmetricMatrix__sub(monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > _5715) ;
virtual monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > __mosek_2fusion_2SymmetricMatrix__mul(double _5716) ;
virtual int getdim() ;
}; // struct SymmetricMatrix;

struct p_NDSparseArray
{
NDSparseArray * _pubthis;
static mosek::fusion::p_NDSparseArray* _get_impl(mosek::fusion::NDSparseArray * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_NDSparseArray * _get_impl(mosek::fusion::NDSparseArray::t _inst) { return _get_impl(_inst.get()); }
p_NDSparseArray(NDSparseArray * _pubthis);
virtual ~p_NDSparseArray() { /* std::cout << "~p_NDSparseArray" << std::endl;*/ };
std::shared_ptr< monty::ndarray< double,1 > > cof{};std::shared_ptr< monty::ndarray< long long,1 > > inst{};std::shared_ptr< monty::ndarray< int,1 > > dims{};long long size{};virtual void destroy();
static NDSparseArray::t _new_NDSparseArray(std::shared_ptr< monty::ndarray< int,1 > > _5717,std::shared_ptr< monty::ndarray< int,2 > > _5718,std::shared_ptr< monty::ndarray< double,1 > > _5719);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _5717,std::shared_ptr< monty::ndarray< int,2 > > _5718,std::shared_ptr< monty::ndarray< double,1 > > _5719);
static NDSparseArray::t _new_NDSparseArray(std::shared_ptr< monty::ndarray< int,1 > > _5740,std::shared_ptr< monty::ndarray< long long,1 > > _5741,std::shared_ptr< monty::ndarray< double,1 > > _5742);
void _initialize(std::shared_ptr< monty::ndarray< int,1 > > _5740,std::shared_ptr< monty::ndarray< long long,1 > > _5741,std::shared_ptr< monty::ndarray< double,1 > > _5742);
static NDSparseArray::t _new_NDSparseArray(monty::rc_ptr< ::mosek::fusion::Matrix > _5757);
void _initialize(monty::rc_ptr< ::mosek::fusion::Matrix > _5757);
static  monty::rc_ptr< ::mosek::fusion::NDSparseArray > make(monty::rc_ptr< ::mosek::fusion::Matrix > _5765);
static  monty::rc_ptr< ::mosek::fusion::NDSparseArray > make(std::shared_ptr< monty::ndarray< int,1 > > _5766,std::shared_ptr< monty::ndarray< long long,1 > > _5767,std::shared_ptr< monty::ndarray< double,1 > > _5768);
static  monty::rc_ptr< ::mosek::fusion::NDSparseArray > make(std::shared_ptr< monty::ndarray< int,1 > > _5769,std::shared_ptr< monty::ndarray< int,2 > > _5770,std::shared_ptr< monty::ndarray< double,1 > > _5771);
}; // struct NDSparseArray;

struct p_Matrix
{
Matrix * _pubthis;
static mosek::fusion::p_Matrix* _get_impl(mosek::fusion::Matrix * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_Matrix * _get_impl(mosek::fusion::Matrix::t _inst) { return _get_impl(_inst.get()); }
p_Matrix(Matrix * _pubthis);
virtual ~p_Matrix() { /* std::cout << "~p_Matrix" << std::endl;*/ };
int dimj{};int dimi{};virtual void destroy();
static Matrix::t _new_Matrix(int _5841,int _5842);
void _initialize(int _5841,int _5842);
virtual /* override */ std::string toString() ;
virtual void switchDims() ;
static  monty::rc_ptr< ::mosek::fusion::Matrix > diag(int _5844,monty::rc_ptr< ::mosek::fusion::Matrix > _5845);
static  monty::rc_ptr< ::mosek::fusion::Matrix > diag(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Matrix >,1 > > _5847);
static  monty::rc_ptr< ::mosek::fusion::Matrix > antidiag(int _5865,double _5866,int _5867);
static  monty::rc_ptr< ::mosek::fusion::Matrix > antidiag(int _5868,double _5869);
static  monty::rc_ptr< ::mosek::fusion::Matrix > diag(int _5870,double _5871,int _5872);
static  monty::rc_ptr< ::mosek::fusion::Matrix > diag(int _5873,double _5874);
static  monty::rc_ptr< ::mosek::fusion::Matrix > antidiag(std::shared_ptr< monty::ndarray< double,1 > > _5875,int _5876);
static  monty::rc_ptr< ::mosek::fusion::Matrix > antidiag(std::shared_ptr< monty::ndarray< double,1 > > _5886);
static  monty::rc_ptr< ::mosek::fusion::Matrix > diag(std::shared_ptr< monty::ndarray< double,1 > > _5887,int _5888);
static  monty::rc_ptr< ::mosek::fusion::Matrix > diag(std::shared_ptr< monty::ndarray< double,1 > > _5896);
static  monty::rc_ptr< ::mosek::fusion::Matrix > ones(int _5897,int _5898);
static  monty::rc_ptr< ::mosek::fusion::Matrix > eye(int _5899);
static  monty::rc_ptr< ::mosek::fusion::Matrix > dense(monty::rc_ptr< ::mosek::fusion::Matrix > _5901);
static  monty::rc_ptr< ::mosek::fusion::Matrix > dense(int _5902,int _5903,double _5904);
static  monty::rc_ptr< ::mosek::fusion::Matrix > dense(int _5905,int _5906,std::shared_ptr< monty::ndarray< double,1 > > _5907);
static  monty::rc_ptr< ::mosek::fusion::Matrix > dense(std::shared_ptr< monty::ndarray< double,2 > > _5908);
static  monty::rc_ptr< ::mosek::fusion::Matrix > sparse(monty::rc_ptr< ::mosek::fusion::Matrix > _5909);
static  monty::rc_ptr< ::mosek::fusion::Matrix > sparse(std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Matrix >,1 > >,1 > > _5913);
static  monty::rc_ptr< ::mosek::fusion::Matrix > sparse(std::shared_ptr< monty::ndarray< double,2 > > _5944);
static  monty::rc_ptr< ::mosek::fusion::Matrix > sparse(int _5954,int _5955);
static  monty::rc_ptr< ::mosek::fusion::Matrix > sparse(int _5956,int _5957,std::shared_ptr< monty::ndarray< int,1 > > _5958,std::shared_ptr< monty::ndarray< int,1 > > _5959,double _5960);
static  monty::rc_ptr< ::mosek::fusion::Matrix > sparse(std::shared_ptr< monty::ndarray< int,1 > > _5962,std::shared_ptr< monty::ndarray< int,1 > > _5963,double _5964);
static  monty::rc_ptr< ::mosek::fusion::Matrix > sparse(std::shared_ptr< monty::ndarray< int,1 > > _5969,std::shared_ptr< monty::ndarray< int,1 > > _5970,std::shared_ptr< monty::ndarray< double,1 > > _5971);
static  monty::rc_ptr< ::mosek::fusion::Matrix > sparse(int _5976,int _5977,std::shared_ptr< monty::ndarray< int,1 > > _5978,std::shared_ptr< monty::ndarray< int,1 > > _5979,std::shared_ptr< monty::ndarray< double,1 > > _5980);
virtual double get(int _5985,int _5986) { throw monty::AbstractClassError("Call to abstract method"); }
virtual bool isSparse() { throw monty::AbstractClassError("Call to abstract method"); }
virtual std::shared_ptr< monty::ndarray< double,1 > > getDataAsArray() { throw monty::AbstractClassError("Call to abstract method"); }
virtual void getDataAsTriplets(std::shared_ptr< monty::ndarray< int,1 > > _5987,std::shared_ptr< monty::ndarray< int,1 > > _5988,std::shared_ptr< monty::ndarray< double,1 > > _5989) { throw monty::AbstractClassError("Call to abstract method"); }
virtual monty::rc_ptr< ::mosek::fusion::Matrix > __mosek_2fusion_2Matrix__transpose() { throw monty::AbstractClassError("Call to abstract method"); }
virtual long long numNonzeros() { throw monty::AbstractClassError("Call to abstract method"); }
virtual int numColumns() ;
virtual int numRows() ;
}; // struct Matrix;

struct p_DenseMatrix : public ::mosek::fusion::p_Matrix
{
DenseMatrix * _pubthis;
static mosek::fusion::p_DenseMatrix* _get_impl(mosek::fusion::DenseMatrix * _inst){ return static_cast< mosek::fusion::p_DenseMatrix* >(mosek::fusion::p_Matrix::_get_impl(_inst)); }
static mosek::fusion::p_DenseMatrix * _get_impl(mosek::fusion::DenseMatrix::t _inst) { return _get_impl(_inst.get()); }
p_DenseMatrix(DenseMatrix * _pubthis);
virtual ~p_DenseMatrix() { /* std::cout << "~p_DenseMatrix" << std::endl;*/ };
long long nnz{};std::shared_ptr< monty::ndarray< double,1 > > data{};virtual void destroy();
static DenseMatrix::t _new_DenseMatrix(int _5772,int _5773,std::shared_ptr< monty::ndarray< double,1 > > _5774);
void _initialize(int _5772,int _5773,std::shared_ptr< monty::ndarray< double,1 > > _5774);
static DenseMatrix::t _new_DenseMatrix(monty::rc_ptr< ::mosek::fusion::Matrix > _5775);
void _initialize(monty::rc_ptr< ::mosek::fusion::Matrix > _5775);
static DenseMatrix::t _new_DenseMatrix(std::shared_ptr< monty::ndarray< double,2 > > _5780);
void _initialize(std::shared_ptr< monty::ndarray< double,2 > > _5780);
static DenseMatrix::t _new_DenseMatrix(int _5783,int _5784,double _5785);
void _initialize(int _5783,int _5784,double _5785);
virtual /* override */ std::string toString() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Matrix > __mosek_2fusion_2DenseMatrix__transpose() ;
virtual monty::rc_ptr< ::mosek::fusion::Matrix > __mosek_2fusion_2Matrix__transpose() { return __mosek_2fusion_2DenseMatrix__transpose(); }
virtual /* override */ bool isSparse() ;
virtual /* override */ std::shared_ptr< monty::ndarray< double,1 > > getDataAsArray() ;
virtual /* override */ void getDataAsTriplets(std::shared_ptr< monty::ndarray< int,1 > > _5798,std::shared_ptr< monty::ndarray< int,1 > > _5799,std::shared_ptr< monty::ndarray< double,1 > > _5800) ;
virtual /* override */ double get(int _5804,int _5805) ;
virtual /* override */ long long numNonzeros() ;
}; // struct DenseMatrix;

struct p_SparseMatrix : public ::mosek::fusion::p_Matrix
{
SparseMatrix * _pubthis;
static mosek::fusion::p_SparseMatrix* _get_impl(mosek::fusion::SparseMatrix * _inst){ return static_cast< mosek::fusion::p_SparseMatrix* >(mosek::fusion::p_Matrix::_get_impl(_inst)); }
static mosek::fusion::p_SparseMatrix * _get_impl(mosek::fusion::SparseMatrix::t _inst) { return _get_impl(_inst.get()); }
p_SparseMatrix(SparseMatrix * _pubthis);
virtual ~p_SparseMatrix() { /* std::cout << "~p_SparseMatrix" << std::endl;*/ };
long long nnz{};std::shared_ptr< monty::ndarray< double,1 > > val{};std::shared_ptr< monty::ndarray< int,1 > > subj{};std::shared_ptr< monty::ndarray< int,1 > > subi{};virtual void destroy();
static SparseMatrix::t _new_SparseMatrix(int _5806,int _5807,std::shared_ptr< monty::ndarray< int,1 > > _5808,std::shared_ptr< monty::ndarray< int,1 > > _5809,std::shared_ptr< monty::ndarray< double,1 > > _5810,long long _5811);
void _initialize(int _5806,int _5807,std::shared_ptr< monty::ndarray< int,1 > > _5808,std::shared_ptr< monty::ndarray< int,1 > > _5809,std::shared_ptr< monty::ndarray< double,1 > > _5810,long long _5811);
static SparseMatrix::t _new_SparseMatrix(int _5817,int _5818,std::shared_ptr< monty::ndarray< int,1 > > _5819,std::shared_ptr< monty::ndarray< int,1 > > _5820,std::shared_ptr< monty::ndarray< double,1 > > _5821);
void _initialize(int _5817,int _5818,std::shared_ptr< monty::ndarray< int,1 > > _5819,std::shared_ptr< monty::ndarray< int,1 > > _5820,std::shared_ptr< monty::ndarray< double,1 > > _5821);
virtual std::shared_ptr< monty::ndarray< long long,1 > > formPtrb() ;
virtual /* override */ std::string toString() ;
virtual /* override */ long long numNonzeros() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Matrix > __mosek_2fusion_2SparseMatrix__transpose() ;
virtual monty::rc_ptr< ::mosek::fusion::Matrix > __mosek_2fusion_2Matrix__transpose() { return __mosek_2fusion_2SparseMatrix__transpose(); }
virtual /* override */ bool isSparse() ;
virtual /* override */ std::shared_ptr< monty::ndarray< double,1 > > getDataAsArray() ;
virtual /* override */ void getDataAsTriplets(std::shared_ptr< monty::ndarray< int,1 > > _5833,std::shared_ptr< monty::ndarray< int,1 > > _5834,std::shared_ptr< monty::ndarray< double,1 > > _5835) ;
virtual /* override */ double get(int _5836,int _5837) ;
}; // struct SparseMatrix;

struct p_LinkedBlocks
{
LinkedBlocks * _pubthis;
static mosek::fusion::p_LinkedBlocks* _get_impl(mosek::fusion::LinkedBlocks * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_LinkedBlocks * _get_impl(mosek::fusion::LinkedBlocks::t _inst) { return _get_impl(_inst.get()); }
p_LinkedBlocks(LinkedBlocks * _pubthis);
virtual ~p_LinkedBlocks() { /* std::cout << "~p_LinkedBlocks" << std::endl;*/ };
std::shared_ptr< monty::ndarray< int,1 > > bfirst{};std::shared_ptr< monty::ndarray< int,1 > > bsize{};monty::rc_ptr< ::mosek::fusion::LinkedInts > blocks{};monty::rc_ptr< ::mosek::fusion::LinkedInts > ints{};virtual void destroy();
static LinkedBlocks::t _new_LinkedBlocks();
void _initialize();
static LinkedBlocks::t _new_LinkedBlocks(int _6013);
void _initialize(int _6013);
static LinkedBlocks::t _new_LinkedBlocks(monty::rc_ptr< ::mosek::fusion::LinkedBlocks > _6014);
void _initialize(monty::rc_ptr< ::mosek::fusion::LinkedBlocks > _6014);
virtual void free(int _6015) ;
virtual int alloc(int _6017) ;
virtual int maxidx(int _6022) ;
virtual void get(int _6023,std::shared_ptr< monty::ndarray< int,1 > > _6024,int _6025) ;
virtual int numblocks() ;
virtual int blocksize(int _6026) ;
virtual int capacity() ;
virtual bool validate() ;
}; // struct LinkedBlocks;

struct p_LinkedInts
{
LinkedInts * _pubthis;
static mosek::fusion::p_LinkedInts* _get_impl(mosek::fusion::LinkedInts * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_LinkedInts * _get_impl(mosek::fusion::LinkedInts::t _inst) { return _get_impl(_inst.get()); }
p_LinkedInts(LinkedInts * _pubthis);
virtual ~p_LinkedInts() { /* std::cout << "~p_LinkedInts" << std::endl;*/ };
int nfree{};int last_free{};int first_free{};int first_used{};std::shared_ptr< monty::ndarray< int,1 > > prev{};std::shared_ptr< monty::ndarray< int,1 > > next{};virtual void destroy();
static LinkedInts::t _new_LinkedInts(int _6027);
void _initialize(int _6027);
static LinkedInts::t _new_LinkedInts();
void _initialize();
static LinkedInts::t _new_LinkedInts(monty::rc_ptr< ::mosek::fusion::LinkedInts > _6030);
void _initialize(monty::rc_ptr< ::mosek::fusion::LinkedInts > _6030);
virtual void free(int _6031,int _6032) ;
virtual int alloc() ;
virtual int alloc(int _6038) ;
virtual void alloc(int _6039,std::shared_ptr< monty::ndarray< int,1 > > _6040,int _6041) ;
virtual void get(int _6044,int _6045,std::shared_ptr< monty::ndarray< int,1 > > _6046,int _6047) ;
virtual int maxidx(int _6050,int _6051) ;
virtual int allocblock(int _6055) ;
virtual void recap(int _6061) ;
virtual int capacity() ;
virtual bool validate() ;
}; // struct LinkedInts;

struct p_Parameters
{
Parameters * _pubthis;
static mosek::fusion::p_Parameters* _get_impl(mosek::fusion::Parameters * _inst){ assert(_inst); assert(_inst->_impl); return _inst->_impl; }
static mosek::fusion::p_Parameters * _get_impl(mosek::fusion::Parameters::t _inst) { return _get_impl(_inst.get()); }
p_Parameters(Parameters * _pubthis);
virtual ~p_Parameters() { /* std::cout << "~p_Parameters" << std::endl;*/ };
virtual void destroy();
static  void setParameter(monty::rc_ptr< ::mosek::fusion::Model > _6070,const std::string &  _6071,double _6072);
static  void setParameter(monty::rc_ptr< ::mosek::fusion::Model > _6166,const std::string &  _6167,int _6168);
static  void setParameter(monty::rc_ptr< ::mosek::fusion::Model > _6262,const std::string &  _6263,const std::string &  _6264);
static  int string_to_variabletype_value(const std::string &  _6502);
static  int string_to_nametype_value(const std::string &  _6503);
static  int string_to_iomode_value(const std::string &  _6504);
static  int string_to_presolvemode_value(const std::string &  _6505);
static  int string_to_rescode_value(const std::string &  _6506);
static  int string_to_optimizertype_value(const std::string &  _6507);
static  int string_to_iparam_value(const std::string &  _6508);
static  int string_to_dparam_value(const std::string &  _6509);
static  int string_to_mpsformat_value(const std::string &  _6510);
static  int string_to_simdegen_value(const std::string &  _6511);
static  int string_to_internal_iinf_value(const std::string &  _6512);
static  int string_to_simdupvec_value(const std::string &  _6513);
static  int string_to_startpointtype_value(const std::string &  _6514);
static  int string_to_compresstype_value(const std::string &  _6515);
static  int string_to_sparam_value(const std::string &  _6516);
static  int string_to_onoffkey_value(const std::string &  _6517);
static  int string_to_conetype_value(const std::string &  _6518);
static  int string_to_solsta_value(const std::string &  _6519);
static  int string_to_rescodetype_value(const std::string &  _6520);
static  int string_to_objsense_value(const std::string &  _6521);
static  int string_to_xmlwriteroutputtype_value(const std::string &  _6522);
static  int string_to_language_value(const std::string &  _6523);
static  int string_to_simseltype_value(const std::string &  _6524);
static  int string_to_internal_liinf_value(const std::string &  _6525);
static  int string_to_transpose_value(const std::string &  _6526);
static  int string_to_problemitem_value(const std::string &  _6527);
static  int string_to_scopr_value(const std::string &  _6528);
static  int string_to_value_value(const std::string &  _6529);
static  int string_to_uplo_value(const std::string &  _6530);
static  int string_to_feature_value(const std::string &  _6531);
static  int string_to_soltype_value(const std::string &  _6532);
static  int string_to_callbackcode_value(const std::string &  _6533);
static  int string_to_stakey_value(const std::string &  _6534);
static  int string_to_mark_value(const std::string &  _6535);
static  int string_to_problemtype_value(const std::string &  _6536);
static  int string_to_scalingtype_value(const std::string &  _6537);
static  int string_to_mionodeseltype_value(const std::string &  _6538);
static  int string_to_internal_dinf_value(const std::string &  _6539);
static  int string_to_intpnthotstart_value(const std::string &  _6540);
static  int string_to_checkconvexitytype_value(const std::string &  _6541);
static  int string_to_solitem_value(const std::string &  _6542);
static  int string_to_branchdir_value(const std::string &  _6543);
static  int string_to_scalingmethod_value(const std::string &  _6544);
static  int string_to_miocontsoltype_value(const std::string &  _6545);
static  int string_to_miomode_value(const std::string &  _6546);
static  int string_to_dinfitem_value(const std::string &  _6547);
static  int string_to_parametertype_value(const std::string &  _6548);
static  int string_to_basindtype_value(const std::string &  _6549);
static  int string_to_orderingtype_value(const std::string &  _6550);
static  int string_to_inftype_value(const std::string &  _6551);
static  int string_to_liinfitem_value(const std::string &  _6552);
static  int string_to_boundkey_value(const std::string &  _6553);
static  int string_to_prosta_value(const std::string &  _6554);
static  int string_to_dataformat_value(const std::string &  _6555);
static  int string_to_iinfitem_value(const std::string &  _6556);
static  int string_to_symmattype_value(const std::string &  _6557);
static  int string_to_streamtype_value(const std::string &  _6558);
static  int string_to_simhotstart_value(const std::string &  _6559);
static  int string_to_simreform_value(const std::string &  _6560);
static  int string_to_sensitivitytype_value(const std::string &  _6561);
static  int string_to_purify_value(const std::string &  _6562);
static  int string_to_solveform_value(const std::string &  _6563);
}; // struct Parameters;

}
}
namespace mosek
{
namespace fusion
{
namespace Utils
{
// mosek.fusion.Utils.IntMap from file 'src/fusion/cxx/IntMap_p.h'
struct p_IntMap 
{
  IntMap * _pubself;

  static p_IntMap * _get_impl(IntMap * _inst) { return _inst->_impl.get(); }

  p_IntMap(IntMap * _pubself) : _pubself(_pubself) {}

  static IntMap::t _new_IntMap() { return new IntMap(); }

  ::std::unordered_map<long long,int> m;

  bool hasItem (long long key) { return m.find(key) != m.end(); }
  int  getItem (long long key) { return m.find(key)->second; } // will probably throw something or crash of no such key
  void setItem (long long key, int val) { m[key] = val; }

  std::shared_ptr<monty::ndarray<long long,1>> keys()
  { 
    size_t size = m.size();
    auto res = std::shared_ptr<monty::ndarray<long long,1>>(new monty::ndarray<long long,1>(monty::shape((int)size)));

    ptrdiff_t i = 0;
    for (auto it = m.begin(); it != m.end(); ++it)
      (*res)[i++] = it->first;

    return res;    
  }

  std::shared_ptr<monty::ndarray<int,1>> values()
  {
    size_t size = m.size();
    auto res = std::shared_ptr<monty::ndarray<int,1>>(new monty::ndarray<int,1>(monty::shape((int)size)));

    ptrdiff_t i = 0;
    for (auto it = m.begin(); it != m.end(); ++it)
      (*res)[i++] = it->second;

    return res;
  }

  IntMap::t clone();
  IntMap::t __mosek_2fusion_2Utils_2IntMap__clone();
};



struct p_StringIntMap
{
  StringIntMap * _pubself;

  static p_StringIntMap * _get_impl(StringIntMap * _inst) { return _inst->_impl.get(); }

  p_StringIntMap(StringIntMap * _pubself) : _pubself(_pubself) {}

  static StringIntMap::t _new_StringIntMap() { return new StringIntMap(); }

  ::std::unordered_map<std::string,int> m;

  bool hasItem (const std::string & key) { return m.find(key) != m.end(); }
  int  getItem (const std::string & key) { return m.find(key)->second; } // will probably throw something or crash of no such key
  void setItem (const std::string & key, int val) { m[key] = val; }

  std::shared_ptr<monty::ndarray<std::string,1>> keys()
  {
    size_t size = m.size();
    auto res = std::shared_ptr<monty::ndarray<std::string,1>>(new monty::ndarray<std::string,1>(monty::shape((int)size)));

    ptrdiff_t i = 0;
    for (auto it = m.begin(); it != m.end(); ++it)
      (*res)[i++] = it->first;

    return res;
  }

  std::shared_ptr<monty::ndarray<int,1>> values()
  {
    size_t size = m.size();
    auto res = std::shared_ptr<monty::ndarray<int,1>>(new monty::ndarray<int,1>(monty::shape((int)size)));

    ptrdiff_t i = 0;
    for (auto it = m.begin(); it != m.end(); ++it)
      (*res)[i++] = it->second;

    return res;
  }

  StringIntMap::t clone();
  StringIntMap::t __mosek_2fusion_2Utils_2StringIntMap__clone();
};
// End of file 'src/fusion/cxx/IntMap_p.h'
// mosek.fusion.Utils.StringBuffer from file 'src/fusion/cxx/StringBuffer_p.h'
// namespace mosek::fusion::Utils
struct p_StringBuffer
{
  StringBuffer * _pubthis; 
  std::stringstream ss;

  p_StringBuffer(StringBuffer * _pubthis) : _pubthis(_pubthis) {}

  static p_StringBuffer * _get_impl(StringBuffer::t ptr) { return ptr->_impl.get(); }
  static p_StringBuffer * _get_impl(StringBuffer * ptr) { return ptr->_impl.get(); }

  static StringBuffer::t _new_StringBuffer() { return new StringBuffer(); }

  StringBuffer::t clear ();
  
  StringBuffer::t a (const monty::ndarray<std::string,1> & val);
  StringBuffer::t a (const monty::ndarray<int,1> & val);
  StringBuffer::t a (const monty::ndarray<long long,1> & val);
  StringBuffer::t a (const monty::ndarray<double,1> & val);
  

  StringBuffer::t a (const int & val);
  StringBuffer::t a (const long long & val);
  StringBuffer::t a (const double & val);
  StringBuffer::t a (const bool & val);
  StringBuffer::t a (const std::string & val);

  StringBuffer::t lf ();
  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__clear ();

  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__a (const monty::ndarray<std::string,1> & val);
  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__a (const monty::ndarray<int,1> & val);
  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__a (const monty::ndarray<long long,1> & val);
  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__a (const monty::ndarray<double,1> & val);

  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__a (const int & val);
  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__a (const long long & val);
  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__a (const double & val);
  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__a (const bool & val);
  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__a (const std::string & val);

  StringBuffer::t __mosek_2fusion_2Utils_2StringBuffer__lf ();

  std::string toString () const;
};
// End of file 'src/fusion/cxx/StringBuffer_p.h'
}
}
}
#endif
