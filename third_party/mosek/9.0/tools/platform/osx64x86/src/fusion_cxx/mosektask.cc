#include "mosektask_p.h"
#include "mosektask.h"
#include "monty_iterator.h"

#include <iostream>
#define MSK(name,...) if (MSK_RES_OK != MSK_ ## name(task,__VA_ARGS__)) throw MosekException(task)


namespace mosek
{

  MSKenv_t   Task::env;
  int        Task::env_refc;
  bool       Task::env_release;

  std::mutex Task::env_lock;

  MSKenv_t Task::getEnv(bool borrow)
  {
    std::lock_guard<std::mutex> guard(env_lock);
    if (env == NULL)
    {
      MSKrescodee r = MSK_makeenv(&env, NULL);
      env_refc = 0;
      env_release = false;
      if (r != MSK_RES_OK || env == NULL)
      {
        std::stringstream ss;
        ss << r << ": Failed to create global environment";
        throw MosekException(ss.str());
      }
    }
    if (! borrow)
        ++env_refc;
    return env;
  }

  MSKenv_t Task::getEnv()
  {
    std::lock_guard<std::mutex> guard(env_lock);
    if (env == NULL)
    {
      MSKrescodee r = MSK_makeenv(&env, NULL);
      env_refc = 0;
      env_release = false;
      if (r != MSK_RES_OK || env == NULL)
      {
        std::stringstream ss;
        ss << r << ": Failed to create global environment";
        throw MosekException(ss.str());
      }
    }
    ++env_refc;
    return env;
  }

  void Task::releaseEnv()
  {
    std::lock_guard<std::mutex> guard(env_lock);
    if (env_refc > 0)
    {
        --env_refc;
        if (env_refc == 0 && env_release)
            MSK_deleteenv(&env);
    }
  }

  void Task::releaseGlobalEnv()
  {
      std::lock_guard<std::mutex> guard(env_lock);
      env_release = true;
      if (env != NULL && env_refc == 0)
          MSK_deleteenv(&env);
  }
  void releaseGlobalEnv()
  {
      Task::releaseGlobalEnv();
  }


  void Task::msghandler(void * handle, const char * msg)
  {
    Task * self = (Task*)handle;
    if (self->msguserfunc)
      self->msguserfunc(msg);
  }

  int Task::pgshandler(MSKtask_t         task,
                        void            * handle,
                        MSKcallbackcodee  caller,
                        const double    * dinf,
                        const int       * iinf,
                        const long long * linf)
  {
    Task * self = (Task*)handle;

    /*
    if (self->pgsuserfunc)
        if (0 != self->pgsuserfunc(self->pgshandle,caller))
            self->breakflag = true;
    */
    /*
    if (! self->breakflag &&
        self->infuserfunc)
        if (0 != self->infuserfunc(self->infhandle,caller,dinf,iinf,linf))
            self->breakflag = true;
    */
    if (self->datacbuserfunc)
        if (self->datacbuserfunc(caller,dinf,iinf,(int64_t*)linf))
            self->breakflag = 1;

    if (self->cbuserfunc)
        if (self->cbuserfunc(caller))
            self->breakflag = 1;

    return self->breakflag ? 1 : 0;
  }


  static std::string extract_task_error(MSKtask_t task)
  {
    MSKrescodee lastres;
    MSKint32t   len = 0;
    MSK_getlasterror(task, & lastres, 0, &len, NULL);
    std::vector<char> buf(len+1);
    MSK_getlasterror(task, & lastres, len, NULL, &buf[0]);
    return std::string(&buf[0]);
  }

  // mosektask.h
  MosekException::MosekException(MSKtask_t task) : std::runtime_error(extract_task_error(task).c_str())
  {
  }

  Task::Task() :
    breakflag(false),
    task(nullptr),
    msguserfunc(nullptr)/*,
    infuserfunc(nullptr),
    pgsuserfunc(nullptr),
    pgshandle(nullptr),
    infhandle(nullptr)
    */
  {
    MSKrescodee r = MSK_maketask(getEnv(),0,0,&task);
    if (r != MSK_RES_OK)
    {
      std::stringstream ss; ss << r << ": " << "Failed to create native task object";
      throw MosekException(ss.str());
    }
    MSK_linkfunctotaskstream(task,MSK_STREAM_LOG, this, msghandler);
    MSK_putcallbackfunc(task,pgshandler,this);
  }

