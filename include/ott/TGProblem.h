#ifndef _TGProblem_H_
#define _TGProblem_H_

#include "data_types.h"
#include <eigen3/Eigen/Dense>

#include <fstream>
#include <iostream>
#include <vector>

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include "ott/eigen_boost_serialization.h"

using namespace Eigen;
using namespace std;

class TGProblem
{
public:
	TGProblem()
	{}

	TGProblem(const vector<Box> &corridor_,
	          const MatrixXd &MQM_,
	          const MatrixXd &position_,
	          const MatrixXd &velocity_,
	          const MatrixXd &acceleration_,
	          const double maxVelocity_,
	          const double maxAcceleration_,
	          const int trajectoryOrder_,
	          const double minimizeOrder_,
	          const double margin_,
	          const bool & doLimitVelocity_,
	          const bool & doLimitAcceleration_):
	corridor(corridor_),
	MQM(MQM_),
	position(position_),
	velocity(velocity_),
	acceleration(acceleration_),
	maxVelocity(maxVelocity_),
	maxAcceleration(maxAcceleration_),
	trajectoryOrder(trajectoryOrder_),
	minimizeOrder(minimizeOrder_),
	margin(margin_),
	doLimitVelocity(doLimitVelocity_),
	doLimitAcceleration(doLimitAcceleration_)
	{}

public:
	vector<Box> corridor;
	MatrixXd MQM;
	MatrixXd position;
	MatrixXd velocity;
	MatrixXd acceleration;
	double maxVelocity;
	double maxAcceleration;
	int trajectoryOrder;
	double minimizeOrder;
	double margin;
	bool doLimitVelocity;
	bool doLimitAcceleration;
};

// this tells boost how we can serialize class Box and class TGProblem
namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, Box & box, const unsigned int version)
{
	ar & box.vertex;
	ar & box.center;

	ar & box.valid;
	ar & box.t;

	ar & box.box;
}

template<class Archive>
void serialize(Archive & ar, TGProblem & problem, const unsigned int version)
{
	ar & problem.corridor;
	ar & problem.MQM;
	ar & problem.position;
	ar & problem.velocity;
	ar & problem.acceleration;
	ar & problem.maxVelocity;
	ar & problem.maxAcceleration;
	ar & problem.trajectoryOrder;
	ar & problem.minimizeOrder;
	ar & problem.margin;
	ar & problem.doLimitVelocity;
	ar & problem.doLimitAcceleration;
}

} // namespace serialization
} // namespace boost

// save a T(rajectory)G(eneration)Problem to file using Serialization
inline void saveTGProblemToFile(TGProblem& problem, const std::string fileName)
{
	std::ofstream ofs(fileName);

	// save data to archive
	{
		boost::archive::text_oarchive oa(ofs);
		// write class instance to archive
		oa << problem;
		// archive and stream closed when destructors are called
	}
}

// read a TGProblem from file
inline void loadTGProblemFromFile(const std::string fileName, TGProblem& problem)
{
	{
		// create and open an archive for input
		std::ifstream ifs(fileName);
		boost::archive::text_iarchive ia(ifs);
		// read class state from archive
		ia >> problem;
		// archive and stream closed when destructors are called
	}
    
    // @HACK This is a hack to deal with weired problems caused by Boost library:
    // On Ubuntu16.04 with Boost1.57, the deserialization procedure will weiredly 
    // add 3 pairs of zeros into problem.corridor[i].box, making it a vector of size
    // 6, the right size however should be 3. So we will reset the box everytime after
    // deserialzation.
	for (unsigned int i = 0; i < problem.corridor.size(); ++i)
	{
		problem.corridor[i].setBox();
	}
}


// test if two TGProblems are the same
// @todo still need to complete this function
inline bool sameTGProblem(const TGProblem& problem1, const TGProblem& problem2, const bool verbose=false)
{
	bool result = true;
    
    cout << "This function <sameTGProblem> is not completely implemented! " << endl;
    // compare something
    // compare MQM
	{
		double difference = (problem1.MQM - problem2.MQM).norm();

		if (verbose)
		{
			cout << ">Difference in MQM: " << difference << endl;
		}

		if (difference != 0.0)
		{
			return false;
		}
	}

	return result;
}

inline void printBox(const Box& box, const int index=-1)
{
	if (index >= 0)
	{
		cout << "This is the " << index << "th box" << endl;
	}

	cout << "--> Vertex: " << endl;
	cout << box.vertex << endl;

	cout << "--> Center: " << endl;
	cout << box.center << endl;

	cout << "--> Valid: " << box.valid << endl;
	cout << "--> t: " << box.t << endl;

	cout << "--> box: " << endl;
	for (unsigned int i = 0; i < box.box.size(); ++i)
	{
		cout << i << "th pair: " << "[" << box.box[i].first << ", " << box.box[i].second << "]" << endl;
	}

}

