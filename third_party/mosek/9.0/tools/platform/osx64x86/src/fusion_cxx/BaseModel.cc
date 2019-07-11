#include "mosek.h"
#include "mosektask.h"
#include "mosektask_p.h"
#include "fusion_p.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cassert>
#include <SolverInfo.h>

#include "mosektask_p.h"


namespace mosek 
{ 
  namespace fusion
  {
    //-----------------------------
    BaseModel::BaseModel(p_BaseModel * _impl) : _impl(_impl) 
    {
    }
    BaseModel::~BaseModel()
    {
        //std::cout << "BaseModel::~BaseModel()" << std::endl;
      delete _impl;
      _impl = nullptr;
    }

    void BaseModel::dispose()
    {
        //std::cout << "dispose(). ref count : " <<  this->refcount << std::endl;
        _impl->dispose();
    }

    //-----------------------------
    p_BaseModel::p_BaseModel(BaseModel * _pubthis) :
      synched(false),
      sol_itr(nullptr),
      sol_itg(nullptr),
      sol_bas(nullptr)
    {}

    void p_BaseModel::_initialize( const std::string & name,
                                   const std::string & licpath)
    {
        task = std::unique_ptr<Task>(new Task());
        taskname = name;
        task->taskname(name);
    }

    void p_BaseModel::_initialize( monty::rc_ptr<BaseModel> m)
    {
        task = std::unique_ptr<Task>(new Task(* (m->_impl->task)));
        task->putparam("MSK_IPAR_LOG_EXPAND",0);
        task->putparam("MSK_IPAR_REMOVE_UNUSED_SOLUTIONS",1);
        synched  = m->_impl->synched;
        taskname = m->_impl->taskname;
        sol_itr  = m->_impl->sol_itr.get() ? m->_impl->sol_itr->clone() : nullptr;
        sol_bas  = m->_impl->sol_bas.get() ? m->_impl->sol_bas->clone() : nullptr;
        sol_itg  = m->_impl->sol_itg.get() ? m->_impl->sol_itg->clone() : nullptr;
    }

    void p_BaseModel::task_setLogHandler (const msghandler_t & userfunc) { task->setStreamFunc(userfunc); }
    void p_BaseModel::task_setDataCallbackHandler (const datacbhandler_t & userfunc) { task->setDataCallbackFunc(userfunc); }
    void p_BaseModel::task_setCallbackHandler (const cbhandler_t & userfunc) { task->setCallbackFunc(userfunc); }

    /*
    int  p_BaseModel::alloc_linearvar (const std::string & name, RelationKey relkey, double bnd)
    {
      int n = task->appendvars(1);
      if (name.size() > 0)
        task->varname(n,name);

      switch (relkey)
      {
      case RelationKey::IsFree:
          task->varbound(n, MSK_BK_FR, 0.0, 0.0);
          break;
      case RelationKey::LessThan:
          task->varbound(n, MSK_BK_UP, 0.0, bnd);
          break;
      case RelationKey::GreaterThan:
          task->varbound(n, MSK_BK_LO, bnd, 0.0);
          break;
      default:
          task->varbound(n, MSK_BK_FX, bnd, bnd);
          break;
      }
      return n;
    }

    int  p_BaseModel::alloc_rangedvar (const std::string & name, double lb, double ub)
    {
      int n = task->appendvars(1);
      if (name.size() > 0)
        task->varname(n,name);

      task->varbound(n, MSK_BK_RA,lb,ub);
      return n;
    }
    */

    int  p_BaseModel::task_append_barvar(int dim, int num)
    {
      int idx = task->getnumbarvar();
      for (int i = 0; i < num; ++i)
        task->appendbarvar(dim);
      return idx;
    }

    int  p_BaseModel::task_append_con(int num) { return task->appendcons(num); }
    
    int  p_BaseModel::task_append_var(int num)  { return task->appendvars(num); }
    int  p_BaseModel::task_barvardim(int index) { return task->getbarvardim(index);  }
    