  Task::Task(Task & other) :
    breakflag(false),
    task(nullptr),
    msguserfunc(nullptr)/*,
    infuserfunc(nullptr),
    pgsuserfunc(nullptr),
    pgshandle(nullptr),
    infhandle(nullptr)
    */
  {
      MSKrescodee r = MSK_clonetask(other.task, &task);
      if (r != MSK_RES_OK)
      {
          std::stringstream ss; ss << r << ": " << "Failed to create native task object";
          throw MosekException(ss.str());
      }
      MSK_linkfunctotaskstream(task,MSK_STREAM_LOG, this, msghandler);
      MSK_putcallbackfunc(task,pgshandler,this);
  }

  Task::~Task()
  {
      dispose();
  }

  void Task::dispose()
  {
      if (task)
      {
          MSK_deletetask(&task);
          releaseEnv();
          task = NULL;
      }
  }


  MSKtask_t Task::clone() const
  {
    MSKrescodee r = MSK_RES_OK;
    MSKtask_t t2;
    r = MSK_clonetask(task,&t2);
    if (r != MSK_RES_OK)
    {
      std::stringstream ss; ss << r << ": " << "Failed to create native task object";
      throw MosekException(ss.str());
    }
    return t2;
  }








  int Task::getnumvar()    const { int num; MSK(getnumvar,&num);    return num; }
  int Task::getnumcon()    const { int num; MSK(getnumcon,&num);    return num; }
  int Task::getnumcone()   const { int num; MSK(getnumcone,&num);   return num; }
  int Task::getnumbarvar() const { int num; MSK(getnumbarvar,&num); return num; }

  int Task::getbarvardim(int j) const { int dim; MSK(getdimbarvarj, j, &dim); return dim; }

  int Task::appendvars  (int num) { int idx = getnumvar();    MSK(appendvars,num); return idx; }
  int Task::appendcons  (int num) { int idx = getnumcon();    MSK(appendcons,num); return idx; }
  int Task::appendbarvar(int dim) { int idx = getnumbarvar(); MSK(appendbarvars, 1, &dim); return idx; }
  void Task::appendcones(int numcone)
  {
      for (int i = 0; i < numcone; ++i)
          MSK(appendcone, MSK_CT_ZERO, 0.0, 0, NULL);
  }

  void Task::putcones(MSKconetypee ct, int * coneidxs, int conesize, int numcone, double * alpha, int * membs)
  {
      if (alpha != NULL)
          for (int i = 0, k = 0; i < numcone; ++i, k += conesize) {
              MSK(putcone, coneidxs[i], ct, alpha[i], conesize, membs+k);
          }
      else
          for (int i = 0, k = 0; i < numcone; ++i, k += conesize) {
              MSK(putcone, coneidxs[i], ct, 0.0, conesize, membs+k);
          }
  }
  


void Task::varbound(int idx, MSKboundkeye bk, double lb, double ub) { MSK(putvarbound, idx, bk, lb, ub); }

void Task::conbound(int idx, MSKboundkeye bk, double lb, double ub) { MSK(putconbound, idx, bk, lb, ub); }
  void Task::varboundlist
    ( size_t               num,
      const int          * sub,
      const MSKboundkeye   bk,
      const double       * lb,
      const double       * ub)
  {
      std::vector<MSKboundkeye> bks(num, bk); 
      MSK(putvarboundlist,num,sub,bks.data(),lb,ub);
  }
  void Task::conboundlist
    ( size_t               num,
      const int          * sub,
      const MSKboundkeye   bk,
      const double       * lb,
      const double       * ub)
  {
      std::vector<MSKboundkeye> bks(num, bk); 
      MSK(putconboundlist,num,sub,bks.data(),lb,ub);
  }

  void Task::varname   (int idx, const std::string & name) { MSK(putvarname,    idx, name.c_str()); }
  void Task::conname   (int idx, const std::string & name) { MSK(putconname,    idx, name.c_str()); }
  void Task::conename  (int idx, const std::string & name) { MSK(putconename,   idx, name.c_str()); }
  void Task::barvarname(int idx, const std::string & name) { MSK(putbarvarname, idx, name.c_str()); }
  void Task::objname   (         const std::string & name) { MSK(putobjname,         name.c_str()); }
  void Task::taskname  (         const std::string & name) { MSK(puttaskname,        name.c_str()); }

