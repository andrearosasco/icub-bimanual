    ////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                                                                                //
  //                                       Useful functions                                         //
 //                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <Eigen/Core>                                                                               // Eigen::Vector
#include <iostream>                                                                                 // std::cerr and std::cout
#include <map>                                                                                      // std::map
#include <vector>                                                                                   // std::vector
#include <yarp/os/Bottle.h>                                                                         // yarp::os::Bottle

// This is to attach the joint positions to the times for a joint trajectory
struct JointTrajectory
{
	std::vector<Eigen::VectorXd> waypoints;
	std::vector<double> times;
};

Eigen::VectorXd vector_from_bottle(const yarp::os::Bottle *bottle);                                 // Convert a list of floating point numbers to an Eigen::Vector object

std::vector<std::string> string_from_bottle(const yarp::os::Bottle *bottle);                        // Convert a list of strings to a std::vector<std:string>> 

bool load_joint_configurations(const yarp::os::Bottle *bottle, std::map<std::string,JointTrajectory> &map); // Put joint trajectories from the config file in to a std::map

#endif
