#include "detail/Poses.h"

#include "detail/Pose.h"

#include <LeapSDK/Leap.h>

#include <limits>
#include <string>

namespace LearnedGestures { namespace detail
{

using namespace std;

void Poses::capture(string const& name, Leap::Hand const& hand)
{
    auto& gesture = poses_[name];
    gesture.name() = name;
    gesture.capture(Pose::normalized(hand));
}

string Poses::match(Leap::Hand const& hand) const
{
    auto const& normalized = Pose::normalized(hand);
    
    float minimum_error = numeric_limits<float>::max(), error;
    string name;
    
    for(auto const& gesture : poses_)
    {
        if((error = gesture.second.match(normalized, minimum_error)) < minimum_error)
        {
            minimum_error = error;
            name = gesture.second.name();
        }
    }
    
    return name;
}

map<float, string> Poses::search(Leap::Hand const& hand) const
{
    map<float, string> scores;
    
    auto const& normalized = Pose::normalized(hand);
    
    for(auto const& gesture : poses_)
    {
        scores[gesture.second.match(normalized)] = gesture.first;
    }
    
    return scores;
}

ostream& operator<<(ostream& o, Poses const& g)
{
    o << g.poses_.size() << endl;
    
    for(auto const& gesture : g.poses_)
    {
        o << gesture.second << endl;
    }
    
    return o;
}

istream& operator>>(istream& i, Poses& g)
{
    g.poses_.clear();
    
    size_t size;
    i >> size;
    i.ignore();
    
    for(size_t j = 0; j != size; ++j)
    {
        Pose s;
        i >> s;
        i.ignore();
     
        g.poses_[s.name()] = s;
    }
    
    return i;
}

}}