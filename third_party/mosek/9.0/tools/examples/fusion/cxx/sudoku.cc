//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      sudoku.cc
//
// Purpose:  A MILP-based SUDOKU solver
//
//

#include <iostream>
#include <sstream>
#include <cmath>

#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

void print_solution(int n, Variable::t X)
{
  using namespace std;

  cout << "\n";
  int m( std::sqrt(n) );
  for (int i = 0; i < n; i++)
  {
    stringstream ss;

    for (int j = 0; j < n; j++)
    {
      if (j % m == 0) ss << " |";

      for (int k = 0; k < n; k++)
      {
        auto x = X->index(new_array_ptr<int, 1>({i, j, k}))->level();
        if ( (*x)[0] > 0.5 )
        {
          ss << " " << (k + 1);
          break;
        }
      }
    }
    cout << ss.str() << " |";

    cout << "\n";
    if ((i + 1) % m == 0)
      cout << "\n";
  }
}

int main(int argc, char ** argv)
{

  int m = 3;
  int n = m * m;

  //fixed cells in human readable (i.e. 1-based) format
  auto hr_fixed = new_array_ptr<int, 2>(
  { {1, 5, 4},
    {2, 2, 5}, {2, 3, 8}, {2, 6, 3},
    {3, 2, 1}, {3, 4, 2}, {3, 5, 8}, {3, 7, 9},
    {4, 2, 7}, {4, 3, 3}, {4, 4, 1}, {4, 7, 8}, {4, 8, 4},
    {6, 2, 4}, {6, 3, 1}, {6, 6, 9}, {6, 7, 2}, {6, 8, 7},
    {7, 3, 4}, {7, 5, 6}, {7, 6, 5}, {7, 8, 8},
    {8, 4, 4}, {8, 7, 1}, {8, 8, 6},
    {9, 5, 9}
  }
  );

  int nfixed = hr_fixed->size() / m;

  Model::t M = new Model("SUDOKU"); auto _M = finally([&]() { M->dispose(); });

  M->setLogHandler([ = ](const std::string & msg) { std::cout << msg << std::flush; });

  Variable::t X = M->variable("X", new_array_ptr<int, 1>({n, n, n}), Domain::binary());

  //each value only once per dimension
  for (int d = 0; d < m; d++)
    M->constraint( Expr::sum(X, d), Domain::equalsTo(1.) );

  //each number must appear only once in a block
  for (int k = 0; k < n ; k++)
    for (int i = 0; i < m ; i++)
      for (int j = 0; j < m ; j++)
        M->constraint( Expr::sum( X->slice( new_array_ptr<int, 1>({i * m, j * m, k}),
                                            new_array_ptr<int, 1>({(i + 1)*m, (j + 1)*m, k + 1}) ) ),
                       Domain::equalsTo(1.) );

  auto fixed = std::shared_ptr< ndarray<int, 2> >( new ndarray<int, 2>( shape(nfixed, 3) ) );

  for (int i = 0; i < nfixed; i++)
    for (int d = 0; d < m; d++)
      (*fixed)(i, d) =  (*hr_fixed)(i, d) - 1;

  M->constraint( X->pick( fixed ) , Domain::equalsTo(1.0) ) ;

  M->solve();

  //print the solution, if any...
  if ( M->getPrimalSolutionStatus() == SolutionStatus::Optimal )
    print_solution(n, X);
  else
    std::cout << "No solution found!\n";

  return 0;
}