inline void printTGProblem(const TGProblem& p)
{
	cout << "> Corridor: " << "Number: " << p.corridor.size() << endl;
	for (unsigned int i = 0; i < p.corridor.size(); ++i)
	{
		printBox(p.corridor[i], i);
	}
	
	cout << "> MQM: " << endl;
	cout << p.MQM << endl;
	
	cout << "> position: " << endl;
	cout << p.position << endl;

	cout << "> velocity: " << endl;
	cout << p.velocity << endl;

	cout << "> acceleration: " << endl;
	cout << p.acceleration << endl;

	cout << "> maxVelocity: " << endl;
	cout << p.maxVelocity << endl;

	cout << "> maxAcceleration: " << endl;
	cout << p.maxAcceleration << endl;

	cout << "> trajectoryOrder: " << endl;
	cout << p.trajectoryOrder << endl;

	cout << "> minimizeOrder: " << endl;
	cout << p.minimizeOrder << endl;
	
	cout << "> margin: " << endl;
	cout << p.margin << endl;

	cout << "> doLimitVelocity: " << endl;
	cout << p.doLimitVelocity << endl;
	
	cout << "> doLimitAcceleration: " << endl;
	cout << p.doLimitAcceleration << endl;
}

// We do not slice the first and the last corridor
// The slicing strategy is this: if there is n+2 segments, and we want k more segments
// we will slice each of the n segments (segments in the middle) once:
// E.g. n = 2, k = 3, then we first slice the 2nd segment into 2 segments, then the 3rd (the original), then 2nd again.
inline bool sliceCorridor(TGProblem& problem, vector<char> sliceDirections, const unsigned int sliceTimes)
{
	vector<Box>& corridor = problem.corridor;

	unsigned int corridorLength = corridor.size();

	cout << "Corridor size: " << corridor.size() << endl;

	if (corridor.size() <= 2)
	{
		cout << "[Error]Cannot slice a corridor with #segments <= 2." << endl;
		return false;
	}

	if (corridor.size() != sliceDirections.size())
	{
		cout << "[Error]sliceDirections should have the same #segments as the corridor" << endl;
		return false;
	}

	if (sliceTimes <= 0)
	{
		cout << "sliceTimes must be larger than 0" << endl;
		return false;
	}
    
    // we slice from the 2nd segment
	unsigned int whichToSlice = 1;
	unsigned int localSliceTimes = 0;

	for (unsigned int i = 0; i < sliceTimes; ++i)
	{
		cout << "--> Slicing No." << whichToSlice << endl;
		Box& boxToSlice = corridor[whichToSlice];

		boxToSlice.setBox();

		std::vector< std::pair<double, double> > new_bound1, new_bound2;

		if (!boxToSlice.sliceIntoTwo(sliceDirections[whichToSlice], new_bound1, new_bound2))
		{
			cout << "[Error]Cannot slice a box." << endl;
			return false;

		}
		
		Box newBox1(new_bound1);
	    Box newBox2(new_bound2);

	    newBox1.setBox();
	    newBox2.setBox();

	    newBox1.t = boxToSlice.t / 2;
	    newBox2.t = boxToSlice.t / 2;
        
        // erase the one being sliced
	    corridor.erase(corridor.begin() + whichToSlice);

	    // insert new ones
	    vector<Box> slicedBoxes;
	    slicedBoxes.push_back(newBox1);
	    slicedBoxes.push_back(newBox2);
	    
	    corridor.insert(corridor.begin() + whichToSlice, slicedBoxes.begin(), slicedBoxes.end());

	    sliceDirections.insert(sliceDirections.begin() + whichToSlice, sliceDirections[whichToSlice]);

        whichToSlice += 2;
		localSliceTimes++;
        
        // if we have sliced every segment (exceopt the first and the last)
        // we start from the 2nd segment again
		if (localSliceTimes >= corridorLength - 2)
		{
			localSliceTimes = 0;
			whichToSlice = 1;

			corridorLength = corridor.size();

			cout << "[INFO] We start over again." << endl;
		}

		cout << "Directions: ";
		for (unsigned int i = 0; i < sliceDirections.size(); ++i)
		{
			cout << sliceDirections[i] << " ";
		}
		cout << endl;

	}

	return true;

}

#endif /* _TGProblem_H_ */