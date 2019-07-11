#ifndef _MONTY_ITERATOR_H_
#define _MONTY_ITERATOR_H_

#include <stddef.h>
#include "monty_base.h"
#include <cassert>
#include <iostream>

namespace monty
{
  template<typename T>
  struct range_iterator
  {
    typedef typename _integer_type<T>::t t; // ensure integer type
    typedef T         difference_type;
    typedef T         value_type;
    typedef const T * pointer;
    typedef const T & reference;
    typedef std::input_iterator_tag iterator_category;

    T i, step;
    range_iterator() = delete;
    range_iterator(T start,T step) : i(start),step(step) {}
    range_iterator(const range_iterator<T> & that) : i(that.i), step(that.step) {assert(step != 0);}

    range_iterator & operator++() { i += step; return *this; }
    range_iterator & operator--() { i -= step; return *this; }
    range_iterator   operator++(int) { range_iterator<T> tmp(*this); i += step; return tmp; }
    range_iterator   operator--(int) { range_iterator<T> tmp(*this); i -= step; return tmp; }
    range_iterator & operator+=(ptrdiff_t d) { i += step*d; return *this; }
    range_iterator & operator-=(ptrdiff_t d) { i -= step*d; return *this; }
    range_iterator   operator+(ptrdiff_t d)  { range_iterator<T> r(*this); r += d; return r; }
    range_iterator   operator-(ptrdiff_t d)  { range_iterator<T> r(*this); r -= d; return r; }
    T    operator- (const range_iterator<T> & other) const { assert(step != 0); return step == 0 ? 0 : (i-other.i)/step; }
    bool operator==(const range_iterator<T> & other) const { return i == other.i; }
    bool operator!=(const range_iterator<T> & other) const { return i != other.i; }
    T    operator*() const { return i; }
  };

  template<typename T>
  struct range_t
  {
    typedef range_iterator<T>       iterator;
    typedef range_iterator<T> const_iterator;

    const typename _integer_type<T>::t start, step;
    ptrdiff_t num;



    range_t() = delete;
    range_t(T start, T stop);
    range_t(T start, T stop, T step);
    range_t(const range_t<T> & other) : start(other.start), step(other.step), num(other.num) { assert(step != 0); }

    range_t reverse() const;
    size_t size() const;

    const_iterator begin() const;
    const_iterator end()   const;
    const_iterator reverse_begin() const;
    const_iterator reverse_end() const;
  };

  //-----------------------------------
  // Map iterator
  //

  template<typename T, typename S, typename Iterator>
  struct generator_iterator
  {
    typedef generator_iterator<T,S,Iterator> t;    
    typedef std::function<T(S)> generator_type;

    typedef ptrdiff_t difference_type;
    typedef std::input_iterator_tag iterator_category;
    typedef T   value_type;
    typedef T & reference;
    typedef T * pointer;
    
    generator_type func;
    Iterator       pos;

    // from iterator
    generator_iterator(const generator_type func, Iterator pos) : func(func), pos(pos) {}
    // copy
    generator_iterator(const t & that) : func(that.func), pos(that.pos) {}
    
    value_type      operator*() { return func(*pos); }
    bool            operator==(const t & that) { return pos == that.pos; }
    bool            operator!=(const t & that) { return pos != that.pos; }
    t &             operator++() { ++pos; return *this; } 
    t &             operator--() { --pos; return *this; } 
    t &             operator+=(difference_type d) { pos += d; return *this; } 
    t &             operator-=(difference_type d) { pos -= d; return *this; } 

    t               operator+(difference_type d) { return t(func,pos+d); }
    t               operator-(difference_type d) { return t(func,pos-d); } 
    difference_type operator-(const t & that) { return pos - that.pos;  } 
  };
  
  
  ///\brief Iterable generator object
  //
  // Given either begin/end iterators or an iterable object, 
  template<typename T, typename S, typename Iterator>
  struct generator_t
  {
    typedef generator_iterator<T,S,Iterator> iterator;
    typedef generator_iterator<T,S,Iterator> const_iterator;
    typedef std::function<T(S)>              generator_type;

