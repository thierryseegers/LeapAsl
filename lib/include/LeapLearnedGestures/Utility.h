#pragma once

#include <LeapSDK/Leap.h>

#include <array>
#include <limits>
#include <vector>

namespace LearnedGestures
{
    
using joint_position = Leap::Vector;                        //!< The position of a joint in space.
using finger_position = std::array<joint_position, 4>;      //!< The positions of all of a finger's joints.
using fingers_position = std::array<finger_position, 5>;    //!< The position of all of a hand's fingers' joints.
    
//! Converts a \ref Leap::Hand into a \ref fingers_position.
fingers_position to_position(Leap::Hand const& hand);

//! The sum of the squared distances between all joints of \a and \b.
//!
//! As an optimization, will return as soon as delta_cap is reached.
float difference(fingers_position const& a, fingers_position const& b, float const delta_cap = std::numeric_limits<float>::max());


//! Computes the transformation matrix to apply to joint coordinates so that
//! the hand:
//!  - has its palm centered to the origin
//!  - is scaled to a palm width of 10 centimeters and
//!  - is rotated such that its palm is flat on the X-Z plane and pointing in the direction of the -Z axis
Leap::Matrix normalized_hand_transform(Leap::Hand const& hand);

}