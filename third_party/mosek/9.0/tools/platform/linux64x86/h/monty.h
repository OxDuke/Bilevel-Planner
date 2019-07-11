#ifndef _MONTY_H_
#define _MONTY_H_

#include "monty_rc.h"
#include "monty_shape.h"
#include "monty_ndarray.h"
#include "monty_iterator.h"

#include <initializer_list>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <sstream>
#include <iostream>
#include <stddef.h>
#include <assert.h>
#include <cstddef>
#include <stdexcept>


namespace monty
{
  /**\brief Implements the functionality of the 'finally' construction.
   *
   * Usage:
   * <pre>
   *   {
   *     finally _f([&] { finally code block })
   *     do something;
   *   }
   * </pre>
   * This is used to declare a code block that will be executed when its scope ends.
   */
  class finally
  {
  private:
    std::function<void(void)> func;
  public:
    finally(std::function<void(void)> func) : func(func) {}
    ~finally() { func(); }
  };

  class Throwable : public std::runtime_error
  {
    private:
      std::string msg;
    public:
      //Throwable(const std::string & msg) : msg(msg) {}
      Throwable(const std::string & msg) : std::runtime_error(msg.c_str()),msg(msg) {} // hmm... ok to steal c_str()?
      Throwable(const char * msg)        : std::runtime_error(msg), msg(msg) {}
      Throwable()                        : std::runtime_error(""), msg("") {}

      operator std::string() const { return msg; }
      const std::string & toString() const { return msg; }
  };

  class Exception : public Throwable
  {
    public:
      //Exception(const std::string & msg) : Throwable(msg) {}
      Exception(const std::string & msg) : Throwable(msg) {}
      Exception(const char * msg)        : Throwable(msg) {}
      Exception()                        : Throwable() {}
  };

  class RuntimeException : public Throwable
  {
    public:
      //RuntimeException(const std::string & msg) : Throwable(msg) {}
      RuntimeException(const std::string & msg) : Throwable(msg) {}
      RuntimeException(const char * msg)        : Throwable(msg) {}
      RuntimeException()                        : Throwable() {}
  };

  class NullPointerException : public RuntimeException
  {
    public:
      NullPointerException(const std::string & msg) : RuntimeException(msg) {}
      NullPointerException(const char * msg)        : RuntimeException(msg) {}
      NullPointerException()                        : RuntimeException() {}
  };

  class AbstractClassError : public RuntimeException
  {
    public:
      AbstractClassError(const std::string & msg) : RuntimeException(msg) {}
      AbstractClassError(const char * msg)        : RuntimeException(msg) {}
      AbstractClassError()                        : RuntimeException() {}
  };




  template<typename T> struct cast 
  { 
  };
  template<> struct cast<double>    
  { 
    static double t(double    v) { return v; }
    static double t(int       v) { return v; }
    static double t(long long v) { return v; }
  };
  template<> struct cast<long long>    
  { 
    static long long t(long long v) { return v; }
    static long long t(int       v) { return v; }
  };
  template<> struct cast<int>    
  { 
    static int t(int v)       { return v; }
    static int t(long long v) { assert(v == (v & 0xffffffff)); return (int) v; }
  };
}

#endif
