// Copyright: Copyright (c) MOSEK ApS, Denmark. All rights reserved.
//
// File:      diet.cc
//
// Purpose: Solving Stigler's Nutrition model (DIET,SEQ=7)
//
// Source: GAMS Model library,
//           Dantzig, G B, Chapter 27.1. In Linear Programming and Extensions.
//           Princeton University Press, Princeton, New Jersey, 1963.
//
// Given a set of nutrients, foods with associated nutrient values, allowance of
// nutrients per day, the model will find the cheapest combination of foods
// which will provide the necessary nutrients.
//
//  Arguments for construction:
//  name      - Model name.
//  foods     - List of M names of the foods.
//  nutrients - List of N names of the nutrients.
//  daily_allowance - List of N floats denoting the daily allowance of each
//              nutrient.
//  nutritive_value - Two-dimensional MxN array of floats where each row
//              denotes the nutrient values for a single food per $ spent.

#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "fusion.h"
#include <cassert>

using namespace mosek::fusion;
using namespace monty;


struct DataException : std::runtime_error
{
  DataException(const char * msg) : std::runtime_error(msg) {}
};


typedef std::pair< std::shared_ptr<ndarray<double, 1>>, std::shared_ptr<ndarray<double, 1>> > diet_return_t;

diet_return_t diet
( std::shared_ptr<ndarray<std::string, 1>> foods,
  std::shared_ptr<ndarray<std::string, 1>> nutrients,
  std::shared_ptr<ndarray<double, 1>>      dailyAllowance,
  std::shared_ptr<ndarray<double, 2>>      nutritive_value )
{
  Model::t M = new Model("Stinglers Diet Problem"); auto _M = finally([&]() { M->dispose(); });

  Matrix::t nutrientValue = Matrix::t(Matrix::dense(nutritive_value))->transpose();

  int m = foods->size();
  int n = nutrients->size();

  if (dailyAllowance->size() != n)
    throw DataException("Length of dailyAllowance does not match the number of nutrients");
  if (nutrientValue->numColumns() != m)
    throw DataException("Number of rows in nutrient_value does not match the number of foods");
  if (nutrientValue->numRows() != n)
    throw DataException("Number of columns in nutrient_value does not match the number of nutrients");

  
  Variable::t dailyPurchase =
    M->variable(Set::make(foods),
                Domain::greaterThan(0.0));
  
  Constraint::t dailyNutrients =
    M->constraint(Expr::mul(nutrientValue, dailyPurchase),
                  Domain::greaterThan(dailyAllowance));
  M->objective(ObjectiveSense::Minimize, Expr::sum(dailyPurchase));
  M->solve();

  return diet_return_t(dailyPurchase->level(), dailyNutrients->level());
}

/* Main class with data definitions */
int main(int argc, char ** argv)
{
  auto nutrient_unit = new_array_ptr<std::string, 1>({
    "thousands",  "grams",        "grams",
    "milligrams", "thousand ius", "milligrams",
    "milligrams", "milligrams",   "milligrams"
  });
  auto nutrients = new_array_ptr<std::string, 1>({
    "calorie",    "protein",      "calcium",
    "iron",       "vitamin a",    "vitamin b1",
    "vitamin b2", "niacin",       "vitamin c"
  });
  auto foods = new_array_ptr<std::string, 1>({
    "wheat",        "cornmeal", "cannedmilk", "margarine", "cheese",
    "peanut butter", "lard",     "liver",      "porkroast", "salmon",
    "greenbeans",   "cabbage",  "onions",     "potatoes",  "spinach",
    "sweet potatos", "peaches",  "prunes",     "limabeans", "navybeans"
  });

  auto nutritive_value = new_array_ptr<double, 2>(
  {
    //  calorie       calcium      vitamin a        vitamin b2      vitamin c
    //         protein        iron           vitamin b1      niacin
    {44.7,  1411,   2.0,   365,    0,      55.4,   33.3,  441,     0},  // wheat
    {36,     897,   1.7,    99,   30.9,    17.4,    7.9,  106,     0},  // cornmeal
    { 8.4,   422,  15.1,     9,   26,       3,     23.5,   11,    60},  // cannedmilk
    {20.6,    17,    .6,     6,   55.8,      .2,    0,      0,     0},  // margarine
    { 7.4,   448,  16.4,    19,   28.1,      .8,   10.3,    4,     0},  // cheese
    {15.7,   661,   1,      48,     0,      9.6,    8.1,  471,     0},  // peanut butter
    {41.7,     0,   0,       0,     .2,     0,       .5,    5,     0},  // lard
    { 2.2,   333,    .2,   139,  169.2,     6.4,   50.8,  316,   525},  // liver
    { 4.4,   249,    .3,    37,    0,      18.2,    3.6,   79,     0},  // porkroast
    { 5.8,   705,   6.8,    45,    3.5,     1,      4.9,  209,     0},  // salmon
    { 2.4,   138,   3.7,    80,   69,       4.3,    5.8,   37,   862},  // greenbeans
    { 2.6,   125,   4,      36,    7.2,     9,      4.5,   26,  5369},  // cabbage
    { 5.8,   166,   3.8,    59,   16.6,     4.7,    5.9,   21,  1184},  // onions
    {14.3,   336,   1.8,   118,    6.7,    29.4,    7.1,  198,  2522},  // potatoes
    { 1.1,   106,   0.0,   138,  918.4,     5.7,   13.8,   33,  2755},  // spinach
    { 9.6,   138,   2.7,    54,  290.7,     8.4,    5.4,   83,  1912},  // sweet potatos
    { 8.5,    87,   1.7,   173,   86.8,     1.2,    4.3,   55,    57},  // peaches
    {12.8,    99,   2.5,   154,   85.7,     3.9,    4.3,   65,   257},  // prunes
    {17.4,  1055,   3.7,   459,    5.1,    26.9,   38.2,   93,     0},  // limabeans
    {26.9,  1691,  11.4,   792,    0,      38.4,   24.6,  217,     0}
  });// navybeans

  auto daily_allowance = new_array_ptr<double, 1>(
  {   3.,     70.,   .8,    12.,   5.,      1.8,    2.7,   18.,   75. });

  auto res = diet(foods,
                  nutrients,
                  daily_allowance,
                  nutritive_value);

  std::cout << "Solution:" << std::endl;
  for (int i = 0; i < foods->size(); ++i)
    std::cout << (*foods)[i] << ": " << std::setprecision(2) << std::setw(8) << (*res.first)[i] << std::endl;

  std::cout << "Nutrients:" << std::endl;
  for (int i = 0; i < nutrients->size(); ++i)
    std::cout << "\t" << std::setw(15) << (*nutrients)[i] << ": " << std::setprecision(2) << std::setw(8) << (*res.second)[i]
              << "(" << (*daily_allowance)[i] << ")"
              << std::endl;
}