    int  p_BaseModel::task_numbarvar()          { return task->getnumbarvar(); }
    int  p_BaseModel::task_numcon()             { return task->getnumcon(); }
    int  p_BaseModel::task_numcone()            { return task->getnumcone(); }
    int  p_BaseModel::task_numvar()             { return task->getnumvar(); }

    void p_BaseModel::task_barvar_name(int idx, const std::string & name) { task->barvarname(idx,name); }
    void p_BaseModel::task_con_name   (int idx, const std::string & name) { task->conname   (idx,name); }
    void p_BaseModel::task_var_name   (int idx, const std::string & name) { task->varname   (idx,name); }
    void p_BaseModel::task_cone_name  (int idx, const std::string & name) { task->conename  (idx,name); }


    void p_BaseModel::task_format_var_names(const std::shared_ptr<monty::ndarray<int,1>> subj,
                                            const std::string & format,
                                            const std::shared_ptr<monty::ndarray<int,1>> dims,
                                            const std::shared_ptr<monty::ndarray<long long,1>> sp)
    {
        std::vector<int> v_subj(subj->flat_begin(),subj->flat_end());
        std::vector<int> v_dims(dims->flat_begin(),dims->flat_end());
        if (sp.get() != NULL) {
            std::vector<long long> v_sp(sp->flat_begin(),sp->flat_end());
            task->generatevarnames((int) v_subj.size(), v_subj.data(), format, (int)v_dims.size(), v_dims.data(), v_sp.data());
        }
        else {
            task->generatevarnames((int) v_subj.size(), v_subj.data(), format, (int)v_dims.size(), v_dims.data(), NULL);
        }
    }
    void p_BaseModel::task_format_con_names(const std::shared_ptr<monty::ndarray<int,1>> subi,
                                            const std::string & format,
                                            const std::shared_ptr<monty::ndarray<int,1>> dims,
                                            const std::shared_ptr<monty::ndarray<long long,1>> sp)
    {
        std::vector<int> v_subi(subi->flat_begin(),subi->flat_end());
        std::vector<int> v_dims(dims->flat_begin(),dims->flat_end());
        if (sp.get() != NULL) {
            std::vector<long long> v_sp(sp->flat_begin(),sp->flat_end());
            task->generateconnames((int) v_subi.size(), v_subi.data(), format, (int) v_dims.size(), v_dims.data(), v_sp.data());
        }
        else {
            task->generateconnames((int) v_subi.size(), v_subi.data(), format, (int) v_dims.size(), v_dims.data(), NULL);
        }
    }
    void p_BaseModel::task_format_cone_names(const std::shared_ptr<monty::ndarray<int,1>> subk,
                                             const std::string & format,
                                             const std::shared_ptr<monty::ndarray<int,1>> dims,
                                             const std::shared_ptr<monty::ndarray<long long,1>> sp)
    {
        std::vector<int> v_subk(subk->flat_begin(),subk->flat_end());
        std::vector<int> v_dims(dims->flat_begin(),dims->flat_end());
        if (sp.get() != NULL) {
            std::vector<long long> v_sp(sp->flat_begin(),sp->flat_end());
            task->generateconenames((int) v_subk.size(), v_subk.data(), format,  (int) v_dims.size(), v_dims.data(), v_sp.data());
        }
        else {
            task->generateconenames((int) v_subk.size(), v_subk.data(), format,  (int) v_dims.size(), v_dims.data(), NULL);
        }
    }


    
    void p_BaseModel::task_put_param(const std::string & name, double value) { task->putparam(name,value); }
    void p_BaseModel::task_put_param(const std::string & name, int    value) { task->putparam(name,value); }
    void p_BaseModel::task_put_param(const std::string & name, const std::string & value) { task->putparam(name,value); }
    
    double p_BaseModel::task_get_dinf (const std::string & name)
    {
      MSKdinfiteme key;
      if (SolverInfo::getdouinf(name,key))
        return task->getdinfitem(key);
      else
        throw NameError("Invalid double information item name");
    }

    int       p_BaseModel::task_get_iinf (const std::string & name)
    {
      MSKiinfiteme key;
      if (SolverInfo::getintinf(name,key))
        return task->getiinfitem(key);
      else
        throw NameError("Invalid integer information item name");
    }
    
