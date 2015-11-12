#include "detail/Pose.h"

#include "NormalizedHandTransform.h"

#include <LeapSDK/Leap.h>

#include <array>
#include <iostream>
#include <string>

namespace LearnedGestures { namespace detail
{

using namespace std;

float distance_squared(Pose::joint_position const& first, Pose::joint_position const& second)
{
    return pow(first.x - second.x, 2.) + pow(first.y - second.y, 2.) + pow(first.z - second.z, 2.);
}

Pose::fingers_capture Pose::normalized(Leap::Hand const& hand)
{
    fingers_capture capture;
    
    auto const& normalized = normalized_hand_transform(hand);
    auto const& fingers = hand.fingers();
    
    for(int f = 0; f != 5; ++f)
    {
        auto const& finger = fingers[f];
        
        for(int b = Leap::Bone::TYPE_METACARPAL; b != Leap::Bone::TYPE_DISTAL; ++b)
        {
            auto const& bone = finger.bone((Leap::Bone::Type)b);
            
            if(b == Leap::Bone::TYPE_METACARPAL)
            {
                capture[f][b] += normalized.transformPoint(bone.prevJoint());
                capture[f][b + 1] = normalized.transformPoint(bone.nextJoint());
            }
            else
            {
                capture[f][b + 1] = normalized.transformPoint(bone.nextJoint());
            }
        }
    }
    
    return capture;
}


string& Pose::name()
{
    return name_;
}

string const& Pose::name() const
{
    return name_;
}

void Pose::capture(fingers_capture const& capture)
{
    fingers_capture_ = capture;
}

float Pose::match(fingers_capture const& hand, float const error_cap) const
{
    float error = 0.;
    
    for(auto i = 0; i != fingers_capture_.size() && error < error_cap; ++i)
    {
        for(auto j = 0; j != fingers_capture_[i].size() && error < error_cap; ++j)
        {
            error += distance_squared(hand[i][j], fingers_capture_[i][j]);
        }
    }
    
    return error;
}

ostream& operator<<(ostream& o, Pose const& g)
{
    o << g.name_ << endl;
    
    for(auto const& finger : g.fingers_capture_)
    {
        for(auto const& joint : finger)
        {
            o << joint.x << joint.y << joint.z;
        }
    }
    
    return o;
}

istream& operator>>(istream& i, Pose& g)
{
    i >> g.name_;
    i.ignore();
    
    for(auto& finger : g.fingers_capture_)
    {
        for(auto& joint : finger)
        {
            i >> joint.x >> joint.y >> joint.z;
        }
    }
    
    return i;
}
    
}}
