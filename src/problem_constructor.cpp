/*
 * problem_constructor.cpp
 * Copyright (C) 2018 Gao Tang <gt70@duke.edu>
 *
 * Distributed under terms of the MIT license.
 */

#include <eigen3/Eigen/Dense>
#include <tuple>
#include <limits>

#include "ott/pybind_box_type.h"

typedef int MSKint32t;


int PRINTLEVEL = 0;

double LMD_TOL = 1e-4;

void set_print_level(int level){
    PRINTLEVEL = level;
}

// Generate the P matrix for the problem
// type = "l" if lower triangular is wanted; "u" is upper is desired; "f" is full matrix is desired
std::tuple<VX, lVX, lVX> construct_P_matrix(double minimize_order, int segment_num, int poly_order, RefVX room_time, RefMX MQM, std::string &type){
    int min_order_l = floor(minimize_order);
    int min_order_u = ceil (minimize_order);

    int NUMQNZ = 0;
    int NUMQ_blk = (poly_order + 1);                       // default minimize the jerk and minimize_order = 3
    if(type == "f" || type == "F")
        NUMQNZ = segment_num * 3 * pow(NUMQ_blk, 2);
    else
        NUMQNZ = segment_num * 3 * NUMQ_blk * (NUMQ_blk + 1) / 2;

    VX qval = VX::Zero(NUMQNZ);
    lVX qsubi = lVX::Zero(NUMQNZ);
    lVX qsubj = lVX::Zero(NUMQNZ);

    int sub_shift = 0;
    int idx = 0;
    int s1d1CtrlP_num = poly_order + 1;
    int s1CtrlP_num = 3 * s1d1CtrlP_num;
    for (int k = 0; k < segment_num; k++)
    {
        double scale_k = room_time(k);
        for (int p = 0; p < 3; p ++ )
            for ( int i = 0; i < s1d1CtrlP_num; i ++ )
                for ( int j = 0; j < s1d1CtrlP_num; j ++ ){
                    if(((type == "l" || type == "L") && i >= j) || ((type == "u" || type == "U") && i <= j) || (type == "f" || type == "F")) {
                        qsubi(idx) = sub_shift + p * s1d1CtrlP_num + i;
                        qsubj(idx) = sub_shift + p * s1d1CtrlP_num + j;
                        //qval[idx]  = MQM(i, j) /(double)pow(scale_k, 3);
                        if (min_order_l == min_order_u)
                            qval(idx)  = MQM(i, j) / (double)pow(scale_k, 2 * min_order_u - 3);
                        else
                            qval(idx) = ( (minimize_order - min_order_l) / (double)pow(scale_k, 2 * min_order_u - 3)
                                          + (min_order_u - minimize_order) / (double)pow(scale_k, 2 * min_order_l - 3) ) * MQM(i, j);
                        idx ++ ;
                    }
                }

        sub_shift += s1CtrlP_num;
    }
    return std::make_tuple(qval, qsubi, qsubj);
}

