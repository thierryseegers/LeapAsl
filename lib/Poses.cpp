#include "detail/Poses.h"

#include "NormalizedHandTransform.h"

#include <LeapSDK/Leap.h>

#include <array>
#include <limits>
#include <map>
#include <string>

namespace LearnedGestures { namespace detail
{

using namespace std;

float distance_squared(Leap::Vector const& first, Leap::Vector const& second)
{
    return pow(first.x - second.x, 2.) + pow(first.y - second.y, 2.) + pow(first.z - second.z, 2.);
}

// Returns the sum of the distances between Leap::Vectors of a and b.
//
// As an available optimization, will return as soon as error_cap is reached.
float match(array<array<Leap::Vector, 4>, 5> const& a, array<array<Leap::Vector, 4>, 5> const& b, float const error_cap = numeric_limits<float>::max())
{
    float error = 0.;
    
    for(auto i = 0; i != a.size() && error < error_cap; ++i)
    {
        for(auto j = 0; j != a[i].size() && error < error_cap; ++j)
        {
            error += distance_squared(a[i][j], b[i][j]);
        }
    }
    
    return error;
}

// 
array<array<Leap::Vector, 4>, 5> normalize(Leap::Hand const& hand)
{
    array<array<Leap::Vector, 4>, 5> capture;
    
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


void Poses::capture(string const& name, Leap::Hand const& hand)
{
    poses_[name] = normalize(hand);
}

string Poses::match(Leap::Hand const& hand) const
{
    auto const normalized = normalize(hand);
    
    float error_cap = numeric_limits<float>::max(), error;
    string name;
    
    for(auto const& pose : poses_)
    {
        if((error = detail::match(pose.second, normalized, error_cap)) < error_cap)
        {
            error_cap = error;
            name = pose.first;
        }
    }
    
    return name;
}

multimap<float, string> Poses::compare(Leap::Hand const& hand) const
{
    multimap<float, string> scores;
    
    auto const normalized = normalize(hand);
    
    for(auto const& pose : poses_)
    {
        scores.emplace(detail::match(pose.second, normalized), pose.first);
    }
    
    return scores;
}

ostream& operator<<(ostream& o, Poses const& g)
{
    o << g.poses_.size() << endl;
    
    for(auto const& gesture : g.poses_)
    {
        o << gesture.first << endl;
        
        for(auto const& finger : gesture.second)
        {
            for(auto const& joint : finger)
            {
                o << joint.x << joint.y << joint.z;
            }
        }
        
        o << endl;
    }
    
    return o;
}

istream& operator>>(istream& i, Poses& g)
{
    g.poses_.clear();
    
    size_t size;
    i >> size >> ws;
    
    string name;
    for(size_t j = 0; j != size; ++j)
    {
        i >> name >> ws;
        
        for(auto& finger : g.poses_[name])
        {
            for(auto& joint : finger)
            {
                i >> joint.x >> joint.y >> joint.z;
            }
        }

        i >> ws;
    }
    
    return i;
}

}}