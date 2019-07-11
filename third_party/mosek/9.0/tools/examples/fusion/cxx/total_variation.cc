//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      total_variation.cc
//
// Purpose:   Demonstrates how to solve a total
//            variation problem using the Fusion API.

#include <iostream>
#include <vector>
#include <random>

#include "monty.h"
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  int nrows  = 50;
  int ncols  = 50;
  int ncells = nrows * ncols;
  int seed   = 0;
  double sigma = 1.0;

  std::uniform_real_distribution<double> udistr(0., 1.);
  std::normal_distribution<double>       ndistr(0., 1.);
  std::mt19937 engine(seed);

  auto f = std::shared_ptr<ndarray<double, 1> >(new ndarray<double, 1>(ncells) );

  //Random signal with Gaussian noise
  for (int i = 0; i < ncells; i++)
    (*f)[i] = std::max(0., std::min(1.0, udistr(engine) + ndistr(engine) ) ) ;

  Model::t M = new Model("TV"); auto _M = finally([&]() { M->dispose(); });

  auto u = M->variable(new_array_ptr<int, 1>({nrows + 1, ncells + 1}), Domain::inRange(0., 1.));
  auto t = M->variable(new_array_ptr<int, 1>({nrows, ncols}), Domain::unbounded());
  auto ucore = u->slice(new_array_ptr<int, 1>({0, 0}), new_array_ptr<int, 1>({nrows, ncols}));

  auto deltax = Expr::sub( u->slice( new_array_ptr<int, 1>({1, 0}), new_array_ptr<int, 1>({nrows + 1, ncols}) ), ucore);
  auto deltay = Expr::sub( u->slice( new_array_ptr<int, 1>({0, 1}) , new_array_ptr<int, 1>({nrows, ncols + 1}) ), ucore);

  M->constraint( Expr::stack(2., t, deltax, deltay), Domain::inQCone()->axis(2) );

  M->constraint( Expr::vstack(sigma, Expr::flatten( Expr::sub( Matrix::dense(nrows, ncols, f),  ucore ) )  ),  Domain::inQCone() );

  M->objective( ObjectiveSense::Minimize, Expr::sum(t) );
  M->setLogHandler([](const std::string & msg) { std::cout << msg << std::flush; } );
  M->solve();

  std::vector<double> deltas(ncells);
  auto uu = *(u->level());
  for (int i = 0; i < ncells; i++)
    deltas[i] =  std::abs( uu[i] - (*f)[i]);

  std::cerr << "max deltas= " << *max_element(deltas.begin(), deltas.end()) << std::endl;
  std::cerr << "min deltas= " << *min_element(deltas.begin(), deltas.end()) << std::endl;

  return 0;
}