  void Task::generatevarnames( int num, int * subj, const std::string & format, int ndims, int * dims, long long * sp) { MSK(generatevarnames, num,subj,format.c_str(),ndims,dims,sp); }
  void Task::generateconnames( int num, int * subi, const std::string & format, int ndims, int * dims, long long * sp) { MSK(generateconnames, num,subi,format.c_str(),ndims,dims,sp); }
  void Task::generateconenames(int num, int * subk, const std::string & format, int ndims, int * dims, long long * sp) { MSK(generateconenames,num,subk,format.c_str(),ndims,dims,sp); }

  void Task::putarowlist
  ( int num,
    const int *       idxs,
    const long long * ptrb,
    const int       * subj,
    const double    * cof)
  {
      MSK(putarowlist64,num,idxs, ptrb,ptrb+1,subj,cof);
  }


  void Task::putbararowlist(int num,
                            const int       * subi,
                            const long long * ptr,
                            const int       * subj,
                            const long long * matidx) {
      std::vector<long long> nummat(ptr[num]);
      std::vector<double> alpha(ptr[num]);
      for (ptrdiff_t i = 0; i < nummat.size(); ++i) {
          nummat[i] = 1;
          alpha[i] = 1.0;
      }
          
      MSK(putbararowlist,num,subi,ptr,ptr+1,subj,nummat.data(),matidx,alpha.data());
  }
  
  void Task::putbarc(long long num,
                     const int * subj,
                     const int * subk,
                     const int * subl,
                     const double * val) {
      MSK(putbarcblocktriplet,num,subj,subk,subl,val);
  }

  void Task::putbaraijlist(long long num,
                           const int       * subi,
                           const int       * subj,
                           const long long * alphaptrb,
                           const long long * alphaptre,
                           const long long * matidx,
                           const double    * weight)
  {
      if (num > 0)
          MSK(putbaraijlist,num,subi,subj,alphaptrb,alphaptre,matidx,weight);
  }

  void Task::putaijlist
  ( const int    * subi,
    const int    * subj,
    const double * cof,
    long long      num)
  {
      if (num > 0) // bug1989
          MSK(putaijlist64,num,subi,subj,cof);
  }

  void Task::putobjsense(MSKobjsensee sense) { MSK(putobjsense, sense); }
  void Task::putclist(size_t num, const int * subj,const double * c)
  {
    MSK(putclist, (int)num, &subj[0], &c[0]);
  }
  void Task::putcfix    (double cfix) { MSK(putcfix, cfix); }

  void Task::appendsymmatlist(int num,
                              const int    * dim,
                              const long long * nz,
                              const int    * subk,
                              const int    * subl,
                              const double * val,
                              long long    * res )
  {
    MSK(appendsparsesymmatlist,num,dim,nz,subk,subl,val,res);
  }

  void Task::breakOptimize()
  {
      breakflag = true;
  }

  MSKrescodee Task::optimize()
  {
    MSKrescodee r = MSK_RES_OK;
    breakflag = false;
    MSK(optimizetrm, &r);
    return r;
  }

  void Task::solutionsummary(MSKstreamtypee strm)
  {
    MSK(solutionsummary,strm);
  }

  bool Task::solutiondef(MSKsoltypee sol)
  {
    int soldef = 0;
    MSK(solutiondef,sol,&soldef);
    return soldef != 0;
  }

  void Task::getsolution
    ( MSKsoltypee   whichsol,
      MSKprostae &  prosta,
      MSKsolstae &  solsta,
      MSKstakeye *  skc,
      MSKstakeye *  skx,
      MSKstakeye *  skn,
      double     *  xc,
      double     *  xx,
      double     *  y,
      double     *  slc,
      double     *  suc,
      double     *  slx,
      double     *  sux,
      double     *  snx)
  {
    MSK(getsolution, whichsol, &prosta, &solsta, skc,skx,skn,xc,xx,y,slc,suc,slx,sux,snx);
  }

  void Task::getbarxslice(MSKsoltypee sol, int first, int last, size_t capacity, double* barxj)
  {
      MSK(getbarxslice,sol,first,last,capacity,barxj);
  }
  void Task::getbarsslice(MSKsoltypee sol, int first, int last, size_t capacity, double* barxj)
  {
      MSK(getbarsslice,sol,first,last,capacity,barxj);
  }

  double Task::getprimalobj(MSKsoltypee sol)
  {
    double obj;
    MSK(getprimalobj,sol,&obj);
    return obj;
  }

  double Task::getdualobj(MSKsoltypee sol)
  {
    double obj;
    MSK(getdualobj,sol,&obj);
    return obj;
  }