VX gradient_from_P(double minimize_order, int segment_num, int poly_order, RefVX room_time, RefMX MQM, RefVX sol){
    int min_order_l = floor(minimize_order);
    int min_order_u = ceil (minimize_order);

    VX pgrad = VX::Zero(segment_num);  // record the results

    int s1d1CtrlP_num = poly_order + 1;
    int s1CtrlP_num = 3 * s1d1CtrlP_num;
    MatrixXd subH(s1CtrlP_num, s1CtrlP_num);
    for (int k = 0; k < segment_num; k++) {
        subH.setZero();
        double scale_k = room_time(k);
        RefVX x = sol.segment(k * s1CtrlP_num, s1CtrlP_num);
        for (int p = 0; p < 3; p++ ){
            for ( int i = 0; i < s1d1CtrlP_num; i ++ ){
                for ( int j = 0; j < s1d1CtrlP_num; j ++ ){
                    double deriv = 0;
                    if (min_order_l == min_order_u)
                        deriv = MQM(i, j) * (3 - 2 * min_order_u) * (double)pow(scale_k, 2 - 2 * min_order_u);
                    else
                        deriv = ((minimize_order - min_order_l) * (double)pow(scale_k, 2 - 2 * min_order_u) * (3 - 2 * min_order_u)
                                 + (min_order_u - minimize_order) * (3 - 2 * min_order_l) * (double)pow(scale_k, 2 - 2 * min_order_l)) * MQM(i, j);
                    subH(p * s1d1CtrlP_num + i, p * s1d1CtrlP_num + j) = deriv;
                }
            }
        }
        pgrad(k) = 0.5 * x.dot(subH * x);
    }
    return pgrad;
}


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
            const bool & isLimitAcc
        ){
    ConstraintTape var_tape;  // records bounds on variables
    ConstraintTape con_tape;  // records bounds on constraints

    double initScale = corridor.front().t;
    double lstScale  = corridor.back().t;
    int segment_num  = corridor.size();

    int n_poly = traj_order + 1;
    int s1d1CtrlP_num = n_poly;
    int s1CtrlP_num   = 3 * s1d1CtrlP_num;

    int equ_con_s_num = 3 * 3; // p, v, a in x, y, z axis at the start point
    int equ_con_e_num = 3 * 3; // p, v, a in x, y, z axis at the end point
    int equ_con_continuity_num = 3 * 3 * (segment_num - 1);
    int equ_con_num   = equ_con_s_num + equ_con_e_num + equ_con_continuity_num; // p, v, a in x, y, z axis in each segment's joint position

    int vel_con_num = 3 *  traj_order * segment_num;
    int acc_con_num = 3 * (traj_order - 1) * segment_num;

    if ( !isLimitVel )
        vel_con_num = 0;

    if ( !isLimitAcc)
        acc_con_num = 0;


    //int high_order_con_num = vel_con_num + acc_con_num;
    //int high_order_con_num = 0; //3 * traj_order * segment_num;

    //int con_num   = equ_con_num + high_order_con_num;
    int ctrlP_num = segment_num * s1CtrlP_num;

    if (isLimitVel)
    {
        /***  Stack the bounding value for the linear inequality for the velocity constraints  ***/
        for (int i = 0; i < vel_con_num; i++)
        {
            con_tape.add_bound(-maxVel, maxVel);
        }
    }

    if (isLimitAcc)
    {
        /***  Stack the bounding value for the linear inequality for the acceleration constraints  ***/
        for (int i = 0; i < acc_con_num; i++)
        {
            con_tape.add_bound(-maxAcc, maxAcc);
        }
    }


    for (int i = 0; i < equ_con_num; i ++ ) {
        double beq_i;
        if (i < 3)                    beq_i = pos(0, i);
        else if (i >= 3  && i < 6  ) beq_i = vel(0, i - 3);
        else if (i >= 6  && i < 9  ) beq_i = acc(0, i - 6);
        else if (i >= 9  && i < 12 ) beq_i = pos(1, i - 9 );
        else if (i >= 12 && i < 15 ) beq_i = vel(1, i - 12);
        else if (i >= 15 && i < 18 ) beq_i = acc(1, i - 15);
        else beq_i = 0.0;

        con_tape.add_bound(beq_i, beq_i);
    }

    /* ## define a container for control points' boundary and boundkey ## */
    /* ## dataType in one tuple is : boundary type, lower bound, upper bound ## */

    for (int k = 0; k < segment_num; k++)
    {
        pyBox cube_     = corridor[k];
        double scale_k = cube_.t;

        for (int i = 0; i < 3; i++ )
        {
            for (int j = 0; j < n_poly; j ++ )
            {
                double lo_bound, up_bound;
                if (k > 0)
                {
                    lo_bound = (cube_.box[i].first  + margin) / scale_k;
                    up_bound = (cube_.box[i].second - margin) / scale_k;
                }
                else
                {
                    lo_bound = (cube_.box[i].first)  / scale_k;
                    up_bound = (cube_.box[i].second) / scale_k;
                }

                var_tape.add_bound(lo_bound, up_bound);

            }
        }
    }


    int row_idx = 0;
    // The velocity constraints
    if (isLimitVel)
    {
        for (int k = 0; k < segment_num ; k ++ )
        {
            for (int i = 0; i < 3; i++)
            {   // for x, y, z loop
                for (int p = 0; p < traj_order; p++)
                {
                    const int nzi = 2;
                    int asub[nzi];
                    double aval[nzi];

                    aval[0] = -1.0 * traj_order;
                    aval[1] =  1.0 * traj_order;

                    asub[0] = k * s1CtrlP_num + i * s1d1CtrlP_num + p;
                    asub[1] = k * s1CtrlP_num + i * s1d1CtrlP_num + p + 1;

                    con_tape.putarow(row_idx, nzi, asub, aval);
                    row_idx ++;
                }
            }
        }
    }


    // The acceleration constraints
    if (isLimitAcc)
    {
        for (int k = 0; k < segment_num ; k ++ )
        {
            for (int i = 0; i < 3; i++)
            {
                for (int p = 0; p < traj_order - 1; p++)
                {
                    const int nzi = 3;
                    int asub[nzi];
                    double aval[nzi];

                    aval[0] =  1.0 * traj_order * (traj_order - 1) / corridor[k].t;
                    aval[1] = -2.0 * traj_order * (traj_order - 1) / corridor[k].t;
                    aval[2] =  1.0 * traj_order * (traj_order - 1) / corridor[k].t;
                    asub[0] = k * s1CtrlP_num + i * s1d1CtrlP_num + p;
                    asub[1] = k * s1CtrlP_num + i * s1d1CtrlP_num + p + 1;
                    asub[2] = k * s1CtrlP_num + i * s1d1CtrlP_num + p + 2;

                    con_tape.putarow(row_idx, nzi, asub, aval);
                    row_idx ++;
                }
            }
        }
    }
    /*   Start position  */
    {
        // position :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 1;
            int asub[nzi];
            double aval[nzi];
            aval[0] = 1.0 * initScale;
            asub[0] = i * s1d1CtrlP_num;
            con_tape.putarow(row_idx, nzi, asub, aval);
            row_idx ++;
        }
        // velocity :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 2;
            MSKint32t asub[nzi];
            double aval[nzi];
            aval[0] = - 1.0 * traj_order;
            aval[1] =   1.0 * traj_order;
            asub[0] = i * s1d1CtrlP_num;
            asub[1] = i * s1d1CtrlP_num + 1;
            con_tape.putarow(row_idx, nzi, asub, aval);
            row_idx ++;
        }
        // acceleration :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 3;
            MSKint32t asub[nzi];
            double aval[nzi];
            aval[0] =   1.0 * traj_order * (traj_order - 1) / initScale;
            aval[1] = - 2.0 * traj_order * (traj_order - 1) / initScale;
            aval[2] =   1.0 * traj_order * (traj_order - 1) / initScale;
            asub[0] = i * s1d1CtrlP_num;
            asub[1] = i * s1d1CtrlP_num + 1;
            asub[2] = i * s1d1CtrlP_num + 2;
            con_tape.putarow(row_idx, nzi, asub, aval);
            row_idx ++;
        }
    }

    /*   End position  */
    //ROS_WARN(" end position");
    {
        // position :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 1;
            MSKint32t asub[nzi];
            double aval[nzi];
            asub[0] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num;
            aval[0] = 1.0 * lstScale;
            con_tape.putarow(row_idx, nzi, asub, aval);
            row_idx ++;
        }
        // velocity :
        // This (terminal velocity) is not correctly implemented in Gao Fei's code
        for (int i = 0; i < 3; i++)
        {
            int nzi = 2;
            MSKint32t asub[nzi];
            double aval[nzi];
            asub[0] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num - 1;
            asub[1] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num;
            aval[0] = - 1.0;
            aval[1] =   1.0;
            con_tape.putarow(row_idx, nzi, asub, aval);
            row_idx ++;
        }
        // acceleration :
        // This (terminal acceleration) is not correctly implemented in Gao Fei's code
        for (int i = 0; i < 3; i++)
        {
            int nzi = 3;
            MSKint32t asub[nzi];
            double aval[nzi];
            asub[0] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num - 2;
            asub[1] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num - 1;
            asub[2] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num;
            aval[0] =   1.0 / lstScale;
            aval[1] = - 2.0 / lstScale;
            aval[2] =   1.0 / lstScale;
            con_tape.putarow(row_idx, nzi, asub, aval);
            row_idx ++;
        }
    }

    /*   joint points  */
    //ROS_WARN(" joint position");
    {
        int sub_shift = 0;
        double val0, val1;
        for (int k = 0; k < (segment_num - 1); k ++ )
        {
            double scale_k = corridor[k].t;
            double scale_n = corridor[k + 1].t;
            // position :
            val0 = scale_k;
            val1 = scale_n;
            for (int i = 0; i < 3; i++)
            {   // loop for x, y, z
                int nzi = 2;
                MSKint32t asub[nzi];
                double aval[nzi];

                // This segment's last control point
                aval[0] = 1.0 * val0;
                asub[0] = sub_shift + (i + 1) * s1d1CtrlP_num - 1;

                // Next segment's first control point
                aval[1] = -1.0 * val1;
                asub[1] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num;
                con_tape.putarow(row_idx, nzi, asub, aval);
                row_idx ++;
            }

            for (int i = 0; i < 3; i++)
            {
                int nzi = 4;
                MSKint32t asub[nzi];
                double aval[nzi];

                // This segment's last velocity control point
                aval[0] = -1.0;
                aval[1] =  1.0;
                asub[0] = sub_shift + (i + 1) * s1d1CtrlP_num - 2;
                asub[1] = sub_shift + (i + 1) * s1d1CtrlP_num - 1;
                // Next segment's first velocity control point
                aval[2] =  1.0;
                aval[3] = -1.0;

                asub[2] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num;
                asub[3] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num + 1;

                con_tape.putarow(row_idx, nzi, asub, aval);
                row_idx ++;
            }
            // acceleration :
            val0 = 1.0 / scale_k;
            val1 = 1.0 / scale_n;
            for (int i = 0; i < 3; i++)
            {
                int nzi = 6;
                MSKint32t asub[nzi];
                double aval[nzi];

                // This segment's last velocity control point
                aval[0] =  1.0  * val0;
                aval[1] = -2.0  * val0;
                aval[2] =  1.0  * val0;
                asub[0] = sub_shift + (i + 1) * s1d1CtrlP_num - 3;
                asub[1] = sub_shift + (i + 1) * s1d1CtrlP_num - 2;
                asub[2] = sub_shift + (i + 1) * s1d1CtrlP_num - 1;
                // Next segment's first velocity control point
                aval[3] =  -1.0  * val1;
                aval[4] =   2.0  * val1;
                aval[5] =  -1.0  * val1;
                asub[3] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num;
                asub[4] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num + 1;
                asub[5] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num + 2;

                con_tape.putarow(row_idx, nzi, asub, aval);
                row_idx ++;
            }

            sub_shift += s1CtrlP_num;
        }
    }
    LinearConstr lincon(con_tape, var_tape);
    return lincon;
}


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
        ){
    double initScale = corridor.front().t;
    double lstScale  = corridor.back().t;
    int segment_num  = corridor.size();

    VX agrad = VX::Zero(segment_num);  // store gradient here

    int n_poly = traj_order + 1;
    int s1d1CtrlP_num = n_poly;
    int s1CtrlP_num   = 3 * s1d1CtrlP_num;

    // int equ_con_s_num = 3 * 3; // p, v, a in x, y, z axis at the start point
    // int equ_con_e_num = 3 * 3; // p, v, a in x, y, z axis at the end point
    // int equ_con_continuity_num = 3 * 3 * (segment_num - 1);
    // int equ_con_num   = equ_con_s_num + equ_con_e_num + equ_con_continuity_num; // p, v, a in x, y, z axis in each segment's joint position

    int vel_con_num = 3 *  traj_order * segment_num;
    int acc_con_num = 3 * (traj_order - 1) * segment_num;

    int ctrlP_num = segment_num * s1CtrlP_num;

    if ( !isLimitVel )
        vel_con_num = 0;

    if ( !isLimitAcc)
        acc_con_num = 0;

    /* ## define a container for control points' boundary and boundkey ## */
    /* ## dataType in one tuple is : boundary type, lower bound, upper bound ## */

    int var_idx = 0;
    for (int k = 0; k < segment_num; k++)
    {
        pyBox cube_     = corridor[k];
        double scale_k = cube_.t;

        for (int i = 0; i < 3; i++ )
        {
            for (int j = 0; j < n_poly; j ++ )
            {
                double lo_bound, up_bound;
                if (k > 0)
                {
                    lo_bound = (cube_.box[i].first  + margin) / scale_k;
                    up_bound = (cube_.box[i].second - margin) / scale_k;
                }
                else
                {
                    lo_bound = (cube_.box[i].first)  / scale_k;
                    up_bound = (cube_.box[i].second) / scale_k;
                }

                if(lmdz(var_idx) > 0)
                    agrad(k) -= lmdz(var_idx) * up_bound * scale_k * (-1) / scale_k / scale_k;
                else
                    agrad(k) -= lmdz(var_idx) * lo_bound * scale_k * (-1) / scale_k / scale_k;

                if(PRINTLEVEL > 0 && abs(lmdz(var_idx)) > LMD_TOL){
                    printf("At %d lmdz = %f xbound at (%d, %d, %d) value = %f lb = %f ub = %f\n", var_idx, lmdz(var_idx), k, i, j, sol(var_idx), lo_bound, up_bound);
                }

                var_idx += 1;
            }
        }
    }


    int row_idx = 0;
    // The velocity constraints
    if (isLimitVel)  // there is nothing we should do
    {
        for (int k = 0; k < segment_num ; k ++ )
        {
            for (int i = 0; i < 3; i++)
            {   // for x, y, z loop
                for (int p = 0; p < traj_order; p++)
                {
                    row_idx ++;
                }
            }
        }
    }


    // The acceleration constraints
    if (isLimitAcc)
    {
        for (int k = 0; k < segment_num ; k ++ )
        {
            for (int i = 0; i < 3; i++)
            {
                for (int p = 0; p < traj_order - 1; p++)
                {
                    const int nzi = 3;
                    int asub[nzi];
                    double aval[nzi];
                    double val_grad[nzi];

                    aval[0] =  1.0 * traj_order * (traj_order - 1) / corridor[k].t;
                    val_grad[0] = -aval[0] / corridor[k].t;
                    aval[1] = -2.0 * traj_order * (traj_order - 1) / corridor[k].t;
                    val_grad[1] = -aval[1] / corridor[k].t;
                    aval[2] =  1.0 * traj_order * (traj_order - 1) / corridor[k].t;
                    val_grad[2] = -aval[2] / corridor[k].t;
                    asub[0] = k * s1CtrlP_num + i * s1d1CtrlP_num + p;
                    asub[1] = k * s1CtrlP_num + i * s1d1CtrlP_num + p + 1;
                    asub[2] = k * s1CtrlP_num + i * s1d1CtrlP_num + p + 2;

                    agrad(k) += lmdy(row_idx) * (val_grad[0] * sol(asub[0]) + val_grad[1] * sol(asub[1]) + val_grad[2] * sol(asub[2]));

                    if(PRINTLEVEL > 0 && abs(lmdy(row_idx)) > LMD_TOL){
                        double fval = aval[0] * sol(asub[0]) + aval[1] * sol(asub[1]) + aval[2] * sol(asub[2]);
                        printf("At row %d lmdy = %f acceleration limit (%d, %d, %d) value %f\n", row_idx, lmdy(row_idx), k, i, p, fval);
                    }

                    row_idx ++;
                }
            }
        }
    }
    /*   Start position  */
    {
        // position :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 1;
            int asub[nzi];
            double aval[nzi];
            aval[0] = 1.0 * initScale;
            asub[0] = i * s1d1CtrlP_num;
            agrad(0) += lmdy(row_idx) * sol(asub[0]);  // since p aval[0] p t = 1

            if(PRINTLEVEL > 0 && abs(lmdy(row_idx)) > LMD_TOL){
                printf("At row %d lmdy = %f start position (%d) limit value %f\n", row_idx, lmdy(row_idx), i, aval[0] * sol(asub[0]));
            }

            row_idx ++;
        }
        // velocity :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            row_idx ++;
        }
        // acceleration :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 3;
            MSKint32t asub[nzi];
            double aval[nzi], val_grad[nzi];
            aval[0] =   1.0 * traj_order * (traj_order - 1) / initScale;
            val_grad[0] = -aval[0] / initScale;
            aval[1] = - 2.0 * traj_order * (traj_order - 1) / initScale;
            val_grad[1] = -aval[1] / initScale;
            aval[2] =   1.0 * traj_order * (traj_order - 1) / initScale;
            val_grad[2] = -aval[2] / initScale;
            asub[0] = i * s1d1CtrlP_num;
            asub[1] = i * s1d1CtrlP_num + 1;
            asub[2] = i * s1d1CtrlP_num + 2;
            for(int j = 0; j < nzi; j++)
                agrad(0) += lmdy(row_idx) * (val_grad[j] * sol(asub[j]));

            if(PRINTLEVEL > 0 && abs(lmdy(row_idx)) > LMD_TOL){
                double fval = aval[0] * sol(asub[0]) + aval[1] * sol(asub[1]) + aval[2] * sol(asub[2]);
                printf("At row %d lmdy = %f start acceleration (%d) limit value %f\n", row_idx, lmdy(row_idx), i, fval);
            }

            row_idx ++;
        }
    }

    /*   End position  */
    //ROS_WARN(" end position");
    {
        // position :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 1;
            MSKint32t asub[nzi];
            double aval[nzi];
            asub[0] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num;
            aval[0] = 1.0 * lstScale;
            agrad(segment_num - 1) += lmdy(row_idx) * sol(asub[0]);

            if(PRINTLEVEL > 0 && abs(lmdy(row_idx)) > LMD_TOL){
                double fval = aval[0] * sol(asub[0]);
                printf("At row %d lmdy = %f end position (%d) limit value %f\n", row_idx, lmdy(row_idx), i, fval);
            }

            row_idx ++;
        }
        // velocity :
        for (int i = 0; i < 3; i++)
        {
            row_idx ++;
        }
        // acceleration :
        for (int i = 0; i < 3; i++)
        {
            int nzi = 3;
            MSKint32t asub[nzi];
            double aval[nzi];
            asub[0] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num - 2;
            asub[1] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num - 1;
            asub[2] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num;
            aval[0] =   1.0 / lstScale;
            aval[1] = - 2.0 / lstScale;
            aval[2] =   1.0 / lstScale;
            for(int j = 0; j < nzi; j++)
                agrad(segment_num - 1) += lmdy(row_idx) * (-aval[j] / lstScale * sol(asub[j]));

            if(PRINTLEVEL > 0 && abs(lmdy(row_idx)) > LMD_TOL){
                double fval = aval[0] * sol(asub[0]) + aval[1] * sol(asub[1]) + aval[2] * sol(asub[2]);
                printf("At row %d lmdy = %f end acceleration (%d) limit value %f\n", row_idx, lmdy(row_idx), i, fval);
            }

            row_idx ++;
        }
    }

    /*   joint points  */
    //ROS_WARN(" joint position");
    {
        int sub_shift = 0;
        double val0, val1;
        for (int k = 0; k < (segment_num - 1); k ++ )
        {
            double scale_k = corridor[k].t;
            double scale_n = corridor[k + 1].t;
            // position :
            val0 = scale_k;
            val1 = scale_n;
            for (int i = 0; i < 3; i++)
            {   // loop for x, y, z
                int nzi = 2;
                MSKint32t asub[nzi];
                double aval[nzi], val_grad[nzi];

                // This segment's last control point
                aval[0] = 1.0 * val0;
                asub[0] = sub_shift + (i + 1) * s1d1CtrlP_num - 1;
                val_grad[0] = 1.0;

                // Next segment's first control point
                aval[1] = -1.0 * val1;
                asub[1] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num;
                val_grad[1] = -1.0;
                agrad(k) += lmdy(row_idx) * val_grad[0] * sol(asub[0]);
                agrad(k + 1) += lmdy(row_idx) * val_grad[1] * sol(asub[1]);

                if(PRINTLEVEL > 0 && abs(lmdy(row_idx)) > LMD_TOL){
                    double fval = aval[0] * sol(asub[0]) + aval[1] * sol(asub[1]);
                    printf("At row %d lmdy = %f joint position (%d, %d) limit value %f\n", row_idx, lmdy(row_idx), k, i, fval);
                }

                row_idx ++;
            }

            for (int i = 0; i < 3; i++)
            {
                row_idx ++;
            }
            // acceleration :
            val0 = 1.0 / scale_k;
            val1 = 1.0 / scale_n;
            for (int i = 0; i < 3; i++)
            {
                int nzi = 6;
                MSKint32t asub[nzi];
                double aval[nzi];

                // This segment's last velocity control point
                aval[0] =  1.0  * val0;
                aval[1] = -2.0  * val0;
                aval[2] =  1.0  * val0;
                asub[0] = sub_shift + (i + 1) * s1d1CtrlP_num - 3;
                asub[1] = sub_shift + (i + 1) * s1d1CtrlP_num - 2;
                asub[2] = sub_shift + (i + 1) * s1d1CtrlP_num - 1;
                for(int j = 0; j < 3; j++)
                    agrad(k) += lmdy(row_idx) * (-aval[j] / scale_k * sol(asub[j]));
                // Next segment's first velocity control point
                aval[3] =  -1.0  * val1;
                aval[4] =   2.0  * val1;
                aval[5] =  -1.0  * val1;
                asub[3] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num;
                asub[4] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num + 1;
                asub[5] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num + 2;

                for(int j = 0; j < 3; j++)
                    agrad(k + 1) += lmdy(row_idx) * (-aval[j + 3] / scale_n * sol(asub[j + 3]));

                if(PRINTLEVEL > 0 && abs(lmdy(row_idx)) > LMD_TOL){
                    double fval = 0;
                    for(int j = 0; j < 6; j++)
                        fval += aval[j] * sol(asub[j]);
                    printf("At row %d lmdy = %f joint acceleration (%d, %d) limit value %f\n", row_idx, lmdy(row_idx), k, i, fval);
                }

                row_idx ++;
            }

            sub_shift += s1CtrlP_num;
        }
    }
    return agrad;
}


