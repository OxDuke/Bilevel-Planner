#include "fusion_p.h"
#include <string>
#include "monty.h"

//#include "mosektask_p.h"

namespace mosek
{
  namespace fusion
  {
    Debug::Debug() : ptr(new p_Debug(this)) {}
    Debug::t Debug::o() { return new Debug(); }
    Debug::t Debug::p(const std::string & val)           { return ptr->p(val); }
    Debug::t Debug::p(   int val)                        { return ptr->p(val); }
    Debug::t Debug::p(long long val)                     { return ptr->p(val); }
    Debug::t Debug::p(double val)                        { return ptr->p(val); }
    Debug::t Debug::p(  bool val)                        { return ptr->p(val); }
    Debug::t Debug::p(const std::shared_ptr<monty::ndarray<double,1>>    & val) { return ptr->p(val); }
    Debug::t Debug::p(const std::shared_ptr<monty::ndarray<int,1>>       & val) { return ptr->p(val); }
    Debug::t Debug::p(const std::shared_ptr<monty::ndarray<long long,1>> & val) { return ptr->p(val); }
    Debug::t Debug::lf()                                 { return ptr->lf();   }

    Debug::~Debug() { delete ptr; ptr = NULL; }

  }
}
