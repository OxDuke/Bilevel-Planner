//-*-c++-*-
#ifndef _MONTY_NDARRAY_H_
#define _MONTY_NDARRAY_H_

#include <algorithm>
#include <memory>
#include <vector>
#include <iostream>
#include "monty_shape.h"
#include "monty_base.h"

namespace monty
{
  struct ArrayInitializerException {};
  

  /**\brief This class defines functionality to handle n-dimensional initializer_lists.
   */
  template <typename T, int N>
  struct _ndinitlist
  {
    ///\brief Define the type of an n-dimensional initializer_list
    typedef typename _ndinitlist<T,N-1>::init_t next_init_t;
    typedef std::initializer_list< next_init_t > init_t;
    ///\brief Define the type of an n-dimensional dimension tuple
    typedef shape_t<N> nd_t;

    ///\brief Function that traverses an n-dimensional initializer_list and copies the content to a flat vector
    //
    // \param vals Copy values to this vector
    // \param p    Start at this offset into vals; p is incremented corresponding to values copied

    template<typename I>
    static void __copy(const init_t & init, I & dst)
    {
      for (auto i : init)
        _ndinitlist<T,N-1>::__copy(i,dst);
    }
    
    template<typename I>
    static void copy(const init_t & init, I dst) { I dst_ = dst; __copy(init,dst_); }

    ///\brief Compute the total number of elements in the initializer
    static size_t totalsize(const init_t & init)
    {
      size_t sz = 0;
      for (auto i : init) sz += _ndinitlist<T,N-1>::totalsize(i);
      return sz;
    }
    ///\brief Given an initializer_list and a dimension tuple, validate that the initializer_lists shape matches the tuple
    static bool _validate (const init_t init, nd_t dims)
    {
      if (dims.dim != init.size())
        return false;
      for (auto i : init)
        if (! _ndinitlist<T,N-1>::_validate(i,dims.rest))
          return false;
      return true;
    }

    ///\brief Helper method for dims() method
    static nd_t _dims (const init_t & init)
    {
      return nd_t(init.size(), _ndinitlist<T,N-1>::_dims(*init.begin()));
    }   

    ///\brief Deduce and check the dimension tuple from an initializer_list
    static nd_t dims(const init_t & init)
    {
      auto res = _dims(init);
      if (! _validate(init,res))
        throw ArrayInitializerException();
      return res;
    }
  };

  // specialized class for 1d case of copying nd-initializer to vector
  template <typename T>
  struct _ndinitlist<T,1>
  {
    typedef std::initializer_list<T> init_t;
    typedef shape_t<1> nd_t;

    template<typename I>
    static void __copy(const init_t & init, I & dst) { for (auto i : init) *(dst++) = i; }
    template<typename I>
    static void copy(const init_t & init, I dst) { I dst_ = dst; __copy(init,dst_); }

    static bool _validate (const init_t init, nd_t dims) { return dims.dim == init.size(); }
    static size_t totalsize(const init_t & init) { return init.size(); }
    static nd_t _dims(const init_t & init) { return nd_t(init.size()); }

    static nd_t dims(const init_t & init) { return _dims(init); }
  };


  template <typename T, int N = 1,class Alloc = std::allocator<T>>
  class ndarray
  {
  public:
    const shape_t<N> shape;
  private:
    typedef typename _ndinitlist<T,N>::init_t init_t;
    Alloc mem;
    T * data;

  public:
    typedef std::input_iterator_tag iterator_category;
    typedef ptrdiff_t index_type;
    typedef T         value_type;
    typedef T *       pointer;
    typedef T &       reference;
    typedef const T * const_pointer;
    typedef const T & const_reference;

    typedef const_pointer const_iterator;
    typedef pointer       iterator;


    ///\brief Blank array of a given size
    ndarray(shape_t<N> shape);
    ///\brief Array filled with given value
    ndarray(shape_t<N> shape, T value);
    ///\brief Array filled using a function taking the linear index as argument
    ndarray(const shape_t<N> & shape, const std::function<T(ptrdiff_t)> & f);
    ///\brief Array filled using a function taking the index as argument
    ndarray(const shape_t<N> & shape, const std::function<T(const shape_t<N> &)> & f);
    // construct from pointer
    ndarray(T * vals,shape_t<N> shape);
    // construct from initializer list
    ndarray(const init_t & init);
    // construct from flat iterators
    template<typename Iterator>
    ndarray(const shape_t<N> & shape, Iterator begin, Iterator end);