/* Even if we use snopt, I realized all constraints can be written as linear.
 * This means, non-linearity only comes from cost function and snopt should be fast for this problem.
 * I am starting to get worried.
 *
 */

void assign_linear_constraint(RefVX F, cRefVX coef, int row_idx, int nz, double *aval, int *asub, double timefactor){
    F(row_idx) = 0;
    for(int i = 0; i < nz; i++)
        F(row_idx) += aval[i] * timefactor * coef(asub[i]);
}

int assign_G(RefVX G, ReflVX row, ReflVX col, int row_idx, int nG, int nz, double *aval, int *asub, double timefactor, bool rec){
    for(int i = 0; i < nz; i++){
        G[nG] = timefactor * aval[i];
        if(rec){
            row[nG] = row_idx;
            col[nG] = asub[i];
        }
        nG++;
    }
    return nG;
}


double cost_eval_with_grad(cRefVX coef, cRefVX room_time, int traj_order, double minimize_order, cRefMX MQM, RefVX G, bool needg){
    int segment_num = room_time.size();
    int n_poly = traj_order + 1;
    int s1d1CtrlP_num = n_poly;
    int s1CtrlP_num   = 3 * s1d1CtrlP_num;
    int n_coef = s1CtrlP_num * segment_num;  // number of coefficients

    int min_order_l = floor(minimize_order);
    int min_order_u = ceil (minimize_order);

    int sub_shift = 0;
    double cost = 0;  // record total cost
    for (int k = 0; k < segment_num; k++)
    {
        double scale_k = room_time(k);
        double djdt = 0;
        for (int p = 0; p < 3; p ++ ){
            for ( int i = 0; i < s1d1CtrlP_num; i ++ ){
                int row_id = sub_shift + p * s1d1CtrlP_num + i;
                if(needg)
                    G(row_id) = 0;
                for ( int j = 0; j < s1d1CtrlP_num; j ++ ){
                    int col_id = sub_shift + p * s1d1CtrlP_num + j;
                    double val = 0;
                    double dvdt = 0;
                    //qval[idx]  = MQM(i, j) /(double)pow(scale_k, 3);
                    if (min_order_l == min_order_u){
                        val  = MQM(i, j) / (double)pow(scale_k, 2 * min_order_u - 3);
                        dvdt = MQM(i, j) * (double)pow(scale_k, 2 - 2 * min_order_u) * (3 - 2 * min_order_u);
                    }
                    else{
                        val = ( (minimize_order - min_order_l) / (double)pow(scale_k, 2 * min_order_u - 3)
                                      + (min_order_u - minimize_order) / (double)pow(scale_k, 2 * min_order_l - 3) ) * MQM(i, j);
                        dvdt = ( (minimize_order - min_order_l) / (double)pow(scale_k, 2 * min_order_u - 2) * (3 - 2 * min_order_u)
                                      + (min_order_u - minimize_order) / (double)pow(scale_k, 2 * min_order_l - 2) * (3 - 2 * min_order_l) ) * MQM(i, j);
                    }
                    cost += 0.5 * coef(row_id) * val * coef(col_id);
                    djdt += 0.5 * coef(row_id) * dvdt * coef(col_id);
                    if(needg)
                        G(row_id) += val * coef(col_id);
                }
            }
        }
        if(needg)
            G(n_coef + k) = djdt;
        sub_shift += s1CtrlP_num;
    }
    return cost;
}

