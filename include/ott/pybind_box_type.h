/*
 * pybind_box_type.h
 * Copyright (C) 2018 Gao Tang <gt70@duke.edu>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PYBIND_BOX_TYPE_H
#define PYBIND_BOX_TYPE_H
#include "ott/data_types.h"
#include "ott/TGProblem.h"
#include <eigen3/Eigen/Dense>


typedef Eigen::Matrix<double, -1, -1> MX;
typedef Eigen::Matrix<double, -1, 1> VX;
typedef Eigen::Matrix<int, -1, 1> lVX;
typedef Eigen::Ref<VX> RefVX;
typedef Eigen::Ref<lVX> ReflVX;
typedef Eigen::Ref<MX> RefMX;
typedef Eigen::Map<VX> MapVX;
typedef Eigen::Map<MX> MapMX;
typedef const Eigen::Ref<const VX> cRefVX;
typedef Eigen::Ref<const MX> cRefMX;


class ConstraintTape{
public:
    std::vector<int> row;
    std::vector<int> col;
    std::vector<double> val;
    std::vector<double> lb;
    std::vector<double> ub;
    size_t n_bound = 0;
    size_t a_nnz = 0;
    size_t a_row = 0;

    void add_bound(double l, double u){
        lb.push_back(l);
        ub.push_back(u);
        n_bound += 1;
    }

    void putarow(int row_idx, int nzi, int *asub, double *aval){
        for(int i = 0; i < nzi; i++){
            row.push_back(row_idx);
            col.push_back(asub[i]);
            val.push_back(aval[i]);
        }
        a_row += 1;
        a_nnz += nzi;
    }
};


class LinearConstr{
public:
    VX xlb, xub, clb, cub;
    VX aval;
    lVX arow, acol;
    size_t n_var, n_con, n_nnz;

    LinearConstr(){}

    LinearConstr(ConstraintTape &con_tape, ConstraintTape &var_tape){
        n_var = var_tape.lb.size();
        xlb = VX::Zero(n_var);
        xub = xlb;
        for(size_t i = 0; i < n_var; i++){
            xlb(i) = var_tape.lb[i];
            xub(i) = var_tape.ub[i];
        }
        n_con = con_tape.lb.size();
        n_nnz = con_tape.val.size();
        //std::cout << "var " << n_var << " con " << n_con << " nnz " << n_nnz << std::endl;
        clb = VX::Zero(n_con);
        cub = VX::Zero(n_con);
        aval = VX::Zero(n_nnz);
        arow = lVX::Zero(n_nnz);
        acol = lVX::Zero(n_nnz);
        for(size_t i = 0; i < n_con; i++){
            clb(i) = con_tape.lb[i];
            cub(i) = con_tape.ub[i];
        }
        for(size_t i = 0; i < n_nnz; i++){
            arow(i) = con_tape.row[i];
            acol(i) = con_tape.col[i];
            aval(i) = con_tape.val[i];
        }
    }
};


class pyBox : public Box{
    typedef Eigen::Matrix<double, -1, -1, Eigen::RowMajor> rMX;
public:
    pyBox() : Box(){}

    pyBox(Eigen::MatrixXd vertex_, Eigen::Vector3d center_) : Box(vertex_, center_){}

    pyBox(Box& box_){
        vertex = box_.vertex;
        center = box_.center;
        valid = box_.valid;
        t = box_.t;
        box = box_.box;
    }

    rMX getBox(){
        size_t len_box = box.size();
        rMX rbox = rMX::Zero(len_box, 2);
        for(size_t i = 0; i < len_box; i++){
            rbox(i, 0) = box[i].first;
            rbox(i, 1) = box[i].second;
        }
        return rbox;
    }

    rMX getVertex(){
        rMX rvtx = vertex;
        return rvtx;
    }
};


class pyTGProblem : public TGProblem{
public:
    pyTGProblem() : TGProblem(){}
    pyTGProblem(const TGProblem &tgp): TGProblem(tgp){}

    std::vector<pyBox> getCorridor(){
        std::vector<pyBox> result;
        for(auto &box : corridor)
            result.push_back(pyBox(box));
        return result;
    }

    void updateCorridorTime(cRefVX time){
        size_t num_box = corridor.size();
        for(size_t i = 0; i < num_box; i++)
            corridor[i].t = time(i);
    }

    RefMX get_position(){
        return RefMX(position);
    }

    RefMX get_velocity(){
        return RefMX(velocity);
    }

    RefMX get_acceleration(){
        return RefMX(acceleration);
    }
};


#endif /* !PYBIND_BOX_TYPE_H */
