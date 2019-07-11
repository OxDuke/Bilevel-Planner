//
// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      baker.cc
//
// Purpose: Demonstrates a small linear problem.
//
// Source: "Linaer Algebra" by Knut Sydsaeter and Bernt Oeksendal.
//
// The problem: A baker has 150 kg flour, 22 kg sugar, 25 kg butter and two
// recipes:
//   1) Cakes, requiring 3.0 kg flour, 1.0 kg sugar and 1.2 kg butter per dozen.
//   2) Breads, requiring 5.0 kg flour, 0.5 kg sugar and 0.5 kg butter per dozen.
// Let the revenue per dozen cakes be $4 and the revenue per dozen breads be $6.
//
// We now wish to compute the combination of cakes and breads that will optimize
// the total revenue.


#include <iostream>
#include <string>
#include "monty.h"
#include "fusion.h"

using namespace mosek::fusion;
using namespace monty;

int main(int argc, char ** argv)
{
  auto ingredientnames = new_array_ptr<std::string, 1>({ "Flour", "Sugar", "Butter" });
  auto productnames = new_array_ptr<std::string, 1>({ "Cakes", "Breads" });
  auto stock = new_array_ptr<double, 1>(
  { 150.0,   22.0,    25.0 });
  auto recipe_data = new_array_ptr<double, 2>(
  { { 3.0, 5.0 },
    { 1.0, 0.5 },
    { 1.2, 0.5 }
  });

  auto revenue = new_array_ptr<double, 1>({ 4.0, 6.0 });
  Matrix::t recipe = Matrix::dense(recipe_data);
  Model::t M = new Model("Recipe"); auto _M = finally([&]() { M->dispose(); });
  // "production" defines the amount of each product to bake.
  Variable::t production = M->variable("production",
                                       Set::make(productnames),
                                       Domain::greaterThan(0.0));
  // The objective is to maximize the total revenue.
  M->objective("revenue",
               ObjectiveSense::Maximize,
               Expr::dot(revenue, production));
  // The prodoction is constrained by stock:
  M->constraint(Expr::mul(recipe, production), Domain::lessThan(stock));
  M->setLogHandler([](const std::string & msg) { std::cout << msg << std::flush; });

  // We solve and fetch the solution:
  M->solve();
  auto res = production->level();
  std::cout << "Solution:" << std::endl;
  for (int i = 0; i < res->size(); ++i)
    std::cout << "  Number of " << (*productnames)[i] << " : " << (*res)[i] << std::endl;
  std::cout << " Revenue : $" << ((*res)[0] * (*revenue)[0] + (*res)[1] * (*revenue)[1]) << std::endl;
}
