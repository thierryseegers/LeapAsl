#pragma once

#include <LeapSDK/Leap.h>

// Computes the transformation matrix to apply to joint coordinates so that
// the hand appears centered to the origin, scaled to a palm width of 10 centimeters and
// rotated such that its palm is flat on the X-Z plane and pointing in the direction of the -Z axis.
Leap::Matrix normalized_hand_transform(Leap::Hand const& hand);
