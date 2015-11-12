#include "include/LeapLearnedGestures/NormalizedHandTransform.h"

#include <LeapSDK/Leap.h>

Leap::Matrix normalized_hand_transform(Leap::Hand const& hand)
{
    Leap::Matrix normalized;
    
    normalized *= {{0, 0, 1}, hand.palmNormal().roll()};    // Negate rotations.
    normalized *= {{0, -1, 0}, hand.direction().yaw()};
    normalized *= {{1, 0, 0}, hand.direction().pitch()};
    
    float const scale_factor = 100. / hand.palmWidth();
    normalized *= {{scale_factor, 0, 0}, {0, scale_factor, 0}, {0, 0, scale_factor}};   // Normalize scale to 10 cm. palm width.
    
    normalized *= {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, -hand.palmPosition()}; // Translate to origin.
    
    return normalized;
}