    // construct from values from some iterable type (ndarray, std::vector, std::list etc.)
    template<typename Iterable>
    ndarray(const shape_t<N> & shape, const typename const_iterable_t<Iterable>::t & that);
    
    template<typename Iterable>
    ndarray(const shape_t<N> & shape, const const_iterable_t<Iterable> & that);

    // copy constructor
    ndarray(const ndarray<T,N,Alloc> & that);
    
    // move constructor
    ndarray(ndarray<T,N,Alloc> && that);

    ~ndarray();

    // linear access
    reference       operator[](index_type idx) { return data[idx]; }
    const_reference operator[](index_type idx) const { return data[idx]; }
    // n-dimensional access
    template <typename... Ts>
    reference       operator()(Ts... idxs) { return data[shape.tolinear(idxs...)]; }
    template <typename... Ts>
    const_reference operator()(Ts... idxs) const { return data[shape.tolinear(idxs...)]; }
    
    reference       operator[](const shape_t<N> & idx)       { return data[shape.tolinear(idx)]; }
    const_reference operator[](const shape_t<N> & idx) const { return data[shape.tolinear(idx)]; }

    // iterators
    iterator       flat_begin()       { return data; }
    iterator       flat_end()         { return data+shape.size; }    
    const_iterator flat_begin() const { return data; }
    const_iterator flat_end()   const { return data+shape.size; }
    
    iterator       begin()            { return data; }
    iterator       end()              { return data+shape.size; }    
    const_iterator begin()      const { return data; }
    const_iterator end()        const { return data+shape.size; }
     
    const_pointer  raw() const        { return data; }
    pointer        raw()              { return data; }

    // size
    size_t size(ptrdiff_t i) const { return shape[i]; }
    size_t size()            const { return shape.size; }

    // formatting
    
    void to_stream (std::ostream & os) const;
  };

  //-------------------------
  // Implementation
  //
  template<typename T, int N, typename Alloc>
  ndarray<T,N,Alloc>::ndarray(shape_t<N> shape) : 
    shape(shape), 
    //data(new T[shape.size])
    data(mem.allocate(shape.size))
  {
      for (ptrdiff_t i = 0; i < shape.size; ++i)
          new(data+i)T();
  }

  ///\brief Array filled with given value
  template<typename T, int N, typename Alloc>
  ndarray<T,N,Alloc>::ndarray(shape_t<N> shape, T value) : 
    ndarray(shape)
  { std::fill(data,data+shape.size,value); }


  ///\brief Array filled using a function taking the linear index as argument
  template<typename T, int N, typename Alloc>
  ndarray<T,N,Alloc>::ndarray(const shape_t<N> & shape, const std::function<T(ptrdiff_t)> & f) :
    ndarray(shape)
  {
    for (ptrdiff_t i = 0; i < shape.size; ++i) data[i] = f(i);
  }

  template<typename T, int N, typename Alloc>
  ndarray<T,N,Alloc>::ndarray(const shape_t<N> & shape, const std::function<T(const shape_t<N>&)> & f) : 
    ndarray(shape)
  {
    auto it = shape.begin();
    for (ptrdiff_t i = 0; i < shape.size; ++i, ++it) data[i] = f(*it);
  }



  // construct from pointer
  template<typename T, int N, typename Alloc>
  ndarray<T,N,Alloc>::ndarray(T * vals,shape_t<N> shape) : 
    ndarray(shape)
  {
    std::copy(vals, vals+shape.size, data);
  }

  // construct from initializer list
  template<typename T, int N, typename Alloc>
  ndarray<T,N,Alloc>::ndarray(const init_t & init) : 
    ndarray(_ndinitlist<T,N>::dims(init))
  {
    _ndinitlist<T,N>::copy(init,data);
  }

