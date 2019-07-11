//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      TrafficNetworkModel.cc
//
// Purpose:   Demonstrates a traffic network problem as a conic quadratic problem.
//
// Source:    Robert Fourer, "Convexity Checking in Large-Scale Optimization",
//            OR 53 --- Nottingham 6-8 September 2011.
//
// The problem:
//            Given a directed graph representing a traffic network
//            with one source and one sink, we have for each arc an
//            associated capacity, base travel time and a
//            sensitivity. Travel time along a specific arc increases
//            as the flow approaches the capacity.
//
//            Given a fixed inflow we now wish to find the
//            configuration that minimizes the average travel time.

#include <string>
#include <iostream>
#include <iomanip>
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

struct TrafficNetworkError : std::runtime_error
{
  TrafficNetworkError(const char * msg) : std::runtime_error(msg) {}
};

std::shared_ptr<ndarray<double, 1>>
                                 TrafficNetworkModel
                                 ( int                                numberOfNodes,
                                   int                                source_idx,
                                   int                                sink_idx,
                                   std::shared_ptr<ndarray<int, 1>>    arc_i,
                                   std::shared_ptr<ndarray<int, 1>>    arc_j,
                                   std::shared_ptr<ndarray<double, 1>> arcSensitivity,
                                   std::shared_ptr<ndarray<double, 1>> arcCapacity,
                                   std::shared_ptr<ndarray<double, 1>> arcBaseTravelTime,
                                   double                             T)
{
  Model::t M = new Model("Traffic Network"); auto _M = finally([&]() { M->dispose(); });
  M->setLogHandler([](const std::string & msg) { std::cout << msg << std::flush; } );

  int n = numberOfNodes;
  int narcs = arc_j->size(0);

  Matrix::t sens =
    Matrix::sparse(n, n, arc_i, arc_j, arcSensitivity);
  Matrix::t cap =
    Matrix::sparse(n, n, arc_i, arc_j, arcCapacity);
  Matrix::t basetime =
    Matrix::sparse(n, n, arc_i, arc_j, arcBaseTravelTime);
  Matrix::t e =
    Matrix::sparse(n, n, arc_i, arc_j, 1.0);
  Matrix::t e_e =
    Matrix::sparse(n, n,
                   new_array_ptr<int, 1>({ sink_idx }),
                   new_array_ptr<int, 1>({ source_idx }),
                   1.0);
  std::shared_ptr<ndarray<double, 1>>
  cs_inv(new ndarray<double, 1>(narcs, std::function<double(ptrdiff_t)>([&](ptrdiff_t i) { return 1.0 / ((*arcSensitivity)[i] * (*arcCapacity)[i]); }))),
  s_inv (new ndarray<double, 1>(narcs, std::function<double(ptrdiff_t)>([&](ptrdiff_t i) { return 1.0 / (*arcSensitivity)[i]; })));

  Matrix::t cs_inv_matrix = Matrix::sparse(n, n, arc_i, arc_j, cs_inv);
  Matrix::t s_inv_matrix  = Matrix::sparse(n, n, arc_i, arc_j, s_inv);

  Variable::t flow       = M->variable("traffic_flow", Set::make(n,n), Domain::greaterThan(0.0));

  Variable::t x = flow;
  Variable::t t = M->variable("travel_time", Set::make(n,n), Domain::greaterThan(0.0));
  Variable::t d = M->variable("d",           Set::make(n,n), Domain::greaterThan(0.0));
  Variable::t z = M->variable("z",           Set::make(n,n), Domain::greaterThan(0.0));

  // Set the objective:
  M->objective("Average travel time",
               ObjectiveSense::Minimize,
               Expr::mul(1.0 / T, Expr::add(Expr::dot(basetime, x), Expr::dot(e, d))));

  // Set up constraints
  // Constraint (1a)
  M->constraint("(1a)",
                Var::hstack(d->pick(arc_i, arc_j),
                            z->pick(arc_i, arc_j),
                            x->pick(arc_i, arc_j)),
                Domain::inRotatedQCone(narcs, 3));

  // Constraint (1b)
  M->constraint("(1b)",
                Expr::sub(Expr::add(Expr::mulElm(z, e),
                                    Expr::mulElm(x, cs_inv_matrix)),
                          s_inv_matrix),
                Domain::equalsTo(0.0));

  // Constraint (2)
  M->constraint("(2)",
                Expr::sub(Expr::add(Expr::mulDiag(x, e->transpose()),
                                    Expr::mulDiag(x, e_e->transpose())),
                          Expr::add(Expr::mulDiag(x->transpose(), e),
                                    Expr::mulDiag(x->transpose(), e_e))),
                Domain::equalsTo(0.0));
  // Constraint (3)
  M->constraint("(3)",
                x->index(sink_idx, source_idx), Domain::equalsTo(T));
  M->solve();
  return flow->level();
}


int main(int argc, char ** argv)
{
  std::shared_ptr<ndarray<int, 1>>    arc_i    = new_array_ptr<int, 1>   ({  0,    0,    2,    1,    2   });
  std::shared_ptr<ndarray<int, 1>>    arc_j    = new_array_ptr<int, 1>   ({  1,    2,    1,    3,    3   });
  std::shared_ptr<ndarray<double, 1>> arc_base = new_array_ptr<double, 1>({  4.0,  1.0,  2.0,  1.0,  6.0 });
  std::shared_ptr<ndarray<double, 1>> arc_cap  = new_array_ptr<double, 1>({ 10.0, 12.0, 20.0, 15.0, 10.0 });
  std::shared_ptr<ndarray<double, 1>> arc_sens = new_array_ptr<double, 1>({  0.1,  0.7,  0.9,  0.5,  0.1 });

  int      n          = 4;
  double   T          = 20.0;
  int      source_idx = 0;
  int      sink_idx   = 3;

  auto flow = TrafficNetworkModel(
                n, source_idx, sink_idx,
                arc_i, arc_j,
                arc_sens,
                arc_cap,
                arc_base,
                T);
  std::cout << "Optimal flow:" << std::endl;
  for (int i = 0; i < arc_i->size(); ++i)
    std::cout << "\tflow " << (*arc_i)[i] << " -> " << (*arc_j)[i] << " = " << (*flow)[(*arc_i)[i] * n + (*arc_j)[i]] << std::endl;
}