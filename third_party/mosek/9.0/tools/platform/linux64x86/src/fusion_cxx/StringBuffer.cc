#include "fusion_p.h"
#include <string>
#include <vector>

namespace mosek
{
  namespace fusion
  {
    namespace Utils
    {
      StringBuffer::StringBuffer() : _impl(new p_StringBuffer(this)) {}
      StringBuffer::t StringBuffer::clear ()                                        { return _impl->clear(); }
      StringBuffer::t StringBuffer::a (int         value)                           { return _impl->a(value); }
      StringBuffer::t StringBuffer::a (long long   value)                           { return _impl->a(value); }
      StringBuffer::t StringBuffer::a (double      value)                           { return _impl->a(value); }
      StringBuffer::t StringBuffer::a (const std::string & value)                   { return _impl->a(value); }
      StringBuffer::t StringBuffer::a (bool        value)                           { return _impl->a(value); }
      StringBuffer::t StringBuffer::a (std::shared_ptr<monty::ndarray<std::string,1>> value) { return _impl->a(*value.get()); }
      StringBuffer::t StringBuffer::a (std::shared_ptr<monty::ndarray<int,1>>         value) { return _impl->a(*value.get()); }
      StringBuffer::t StringBuffer::a (std::shared_ptr<monty::ndarray<long long,1>>   value) { return _impl->a(*value.get()); }
      StringBuffer::t StringBuffer::a (std::shared_ptr<monty::ndarray<double,1>>      value) { return _impl->a(*value.get()); }
      StringBuffer::t StringBuffer::lf ()                                           { return _impl->lf(); }


      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__clear ()                                        { return _impl->clear(); }
      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (int         value)                           { return _impl->a(value); }
      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (long long   value)                           { return _impl->a(value); }
      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (double      value)                           { return _impl->a(value); }
      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (const std::string & value)                   { return _impl->a(value); }
      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (bool        value)                           { return _impl->a(value); }
      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (std::shared_ptr<monty::ndarray<std::string,1>> value) { return _impl->a(*value.get()); }
      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (std::shared_ptr<monty::ndarray<int,1>>         value) { return _impl->a(*value.get()); }
      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (std::shared_ptr<monty::ndarray<long long,1>>   value) { return _impl->a(*value.get()); }
      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (std::shared_ptr<monty::ndarray<double,1>>      value) { return _impl->a(*value.get()); }
      StringBuffer::t StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__lf ()                                           { return _impl->lf(); }



      std::string StringBuffer::toString () const                  { return _impl->toString(); }



      template<class T> 
      void ssadd(std::stringstream & ss, const monty::ndarray<T,1> & val) {
          if (val.size() > 0)
          {
              ss << val[0];
              for (int i = 1; i < val.size(); ++i)
                  ss << "," << val[i];
          }
      }

      StringBuffer::t p_StringBuffer::a (const monty::ndarray<std::string,1> & val) { ssadd(ss,val); return _pubthis; }
      StringBuffer::t p_StringBuffer::a (const monty::ndarray<int,1> & val)         { ssadd(ss,val); return _pubthis; }
      StringBuffer::t p_StringBuffer::a (const monty::ndarray<long long,1> & val)   { ssadd(ss,val); return _pubthis; }
      StringBuffer::t p_StringBuffer::a (const monty::ndarray<double,1> & val)      { ssadd(ss,val); return _pubthis; }

      StringBuffer::t p_StringBuffer::a (const int & val)         { ss << val; return _pubthis; }
      StringBuffer::t p_StringBuffer::a (const long long & val)   { ss << val; return _pubthis; }
      StringBuffer::t p_StringBuffer::a (const double & val)      { ss << val; return _pubthis; }
      StringBuffer::t p_StringBuffer::a (const bool & val)        { ss << val; return _pubthis; }
      StringBuffer::t p_StringBuffer::a (const std::string & val) { ss << val; return _pubthis; }
      
      StringBuffer::t p_StringBuffer::lf    () { ss << std::endl; return StringBuffer::t(_pubthis); } 
      StringBuffer::t p_StringBuffer::clear () { ss.str(""); return StringBuffer::t(_pubthis); }

      
      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (const monty::ndarray<std::string,1> & val) { ssadd(ss,val); return _pubthis; }
      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (const monty::ndarray<int,1> & val)         { ssadd(ss,val); return _pubthis; }
      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (const monty::ndarray<long long,1> & val)   { ssadd(ss,val); return _pubthis; }
      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (const monty::ndarray<double,1> & val)      { ssadd(ss,val); return _pubthis; }

      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (const int & val)         { ss << val; return _pubthis; }
      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (const long long & val)   { ss << val; return _pubthis; }
      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (const double & val)      { ss << val; return _pubthis; }
      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (const bool & val)        { ss << val; return _pubthis; }
      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__a (const std::string & val) { ss << val; return _pubthis; }

      
      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__lf    () { ss << std::endl; return StringBuffer::t(_pubthis); } 
      StringBuffer::t p_StringBuffer::__mosek_2fusion_2Utils_2StringBuffer__clear () { ss.str(""); return StringBuffer::t(_pubthis); }

      std::string 
      p_StringBuffer::toString () const           { return std::string(ss.str()); }
    }
  }
}
