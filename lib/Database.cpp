#include "include/LeapLearnedGestures/Database.h"

#include <LeapSDK/Leap.h>

#include <iostream>
#include <mutex>
#include <string>

namespace LearnedGestures
{

using namespace std;

void Database::capture(string const& name, Leap::Frame const& frame)
{
    lock_guard<mutex> l(m_);
    
    gestures_[name] = make_pair(frame, to_position(frame.hands()[0]));
}

string Database::match(Leap::Hand const& hand) const
{
    auto const positions = to_position(hand);
    
    double delta_cap = numeric_limits<double>::max(), delta;
    string name;
    
    
    lock_guard<mutex> l(m_);
    
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

multimap<double, string> Database::compare(Leap::Hand const& hand) const
{
    multimap<double, string> scores;
    
    auto const normalized = to_position(hand);
    
    
    lock_guard<mutex> l(m_);
    
    for(auto const& gesture : gestures_)
    {
        scores.emplace(difference(gesture.second.second, normalized), gesture.first);
    }
    
    return scores;
}

Leap::Hand Database::hand(string const& name) const
{
    lock_guard<mutex> l(m_);
    
    return gestures_.at(name).first.hands()[0];
}

ostream& operator<<(ostream& o, Database const& t)
{
    o << t.gestures_.size() << '\n';
    
    for(auto const& gesture : t.gestures_)
    {
        o << gesture.first << '\n';
        
        for(auto const& finger : gesture.second.second)
        {
            for(auto const& joint : finger)
            {
                o << joint.x << '\n' << joint.y << '\n' << joint.z << '\n';
            }
        }
        
        auto const serialized_frame = gesture.second.first.serialize();
        
        o << serialized_frame.length() << '\n' << serialized_frame << '\n';
    }
    
    return o;
}

istream& operator>>(istream& i, Database& t)
{
    Leap::Controller controller;
    
    t.gestures_.clear();
    
    size_t gesture_count;
    i >> gesture_count;
    i.ignore();
    
    string name;
    for(size_t j = 0; j != gesture_count; ++j)
    {
        getline(i, name);
        
        auto& data = t.gestures_[name];
        
        for(auto& finger : data.second)
        {
            for(auto& joint : finger)
            {
                i >> joint.x >> joint.y >> joint.z;
            }
        }
        
        string::size_type serialized_length;
        i >> serialized_length >> ws;
        
        string serialized_frame(serialized_length, '\0');
        i.read(&*serialized_frame.begin(), serialized_length) >> ws;
        
        data.first.deserialize(serialized_frame);
    }
    
    return i;
}

}
