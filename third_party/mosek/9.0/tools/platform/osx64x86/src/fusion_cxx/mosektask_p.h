#ifndef _MOSEKTASK_P_H_
#define _MOSEKTASK_P_H_

#include <mosek.h>
#include <mutex>
#include <vector>
#include <memory>

#include "mosektask.h"

namespace mosek
{
  class Task
  {
  public:
    typedef std::function<void(const std::string &)>                                                      msghandler_t;
    /*
    typedef std::function<int(void * handle, MSKcallbackcodee)>                                           pgshandler_t;
    typedef std::function<int(void * handle, MSKcallbackcodee,const double*,const int*,const long long*)> infhandler_t;
    */
  private:
    bool         breakflag;
    MSKtask_t    task;

    msghandler_t      msguserfunc;
    //infhandler_t      infuserfunc;
    //pgshandler_t      pgsuserfunc;
    datacbhandler_t   datacbuserfunc;
    cbhandler_t       cbuserfunc;
    /*void            * pgshandle;*/
    //void            * infhandle;

    static void msghandler(void * handle, const char * msg);
    static int  pgshandler(MSKtask_t         task,
                           void            * handle,
                           MSKcallbackcodee  caller,
                           const double    * dinf,
                           const int       * iinf,
                           const long long * linf);

    //---------------------------------
    static MSKenv_t env;
    static int      env_refc;
    static bool     env_release;
    static std::mutex env_lock;

    //---------------------------------
    static MSKenv_t getEnv();
    static void releaseEnv();
  public:
    static MSKenv_t getEnv(bool borrow);

    Task();
    Task(Task & other);
    ~Task();
    void dispose();
    MSKtask_t clone() const;

    void setDataCallbackFunc(datacbhandler_t userfunc) { datacbuserfunc = userfunc; }
    void setCallbackFunc(cbhandler_t userfunc)         { cbuserfunc = userfunc; }
    /*
    void setProgressFunc(pgshandler_t userfunc, void * handle = nullptr ) { pgsuserfunc = userfunc; pgshandle = handle; }
    void setInfoFunc    (infhandler_t userfunc, void * handle = nullptr ) { infuserfunc = userfunc; infhandle = handle; }
    */
    void setStreamFunc  (msghandler_t userfunc)                           { msguserfunc = userfunc; }
    /*
    void removeProgressFunc()                   { pgsuserfunc = nullptr; }
    void removeInfoFunc()                       { infuserfunc = nullptr; }
    */
    void removeStreamFunc()                     { msguserfunc = nullptr; }

    int getnumvar() const;
    int getnumcon() const;
    int getnumcone() const;
    int getnumbarvar() const;

    int getbarvardim(int idx) const;

    int appendvars(int num);
    int appendcons(int num);
    int appendbarvar(int dim);
    void appendcones(int numcone);
    void putcones(MSKconetypee ct, int * coneidxs, int conesize, int numcone, double * alpha, int * membs);

    // revert the task to this size
    void removevar(int idx);
    void revert(int numvar, int numcon, int numcone, int numbarvar);
    void fixvars(int numvar);

    void varbound     (int idx, MSKboundkeye bk, double lb, double ub);
    void varboundlist (size_t num, const int * sub,     const MSKboundkeye bk, const double * lb, const double * ub);
    void conbound     (int idx, MSKboundkeye bk, double lb, double ub);
    void conboundlist (size_t num, const int * sub,     const MSKboundkeye bk, const double * lb, const double * ub);

    void varname   (int idx, const std::string & name);
    void conname   (int idx, const std::string & name);
    void conename  (int idx, const std::string & name);
    void barvarname(int idx, const std::string & name);
    void objname   (         const std::string & name);
    void taskname  (         const std::string & name);


    void generatevarnames(int num,int * subj, const std::string & format, int ndims, int * dims, long long * sp);
    void generateconnames(int num,int * subi, const std::string & format, int ndims, int * dims, long long * sp);
    void generateconenames(int num,int * subk, const std::string & format, int ndims, int * dims, long long * sp);

    void putarowlist( int num,
                      const int       * idxs,
                      const long long * ptrb,
                      const int       * subj,
                      const double    * cof);
    // putacolslice
    void putaijlist(const int    * subi,
                    const int    * subj,
                    const double * cof,
                    long long      num);



    void putbararowlist(int               num,
                        const int       * subi,
                        const long long * ptr,
                        const int       * subj,
                        const long long * matidx);

    void putbaraijlist(long long num,
                       const int       * subi,
                       const int       * subj,
                       const long long * alphaptrb,
                       const long long * alphaptre,
                       const long long * matidx,
                       const double    * weight);

   
    void putobjsense(MSKobjsensee sense);
    void putclist   (size_t num, const int * subj, const double * c);
    void putcfix    (double cfix);
    void putbarc (long long num,
                  const int *subj,
                  const int *subk,
                  const int *subl,
                  const double * val);
    
    //--------------------------
    void appendsymmatlist(int num,
                          const int    * dim,
                          const long long * nz,
                          const int    * subk,
                          const int    * subl,
                          const double * val,
                          long long    * res );

    //--------------------------
    
    void breakOptimize();
    MSKrescodee optimize();

    void solutionsummary(MSKstreamtypee strm);
    bool solutiondef(MSKsoltypee sol);
    void getsolution
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
        double     *  snx); 
    void getbarxslice(MSKsoltypee sol, int first, int last, size_t barvarvecle, double* barxj);
    void getbarsslice(MSKsoltypee sol, int first, int last, size_t barvarvecle, double* barxj);

    double getprimalobj(MSKsoltypee sol);
    double getdualobj(MSKsoltypee sol);
    
    //--------------------------

    void putxxslice(MSKsoltypee which, int first, int last, double * xx);

    //--------------------------
    
    void putintlist(size_t num, const int * subj);
    void putcontlist(size_t num, const int * subj);

    //--------------------------
    
    void write(const std::string & filename);

    //--------------------------
    
    void putparam(const std::string & name, double value);
    void putparam(const std::string & name, int value);
    void putparam(const std::string & name, const std::string & value);

    double    getdinfitem (MSKdinfiteme  key);
    int       getiinfitem (MSKiinfiteme  key);
    long long getliinfitem(MSKliinfiteme key);
  
    static void env_syeig (int n, const double * a, double * w);
    //static void env_potrf (int n, double * a);                        
    static void env_syevd (int n, double * a, double * w);

    static void releaseGlobalEnv();
  };

  
  void putlicensecode(int* code);
  void putlicensepath(const std::string & path);
  void putlicensewait(int wait);
}
#endif