  void      Task::putparam(const std::string & name, double value)               { MSK(putnadouparam,name.c_str(),value); }
  void      Task::putparam(const std::string & name, int    value)               { MSK(putnaintparam,name.c_str(),value); }
  void      Task::putparam(const std::string & name, const  std::string & value) { MSK(putnastrparam,name.c_str(),value.c_str()); }
  double    Task::getdinfitem (MSKdinfiteme  key) { double     val; MSK(getdouinf,key,&val);  return val; }
  int       Task::getiinfitem (MSKiinfiteme  key) { int        val; MSK(getintinf,key,&val);  return val; }
  long long Task::getliinfitem(MSKliinfiteme key) { long long  val; MSK(getlintinf,key,&val); return val; }

  void Task::putintlist(size_t num, const int * subj) { for (int j = 0; j < num; ++j) MSK(putvartype, subj[j], MSK_VAR_TYPE_INT); }
  void Task::putcontlist(size_t num, const int * subj) { for (int j = 0; j < num; ++j) MSK(putvartype, subj[j], MSK_VAR_TYPE_CONT); }
  void Task::write(const std::string & filename) { MSK(writedata,filename.c_str()); }

  void Task::putxxslice(MSKsoltypee which, int first, int last, double * xx)
  {
    MSK(putxxslice,which,first,last,xx);
  }

  void Task::fixvars(int numvar)
  {
    int tasknumvar    = getnumvar();
    if (numvar < tasknumvar)
    {
        std::vector<MSKboundkeye> bk(tasknumvar-numvar);
        std::vector<double> bnd(tasknumvar-numvar);
        MSK(putvarboundslice, numvar, tasknumvar, &bk[0], &bnd[0],&bnd[0]);
    }
  }

  void Task::removevar(int idx)
  {
      MSK(removevars, 1, &idx);
  }

  void putlicensecode(int* code)
  {
      MSKrescodee r = MSK_putlicensecode(Task::getEnv(true), code);
      if (r != MSK_RES_OK) throw MosekException("Failed to put license code");
  }

  void putlicensepath(const std::string & path)
  {
      MSKrescodee r = MSK_putlicensepath(Task::getEnv(true), path.c_str());
      if (r != MSK_RES_OK) throw MosekException("Failed to put license path");
  }

  void putlicensewait(int wait)
  {
      MSK_putlicensewait(Task::getEnv(true), wait);
  }

  void Task::revert(int numvar, int numcon, int numcone, int numbarvar)
  {
    int tasknumvar    = getnumvar();
    int tasknumcon    = getnumcon();
    int tasknumcone   = getnumcone();
    int tasknumbarvar = getnumbarvar();

    if (tasknumvar > numvar)
    {
      std::vector<int> subj(tasknumvar-numvar);
      for (int i = numvar; i < tasknumvar; ++i) subj[i-numvar] = i;
      MSK(removevars, tasknumvar-numvar, &subj[0]);
    }
    if (tasknumcone > numcone)
    {
      std::vector<int> subj(tasknumcone-numcone);
      for (int i = numcone; i < tasknumcone; ++i) subj[i-numcone] = i;
      MSK(removecones, tasknumcone-numcone, &subj[0]);
    }

    if (tasknumbarvar > numbarvar)
    {
      std::vector<int> subj(tasknumcon-numcon);
      for (int i = numbarvar; i < tasknumbarvar; ++i) subj[i-numbarvar] = i;
      MSK(removebarvars, tasknumbarvar-numbarvar, &subj[0]);
    }
    if (tasknumcon > numcon)
    {
      std::vector<int> subj(tasknumcon-numcon);
      for (int i = numcon; i < tasknumcon; ++i) subj[i-numcon] = i;
      MSK(removecons, tasknumcon-numcon, &subj[0]);
    }
  }

}




void mosek::LinAlg::axpy
( int n,
  double alpha,
  std::shared_ptr<monty::ndarray<double,1>> x,
  std::shared_ptr<monty::ndarray<double,1>> y)
{
    if (n > y->size() ||
        n > x->size())
        throw ArgumentError("Mismatching argument lengths");
    MSKrescodee r = MSK_axpy(Task::getEnv(true),n,alpha,x->raw(),y->raw());
    if (r != MSK_RES_OK)
        throw MosekException("Axpy() failed");
}

void mosek::LinAlg::dot
( int n,
  std::shared_ptr<monty::ndarray<double,1>> x,
  std::shared_ptr<monty::ndarray<double,1>> y,
  double & xty)
{
    if (n != y->size() ||
        n != x->size())
        throw ArgumentError("Mismatching argument lengths");
    MSKrescodee r = MSK_dot(Task::getEnv(true),n,x->raw(),y->raw(),&xty);
    if (r != MSK_RES_OK)
        throw MosekException("dot() failed");
}

