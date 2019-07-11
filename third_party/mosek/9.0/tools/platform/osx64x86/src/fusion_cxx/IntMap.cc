#include "fusion_p.h"
#include <string>
#include "monty.h"

namespace mosek
{
  namespace fusion
  {
    namespace Utils
    {
      IntMap::IntMap() : _impl(new p_IntMap(this)) {}
      bool IntMap::hasItem (long long key) { return _impl->hasItem(key); }
      int  IntMap::getItem (long long key) { return _impl->getItem(key); }
      void IntMap::setItem (long long key, int val) { _impl->setItem(key,val); }
      std::shared_ptr<monty::ndarray<long long,1>> IntMap::keys() { return _impl->keys(); }
      std::shared_ptr<monty::ndarray<int,1>>       IntMap::values() { return _impl->values(); }

      IntMap::t IntMap::clone() { return _impl->clone(); }
      IntMap::t IntMap::__mosek_2fusion_2Utils_2IntMap__clone() { return _impl->clone(); }
      IntMap::t p_IntMap::__mosek_2fusion_2Utils_2IntMap__clone() { return clone(); }
      IntMap::t p_IntMap::clone()
      {
          IntMap::t pubres = _new_IntMap();
          p_IntMap * res = p_IntMap::_get_impl(pubres.get());
          for (auto iter = m.begin(); iter != m.end(); ++iter)
              res->m[iter->first] = iter->second;

          return pubres;
      }
    




      StringIntMap::StringIntMap() : _impl(new p_StringIntMap(this)) {}
      bool StringIntMap::hasItem (const std::string & key) { return _impl->hasItem(key); }
      int  StringIntMap::getItem (const std::string & key) { return _impl->getItem(key); }
      void StringIntMap::setItem (const std::string & key, int val) { _impl->setItem(key,val); }
      std::shared_ptr<monty::ndarray<std::string,1>> StringIntMap::keys() { return _impl->keys(); }
      std::shared_ptr<monty::ndarray<int,1>>       StringIntMap::values() { return _impl->values(); }

      StringIntMap::t StringIntMap::clone() { return _impl->clone(); }
      StringIntMap::t StringIntMap::__mosek_2fusion_2Utils_2StringIntMap__clone() { return _impl->clone(); }
      StringIntMap::t p_StringIntMap::__mosek_2fusion_2Utils_2StringIntMap__clone() { return clone(); }
      StringIntMap::t p_StringIntMap::clone() {
          StringIntMap::t pubres = _new_StringIntMap();
          p_StringIntMap * res = p_StringIntMap::_get_impl(pubres.get());
          for (auto iter = m.begin(); iter != m.end(); ++iter)
              res->m[iter->first] = iter->second;

          return pubres;
      }
  

    }
  }
}
