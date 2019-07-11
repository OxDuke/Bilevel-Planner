//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      breaksolver.cc
//
//  Purpose: Show how to break a long-running task.
//
//  Requires a parameter defining a timeout in seconds.
//

#include <fusion.h>
#include <iostream>
#include <random>
#include <thread>
#include <stdlib.h>
#include <time.h>
#include <functional>
#include <algorithm>
#include <vector>
#include <stdlib.h>


#include "assert.h"

using namespace mosek::fusion;
using namespace monty;


template<class T>
static std::ostream & operator<<(std::ostream & strm, const std::vector<T> & arr)
{
  strm << "[";
  if (arr.size() > 0)
    strm << arr[0];
  for (auto iter = arr.begin() + 1; iter != arr.end(); ++iter)
    strm << " ," << *iter;

  strm << "]";
  return strm;
}

int main(int argc, char ** argv)
{
  int timeout = 5;

  int n = 200;   // number of binary variables
  int m = n / 3; // number of constraints
  int p = n / 5; // Each constraint picks p variables and requires that exactly half of them are 1

  std::cout << "Build problem...\n";


  std::vector<int> idxs(n); for (int i = 0; i < n; ++i) idxs[i] = i;
  std::shared_ptr< ndarray<int> > cidxs(new ndarray<int>(shape(p)));
  //auto rand = std::bind(std::uniform_int_distribution<int>(0,n-1), std::mt19937(0));

  Model::t M = new Model("SolveBinary"); auto _M = finally([&]() { M->dispose(); } );
  M->setLogHandler([](const std::string & msg) { std::cout << msg << std::flush; } );
  Variable::t x = M->variable("x", n, Domain::binary());

  for (int i = 0; i < m; ++i)
  {
    std::random_shuffle(idxs.begin(), idxs.end(), [ = ](int i) { return std::rand() % i; });
    std::copy(idxs.begin(), idxs.begin() + p, cidxs->begin());
    M->constraint(Expr::sum(x->pick(cidxs)), Domain::equalsTo(p / 2));
  }
  M->objective(ObjectiveSense::Minimize, Expr::sum(x));

  std::cout << "Start thread...\n";
  bool alive = true;
  std::thread T(std::function<void(void)>([&]() { M->solve(); alive = false; }) );

  time_t T0 = time(NULL);
  while (true)
  {
    if (time(NULL) - T0 > timeout)
    {
      std::cout << "Solver terminated due to timeout!\n";
      M->breakSolver();
      T.join();
      break;
    }
    if (! alive)
    {
      std::cout << "Solver terminated before anything happened!\n";
      T.join();
      break;
    }
  }
  return 0;
}