  template<typename T, int N, typename Alloc>
  template<typename Iterator>
  ndarray<T,N,Alloc>::ndarray(const shape_t<N> & shape, Iterator begin, Iterator end) : 
    ndarray(shape)
  {  
    if (end - begin != shape.size)
      throw ArrayInitializerException();
    std::copy(begin,end,data);
  }

  template<typename T, int N, typename Alloc>
  template<typename Iterable>
  ndarray<T,N,Alloc>::ndarray(const shape_t<N> & shape, const typename const_iterable_t<Iterable>::t & that) : ndarray(shape, that.begin(),that.end()) {}

  template<typename T, int N, typename Alloc>
  template<typename Iterable>
  ndarray<T,N,Alloc>::ndarray(const shape_t<N> & shape, const const_iterable_t<Iterable> & that) : ndarray(shape, that.itable.begin(),that.itable.end()) {}

  // copy constructor
  template<typename T, int N, typename Alloc>
  ndarray<T,N,Alloc>::ndarray(const ndarray<T,N,Alloc> & that) : 
    ndarray(that.shape)
  { std::copy(that.data,that.data+that.shape.size, data); }
  
  // move constructor
  template<typename T, int N, typename Alloc>
  ndarray<T,N,Alloc>::ndarray(ndarray<T,N,Alloc> && that) : shape(that.shape), data(that.data) { that.data = nullptr; }
  
  // destructor
  template<typename T, int N, typename Alloc>
    ndarray<T,N,Alloc>::~ndarray()
  {
      if (data)
      {
          for (ptrdiff_t i = 0; i < shape.size; ++i)
              (data+i)->~T();
          mem.deallocate(data, shape.size);
      }
  }


  // stream
  template<typename T, int N, typename Alloc>  
  void ndarray<T,N,Alloc>::to_stream (std::ostream & os) const
  {
      os << "[";
      if (shape.size > 0) 
      {
          os << data[0]; 
          for (ptrdiff_t i = 1; i < shape.size; ++i)
              os << "," << data[i];
      }
      os << "]";
  }

  template<typename T, int N=1>
  ndarray<T,N> 
  make_array(const typename _ndinitlist<T,N>::init_t & init) { return ndarray<T,N>(init); }
  
  template<typename T, int N=1>
  ndarray<T,N> 
  new_array(const typename _ndinitlist<T,N>::init_t & init) { return new ndarray<T,N>(init); }

  template<typename T, int N=1>
  std::shared_ptr<ndarray<T,N>>
  new_array_ptr(typename _ndinitlist<T,N>::init_t init) { return std::shared_ptr<ndarray<T,N>>(new ndarray<T,N>(init)); }
  
  template<typename T, int N=1>
  std::shared_ptr<ndarray<T,N>>
  new_array_ptr(shape_t<N> sz) { return std::shared_ptr<ndarray<T,N>>(new ndarray<T,N>(sz)); }

  // 1D vector to ndarray
  template<typename T>
  std::shared_ptr<ndarray<T,1>>
  new_array_ptr(const std::vector<T> & x) { 
      return std::make_shared<ndarray<T,1>>(shape(x.size()), x.begin(), x.end()); 
  }

  // 2D vector to ndarray
  template<typename T>
  std::shared_ptr<ndarray<T,2>>
  new_array_ptr(const std::vector<std::vector<T>> & X) { 
      int n = X.size(), m = 0;
      if (n > 0) m = X[0].size();
      for(int i = 0; i < n; i++)
          if (X[i].size() != m) 
              throw ArrayInitializerException();
      return std::make_shared<ndarray<T,2>>(
                  shape(n,m), 
                  std::function<T(const shape_t<2> &)>([X](const shape_t<2> & i) { return X[i[0]][i[1]]; }));
  }

  // 1D ndarray to vector
  template<typename T>
  std::vector<T>
  new_vector_from_array_ptr(std::shared_ptr<ndarray<T,1>> & a) { 
      return std::vector<T>((*a).begin(), (*a).end());
  }

  template<typename T, int N, typename Alloc>
  std::ostream & operator<<(std::ostream & os, const ndarray<T,N,Alloc> & a) { a.to_stream(os); return os; }
}

#endif
