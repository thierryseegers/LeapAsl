#include "LeapLearnedGestures/detail/Gestures.h"

#include "LeapLearnedGestures/Utility.h"

#include <LeapSDK/Leap.h>

#include <array>
#include <fstream>
#include <limits>
#include <map>
#include <string>

namespace LearnedGestures { namespace detail
{

using namespace std;

void Gestures::capture(string const& name, Leap::Frame const& frame)
{
    gestures_[name] = make_pair(frame, to_position(frame.hands()[0]));
}

string Gestures::match(Leap::Hand const& hand) const
{
    auto const positions = to_position(hand);
    
    double delta_cap = numeric_limits<double>::max(), delta;
    string name;
    
    for(auto const& gesture : gestures_)
    {
        if((delta = difference(gesture.second.second, positions, delta_cap)) < delta_cap)
        {
            delta_cap = delta;
            name = gesture.first;
        }
    }
    
    return name;
}

multimap<double, string> Gestures::compare(Leap::Hand const& hand) const
{
    multimap<double, string> scores;
    
    auto const normalized = to_position(hand);
    
    for(auto const& gesture : gestures_)
    {
        scores.emplace(difference(gesture.second.second, normalized), gesture.first);
    }
    
    return scores;
}

Leap::Hand Gestures::hand(string const& name) const
{
    return gestures_.at(name).first.hands()[0];
}

ostream& operator<<(ostream& o, Gestures const& g)
{
    o << g.gestures_.size() << endl;
    
    for(auto const& gesture : g.gestures_)
    {
        o << gesture.first << endl;
        
        for(auto const& finger : gesture.second.second)
        {
            for(auto const& joint : finger)
            {
                o << joint.x << '\n' << joint.y << '\n' << joint.z << '\n';
            }
        }
        
        auto const serialized_frame = gesture.second.first.serialize();

        o << serialized_frame.length() << endl << serialized_frame << endl;
    }
    
    return o;
}

istream& operator>>(istream& i, Gestures& g)
{
    Leap::Controller controller;
    
    g.gestures_.clear();
    
    size_t gesture_count;
    i >> gesture_count;
    i.ignore();
    
    string name;
    for(size_t j = 0; j != gesture_count; ++j)
    {
        getline(i, name);
        
        auto& data = g.gestures_[name];
        
        for(auto& finger : data.second)
        {
            for(auto& joint : finger)
            {
                i >> joint.x >> joint.y >> joint.z;
            }
        }
        
        string::size_type serialized_length;
        i >> serialized_length;
        i.ignore();
        
        string serialized_frame(serialized_length, '\0');
        i.read(&*serialized_frame.begin(), serialized_length);
        
        data.first.deserialize(serialized_frame);
        
        i.ignore();
    }
    
    return i;
}

}}