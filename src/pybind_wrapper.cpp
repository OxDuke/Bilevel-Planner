/*
 * pybind_wrapper.cpp
 * Copyright (C) 2018 Gao Tang <gt70@duke.edu>
 *
 * Distributed under terms of the MIT license.
 */

#include "pybind11/eigen.h"
#include "pybind11/functional.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/chrono.h"
#include "time.h"
#include "stdlib.h"
#include "ott/data_types.h"
#include "ott/pybind_box_type.h"
#include "ott/TGProblem.h"


namespace py = pybind11;
using namespace pybind11::literals;


pyTGProblem loadTGP(const std::string fileName){
    TGProblem prob;
    loadTGProblemFromFile(fileName, prob);
    pyTGProblem problem(prob);
    return problem;
}

void printTGP(const pyTGProblem& p){
    printTGProblem(p);
}


void set_print_level(int level);
// subroutines for problem construction
std::tuple<VX, lVX, lVX> construct_P_matrix(double minimize_order, int segment_num, int poly_order, RefVX room_time, RefMX MQM, std::string &type);

VX gradient_from_P(double minimize_order, int segment_num, int poly_order, RefVX room_time, RefMX MQM, RefVX sol);

LinearConstr construct_A_matrix(
    const vector<pyBox> &corridor,
    const MatrixXd &MQM,
    const MatrixXd &pos,
    const MatrixXd &vel,
    const MatrixXd &acc,
    const double maxVel,
    const double maxAcc,
    const int traj_order,
    const double minimize_order,
    const double margin,
    const bool & isLimitVel,
    const bool & isLimitAcc);

VX gradient_from_A(
            const vector<pyBox> &corridor,
            const MatrixXd &MQM,
            const MatrixXd &pos,
            const MatrixXd &vel,
            const MatrixXd &acc,
            const double maxVel,
            const double maxAcc,
            const int traj_order,
            const double minimize_order,
            const double margin,
            const bool & isLimitVel,
            const bool & isLimitAcc,
            RefVX sol,
            RefVX lmdy,  //lmdy is for constraints
            RefVX lmdz  // lmdz is for bounds on variables
        );


std::pair<int, int> snopt_eval(
            const vector<pyBox> &corridor,
            const MatrixXd &MQM,
            const MatrixXd &pos,
            const MatrixXd &vel,
            const MatrixXd &acc,
            const double maxVel,
            const double maxAcc,
            const int traj_order,
            const double minimize_order,
            const double margin,
            const bool & isLimitVel,
            const bool & isLimitAcc,
            cRefVX coef,  // the coefficients
            RefVX F,  // records actual function values
            RefVX lb,  // records lower bound since previous one does not apply
            RefVX ub,  // records upper bound since previous one does not apply
            RefVX G,  // record gradients, treat all things nonlinear
            ReflVX row,  // record rows of gradients
            ReflVX col,  // record cols of gradients
            bool needg,  // enable recording of G
            bool rec,
            bool needlub  // enable recording of lb and ub
        );
std::pair<double, VX> eval_f(cRefVX coef, cRefVX room_time, int traj_order, double minimize_order, cRefMX MQM, bool needg);


PYBIND11_MODULE(libott, m){
    py::class_<pyBox>(m, "PyBox")
        .def(py::init<>())
        .def(py::init<Eigen::MatrixXd, Eigen::Vector3d>())
        .def("setVertex", &pyBox::setVertex)
        .def("setBox", &pyBox::setBox)
        .def("printBox", &pyBox::printBox)
        .def("ifContains", &pyBox::ifContains)
        .def("getBox", &pyBox::getBox)
        .def("getVertex", &pyBox::getVertex)
        .def_readwrite("valid", &pyBox::valid)
        .def_readwrite("t", &pyBox::t)
        .def_readwrite("box", &pyBox::box)
        .def_readwrite("vertex", &pyBox::vertex)   // p1 to p8
        .def_readwrite("center", &pyBox::center)  // center of the box
        ;

    py::class_<TGProblem>(m, "TGProblem")
        .def(py::init<>())
        .def_readwrite("corridor", &TGProblem::corridor)
        ;

    py::class_<pyTGProblem>(m, "PyTGProblem")
        .def(py::init<>())
        .def("getCorridor", &pyTGProblem::getCorridor)
        .def("updateCorridorTime", &pyTGProblem::updateCorridorTime)
        .def("get_position", &pyTGProblem::get_position)
        .def("get_velocity", &pyTGProblem::get_velocity)
        .def("get_acceleration", &pyTGProblem::get_acceleration)
        .def_readwrite("MQM", &pyTGProblem::MQM)
        .def_readwrite("position", &pyTGProblem::position)
        .def_readwrite("velocity", &pyTGProblem::velocity)
        .def_readwrite("acceleration", &pyTGProblem::acceleration)
        .def_readwrite("maxVelocity", &pyTGProblem::maxVelocity)
        .def_readwrite("maxAcceleration", &pyTGProblem::maxAcceleration)
        .def_readwrite("trajectoryOrder", &pyTGProblem::trajectoryOrder)
        .def_readwrite("minimizeOrder", &pyTGProblem::minimizeOrder)
        .def_readwrite("margin", &pyTGProblem::margin)
        .def_readwrite("doLimitVelocity", &pyTGProblem::doLimitVelocity)
        .def_readwrite("doLimitAcceleration", &pyTGProblem::doLimitAcceleration)
        ;

    py::class_<LinearConstr>(m, "LinearConstr")
        .def(py::init<>())
        .def_readwrite("xlb", &LinearConstr::xlb)
        .def_readwrite("xub", &LinearConstr::xub)
        .def_readwrite("clb", &LinearConstr::clb)
        .def_readwrite("cub", &LinearConstr::cub)
        .def_readwrite("aval", &LinearConstr::aval)
        .def_readwrite("arow", &LinearConstr::arow)
        .def_readwrite("acol", &LinearConstr::acol)
        .def_readwrite("n_var", &LinearConstr::n_var)
        .def_readwrite("n_con", &LinearConstr::n_con)
        .def_readwrite("n_nnz", &LinearConstr::n_nnz)
        ;

    m.def("loadTGP", &loadTGP);
    m.def("printTGP", &printTGP);
    m.def("printBox", &printBox);


    // sections for problem construction
    m.def("construct_P", &construct_P_matrix);

    m.def("construct_A", &construct_A_matrix);

    m.def("gradient_from_P", &gradient_from_P);

    m.def("gradient_from_A", &gradient_from_A);

    m.def("set_print_level", &set_print_level);

    m.def("snopt_eval", &snopt_eval);

    m.def("eval_f", &eval_f);

}
