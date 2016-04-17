#include "LeapLearnedGestures/detail/Poses.h"

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

void Poses::capture(string const& name, Leap::Frame const& frame)
{
    auto const p = to_position(frame.hands()[0]);
    
    poses_[name] = make_pair(frame, to_position(frame.hands()[0]));
}

string Poses::match(Leap::Hand const& hand) const
{
    auto const positions = to_position(hand);
    
    double delta_cap = numeric_limits<double>::max(), delta;
    string name;
    
    for(auto const& pose : poses_)
    {
        if((delta = difference(pose.second.second, positions, delta_cap)) < delta_cap)
        {
            delta_cap = delta;
            name = pose.first;
        }
    }
    
    return name;
}

multimap<double, string> Poses::compare(Leap::Hand const& hand) const
{
    multimap<double, string> scores;
    
    auto const normalized = to_position(hand);
    
    for(auto const& pose : poses_)
    {
        scores.emplace(difference(pose.second.second, normalized), pose.first);
    }
    
    return scores;
}

Leap::Hand Poses::hand(string const& name) const
{
    return poses_.at(name).first.hands()[0];
}

ostream& operator<<(ostream& o, Poses const& g)
{
    o << g.poses_.size() << endl;
    
    for(auto const& gesture : g.poses_)
    {
        o << gesture.first << endl;
        
        for(auto const& finger : gesture.second.second)
        {
            for(auto const& joint : finger)
            {
                o << joint.x << '\n' << joint.y << '\n' << joint.z << '\n';
            }
        }
        
        ofstream frame_stream(gesture.first + ".frame.data");
        auto const s = gesture.second.first.serialize();
        
        frame_stream << s;
        
        o << endl;
    }
    
    return o;
}

istream& operator>>(istream& i, Poses& g)
{
    Leap::Controller controller;
    
    g.poses_.clear();
    
    size_t size;
    i >> size;
    i.ignore();
    
    string name;
    for(size_t j = 0; j != size; ++j)
    {
        getline(i, name);
        
        auto& data = g.poses_[name];
        
        for(auto& finger : data.second)
        {
            for(auto& joint : finger)
            {
                i >> joint.x >> joint.y >> joint.z;
            }
        }
        /*
        string::size_type serialized_length;
        i >> serialized_length;
        i.ignore();
        
        /*
        string serialized_frame(serialized_length, '\0');
        i.read(&*serialized_frame.begin(), serialized_length);
        
        data.first.deserialize(serialized_frame);
        
        auto b = data.first.isValid();
        */

        ifstream frame_stream(name + ".frame.data");
        string const serialized_frame{istreambuf_iterator<char>(frame_stream), istreambuf_iterator<char>()};

        auto const l = serialized_frame.length();
        
        data.first.deserialize(serialized_frame);
        
        auto const p = to_position(data.first.hands()[0]);
        
        i >> ws;
    }
    
    return i;
}

}}