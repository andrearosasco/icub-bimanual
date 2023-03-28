    ////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                                                                                //
  //          A trajectory across two or more poses (position & orientation) in 3D space.           //
 //                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <CartesianTrajectory.h>

  ////////////////////////////////////////////////////////////////////////////////////////////////////
 //                                         Constructor                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////
CartesianTrajectory::CartesianTrajectory(const std::vector<Eigen::Isometry3d> &poses,
                                         const std::vector<double> &times) :
                                         numPoses(poses.size())
{
	// Check that the inputs are sound
	std::string message = "[ERROR] [CARTESIAN TRAJECTORY] Constructor: ";
	
	if(poses.size() != times.size())
	{
		message += "Pose vector had " + std::to_string(poses.size()) + " elements, and the "
		           "time vector had " + std::to_string(times.size()) + " elements.";
		
		throw std::invalid_argument(message);
	}
	else if(poses.size() < 2)
	{
		message += "A minimum of 2 poses is needed to create a trajectory.";
		
		throw std::invalid_argument(message);
	}
	else
	{
		std::vector<std::vector<double>> points; points.resize(6);                          // 6 dimension in 3D space
		for(int i = 0; i < 6; i++) points[i].resize(this->numPoses);
		
		// Extract all the positions and euler angles for each pose
		for(int i = 0; i < this->numPoses; i++)
		{
			// Extract the translation and put it in the points array
			Eigen::Vector3d translation = poses[i].translation();
			for(int j = 0; j < 3; j++) points[j][i] = translation[j];
			
			// Extract the orientation as Euler anggles
			// NOTE: This is not ideal due to gimbal lock...
			
			Eigen::Matrix<double,3,3> R = poses[i].rotation();                          // SO(3) matrix
		 	double roll, pitch, yaw;
		 	
		 	if(abs(R(0,2)) != 1)
		 	{
		 		pitch = asin(R(0,2));
		 		roll  = atan2(-R(1,2),R(2,2));
		 		yaw   = atan2(-R(0,1),R(0,0));
		 	}
		 	else // Gimbal lock; yaw - roll = atan2(R(1,0),R(1,1));
		 	{
		 		pitch = -M_PI/2;
		 		roll  = atan2(-R(1,0),R(1,1));
		 		yaw   = 0;
		 	}
		 	
		 	points[3][i] = roll;
		 	points[4][i] = pitch;
		 	points[5][i] = yaw;
		}
		 
		// Now insert them in to the iDynTree::CubicSpline object
		this->spline.resize(6);
		for(int i = 0; i < 6; i++)
		{
			this->spline[i].setData(iDynTree::VectorDynSize(times),
		 	                        iDynTree::VectorDynSize(points[i]));
		}
		
		this->isValid = true;
	}
}

  ////////////////////////////////////////////////////////////////////////////////////////////////////
 //                          Get the desired pose for the given time                               //
////////////////////////////////////////////////////////////////////////////////////////////////////
Eigen::Isometry3d CartesianTrajectory::get_pose(const double &time)
{
	double t = time;
	
	if(not this->isValid)
	{
		std::cerr << "[ERROR] [CARTESIAN TRAJECTORY] get_pose(): "
		          << "There was a problem during the construction of this object. "
		          << "Could not get the pose." << std::endl;
		          
		t = 0.0;
	}

	double pos[3];
	double rpy[3];
	
	for(int i = 0; i < 3; i++)
	{
		pos[i] = this->spline[ i ].evaluatePoint(t);
		rpy[i] = this->spline[i+3].evaluatePoint(t);
	}
	
	// Constructor and return the pose object
	return  Eigen::Translation3d(pos[0],pos[1],pos[2])                                          // Translation first
	      * Eigen::AngleAxisd(rpy[0], Eigen::Vector3d::UnitX())                                 // Rotation about x
              * Eigen::AngleAxisd(rpy[1], Eigen::Vector3d::UnitY())                                 // Rotation about y
              * Eigen::AngleAxisd(rpy[2], Eigen::Vector3d::UnitZ());                                // Rotation about z
}

  ////////////////////////////////////////////////////////////////////////////////////////////////////
 //                          Get the desired state for the given time                              //
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CartesianTrajectory::get_state(Eigen::Isometry3d         &pose,
                                    Eigen::Matrix<double,6,1> &vel,
                                    Eigen::Matrix<double,6,1> &acc,
                                    const double              &time)
{
	if(not this->isValid)
	{
		std::cerr << "[ERROR] [CARTESIAN TRAJECTORY] get_state(): "
		          << "There was a problem during the construction of this object. "
		          << "Could not get the state." << std::endl;
		
		return false;
	}
	else
	{
		double pos[3];                                                                      // Position vector
		double rpy[3];                                                                      // Euler angles
		
		for(int i = 0; i < 3; i++)
		{
			pos[i] = this->spline[ i ].evaluatePoint(time, vel[i]  , acc[i]);
			rpy[i] = this->spline[i+3].evaluatePoint(time, vel[i+3], acc[i+3]);
		}
		
		// Now construct the pose/transform object
		pose = Eigen::Translation3d(pos[0],pos[1],pos[2])                                   // Translation first
		     * Eigen::AngleAxisd(rpy[0], Eigen::Vector3d::UnitX())                          // Rotation about x
                     * Eigen::AngleAxisd(rpy[1], Eigen::Vector3d::UnitY())                          // Rotation about y
                     * Eigen::AngleAxisd(rpy[2], Eigen::Vector3d::UnitZ());                         // Rotation about z
		
		return true;
	}
}

#endif