    long long p_BaseModel::task_get_liinf(const std::string & name)
    {
      MSKliinfiteme key;
      if (SolverInfo::getlintinf(name,key))
        return task->getliinfitem(key);
      else
        throw NameError("Invalid long integer information item name");
    }

    
    void p_BaseModel::task_putaijlist
      ( const std::shared_ptr<monty::ndarray<int,1>>    & subi, 
        const std::shared_ptr<monty::ndarray<int,1>>    & subj, 
        const std::shared_ptr<monty::ndarray<double,1>> & cof, 
        long long                        num )
    { task->putaijlist(subi->raw(),subj->raw(),cof->raw(),num); }

    void p_BaseModel::task_putarowlist
      ( const std::shared_ptr<monty::ndarray<int,1>>       & idxs,
        const std::shared_ptr<monty::ndarray<long long,1>> & ptrb,
        const std::shared_ptr<monty::ndarray<int,1>>       & subj, 
        const std::shared_ptr<monty::ndarray<double,1>>    & cof) 
    { 
        assert(ptrb->size() == idxs->size()+1);
        for (auto i = ptrb->flat_begin(), e = ptrb->flat_end(); i != e; ++i)
        {            
            assert(*i <= subj->size());
            assert(*i <= cof->size());
        }
        task->putarowlist((int)idxs->size(), idxs->raw(), ptrb->raw(),subj->raw(),cof->raw()); 
    }






    void p_BaseModel::task_putbararowlist
    ( const std::shared_ptr<monty::ndarray<int,1>>       subi,
      const std::shared_ptr<monty::ndarray<long long,1>> ptr,
      const std::shared_ptr<monty::ndarray<int,1>>       subj,
      const std::shared_ptr<monty::ndarray<long long,1>> matidx) {
        task->putbararowlist(subi->size(),
                             subi->raw(),
                             ptr->raw(),
                             subj->raw(),
                             matidx->raw());
    }

    void p_BaseModel::task_putbaraijlist
    ( const std::shared_ptr<monty::ndarray<int,1>>       subi,
      const std::shared_ptr<monty::ndarray<int,1>>       subj,
      const std::shared_ptr<monty::ndarray<long long,1>> matidx) {
        std::vector<long long> ptr(subi->size()+1);
        std::vector<double> weight(subi->size());
        for (ptrdiff_t i = 0, e = subi->size()+1; i < e; ++i) ptr[i] = i;
        for (ptrdiff_t i = 0, e = subi->size(); i < e; ++i) weight[i] = 1.0;
        task->putbaraijlist(subi->size(),
                            subi->raw(),
                            subj->raw(),
                            ptr.data(),
                            ptr.data()+1,
                            matidx->raw(),
                            weight.data());
    }
  
    void p_BaseModel::task_putbarc
    ( const std::shared_ptr<monty::ndarray<int,1>>    subj,
      const std::shared_ptr<monty::ndarray<int,1>>    subk,
      const std::shared_ptr<monty::ndarray<int,1>>    subl,
      const std::shared_ptr<monty::ndarray<double,1>> val) {
        task->putbarc(subj->size(),
                      subj->raw(),
                      subk->raw(),
                      subl->raw(),
                      val->raw());
    }
  
    std::shared_ptr<monty::ndarray<long long,1>> p_BaseModel::task_appendsymmatlist
      ( const std::shared_ptr<monty::ndarray<int,1>> & dim, 
        const std::shared_ptr<monty::ndarray<long long,1>> & nz, 
        const std::shared_ptr<monty::ndarray<int,1>> & subk, 
        const std::shared_ptr<monty::ndarray<int,1>> & subl, 
        const std::shared_ptr<monty::ndarray<double,1>> & val) {
        std::shared_ptr<monty::ndarray<long long,1>> r = std::shared_ptr<monty::ndarray<long long,1>>(new monty::ndarray<long long,1>(dim->size()));
        task->appendsymmatlist(dim->size(),
                               dim->raw(),
                               nz->raw(),
                               subk->raw(),
                               subl->raw(),
                               val->raw(),
                               r->raw());
        return r;
    }
    