// evaluate cost function using this code
std::pair<double, VX> eval_f(cRefVX coef, cRefVX room_time, int traj_order, double minimize_order, cRefMX MQM, bool needg){
    VX G(1);
    if(needg)
        G.resize(room_time.size() + coef.size());
    double cost = cost_eval_with_grad(coef, room_time, traj_order, minimize_order, MQM, G, needg);
    return std::make_pair(cost, G);
}

// evaluate the snopt constraint function, return nF and nG, this is good.
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
            bool needlub
        ){
    int nG = 0;

    double initScale = corridor.front().t;
    double lstScale  = corridor.back().t;
    int segment_num  = corridor.size();
    VX room_time(segment_num);
    for(int i = 0; i < segment_num; i++)
        room_time(i) = corridor[i].t;
    int ncoef = coef.size();
    if(PRINTLEVEL > 0)
        std::cout << "ncoef = " << ncoef << " num_room = " << segment_num << std::endl;

    int n_poly = traj_order + 1;
    int s1d1CtrlP_num = n_poly;
    int s1CtrlP_num   = 3 * s1d1CtrlP_num;

    int equ_con_s_num = 3 * 3; // p, v, a in x, y, z axis at the start point
    int equ_con_e_num = 3 * 3; // p, v, a in x, y, z axis at the end point
    int equ_con_continuity_num = 3 * 3 * (segment_num - 1);
    int equ_con_num   = equ_con_s_num + equ_con_e_num + equ_con_continuity_num; // p, v, a in x, y, z axis in each segment's joint position

    int vel_con_num = 3 *  traj_order * segment_num;
    int acc_con_num = 3 * (traj_order - 1) * segment_num;

    if ( !isLimitVel )
        vel_con_num = 0;

    if ( !isLimitAcc)
        acc_con_num = 0;


    //int high_order_con_num = vel_con_num + acc_con_num;
    //int high_order_con_num = 0; //3 * traj_order * segment_num;
    if(needlub){
        lb(0) = -std::numeric_limits<double>::infinity();
        ub(0) = std::numeric_limits<double>::infinity();
    }
    F(0) = cost_eval_with_grad(coef, room_time, traj_order, minimize_order, MQM, G, needg);
    if(rec){
        for(int i = 0; i < ncoef + segment_num; i++){
            row(i) = 0;
            col(i) = i;
        }
    }
    nG = ncoef + segment_num;

    //int con_num   = equ_con_num + high_order_con_num;
    int ctrlP_num = segment_num * s1CtrlP_num;

    int row_idx = 1;  //this is easy and obvious, the first row is for cost function
    // The velocity constraints
    if (isLimitVel)
    {
        for (int k = 0; k < segment_num ; k ++ )
        {
            for (int i = 0; i < 3; i++)
            {   // for x, y, z loop
                for (int p = 0; p < traj_order; p++)
                {
                    const int nzi = 2;
                    int asub[nzi];
                    double aval[nzi];

                    aval[0] = -1.0 * traj_order;
                    aval[1] =  1.0 * traj_order;

                    asub[0] = k * s1CtrlP_num + i * s1d1CtrlP_num + p;
                    asub[1] = k * s1CtrlP_num + i * s1d1CtrlP_num + p + 1;

                    assign_linear_constraint(F, coef, row_idx, nzi, aval, asub, 1);  // no need to register this one
                    if(needg)
                        nG = assign_G(G, row, col, row_idx, nG, nzi, aval, asub, 1, rec);
                    if(needlub){
                        lb(row_idx) = -maxVel;
                        ub(row_idx) = maxVel;
                    }
                    row_idx ++;
                }
            }
        }
        if(PRINTLEVEL > 0)
            std::cout << "Finish Vel limit\n";
    }


    // The acceleration constraints
    if (isLimitAcc)
    {
        for (int k = 0; k < segment_num ; k ++ )
        {
            double kt = corridor[k].t;
            for (int i = 0; i < 3; i++)
            {
                for (int p = 0; p < traj_order - 1; p++)
                {
                    const int nzi = 3;
                    int asub[nzi];
                    double aval[nzi];

                    aval[0] =  1.0 * traj_order * (traj_order - 1) / kt;
                    aval[1] = -2.0 * traj_order * (traj_order - 1) / kt;
                    aval[2] =  1.0 * traj_order * (traj_order - 1) / kt;
                    asub[0] = k * s1CtrlP_num + i * s1d1CtrlP_num + p;
                    asub[1] = k * s1CtrlP_num + i * s1d1CtrlP_num + p + 1;
                    asub[2] = k * s1CtrlP_num + i * s1d1CtrlP_num + p + 2;

                    assign_linear_constraint(F, coef, row_idx, nzi, aval, asub, kt);
                    F[row_idx] += maxAcc * kt;
                    if(needlub){
                        lb(row_idx) = 0;
                        ub(row_idx) = 1e20;
                    }
                    if(needg){
                        nG = assign_G(G, row, col, row_idx, nG, nzi, aval, asub, kt, rec);
                        G[nG] = maxAcc;
                        if(rec){
                            row[nG] = row_idx;
                            col[nG] = ncoef + k;
                        }
                        nG++;
                    }
                    row_idx++;

                    assign_linear_constraint(F, coef, row_idx, nzi, aval, asub, kt);
                    F[row_idx] -= maxAcc * kt;
                    if(needlub){
                        lb(row_idx) = -1e20;
                        ub(row_idx) = 0;
                    }
                    if(needg){
                        nG = assign_G(G, row, col, row_idx, nG, nzi, aval, asub, kt, rec);
                        G[nG] = -maxAcc;
                        if(rec){
                            row[nG] = row_idx;
                            col[nG] = ncoef + k;
                        }
                        nG++;
                    }
                    row_idx ++;
                }
            }
        }
        if(PRINTLEVEL > 0)
            std::cout << "Finish Acc limit\n";
    }
    /*   Start position  */
    {
        // position :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 1;
            int asub[nzi];
            double aval[nzi];
            aval[0] = 1.0 * initScale;
            asub[0] = i * s1d1CtrlP_num;
            F(row_idx) = aval[0] * coef(asub[0]);
            if(needlub){
                lb(row_idx) = pos(0, i);
                ub(row_idx) = pos(0, i);
            }
            if(needg){
                G[nG] = aval[0];  // w.r.t. coef
                if(rec){
                    row[nG] = row_idx;
                    col[nG] = asub[0];
                }
                nG++;
                G[nG] = coef(asub[0]);  // w.r.t time
                if(rec){
                    row[nG] = row_idx;
                    col[nG] = ncoef + 0;
                }
                nG++;
            }
            row_idx ++;
        }
        if(PRINTLEVEL > 0)
            std::cout << "Finish start position\n";
        // velocity :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 2;
            MSKint32t asub[nzi];
            double aval[nzi];
            aval[0] = - 1.0 * traj_order;
            aval[1] =   1.0 * traj_order;
            asub[0] = i * s1d1CtrlP_num;
            asub[1] = i * s1d1CtrlP_num + 1;
            assign_linear_constraint(F, coef, row_idx, nzi, aval, asub, 1);  // pure linear, simple
            if(needg)
                nG = assign_G(G, row, col, row_idx, nG, nzi, aval, asub, 1, rec);  // pure linear G, no interaction
            if(needlub){
                lb(row_idx) = vel(0, i);
                ub(row_idx) = vel(0, i);
            }
            row_idx ++;
        }
        if(PRINTLEVEL > 0)
            std::cout << "Finish start vel\n";
        // acceleration :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 3;
            MSKint32t asub[nzi];
            double aval[nzi];
            aval[0] =   1.0 * traj_order * (traj_order - 1) / initScale;
            aval[1] = - 2.0 * traj_order * (traj_order - 1) / initScale;
            aval[2] =   1.0 * traj_order * (traj_order - 1) / initScale;
            asub[0] = i * s1d1CtrlP_num;
            asub[1] = i * s1d1CtrlP_num + 1;
            asub[2] = i * s1d1CtrlP_num + 2;

            assign_linear_constraint(F, coef, row_idx, nzi, aval, asub, initScale);
            F(row_idx) -= initScale * acc(0, i);
            if(needlub){
                lb(row_idx) = 0;
                ub(row_idx) = 0;
            }
            if(needg){
                nG = assign_G(G, row, col, row_idx, nG, nzi, aval, asub, initScale, rec);
                G(nG) = -acc(0, i);
                if(rec){
                    row(nG) = row_idx;
                    col(nG) = ncoef + 0;
                }
                nG++;
            }
            row_idx ++;
        }
        if(PRINTLEVEL > 0)
            std::cout << "Finish start acc\n";
    }

    /*   End position  */
    //ROS_WARN(" end position");
    {
        // position :
        for (int i = 0; i < 3; i++)
        {   // loop for x, y, z
            int nzi = 1;
            MSKint32t asub[nzi];
            double aval[nzi];
            asub[0] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num;
            aval[0] = 1.0 * lstScale;

            F(row_idx) = aval[0] * coef(asub[0]);
            if(needlub){
                lb(row_idx) = pos(1, i);
                ub(row_idx) = pos(1, i);
            }
            if(needg){
                G(nG) = aval[0];  // wrt coef
                if(rec){
                    row(nG) = row_idx;
                    col(nG) = asub[0];
                }
                nG++;
                G(nG) = coef(asub[0]);  // wrt time
                if(rec){
                    row(nG) = row_idx;
                    col(nG) = ncoef + segment_num - 1;
                }
                nG++;
            }
            row_idx ++;
        }
        if(PRINTLEVEL > 0)
            std::cout << "Finish end pos\n";
        // velocity :
        for (int i = 0; i < 3; i++)
        {
            int nzi = 2;
            MSKint32t asub[nzi];
            double aval[nzi];
            asub[0] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num - 1;
            asub[1] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num;
            aval[0] = - 1.0;
            aval[1] =   1.0;
            assign_linear_constraint(F, coef, row_idx, nzi, aval, asub, 1.0);
            if(needlub){
                lb(row_idx) = vel(1, i);
                ub(row_idx) = vel(1, i);
            }
            if(needg)
                nG = assign_G(G, row, col, row_idx, nG, nzi, aval, asub, 1.0, rec);
            row_idx ++;
        }
        if(PRINTLEVEL > 0)
            std::cout << "Finish end vel\n";
        // acceleration :
        for (int i = 0; i < 3; i++)
        {
            int nzi = 3;
            MSKint32t asub[nzi];
            double aval[nzi];
            asub[0] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num - 2;
            asub[1] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num - 1;
            asub[2] = ctrlP_num - 1 - (2 - i) * s1d1CtrlP_num;
            aval[0] =   1.0 / lstScale;
            aval[1] = - 2.0 / lstScale;
            aval[2] =   1.0 / lstScale;
            assign_linear_constraint(F, coef, row_idx, nzi, aval, asub, lstScale);
            F[row_idx] -= acc(1, i) * lstScale;
            if(needlub){
                lb(row_idx) = 0;
                ub(row_idx) = 0;
            }
            if(needg){
                nG = assign_G(G, row, col, row_idx, nG, nzi, aval, asub, lstScale, rec);
                G(nG) = acc(1, i);
                if(rec){
                    row(nG) = row_idx;
                    col(nG) = ncoef + segment_num - 1;
                }
                nG++;
            }
            row_idx ++;
        }
        if(PRINTLEVEL > 0)
            std::cout << "Finish end acc\n";
    }

    /*   joint points  */
    //ROS_WARN(" joint position");
    {
        int sub_shift = 0;
        double val0, val1;
        for (int k = 0; k < (segment_num - 1); k ++ )
        {
            double scale_k = corridor[k].t;
            double scale_n = corridor[k + 1].t;
            // position :
            val0 = scale_k;
            val1 = scale_n;
            for (int i = 0; i < 3; i++)
            {   // loop for x, y, z
                int nzi = 2;
                MSKint32t asub[nzi];
                double aval[nzi];

                // This segment's last control point
                aval[0] = 1.0 * val0;
                asub[0] = sub_shift + (i + 1) * s1d1CtrlP_num - 1;

                // Next segment's first control point
                aval[1] = -1.0 * val1;
                asub[1] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num;
                F(row_idx) = aval[0] * coef(asub[0]) + aval[1] * coef(asub[1]);
                if(needlub){
                    lb(row_idx) = 0;
                    ub(row_idx) = 0;
                }
                if(needg){
                    G(nG) = aval[0]; // coef 1
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = asub[0];
                    }
                    nG++;
                    G(nG) = coef(asub[0]);  // time 1
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = ncoef + k;
                    }
                    nG++;
                    G(nG) = aval[1];  // coef 2
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = asub[1];
                    }
                    nG++;
                    G(nG) = coef(asub[1]);
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = ncoef + k + 1;
                    }
                    nG++;
                }
                row_idx ++;
            }

            for (int i = 0; i < 3; i++)
            {
                int nzi = 4;
                MSKint32t asub[nzi];
                double aval[nzi];

                // This segment's last velocity control point
                aval[0] = -1.0;
                aval[1] =  1.0;
                asub[0] = sub_shift + (i + 1) * s1d1CtrlP_num - 2;
                asub[1] = sub_shift + (i + 1) * s1d1CtrlP_num - 1;
                // Next segment's first velocity control point
                aval[2] =  1.0;
                aval[3] = -1.0;

                asub[2] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num;
                asub[3] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num + 1;

                F(row_idx) = 0;
                for(int j = 0; j < nzi; j++){
                    F(row_idx) += aval[j] * coef(asub[j]);
                }
                if(needlub){
                    lb(row_idx) = 0;
                    ub(row_idx) = 0;
                }

                if(needg){
                    for(int j = 0; j < nzi; j++){
                        G(nG) = aval[j];
                        if(rec){
                            row(nG) = row_idx;
                            col(nG) = asub[j];
                        }
                        nG++;
                    }
                }
                row_idx ++;
            }
            // acceleration :
            val0 = 1.0 / scale_k;
            val1 = 1.0 / scale_n;
            for (int i = 0; i < 3; i++)
            {
                int nzi = 6;
                MSKint32t asub[nzi];
                double aval[nzi];

                // This segment's last velocity control point
                aval[0] =  1.0  * val0;
                aval[1] = -2.0  * val0;
                aval[2] =  1.0  * val0;
                asub[0] = sub_shift + (i + 1) * s1d1CtrlP_num - 3;
                asub[1] = sub_shift + (i + 1) * s1d1CtrlP_num - 2;
                asub[2] = sub_shift + (i + 1) * s1d1CtrlP_num - 1;
                // Next segment's first velocity control point
                aval[3] =  -1.0  * val1;
                aval[4] =   2.0  * val1;
                aval[5] =  -1.0  * val1;
                asub[3] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num;
                asub[4] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num + 1;
                asub[5] = sub_shift + s1CtrlP_num + i * s1d1CtrlP_num + 2;

                F(row_idx) = scale_n * (coef(asub[0]) - 2 * coef(asub[1]) + coef(asub[2])) +
                            scale_k * (-coef(asub[3]) + 2 * coef(asub[4]) - coef(asub[5]));
                if(needlub){
                    lb(row_idx) = 0;
                    ub(row_idx) = 0;
                }
                if(needg){
                    G(nG) = scale_n;  // coef 0
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = asub[0];
                    }
                    nG++;
                    G(nG) = - 2 * scale_n;  // coef 1
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = asub[1];
                    }
                    nG++;
                    G(nG) = scale_n;  // coef 2
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = asub[2];
                    }
                    nG++;
                    G(nG) = -scale_k;  // coef 3
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = asub[3];
                    }
                    nG++;
                    G(nG) = 2 * scale_k;  // coef 4
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = asub[4];
                    }
                    nG++;
                    G(nG) = -scale_k;  // coef 5
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = asub[5];
                    }
                    nG++;
                    G(nG) = coef(asub[0]) - 2 * coef(asub[1]) + coef(asub[2]);  // scale_n
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = ncoef + k + 1;
                    }
                    nG++;
                    G(nG) = (-coef(asub[3]) + 2 * coef(asub[4]) - coef(asub[5]));  // scale_k
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = ncoef + k;
                    }
                    nG++;
                }
                row_idx ++;
            }

            sub_shift += s1CtrlP_num;
        }
    }
    if(PRINTLEVEL > 0)
        std::cout << "Finish joint \n";

    /* bounds on variables, they are linear constraints now */
    /* ## define a container for control points' boundary and boundkey ## */
    /* ## dataType in one tuple is : boundary type, lower bound, upper bound ## */

    int var_idx = 0;
    for (int k = 0; k < segment_num; k++)
    {
        pyBox cube_     = corridor[k];
        double scale_k = cube_.t;

        for (int i = 0; i < 3; i++ )
        {
            for (int j = 0; j < n_poly; j ++ )
            {
                double lo_bound, up_bound;
                if (k > 0)
                {
                    lo_bound = (cube_.box[i].first  + margin) / scale_k;
                    up_bound = (cube_.box[i].second - margin) / scale_k;
                }
                else
                {
                    lo_bound = (cube_.box[i].first)  / scale_k;
                    up_bound = (cube_.box[i].second) / scale_k;
                }

                F(row_idx) = scale_k * coef(var_idx);
                if(needlub){
                    lb(row_idx) = lo_bound * scale_k;
                    ub(row_idx) = up_bound * scale_k;
                }
                if(needg){
                    G(nG) = scale_k;  // coef
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = var_idx;
                    }
                    nG++;
                    G(nG) = coef(var_idx);  // scale
                    if(rec){
                        row(nG) = row_idx;
                        col(nG) = ncoef + k;
                    }
                    nG++;
                }
                row_idx++;
                var_idx++;
            }
        }
    }

    if(PRINTLEVEL > 0)
        std::cout << "Finish bound \n";

    // The final constraint on total time, we may or may not limit it
    F(row_idx) = 0;
    for(int k = 0; k < segment_num; k++){
        F(row_idx) += corridor[k].t;
        if(needg){
            G(nG) = 1;
            if(rec){
                row(nG) = row_idx;
                col(nG) = ncoef + k;
            }
            nG++;
        }
    }
    row_idx++;
    if(PRINTLEVEL > 0)
        std::cout << "Finish sum time \n";
    return std::make_pair(row_idx, nG);
}


