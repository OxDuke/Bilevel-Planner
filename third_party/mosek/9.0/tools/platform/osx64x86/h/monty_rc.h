#ifndef _MONTY_RC_H_
#define _MONTY_RC_H_

#include <stddef.h>
#include <iostream>

namespace monty
{
  class RefCounted;

  template <class T>
  struct RefCounted_traits {
    typedef RefCounted parent;
  };

  ///\brief reference counted pointer
  template<typename T>
  class rc_ptr
  {
  private:
    //typedef typename T::_descended_from_RefCounted _validate_descended_from_RefCounted; // make sure we are descended from RefCounted
    T * ref;
    template<typename S> friend class rc_ptr; // Should be for T <: S, but we cant write that
  public:
    typedef T * pointer;
    typedef T & reference;

    rc_ptr();
    rc_ptr(pointer ptr); // from pointer
    rc_ptr(rc_ptr<T> && ptr); 
    rc_ptr(const rc_ptr<T> & that); // const copy

    
    ~rc_ptr();

    reference operator*()  const;
    pointer   operator->() const;
    rc_ptr<T> operator=(const rc_ptr<T> & other);
    rc_ptr<T> operator=(pointer other);
    bool      operator==(const rc_ptr<T> & other);

    pointer get() const;
    void reset();

    template<class S>
    rc_ptr<S> as() { return rc_ptr<S>(ref); }

    // cast to pointer to immediate super-class
    operator rc_ptr<typename RefCounted_traits<T>::parent>() { return rc_ptr<typename RefCounted_traits<T>::parent>(ref); }

    //operator bool() { return ref != nullptr; }
  };



  class RefCounted
  {
    typedef int _descended_from_RefCounted;
    public:
  //protected:
    RefCounted() : refcount(0) {}
  //private:
    typedef ptrdiff_t count_t;
    count_t refcount;
    count_t refincr() { return ++refcount; }
    count_t refdecr() { return --refcount; }

    template<typename T> friend class rc_ptr; // allow rc_ptr to fiddle with the private parts

  //public:
    ~RefCounted()
    {
      //std::cout << "delete " << this << std::endl;
      refcount = -9999;
    }
  };

  class rc_reserve
  {
  private:
    RefCounted * ref;
  public:
    rc_reserve(RefCounted * ref) : ref(ref) { ref->refincr(); }
    ~rc_reserve() { ref->refdecr(); }
  };
  
  //----------------------------------
  // rc_ptr implementation

  // constructors
  template<typename T> rc_ptr<T>::rc_ptr(pointer that)           : ref(that)      
  { 
    if (ref) ref->refincr(); 
  }
  //template<typename T>
  //template<typename S> rc_ptr<T>::rc_ptr(const rc_ptr<S> & that) : ref(that.ref)  
  //{ 
  //  if (ref) ref->refincr(); 
  // }
  template<typename T> rc_ptr<T>::rc_ptr(const rc_ptr<T> & that) : ref(that.ref)             
  { 
    if (ref) ref->refincr(); 
  }

  template<typename T> rc_ptr<T>::rc_ptr(rc_ptr<T> && that) : ref(that.ref)
  {
      that.ref = NULL;;
  }

  template<typename T> rc_ptr<T>::rc_ptr() : ref(nullptr)
  { 
  }

  // destructor
  template<typename T> rc_ptr<T>::~rc_ptr()                                       
  { 
    if (ref && ref->refdecr() == 0) delete ref; 
  }

  // members

  template<typename T> typename rc_ptr<T>::reference rc_ptr<T>::operator*() const { return *ref; }
  template<typename T> typename rc_ptr<T>::pointer rc_ptr<T>::operator->()  const { return  ref; }
  template<typename T> void     rc_ptr<T>::reset()                                
  { 
    if (ref)
      if (ref->refdecr() == 0) 
        delete ref; 
    ref = NULL;
  }
  template<typename T> typename rc_ptr<T>::pointer rc_ptr<T>::get() const         { return ref; }

  template<typename T> rc_ptr<T> rc_ptr<T>::operator=(const rc_ptr<T> & other)
  {
    if (other.ref != nullptr)
     other.ref->refincr(); // if this==other, we have to increment other before we decrement this.
    reset();
    ref = other.ref;
    return *this;
  }
  template<typename T> rc_ptr<T> rc_ptr<T>::operator=(T * other)
  {
    if (other != nullptr)
      other->refincr();
    reset();
    ref = other;
    return *this;
  }

  template<typename T> bool      rc_ptr<T>::operator==(const rc_ptr<T> & other) { return ref == other.ref; }

  template<typename T> bool operator==(const rc_ptr<T> & lhs,const rc_ptr<T> & rhs) { return lhs.get() == rhs.get(); }
  template<typename T> bool operator!=(const rc_ptr<T> & lhs,const rc_ptr<T> & rhs) { return lhs.get() != rhs.get(); }
  template<typename T> bool operator==(const rc_ptr<T> & lhs,const T * rhs) { return lhs.get() == rhs; }
  template<typename T> bool operator==(const T * lhs,const rc_ptr<T> & rhs) { return lhs == rhs.get(); }
  template<typename T> bool operator!=(const rc_ptr<T> & lhs,const T * rhs) { return lhs.get() != rhs; }
  template<typename T> bool operator!=(const T * lhs,const rc_ptr<T> & rhs) { return lhs != rhs.get(); }

}

#endif
