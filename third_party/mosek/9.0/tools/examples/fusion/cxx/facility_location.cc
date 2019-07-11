////
//  Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
//  File:      facility_location.cc
//
//  Purpose: Demonstrates a small one-facility location problem.
//
//  Given 10 customers placed in a grid we wish to place a facility
//  somewhere so that the total sum of distances to customers is
//  minimized.
//
//  The problem is formulated as a conic optimization problem as follows.
//  Let f=(fx,fy) be the (unknown) location of the facility, and let
//  c_i=(cx_i,cy_i) be the (known) customer locations; then we wish to
//  minimize
//      sum_i || f - c_i ||
//  where
//      ||.||
//  denotes the euclidian norm.
//  This is formulated as
//
//  minimize   sum(d_i)
//  such that  d_i ^ 2 > tx_i ^ 2 + ty_i ^ 2, for all i
//             tx_i = cx_i - fx, for all i
//             ty_i = cy_i - fy, for all i
//             d_i > 0, for all i
////

#include <memory>
#include <iostream>
#include <iomanip>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  auto customerloc = new_array_ptr<double, 2>(
  { {12.,  2. },
    {15., 13. },
    {10.,  8. },
    { 0., 10. },
    { 6., 13. },
    { 5.,  8. },
    {10., 12. },
    { 4.,  6. },
    { 5.,  2. },
    { 1., 10. }
  } );
  int N = customerloc->size(0);
  Model::t M = new Model("FacilityLocation"); auto _M = finally([&]() { M->dispose(); });
  // Variable holding the facility location
  Variable::t f = M->variable("facility", Set::make(1, 2), Domain::unbounded());
  // Variable defining the euclidian distances to each customer
  Variable::t d = M->variable("dist", Set::make(N, 1), Domain::greaterThan(0.0));
  // Variable defining the x and y differences to each customer;
  Variable::t t = M->variable("t", Set::make(N, 2), Domain::unbounded());
  M->constraint("dist measure",
                Var::hstack(d, t),
                Domain::inQCone(N, 3));

  Variable::t fxy = Var::repeat(f, N);
  M->constraint("xy diff", Expr::add(t, fxy), Domain::equalsTo(customerloc));


  M->objective("total_dist", ObjectiveSense::Minimize, Expr::sum(d));

  M->solve();

  auto res = f->level();
  std::cout << std::setprecision(2)
            << "Facility location = " << (*res)[0] << "," << (*res)[1] << std::endl;
}