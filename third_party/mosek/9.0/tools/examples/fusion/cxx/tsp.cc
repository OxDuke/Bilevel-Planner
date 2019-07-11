//
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      tsp.cc
//
//  Purpose: Demonstrates a simple technique to the TSP
//           usign the Fusion API.
//
#include <iostream>
#include <list>
#include <vector>

#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;


void tsp(int n, Matrix::t A, Matrix::t C, bool remove_1_hop_loops, bool remove_2_hop_loops)
{
  Model::t M = new Model();
  auto M_ = finally([&]() { M->dispose(); });

  auto x = M->variable("x", Set::make(n,n), Domain::binary());

  M->constraint(Expr::sum(x, 0), Domain::equalsTo(1.0));
  M->constraint(Expr::sum(x, 1), Domain::equalsTo(1.0));
  M->constraint(x, Domain::lessThan( A ));

  M->objective(ObjectiveSense::Minimize, Expr::dot(C, x));

  if (remove_1_hop_loops)
    M->constraint(x->diag(), Domain::equalsTo(0.));

  if (remove_2_hop_loops)
    M->constraint(Expr::add(x, x->transpose()), Domain::lessThan(1.0));

  int it = 0;
  while (true)
  {
    M->solve();
    it++;

    typedef std::vector< std::tuple<int, int> > cycle_t;
    std::list< cycle_t > cycles;

    auto xlevel = x->level();

    for (int i = 0; i < n; i++)
      for (int j = 0; j < n; j++)
      {

        if ( (*xlevel)[i * n + j] <= 0.5 )
          continue;

        bool found = false;
        for (auto && c : cycles)
        {
          for (auto && cc : c)
          {
            if ( i == std::get<0>(cc) || i == std::get<1>(cc) ||
                 j == std::get<0>(cc) || j == std::get<1>(cc) )
            {
              c.push_back( std::make_tuple(i, j) );
              found = true;
              break;
            }
          }
          if (found) break;
        }

        if (!found)
          cycles.push_back(cycle_t(1, std::make_tuple(i, j)));
      }

    std::cout << "Iteration " << it << "\n";
    for (auto c : cycles) {
      for (auto cc : c)
        std::cout << "(" << std::get<0>(cc) << "," << std::get<1>(cc) << ") ";
      std::cout << "\n";
    }

    if (cycles.size() == 1) break;

    for (auto c : cycles)
    {
      int csize = c.size();

      auto tmp = std::shared_ptr<monty::ndarray<int, 2> >(new  ndarray<int, 2>( shape(csize, 2)) );
      for (auto i = 0; i < csize; ++i)
      {
        (*tmp)(i, 0) = std::get<0>(c[i]);
        (*tmp)(i, 1) = std::get<1>(c[i]);
      }

      M->constraint(Expr::sum(x->pick(tmp)), Domain::lessThan( 1.0 * csize - 1 ));
    }
  }
  try {
    auto xlevel = x->level();
    std::cout << "Solution\n";
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++)
        std::cout << (int) (*xlevel)(i * n + j);
      std::cout << "\n";
    }
  } catch (...) {}
}

int main()
{
  auto A_i = new_array_ptr<int, 1>({0, 1, 2, 3, 1, 0, 2, 0});
  auto A_j = new_array_ptr<int, 1>({1, 2, 3, 0, 0, 2, 1, 3});

  auto C_v = new_array_ptr<double, 1>({1., 1., 1., 1., 0.1, 0.1, 0.1, 0.1});

  int n = 4;
  tsp(n, Matrix::sparse(n, n, A_i, A_j, 1.), Matrix::sparse(n, n, A_i, A_j, C_v), true, false);
  tsp(n, Matrix::sparse(n, n, A_i, A_j, 1.), Matrix::sparse(n, n, A_i, A_j, C_v), true, true);

  return 0;
}