    //-----------------------------------------
    void p_BaseModel::task_append_zerocones (int numcone) { task->appendcones(numcone); }
    void p_BaseModel::task_clear_cones   ( const std::shared_ptr<monty::ndarray<int,1>> & idxs ) { task->putcones(MSK_CT_ZERO, idxs->raw(), 0, idxs->size(), NULL, NULL); }
    void p_BaseModel::task_put_zerocones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs ) { task->putcones(MSK_CT_ZERO, idxs->raw(),  conesize, numcone, NULL, membs->raw()); }
    void p_BaseModel::task_put_quadcones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs ) { task->putcones(MSK_CT_QUAD, idxs->raw(),  conesize, numcone, NULL, membs->raw()); }
    void p_BaseModel::task_put_rquadcones( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs ) { task->putcones(MSK_CT_RQUAD, idxs->raw(), conesize, numcone, NULL, membs->raw()); }
    void p_BaseModel::task_put_pexpcones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs ) { task->putcones(MSK_CT_PEXP, idxs->raw(),  conesize, numcone, NULL, membs->raw()); }
    void p_BaseModel::task_put_ppowcones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs, const std::shared_ptr<monty::ndarray<double,1>> & alpha ) { task->putcones(MSK_CT_PPOW, idxs->raw(), conesize, numcone, alpha->raw(), membs->raw()); }
    void p_BaseModel::task_put_dexpcones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs ) { task->putcones(MSK_CT_DEXP, idxs->raw(),  conesize, numcone, NULL, membs->raw()); }
    void p_BaseModel::task_put_dpowcones ( const std::shared_ptr<monty::ndarray<int,1>> & idxs, int conesize, int numcone,  const std::shared_ptr<monty::ndarray<int,1>> & membs, const std::shared_ptr<monty::ndarray<double,1>> & alpha ) { task->putcones(MSK_CT_DPOW, idxs->raw(), conesize, numcone, alpha->raw(), membs->raw()); }

    void p_BaseModel::task_con_putboundlist_fr(const std::shared_ptr<monty::ndarray<int,1>> idxs) { task->conboundlist(idxs->size(), idxs->raw(), MSK_BK_FR, NULL,NULL); }
    void p_BaseModel::task_con_putboundlist_lo(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs) { task->conboundlist(idxs->size(), idxs->raw(), MSK_BK_LO, rhs->raw(),rhs->raw()); }
    void p_BaseModel::task_con_putboundlist_up(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs) { task->conboundlist(idxs->size(), idxs->raw(), MSK_BK_UP, rhs->raw(),rhs->raw()); }
    void p_BaseModel::task_con_putboundlist_fx(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs) { task->conboundlist(idxs->size(), idxs->raw(), MSK_BK_FX, rhs->raw(),rhs->raw()); }
    void p_BaseModel::task_con_putboundlist_ra(const std::shared_ptr<monty::ndarray<int,1>> idxs, 
                                               const std::shared_ptr<monty::ndarray<double,1>> & lb , 
                                               const std::shared_ptr<monty::ndarray<double,1>> & ub ) {
        task->conboundlist(idxs->size(), idxs->raw(), MSK_BK_RA,lb->raw(),ub->raw());
    }

    void p_BaseModel::task_var_putboundlist_fr(const std::shared_ptr<monty::ndarray<int,1>> idxs) {task->varboundlist(idxs->size(), idxs->raw(), MSK_BK_FR, NULL,NULL); }
    void p_BaseModel::task_var_putboundlist_lo(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs) {task->varboundlist(idxs->size(), idxs->raw(), MSK_BK_LO, rhs->raw(),rhs->raw()); }
    void p_BaseModel::task_var_putboundlist_up(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs) {task->varboundlist(idxs->size(), idxs->raw(), MSK_BK_UP, rhs->raw(),rhs->raw()); }
    void p_BaseModel::task_var_putboundlist_fx(const std::shared_ptr<monty::ndarray<int,1>> idxs, const std::shared_ptr<monty::ndarray<double,1>> & rhs) {task->varboundlist(idxs->size(), idxs->raw(), MSK_BK_FX, rhs->raw(),rhs->raw()); }
    void p_BaseModel::task_var_putboundlist_ra(const std::shared_ptr<monty::ndarray<int,1>> idxs, 
                                               const std::shared_ptr<monty::ndarray<double,1>> & lb , 
                                               const std::shared_ptr<monty::ndarray<double,1>> & ub )
    {
        task->varboundlist(idxs->size(), idxs->raw(), MSK_BK_RA, lb->raw(),ub->raw());
    }


    void p_BaseModel::task_objectivename(const std::string & name) { task->objname(name); }

    void p_BaseModel::task_putobjective
      ( bool                                              maximize,
        const std::shared_ptr<monty::ndarray<int,1>>    & subj,
        const std::shared_ptr<monty::ndarray<double,1>> & cof,
        double                                            cfix)
    {
        std::vector<double> c(task->getnumvar());
        std::vector<int>    cidx(task->getnumvar());
        
        for (ptrdiff_t i = 0; i < cidx.size();  ++i) cidx[i] = i;
        for (ptrdiff_t i = 0; i < subj->size(); ++i) c[(*subj)[i]] += (*cof)[i];

        if (cidx.size() > 0)
          task->putclist(cidx.size(), & cidx[0], c.data());
        task->putcfix(cfix);
        task->putobjsense(maximize ? MSK_OBJECTIVE_SENSE_MAXIMIZE : MSK_OBJECTIVE_SENSE_MINIMIZE);
    }

    void p_BaseModel::task_putclist
      ( const std::shared_ptr<monty::ndarray<int,1>>    & subj,
        const std::shared_ptr<monty::ndarray<double,1>> & cof)
    {
        task->putclist(subj->size(), subj->raw(),cof->raw());
    }



    void p_BaseModel::task_putobjectivename(const std::string & name)
    {
      task->objname(name);
    }

    MSKtask_t p_BaseModel::task_get()
    {
      return task->clone();
    }

    MSKtask_t p_BaseModel::__mosek_2fusion_2BaseModel__task_get() { return task_get(); }

    void p_BaseModel::dispose()
    {
        task->dispose();
    }

    //---------------------------
    void p_BaseModel::task_break_solve()
    {
        task->breakOptimize();
    }

    void p_BaseModel::task_solve() 
    { 
      synched = false;
      bool ok = false;

      monty::finally([&]() 
        {  
          if (! ok) // means exception before we reached end
          {
            sol_itr = nullptr;
            sol_bas = nullptr;
            sol_itg = nullptr;
          }
        });

      try
      {     
        task->optimize();
        task->solutionsummary(MSK_STREAM_LOG);
      }
      catch (mosek::MosekException e)
      {
        throw OptimizeError(e.what());
      }

      int numcon    = task->getnumcon();
      int numvar    = task->getnumvar();
      int numcone   = task->getnumcone();
      int numbarvar = task->getnumbarvar();

      bool sol_bas_def = task->solutiondef(MSK_SOL_BAS);
      bool sol_itr_def = task->solutiondef(MSK_SOL_ITR);
      bool sol_itg_def = task->solutiondef(MSK_SOL_ITG);

      sol_itr = nullptr;
      sol_bas = nullptr;
      sol_itg = nullptr;

      MSKprostae prosta;
      MSKsolstae solsta;

      size_t barvarveclen = 0;
      for (int j = 0; j < numbarvar; ++j) {
          int barjdim = task->getbarvardim(j);
          barvarveclen += barjdim*(1+barjdim)/2;
      }

      if (sol_itr_def)
      {    
        sol_itr = new SolutionStruct(numvar,numcon,numcone,barvarveclen);
        p_SolutionStruct * sol_ptr = p_SolutionStruct::_get_impl(sol_itr);

#if 0
        std::cout 
          << "numvar    = " << numvar << std::endl
          << "numcon    = " << numcon << std::endl
          << "numcone   = " << numcone << std::endl
          << "numbarvar = " << numbarvar << std::endl
          << "xc  = " << sol_ptr->xc.get() << std::endl
          << "xx  = " << sol_ptr->xx.get() << std::endl
          << "y   = " << sol_ptr->y.get() << std::endl
          << "slc = " << sol_ptr->slc.get() << std::endl
          << "suc = " << sol_ptr->suc.get() << std::endl
          << "slx = " << sol_ptr->slx.get() << std::endl
          << "sux = " << sol_ptr->sux.get() << std::endl
          << "snx = " << sol_ptr->snx.get() << std::endl;
#endif


        task->getsolution(
          MSK_SOL_ITR,
          prosta,
          solsta,
          nullptr,nullptr,nullptr,
          sol_ptr->xc  ? sol_ptr->xc->raw()  : nullptr,
          sol_ptr->xx  ? sol_ptr->xx->raw()  : nullptr,
          sol_ptr->y   ? sol_ptr->y->raw()   : nullptr,
          sol_ptr->slc ? sol_ptr->slc->raw() : nullptr,
          sol_ptr->suc ? sol_ptr->suc->raw() : nullptr,
          sol_ptr->slx ? sol_ptr->slx->raw() : nullptr,
          sol_ptr->sux ? sol_ptr->sux->raw() : nullptr,
          sol_ptr->snx ? sol_ptr->snx->raw() : nullptr);

        if (sol_ptr->snx != NULL)
          for (int i = 0; i < numvar; ++i)
              (*sol_ptr->yx)[i] = (*sol_ptr->slx)[i] - (*sol_ptr->sux)[i] + (*sol_ptr->snx)[i];
        else
          for (int i = 0; i < numvar; ++i)
              (*sol_ptr->yx)[i] = (*sol_ptr->slx)[i] - (*sol_ptr->sux)[i];
        
        if (numbarvar > 0) {
            task->getbarxslice(MSK_SOL_ITR,0,numbarvar,barvarveclen,sol_ptr->barx->raw());
            task->getbarsslice(MSK_SOL_ITR,0,numbarvar,barvarveclen,sol_ptr->bars->raw());
        }

        sol_ptr->pobj = task->getprimalobj(MSK_SOL_ITR);
        sol_ptr->dobj = task->getdualobj  (MSK_SOL_ITR);
        convertSolutionStatus(MSK_SOL_ITR, sol_ptr, solsta,prosta);
      }

      if (sol_bas_def)
      {    
        sol_bas = new SolutionStruct(numvar,numcon,numcone,barvarveclen);
        p_SolutionStruct * sol_ptr = p_SolutionStruct::_get_impl(sol_bas);
        task->getsolution(
          MSK_SOL_BAS,
          prosta,
          solsta,
          //sol_bas->skc,sol_bas->skx,sol_bas->skn,
          nullptr,nullptr,nullptr,
          sol_ptr->xc  ? sol_ptr->xc->raw()  : nullptr,
          sol_ptr->xx  ? sol_ptr->xx->raw()  : nullptr,
          sol_ptr->y   ? sol_ptr->y->raw()   : nullptr,
          sol_ptr->slc ? sol_ptr->slc->raw() : nullptr,
          sol_ptr->suc ? sol_ptr->suc->raw() : nullptr,
          sol_ptr->slx ? sol_ptr->slx->raw() : nullptr,
          sol_ptr->sux ? sol_ptr->sux->raw() : nullptr,
          sol_ptr->snx ? sol_ptr->snx->raw() : nullptr);

        for (int i = 0; i < sol_ptr->yx->size(); ++i)
            (*sol_ptr->yx)[i] = (*sol_ptr->slx)[i] - (*sol_ptr->sux)[i];

        sol_ptr->pobj = task->getprimalobj(MSK_SOL_BAS);
        sol_ptr->dobj = task->getdualobj  (MSK_SOL_BAS);
        convertSolutionStatus(MSK_SOL_BAS, sol_ptr, solsta,prosta);
      }
      if (sol_itg_def)
      {    
        sol_itg = new SolutionStruct(numvar,numcon,numcone,barvarveclen);
        p_SolutionStruct * sol_ptr = p_SolutionStruct::_get_impl(sol_itg);
        task->getsolution( MSK_SOL_ITG,
                          prosta,
                          solsta,
                          //sol_itg->skc,sol_itg->skx,sol_itg->skn,
                          nullptr,nullptr,nullptr,
                          sol_ptr->xc ? sol_ptr->xc->raw() : nullptr,
                          sol_ptr->xx ? sol_ptr->xx->raw() : nullptr,
                          nullptr /*sol_ptr->y->raw()*/,
                          nullptr,nullptr, //sol_ptr->slc->raw(),sol_ptr->suc->raw(),
                          nullptr,nullptr, //sol_ptr->slx->raw(),sol_ptr->sux->raw(),
                          nullptr//sol_ptr->snx->raw()
                          );

        if (numbarvar > 0) {
            task->getbarxslice(MSK_SOL_ITG,0,numbarvar,barvarveclen,sol_ptr->barx->raw());
            task->getbarsslice(MSK_SOL_ITG,0,numbarvar,barvarveclen,sol_ptr->bars->raw());
        }

        sol_ptr->pobj = task->getprimalobj(MSK_SOL_ITG);
        
        convertSolutionStatus(MSK_SOL_ITG, sol_ptr, solsta,prosta);
      }

      ok = true;
      synched = true;
    }
    
    void p_BaseModel::task_var_putintlist(const std::shared_ptr<monty::ndarray<int,1>> & idxs) { task->putintlist(idxs->size(), idxs->raw()); }
    void p_BaseModel::task_var_putcontlist(const std::shared_ptr<monty::ndarray<int,1>> & idxs) { task->putcontlist(idxs->size(), idxs->raw()); }

    void p_BaseModel::task_write(const std::string & filename) 
    { 
      task->putparam("MSK_IPAR_OPF_WRITE_SOLUTIONS",1);
      task->putparam("MSK_IPAR_WRITE_IGNORE_INCOMPATIBLE_ITEMS",1);
      task->write(filename); 
    }

    void p_BaseModel::task_putxx_slice(SolutionType which, int first, int last, std::shared_ptr<monty::ndarray<double,1>> & xx)
    {
        switch (which)
        {
            case SolutionType::Interior: task->putxxslice(MSK_SOL_ITR, first,last,xx->raw()); break;
            case SolutionType::Integer:  task->putxxslice(MSK_SOL_ITG, first,last,xx->raw()); break;
            case SolutionType::Basic:    task->putxxslice(MSK_SOL_BAS, first,last,xx->raw()); break;
        }
    }

    void p_BaseModel::task_setnumvar(int num)
    {
        int numvar = task->getnumvar();
        if (numvar > num)
        {
            for (int i = numvar; i > num; --i)
                task->removevar(i-1);
        }
    }


    void p_BaseModel::task_cleanup (int inumvar, int inumcon, int inumcone, int inumbarvar) 
    { 
        task->fixvars(inumcone);
        task->revert(task->getnumvar(),inumcon,inumcone,inumbarvar);
    }