    std::function<T(S)> func;
    Iterator orig_begin;
    Iterator orig_end;
    
    generator_t(generator_type func, const Iterator & begin, const Iterator & end) : func(func), orig_begin(begin), orig_end(end) {}
    template<typename Iterable>
    generator_t(generator_type func, Iterable & v) : func(func), orig_begin(v.begin()), orig_end(v.end()) {}
    generator_t(const generator_t<T,S,Iterator> & that) : func(that.func), orig_begin(that.orig_begin), orig_end(that.orig_end) {}

    iterator begin() const { return iterator(func,orig_begin); }
    iterator end()   const { return iterator(func,orig_end); }
  };

  template<typename T, typename S, typename Iterator>
  generator_t<T,S,Iterator> 
  generator(std::function<T(S)> func, const Iterator & begin, const Iterator & end) 
  { return generator_t<T,S,Iterator>(func,begin,end); }

  template<typename T, typename S, typename Iterator>
  generator_t<T,S,Iterator> 
  generator(T (*func)(S), const Iterator & begin, const Iterator & end) 
  { 
    return generator_t<T,S,Iterator>(std::function<T(S)>(func),begin,end); 
  }

  template<typename T, typename S, typename Iterable>
  generator_t<T,S,typename Iterable::iterator> 
  generator(std::function<T(S)> func, const Iterable & v) 
  { return generator_t<T,S,typename Iterable::iterator>(func,v.begin(),v.end()); }

  template<typename T, typename S, typename Iterable>
  generator_t<T,S,typename Iterable::iterator> 
  generator(T (*func)(S), const Iterable & v) 
  { return generator_t<T,S,typename Iterable::iterator>(std::function<T(S)>(func),v.begin(),v.end()); }
  
  template<typename T, typename Iterable> // map with identity
  generator_t<T,T,typename Iterable::iterator> 
  generator(Iterable & v) 
  { 
    std::function<T(T)> id([](const T & i){return i;});
    return generator_t<T,T,typename Iterable::iterator> (id,v.begin(),v.end()); 
  }


  //-----------------------------------
  // Range iterator implementation  

  template<typename T>  
  range_t<T>::range_t(T start, T stop) : 
    start(start), 
    step(start <= stop ? 1 : -1),
    num(start <= stop && step > 0 ? (stop-start-1)/step+1 :
          stop <= start && step < 0 ? (stop-start+1)/step+1 : 0) 
  {
    assert(this->step != 0);
  }

  template<typename T>  
  range_t<T>::range_t(T start, T stop, T step) : 
    start(start), 
    step(start <= stop && step > 0 ? step : 
           stop <= start && step < 0 ? step :
             0), // ok since start==stop in this case
    num(start <= stop && step > 0 ? (stop-start-1)/step+1 :
          stop <= start && step < 0 ? (stop-start+1)/step+1 :
            0)
  {
    assert(this-> step!= 0);
  }

  template<typename T>  
  range_iterator<T> range_t<T>::begin()         const { return range_iterator<T>(start,step); }
  
  template<typename T>  
  range_iterator<T> range_t<T>::end()           const { return range_iterator<T>(start,step)+num; }
     
  template<typename T>  
  range_iterator<T> range_t<T>::reverse_begin() const { return range_iterator<T>(start+(step-1)*num,-step); } 

  template<typename T>  
  range_iterator<T> range_t<T>::reverse_end()   const { return range_iterator<T>(start-step,-step); }

  template<typename T>
  range_t<T> range_t<T>::reverse() const { return range_t<T>(start+(num-1)*step,start-step,-step); }
  
  template<typename T>
  size_t range_t<T>::size() const { return (size_t)num; }

  // functions so we dont have to specify template arguments explicitly
  template<typename T> range_t<T> range(T start, T stop, T step) { return range_t<T>(start,stop,step); }
  template<typename T> range_t<T> range(T start, T stop)         { return range_t<T>(start,stop); }
}


#endif
