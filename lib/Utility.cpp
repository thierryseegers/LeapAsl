#include "include/LeapLearnedGestures/Utility.h"

#include <LeapSDK/Leap.h>

namespace LearnedGestures
{

using namespace std;
    
double distance_squared(Leap::Vector const& first, Leap::Vector const& second)
{
    return pow(first.x - second.x, 2.) + pow(first.y - second.y, 2.) + pow(first.z - second.z, 2.);
}

double difference(fingers_position const& a, fingers_position const& b, double const delta_cap)
{
    double delta = 0.;
    
    for(auto i = 0; i != a.size() && delta < delta_cap; ++i)
    {
        for(auto j = 0; j != a[i].size() && delta < delta_cap; ++j)
        {
            delta += distance_squared(a[i][j], b[i][j]);
        }
    }
    
    return delta;
}

fingers_position to_position(Leap::Hand const& hand)
{
    fingers_position fp;
    
    auto const normalized = normalized_hand_transform(hand);
    auto const& fingers = hand.fingers();
    
    for(int f = 0; f != 5; ++f)
    {
        auto const& finger = fingers[f];
        
        for(int b = Leap::Bone::TYPE_METACARPAL; b != Leap::Bone::TYPE_DISTAL; ++b)
        {
            auto const& bone = finger.bone((Leap::Bone::Type)b);
            
            if(b == Leap::Bone::TYPE_METACARPAL)
            {
                fp[f][b] += normalized.transformPoint(bone.prevJoint());
                fp[f][b + 1] = normalized.transformPoint(bone.nextJoint());
            }
            else
            {
                fp[f][b + 1] = normalized.transformPoint(bone.nextJoint());
            }
        }
    }
    
    return fp;
}

Leap::Matrix normalized_hand_transform(Leap::Hand const& hand)
{
    Leap::Matrix normalized;
    
    // Make it look like the right hand.
    if(hand.isLeft())
    {
        normalized *= {{-1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    }

    normalized *= {{0, 0, 1}, hand.palmNormal().roll()};    // Negate rotations.
    normalized *= {{0, -1, 0}, hand.direction().yaw()};
    normalized *= {{1, 0, 0}, hand.direction().pitch()};
    
    float const scale_factor = 100. / hand.palmWidth();
    normalized *= {{scale_factor, 0, 0}, {0, scale_factor, 0}, {0, 0, scale_factor}};   // Normalize scale to 10 cm. palm width.
    
    return normalized;
}
    
}