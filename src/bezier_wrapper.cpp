/*
 * bazier_wrapper.cpp
 * Copyright (C) 2018 Gao Tang <gt70@duke.edu>
 *
 * Distributed under terms of the MIT license.
 */


// A wrapper for the C++ Bezier code to be used in Python for

#include "pybind11/eigen.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "ott/bezier_base.h"


class pyBezier: public Bernstein{
public:
    pyBezier() : Bernstein(){}
    pyBezier(int poly_order_min, int poly_order_max, double min_order) : Bernstein(poly_order_min, poly_order_max, min_order){
    }
};


namespace py = pybind11;
PYBIND11_MODULE(libbezier, m){
    py::class_<pyBezier>(m, "Bezier")
        .def(py::init<>())
        .def(py::init<int, int, double>())
        .def("M", &pyBezier::getM)
        .def("MQM", &pyBezier::getMQM)
        .def("FM", &pyBezier::getFM)
        .def("C", &pyBezier::getC)
        .def("C_v", &pyBezier::getC_v)
        .def("C_a", &pyBezier::getC_a)
        .def("C_j", &pyBezier::getC_j)
        .def("A_v", &pyBezier::getA_v)
        .def("A_a", &pyBezier::getA_a)
        .def("A_j", &pyBezier::getA_j)
        .def("order_min", &pyBezier::order_min)
        .def("min_order", &pyBezier::min_order)
        .def("order_max", &pyBezier::order_max)
        .def("print_M", &pyBezier::printM)
        ;
}