#if 0

    void p_BaseModel::env_syeig (int n, std::shared_ptr<monty::ndarray<double,1>> & a, std::shared_ptr<monty::ndarray<double,1>> & w)
    {
      if      (a->size() < n * (n+1) / 2)
        throw MosekException("Invalid length if a in call to syeig");
      else if (w->size() < n)
        throw MosekException("Invalid length if w in call to syeig");
      Task::env_syeig(n,a->raw(),w->raw());
    }

    void p_BaseModel::env_potrf (int n, std::shared_ptr<monty::ndarray<double,1>> & a)
    {
      if      (a->size() < n * (n+1) / 2)
        throw MosekException("Invalid length if a in call to potrf");
      Task::env_potrf(n,a->raw());
    }

    void p_BaseModel::env_syevd (int n, std::shared_ptr<monty::ndarray<double,1>> & a, std::shared_ptr<monty::ndarray<double,1>> & w)
    {
      if      (a->size() < n * (n+1) / 2)
        throw MosekException("Invalid length if a in call to syevd");
      else if (w->size() < n)
        throw MosekException("Invalid length if w in call to syevd");
      Task::env_syevd(n,a->raw(),w->raw());
    }
#endif


    void p_BaseModel::env_putlicensecode (std::shared_ptr<monty::ndarray<int,1>> code) { putlicensecode(code->raw()); }
    void p_BaseModel::env_putlicensepath (const std::string & path) { putlicensepath(path); }
    void p_BaseModel::env_putlicensewait (int wait) { putlicensewait(wait); }
    
    std::string p_BaseModel::env_getversion() {
       int major; int minor; int revision;
       char buf[20];
       MSK_getversion(&major, &minor, &revision);
       sprintf(buf, "%d.%d.%d", major, minor, revision);
       return std::string(buf);
    };

    void p_BaseModel::convertSolutionStatus(MSKsoltypee soltype, mosek::fusion::p_SolutionStruct * sol, MSKsolstae status, MSKprostae prosta)
    {
      switch (status)
      {
      case MSK_SOL_STA_OPTIMAL: 
        sol->pstatus = SolutionStatus::Optimal;
        sol->dstatus = SolutionStatus::Optimal;
        break;
      case MSK_SOL_STA_INTEGER_OPTIMAL: 
        sol->pstatus = SolutionStatus::Optimal;
        sol->dstatus = SolutionStatus::Unknown;
        break;
      case MSK_SOL_STA_PRIM_AND_DUAL_FEAS: 
        sol->pstatus = SolutionStatus::Feasible;
        sol->dstatus = SolutionStatus::Feasible;
        break;
      case MSK_SOL_STA_PRIM_FEAS: 
        sol->pstatus = SolutionStatus::Feasible;
        sol->dstatus = SolutionStatus::Unknown;
        break;
      case MSK_SOL_STA_DUAL_FEAS: 
        sol->pstatus = SolutionStatus::Unknown;
        sol->dstatus = SolutionStatus::Feasible;
        break;
      case MSK_SOL_STA_PRIM_INFEAS_CER: 
        sol->pstatus = SolutionStatus::Unknown;
        sol->dstatus = SolutionStatus::Certificate;
        break;
      case MSK_SOL_STA_DUAL_INFEAS_CER: // ok?
        sol->pstatus = SolutionStatus::Certificate;
        sol->dstatus = SolutionStatus::Unknown;
        break;
      case MSK_SOL_STA_PRIM_ILLPOSED_CER: 
        sol->pstatus = SolutionStatus::Unknown;
        sol->dstatus = SolutionStatus::IllposedCert;
        break;
      case MSK_SOL_STA_DUAL_ILLPOSED_CER: // ok?
        sol->pstatus = SolutionStatus::IllposedCert;
        sol->dstatus = SolutionStatus::Unknown;
        break;

      default:
        sol->pstatus = SolutionStatus::Unknown;
        sol->dstatus = SolutionStatus::Unknown;
      }
    
     switch (prosta)
     {
        case MSK_PRO_STA_UNKNOWN  :                   sol->probstatus = ProblemStatus::Unknown; break;
        case MSK_PRO_STA_PRIM_AND_DUAL_FEAS :         sol->probstatus = ProblemStatus::PrimalAndDualFeasible; break;
        case MSK_PRO_STA_PRIM_FEAS :                  sol->probstatus = ProblemStatus::PrimalFeasible; break;
        case MSK_PRO_STA_DUAL_FEAS :                  sol->probstatus = ProblemStatus::DualFeasible; break;
        case MSK_PRO_STA_PRIM_INFEAS :                sol->probstatus = ProblemStatus::PrimalInfeasible; break;
        case MSK_PRO_STA_DUAL_INFEAS :                sol->probstatus = ProblemStatus::DualInfeasible; break;
        case MSK_PRO_STA_PRIM_AND_DUAL_INFEAS :       sol->probstatus = ProblemStatus::PrimalAndDualInfeasible; break;
        case MSK_PRO_STA_ILL_POSED :                  sol->probstatus = ProblemStatus::IllPosed; break;
        case MSK_PRO_STA_PRIM_INFEAS_OR_UNBOUNDED :   sol->probstatus = ProblemStatus::PrimalInfeasibleOrUnbounded; break;
        default:                                      sol->probstatus = ProblemStatus::Unknown; break;
     }
    }



  }
} 

