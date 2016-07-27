#include "LeapAsl/Lexicon.h"

#include <LeapSDK/Leap.h>

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace LeapAsl
{

using namespace std;

void Lexicon::capture(string const& name, Leap::Frame const& frame)
{
    auto const serialized = frame.serialize();
    serialized_data_ += name + '\n' + to_string(serialized.length()) + '\n' + serialized + '\n';
    
    auto const hand = frame.hands()[0];
    
    gestures_.emplace(name, make_pair(hand, to_position(hand)));
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
    map<string, set<double>> multiscores;
    
    for(auto const& gesture : gestures_)
    {
        multiscores[gesture.first].insert(difference(gesture.second.second, normalized));
    }
    
    for(auto const& multiscore : multiscores)
    {
        scores.emplace(*multiscore.second.begin(), multiscore.first);
    }
    
    return scores;
}

vector<Leap::Hand> Lexicon::hands(string const& name) const
{
    vector<Leap::Hand> hands;

    auto const range = gestures_.equal_range(name);
    transform(range.first, range.second, back_inserter(hands), [](auto const& gesture){ return gesture.second.first; });
    
    return hands;
}

ostream& operator<<(ostream& o, Lexicon const& t)
{
    return o << t.serialized_data_;
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
    Leap::Frame frame;
    while(ss)
    {
        // Read the name.
        string name;
        getline(ss, name);
        
        if(ss)
        {
            // Read the serialized frame data.
            string::size_type serialized_length;
            ss >> serialized_length;
            ss.ignore();
            
            string serialized_frame(serialized_length, '\0');
            ss.read(&*serialized_frame.begin(), serialized_length);
            ss.ignore();
            
            //data.first.deserialize(serialized_frame);
            frame.deserialize(serialized_frame);
            
            auto const hand = frame.hands()[0];
            t.gestures_.emplace(name, make_pair(hand, to_position(hand)));
        }
    }
    
    return i;
}

}
