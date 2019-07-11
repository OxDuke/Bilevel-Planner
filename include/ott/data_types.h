//
// Bill's data_type
//


#ifndef _DATA_TYPES_H_
#define _DATA_TYPES_H_

#include <vector>
#include <iostream>
#include <algorithm>

#include <eigen3/Eigen/Dense>

// #include "utils.h"

using namespace Eigen;
using namespace std;

// https://en.wikipedia.org/wiki/Hyperrectangle
// Box is a rectangle in 3d space, flight corridor is composed of boxes
class Box
{

	/*
	           P4------------P3
	           /|           /|              ^
	          / |          / |              | z
	        P1--|---------P2 |              |
	         |  P8--------|--p7             |
	         | /          | /               /--------> y
	         |/           |/               /
	        P5------------P6              / x
	*/

public:

	Box()
	{
		center = Eigen::VectorXd::Zero(3);
		vertex = Eigen::MatrixXd::Zero(8, 3);

		valid = true;
		t = 0.0;
		box.resize(3);
	}
    
    // initialize using axis aligned bounds (a box)
    // center is set to the geometric center of the bound
	Box(std::vector< std::pair<double, double> > axis_aligned_bounds):Box()
	{  
        
        Eigen::MatrixXd vertex_ = Eigen::MatrixXd::Zero(8, 3);
		Eigen::Vector3d center_ = Eigen::VectorXd::Zero(3);

		// lower/upper x
		vertex_(2, 0) = vertex_(3, 0) = vertex_(6, 0) = vertex_(7, 0) = axis_aligned_bounds[0].first;
		vertex_(0, 0) = vertex_(1, 0) = vertex_(4, 0) = vertex_(5, 0) = axis_aligned_bounds[0].second;

		// lower/upper y
		vertex_(0, 1) = vertex_(3, 1) = vertex_(4, 1) = vertex_(7, 1) = axis_aligned_bounds[1].first;
		vertex_(1, 1) = vertex_(2, 1) = vertex_(5, 1) = vertex_(6, 1) = axis_aligned_bounds[1].second;

      
        // lower/upper z
        vertex_(4, 2) = vertex_(5, 2) = vertex_(6, 2) = vertex_(7, 2) = axis_aligned_bounds[2].first;
		vertex_(0, 2) = vertex_(1, 2) = vertex_(2, 2) = vertex_(3, 2) = axis_aligned_bounds[2].second;

		center_(0) = 0.5 * (axis_aligned_bounds[0].first + axis_aligned_bounds[0].second);
		center_(1) = 0.5 * (axis_aligned_bounds[1].first + axis_aligned_bounds[1].second);
		center_(2) = 0.5 * (axis_aligned_bounds[2].first + axis_aligned_bounds[2].second);

        vertex = vertex_;
        center = center_;

	}

	// @todo, change variable names
	// Create a box by its center position and 8 vertices' positions
	Box(Eigen::MatrixXd vertex_, Eigen::Vector3d center_)
	{
		vertex = vertex_;
		center = center_;
		valid = true;
		t = 0.0;
		box.resize(3);
	}

	// @todo add doc for this function
	void setVertex( Eigen::MatrixXd vertex_, double resolution_)
	{
		vertex = vertex_;
		vertex(0, 1) -= resolution_ / 2.0;
		vertex(3, 1) -= resolution_ / 2.0;
		vertex(4, 1) -= resolution_ / 2.0;
		vertex(7, 1) -= resolution_ / 2.0;

		vertex(1, 1) += resolution_ / 2.0;
		vertex(2, 1) += resolution_ / 2.0;
		vertex(5, 1) += resolution_ / 2.0;
		vertex(6, 1) += resolution_ / 2.0;

		vertex(0, 0) += resolution_ / 2.0;
		vertex(1, 0) += resolution_ / 2.0;
		vertex(4, 0) += resolution_ / 2.0;
		vertex(5, 0) += resolution_ / 2.0;

		vertex(2, 0) -= resolution_ / 2.0;
		vertex(3, 0) -= resolution_ / 2.0;
		vertex(6, 0) -= resolution_ / 2.0;
		vertex(7, 0) -= resolution_ / 2.0;

		vertex(0, 2) += resolution_ / 2.0;
		vertex(1, 2) += resolution_ / 2.0;
		vertex(2, 2) += resolution_ / 2.0;
		vertex(3, 2) += resolution_ / 2.0;

		vertex(4, 2) -= resolution_ / 2.0;
		vertex(5, 2) -= resolution_ / 2.0;
		vertex(6, 2) -= resolution_ / 2.0;
		vertex(7, 2) -= resolution_ / 2.0;

		setBox();
	}

