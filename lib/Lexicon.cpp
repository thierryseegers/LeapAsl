#include "LeapAsl/Lexicon.h"

#include <LeapSDK/Leap.h>

#include <iostream>
#include <sstream>
#include <string>

namespace LeapAsl
{

using namespace std;

void Lexicon::capture(string const& name, Leap::Frame const& frame)
{
    added_gestures_.push_back(name);
    
    gestures_[name] = make_pair(frame, to_position(frame.hands()[0]));
}

string Lexicon::match(Leap::Hand const& hand) const
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

multimap<double, string> Lexicon::compare(Leap::Hand const& hand) const
{
    multimap<double, string> scores;
    
    auto const normalized = to_position(hand);
    
    for(auto const& gesture : gestures_)
    {
        scores.emplace(difference(gesture.second.second, normalized), gesture.first);
    }
    
    return scores;
}

Leap::Hand Lexicon::hand(string const& name) const
{
    return gestures_.at(name).first.hands()[0];
}

ostream& operator<<(ostream& o, Lexicon const& t)
{
    o << t.serialized_data_;
    
    for(auto const& name : t.added_gestures_)
    {
        auto const& gesture = t.gestures_.at(name);
        
        // Write the name.
        o << name << '\n';
        
        // Write the spatial positions of the joints.
        o << gesture.second;
        
        // Write the serialized frame data.
        auto const serialized_frame = gesture.first.serialize();
        o << serialized_frame.length() << '\n' << serialized_frame << '\n';
    }
    
    return o;
}

istream& operator>>(istream& i, Lexicon& t)
{
    Leap::Controller controller;
    
    // Copy the file data so we can write it verbatim later on.
    stringstream ss;
    ss << i.rdbuf();
    t.serialized_data_ = ss.str();
    
    // Read gestures data.
    t.gestures_.clear();
    while(ss)
    {
        // Read the name.
        string name;
        getline(ss, name);
        
        if(ss)
        {
            auto& data = t.gestures_[name];
            
            // Read the spatial positions of the joints.
            for(auto& finger : data.second)
            {
                for(auto& joint : finger)
                {
                    ss >> joint.x >> joint.y >> joint.z;
                }
            }
            
            // Read the serialized frame data.
            string::size_type serialized_length;
            ss >> serialized_length;
            ss.ignore();
            
            string serialized_frame(serialized_length, '\0');
            ss.read(&*serialized_frame.begin(), serialized_length);
            ss.ignore();
            
            data.first.deserialize(serialized_frame);
        }
    }
    
    return i;
}

}
