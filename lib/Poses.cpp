#include "LeapLearnedGestures/detail/Poses.h"

#include "LeapLearnedGestures/Utility.h"

#include <LeapSDK/Leap.h>

#include <array>
#include <limits>
#include <map>
#include <string>

namespace LearnedGestures { namespace detail
{

using namespace std;

void Poses::capture(string const& name, Leap::Hand const& hand)
{
    poses_[name] = to_position(hand);
}

string Poses::match(Leap::Hand const& hand) const
{
    auto const positions = to_position(hand);
    
    float delta_cap = numeric_limits<float>::max(), delta;
    string name;
    
    for(auto const& pose : poses_)
    {
        if((delta = difference(pose.second, positions, delta_cap)) < delta_cap)
        {
            delta_cap = delta;
            name = pose.first;
        }
    }
    
    return name;
}

multimap<float, string> Poses::compare(Leap::Hand const& hand) const
{
    multimap<float, string> scores;
    
    auto const normalized = to_position(hand);
    
    for(auto const& pose : poses_)
    {
        scores.emplace(difference(pose.second, normalized), pose.first);
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