	// @todo might want to delete this function
	void setBox()
	{
		box.clear();
		box.resize(3);
		box[0] = std::make_pair( vertex(3, 0), vertex(0, 0) );
		box[1] = std::make_pair( vertex(0, 1), vertex(1, 1) );
		box[2] = std::make_pair( vertex(4, 2), vertex(1, 2) );
	}

	// @todo might want to delete this function
	void printBox(std::string name = "None")
	{   
		std::cout << "Name: " << name << std::endl;
		std::cout << "--> Center of the box: \n" << center << std::endl;
		std::cout << "--> Vertex of the box: \n" << vertex << std::endl;
		
		std::cout << "--> Axis aligned bounds: \n" ;
		for (unsigned int i = 0; i < box.size(); ++i)
		{
			cout << "    " << i << "th pair: " << "[" << box[i].first << ", " << box[i].second << "]" << endl;
		}
	}

	// @todo working on this function
	// This function slices the box into two slightly overlapping boxes,
	// Time t is evenly sliced @todo, slicing time is not implemented
	// overlapping_portion is number from 0.0 to 1.0, that controls the overlapping portion.
	bool sliceIntoTwo(char axis, 
		std::vector< std::pair<double, double> >& newBound1, 
		std::vector< std::pair<double, double> >& newBound2, 
		//std::shared_ptr<Box> newBox1,
		//std::shared_ptr<Box> newBox2,
		double overlapping_portion=0.1)
	{   
		axis = static_cast<char> (tolower(axis));

		// std::cout << "lower: " << axis << "eq: " << (axis == 'x') << std::endl;
		
		if (axis != 'x' && axis != 'y' && axis != 'z')
		{
			std::cout << "Axis: " << axis << " not recognized! " << std::endl;
			return false;
		}

		// std::cout << "Slice axis: " << axis << std::endl;
        
        // set bound
		setBox();

		newBound1 = box;
		newBound2 = box;
        
        //std::vector< std::pair<double, double> > newBound1(box), newBound2(box);
		
		//newBound1.clear();
		//newBound2.clear();

		//newBound1.resize(3);
		//newBound2.resize(3);
        
        // copy box to newBound1, newBound2
        //for (unsigned int i = 0; i < 3; ++i)
        //{
        //	newBound1[i] = std::make_pair(box[i].first, box[i].second);
        //	newBound2[i] = std::make_pair(box[i].first, box[i].second);
        //}
		
		unsigned int index = axis - 'x'; // index = 0 (x axis), 1 (y axis) or 2 (z axis)

		// std::cout << "index: " << index << std::endl;

		//overlapping_portion = std::clamp(overlapping_portion, 0.0, 1.0);

		double newUpperBound = box[index].first + (0.5 + overlapping_portion) * (box[index].second - box[index].first);
		double newLowerBound = box[index].first + (0.5 - overlapping_portion) * (box[index].second - box[index].first);

		newBound1[index] = std::make_pair( box[index].first, newUpperBound );
		newBound2[index] = std::make_pair( newLowerBound, box[index].second );

		//  newBox1 = new Box(newBound1);
		//newBox1.setBox();
		//  newBox2 = new Box(newBound2);
		//newBox2.setBox();


		// std::cout << "Done" << std::endl;

		return true;
	}


public:

	static bool ifContains(const Box box1, const Box box2)
	{
		if ( box1.vertex(0, 0) >= box2.vertex(0, 0) && box1.vertex(0, 1) <= box2.vertex(0, 1) && box1.vertex(0, 2) >= box2.vertex(0, 2) &&
		        box1.vertex(6, 0) <= box2.vertex(6, 0) && box1.vertex(6, 1) >= box2.vertex(6, 1) && box1.vertex(6, 2) <= box2.vertex(6, 2)  )
			return true;
		else
			return false;
	}

public:

	Eigen::MatrixXd vertex; // p1, p2, p3, p4, p5, p6, p7, p8, the 8 vertices of a box
	Eigen::Vector3d center; // the center of the box

	bool valid;    // indicates whether this box should be deleted

	double t; // time allocated to this box

	std::vector< std::pair<double, double> > box;


};

#endif /* _DATA_TYPES_H_ */

