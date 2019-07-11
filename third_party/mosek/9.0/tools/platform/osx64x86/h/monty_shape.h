//-*-c++-*-
#ifndef _MONTY_SHAPE_H_
#define _MONTY_SHAPE_H_

#include <functional>
#include <sstream>
#include <stddef.h>


namespace monty
{
  template <int N> struct shape_t;

  template<int N>
  struct shape_iterator
  {
    typedef shape_iterator<N> t;
    typedef std::input_iterator_tag iterator_category;
    typedef ptrdiff_t    difference_type;
    typedef shape_t<N>   value_type;
    typedef value_type & reference;
    typedef value_type * pointer;
    
    const value_type shape;
    ptrdiff_t        pos;
    shape_iterator(const shape_t<N> & shape, ptrdiff_t pos);

    value_type       operator*();
    // pointer       operator->() // not possible
    bool             operator==(const t & that);
    bool             operator!=(const t & that);
    t &              operator++();
    t &              operator--();
    t &              operator+=(difference_type d);
    t &              operator-=(difference_type d);
    t                operator+ (difference_type d);
    t                operator- (difference_type d);
    difference_type  operator- (const t & that);
  };  

  /**\brief Given n this define an n-tuple of size_ts.
   */
  template<int N>
  class shape_t
  {    
  public:
    typedef size_t dim_t;
  private:
    void _bldstr(std::stringstream & ss) const { ss << "," << dim; rest._bldstr(ss); }    
    template<typename... Is> dim_t _tolinear(dim_t l, dim_t i0, Is... idxs) const { return rest._tolinear(l*dim+i0,idxs...); }
  public:
    friend class shape_t<N+1>;
    typedef shape_iterator<N> iterator;
    typedef shape_iterator<N> const_iterator;

    const dim_t        dim;
    const shape_t<N-1> rest;
    const size_t       size; // total number of elements in this shape

    template <typename... Ts>
    // given N values construct an N-dimensional shape from those vaues
    shape_t(dim_t i0, Ts... is) : dim(i0), rest(is...), size(dim*rest.size) {}
    // given a value and an <N-1>-dimensional shape, create an N-dimensional shape
    shape_t(dim_t i0, shape_t<N-1> is) : dim(i0), rest(is), size(dim*rest.size) {}
    // given a shape<N> and a linear index, construct the shape<N> as the N-dimensional point corresponding to the linear index
    shape_t(const shape_t<N> & shp, ptrdiff_t i) : dim(i/shp.rest.size), rest(shp.rest,i%shp.rest.size), size(dim*rest.size) {}
    // copy
    shape_t(const shape_t<N> & that) : dim(that.dim), rest(that.rest), size(that.size) {}

    dim_t operator[](ptrdiff_t i) const { return i == 0 ? dim : rest[i-1]; }    

    dim_t tolinear(const shape_t<N> & point) const { return point.dim*rest.size + rest.tolinear(point.rest); }
    template<typename... Is> dim_t tolinear(Is... idxs) const { return _tolinear(0,idxs...); }

    
    const_iterator begin() const { return iterator(*this, 0); } 
    const_iterator end()   const { return iterator(*this, size); } 

    operator std::string() const
    {
      std::stringstream ss;
      ss << "(" << dim;
      rest._bldstr(ss);
      ss << ")";
      return ss.str();
    }
  };

  template<>
  class shape_t<1>
  {
  public:
    typedef size_t dim_t;
  private:
    dim_t _tolinear(dim_t l, dim_t i0) const { return l*dim+i0; }
    void _bldstr(std::stringstream & ss) const { ss << "," << dim; }
  public:
    friend class shape_t<2>;

    typedef shape_iterator<1> iterator;
    typedef shape_iterator<1> const_iterator;

    const dim_t dim;
    const size_t size;

    shape_t(dim_t i0) : dim(i0),size(i0) {}
    shape_t(const shape_t<1> & shp, ptrdiff_t i) : dim(i), size(dim) {}

    dim_t operator[](ptrdiff_t i) const { return dim; }
    dim_t tolinear(dim_t i0) const { return _tolinear(0,i0); }
    dim_t tolinear(const shape_t<1> & point) const { return point.dim; }

    const_iterator begin() const { return iterator(*this, 0); } 
    const_iterator end()   const { return iterator(*this, size); } 


    operator std::string() const
    {
      std::stringstream ss;
      ss << "(" << dim << ")";
      return ss.str();
    }
  };

  ///\brief construction used to count the number of template arguments at compile time
  template <typename T, typename... Ts> struct _count_args    { static const int num = 1+_count_args<Ts...>::num; };
  template <typename T>                 struct _count_args<T> { static const int num = 1; };

  //-------------------------------
  // shape_iterator implementation

  template<int N>
  shape_iterator<N>::shape_iterator(const shape_t<N> & shape, ptrdiff_t pos) : shape(shape), pos(pos) {}

  template<int N> shape_t<N> shape_iterator<N>::operator*()
  {
    return shape_t<N>(shape,pos);
  }

  template<int N> bool shape_iterator<N>::operator==(const shape_iterator<N> & that) { return pos == that.pos; }
  template<int N> bool shape_iterator<N>::operator!=(const shape_iterator<N> & that) { return pos != that.pos; } 

  template<int N> shape_iterator<N> & shape_iterator<N>::operator++() { ++pos; return *this; }
  template<int N> shape_iterator<N> & shape_iterator<N>::operator--() { --pos; return *this; }
  template<int N> shape_iterator<N> & shape_iterator<N>::operator+=(difference_type d) { pos+=d; return *this; }
  template<int N> shape_iterator<N> & shape_iterator<N>::operator-=(difference_type d) { pos-=d; return *this; }
  
  template<int N> shape_iterator<N>   shape_iterator<N>::operator+(difference_type d) { return shape_iterator<N>(shape,pos+d); }
  template<int N> shape_iterator<N>   shape_iterator<N>::operator-(difference_type d) { return shape_iterator<N>(shape,pos-d); }

  template<int N> 
  typename shape_iterator<N>::difference_type
  shape_iterator<N>::operator-(const shape_iterator & that) { return pos - that.shape; }

  ///\brief function that creates a shape_t object (without having to specity the number of dimensions as a template argument)
  template <typename... Ts>
  shape_t<_count_args<Ts...>::num> shape(Ts... idxs) { return shape_t<_count_args<Ts...>::num>(idxs...); }

  template <int num>
  std::ostream & operator<<(std::ostream & os, const shape_t<num> & shape) { return os << (std::string)shape; }
}

#endif