void mosek::LinAlg::gemv
( bool     transa,
  int      m,
  int      n,
  double   alpha,
  std::shared_ptr<monty::ndarray<double,1>> a,
  std::shared_ptr<monty::ndarray<double,1>> x,
  double   beta,
  std::shared_ptr<monty::ndarray<double,1>> y)
{
    if ( m*n != a->size() ||
         m > (transa ? x->size() : y->size()) ||
         n > (transa ? y->size() : x->size()) )
        throw ArgumentError("Mismatching argument lengths");
    MSKrescodee r = MSK_gemv(Task::getEnv(true),
                             transa ? MSK_TRANSPOSE_YES : MSK_TRANSPOSE_NO,
                             m,
                             n,
                             alpha,
                             a->raw(),
                             x->raw(),
                             beta,
                             y->raw());
    if (r != MSK_RES_OK)
        throw MosekException("gemv() failed");

}

void mosek::LinAlg::gemm
( bool     transa,
  bool     transb,
  int      m,
  int      n,
  int      k,
  double   alpha,
  std::shared_ptr<monty::ndarray<double,1>> a,
  std::shared_ptr<monty::ndarray<double,1>> b,
  double   beta,
  std::shared_ptr<monty::ndarray<double,1>> c)
{
    if ( m*n != c->size() ||
         n*k != b->size() ||
         m*k != a->size() )
        throw ArgumentError("Mismatching argument lengths");

    MSKrescodee r = MSK_gemm(Task::getEnv(true),
                             transa ? MSK_TRANSPOSE_YES : MSK_TRANSPOSE_NO,
                             transb ? MSK_TRANSPOSE_YES : MSK_TRANSPOSE_NO,
                             m,
                             n,
                             k,
                             alpha,
                             a->raw(),
                             b->raw(),
                             beta,
                             c->raw());
    if (r != MSK_RES_OK)
        throw MosekException("gemm() failed");
}


void mosek::LinAlg::syrk
( MSKuploe uplo,
  bool           transa,
  int            n,
  int            k,
  double         alpha,
  std::shared_ptr<monty::ndarray<double,1>> a,
  double         beta,
  std::shared_ptr<monty::ndarray<double,1>> c)
{
    if ( n*k != a->size() ||
         n*n != c->size() )
        throw ArgumentError("Mismatching argument lengths");

    MSKrescodee r = MSK_syrk(Task::getEnv(true),
                             uplo,
                             transa ? MSK_TRANSPOSE_YES : MSK_TRANSPOSE_NO,
                             n,
                             k,
                             alpha,
                             a->raw(),
                             beta,
                             c->raw());
    if (r != MSK_RES_OK)
        throw MosekException("syrk() failed");
}

void mosek::LinAlg::syeig
( MSKuploe uplo,
  int      n,
  std::shared_ptr<monty::ndarray<double,1>> a,
  std::shared_ptr<monty::ndarray<double,1>> w)
{
    if ( n != a->size()/n ||
         n != w->size() )
        throw ArgumentError("Mismatching argument lengths");

    MSKrescodee r = MSK_syeig(Task::getEnv(true),
                              uplo,
                              n,
                              a->raw(),
                              w->raw());
    if (r != MSK_RES_OK)
        throw MosekException("syeig() failed");
}

void mosek::LinAlg::syevd
( MSKuploe uplo,
  int      n,
  std::shared_ptr<monty::ndarray<double,1>> a,
  std::shared_ptr<monty::ndarray<double,1>> w)
{
    if ( n != a->size()/n ||
         n != w->size() )
        throw ArgumentError("Mismatching argument lengths");

    MSKrescodee r = MSK_syevd(Task::getEnv(true),
                              uplo,
                              n,
                              a->raw(),
                              w->raw());
    if (r != MSK_RES_OK)
        throw MosekException("syevd() failed");
}

void mosek::LinAlg::potrf
( MSKuploe uplo,
  int      n,
  std::shared_ptr<monty::ndarray<double,1>> a)
{
    if ( n != a->size()/n )
        throw ArgumentError("Mismatching argument lengths");

    MSKrescodee r = MSK_potrf(Task::getEnv(true),
                              uplo,
                              n,
                              a->raw());
    if (r != MSK_RES_OK)
        throw MosekException("potrf() failed");
}

