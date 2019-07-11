#ifndef _MONTY_BASE_H_
#define _MONTY_BASE_H_

#include <stddef.h>
#include <cassert>

namespace monty
{
  template<typename T> struct _integer_type { };
  template<>           struct _integer_type<long long int>          { typedef long long int          t; };
  template<>           struct _integer_type<unsigned long long int> { typedef unsigned long long int t; };
  template<>           struct _integer_type<int>                    { typedef int                    t; };
  template<>           struct _integer_type<unsigned int>           { typedef unsigned int           t; };
  template<>           struct _integer_type<short>                  { typedef short int              t; };
  template<>           struct _integer_type<unsigned short>         { typedef unsigned short int     t; };
  template<>           struct _integer_type<signed char>            { typedef signed char            t; };
  template<>           struct _integer_type<unsigned char>          { typedef unsigned char          t; };


  template <typename Iterable> 
  struct const_iterable_t
  { 
    typedef Iterable t; 
    typedef typename t::const_iterator const_iterator; 
    Iterable itable;

    const_iterable_t(const Iterable & itable) : itable(itable) {  }
    const_iterator begin() { return itable.begin(); }
    const_iterator end()   { return itable.end(); }
  };

  template<typename T>
  const_iterable_t<T> iterable(const T & itable)
  {
    return const_iterable_t<T>(itable);
  }

}

#endif
