#pragma once

#include <LeapSDK/Leap.h>
#include <LeapSDK/LeapMath.h>
#include <LeapSDK/util/LeapUtilGL.h>

#include <array>
#include <iosfwd>
#include <limits>
#include <vector>

namespace LeapAsl
{
    
using joint_position = Leap::Vector;                        //!< The position of a joint in space.
using finger_position = std::array<joint_position, 4>;      //!< The positions of all of a finger's joints.
using fingers_position = std::array<finger_position, 5>;    //!< The position of all of a hand's fingers' joints.

using bones_directions = std::array<Leap::Matrix, 20>;      //!< The bases for all the bones.
    
//! Converts a \ref Leap::Hand into a \ref fingers_position.
fingers_position to_position(Leap::Hand const& hand);

//! Extracts the bases for all the bones in a Leap::Hand.
bones_directions to_directions(Leap::Hand const& hand);
    
    
//! The sum of the squared distances between all joints of \a and \b.
//!
//! As an optimization, will return as soon as delta_cap is reached.
double difference(fingers_position const& a, fingers_position const& b, double const delta_cap = std::numeric_limits<double>::max());

//! The sum of the angles between all the bases of \a and \b.
//!
//! As an optimization, will return as soon as delta_cap is reached.
double difference(bones_directions const& a, bones_directions const& b, double const delta_cap = std::numeric_limits<double>::max());
    

//! Computes the transformation matrix to apply to joint coordinates so that
//! the hand:
//!  - has its palm centered to the origin
//!  - is scaled to a palm width of 10 centimeters and
//!  - is rotated such that its palm is flat on the X-Z plane and pointing in the direction of the -Z axis
Leap::Matrix normalized_hand_transform(Leap::Hand const& hand);

//! Draw a skeleton hand using OpenGL.
//!
//! Lifted from sample code, augmented with a transform to apply to the hand.
void draw_skeleton_hand(Leap::Hand const& hand, Leap::Matrix const& transformation = Leap::Matrix::identity(), LeapUtilGL::GLVector4fv const& bone_color = LeapUtilGL::GLVector4fv::One(), LeapUtilGL::GLVector4fv const& joint_color = LeapUtilGL::GLVector4fv::One());

}

namespace std
{
    
std::ostream& operator<<(std::ostream& o, LeapAsl::fingers_position const& positions);
    
}