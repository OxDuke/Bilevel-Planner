#ifndef _MOSEKTASK_H_
#define _MOSEKTASK_H_

#include <mosek.h>
#include <string>
#include <sstream>
#include <memory>
#include <functional>
#include <monty_ndarray.h>
#include <stdexcept>
namespace mosek
{
  struct MosekException : std::runtime_error
  {
    MosekException(const std::string & msg) : std::runtime_error(msg.c_str()) {}
    MosekException(const char * msg) : std::runtime_error(msg) {}
    MosekException(MSKtask_t task);
  };

  typedef std::function<void(const std::string &)> msghandler_t;
  typedef std::function<bool(MSKcallbackcodee, const double *, const int32_t *, const int64_t *)> datacbhandler_t;
  typedef std::function<int(MSKcallbackcodee)> cbhandler_t;

  void releaseGlobalEnv();

  namespace LinAlg
  {
    struct ArgumentError : std::runtime_error
    {
      ArgumentError(const char * msg) : std::runtime_error(msg) {}
    };

    void axpy
      ( int n,
        double alpha,
        std::shared_ptr<monty::ndarray<double,1>> x,
        std::shared_ptr<monty::ndarray<double,1>> y);

    void dot      
      ( int n,
        std::shared_ptr<monty::ndarray<double,1>> x,
        std::shared_ptr<monty::ndarray<double,1>> y,
        double & xty);

    void gemv
      ( bool     transa,
        int      m,
        int      n,
        double   alpha,
        std::shared_ptr<monty::ndarray<double,1>> a,
        std::shared_ptr<monty::ndarray<double,1>> x,
        double   beta,
        std::shared_ptr<monty::ndarray<double,1>> y);

    void gemm
      ( bool     transa,
        bool     transb,
        int      m,
        int      n,
        int      k,
        double   alpha,
        std::shared_ptr<monty::ndarray<double,1>> a,
        std::shared_ptr<monty::ndarray<double,1>> b,
        double   beta,
        std::shared_ptr<monty::ndarray<double,1>> c);
    void syrk
      ( MSKuploe uplo,
        bool           transa,
        int            n,
        int            k,
        double         alpha,
        std::shared_ptr<monty::ndarray<double,1>> a,
        double         beta,
        std::shared_ptr<monty::ndarray<double,1>> c);
    void syeig
      ( MSKuploe uplo,
        int      n,
        std::shared_ptr<monty::ndarray<double,1>> a,
        std::shared_ptr<monty::ndarray<double,1>> w);
    void syevd
      ( MSKuploe uplo,
        int      n,
        std::shared_ptr<monty::ndarray<double,1>> a,
        std::shared_ptr<monty::ndarray<double,1>> w);
    void potrf
      ( MSKuploe uplo,
        int      n,
        std::shared_ptr<monty::ndarray<double,1>> a);


  }
}

#endif
