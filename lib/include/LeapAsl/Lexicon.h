#pragma once

#include "LeapAsl/Utility.h"

#include <LeapSDK/Leap.h>

#include <map>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

namespace LeapAsl
{

class Lexicon
{
public:
    // Add a pose to the set of known poses.
    void capture(char const name, Leap::Frame const& frame);
    
    // Returns highest scoring gesture.
    char match(Leap::Hand const& hand) const;
    
    // Returns sorted scores of all gestures.
    std::multimap<double, char> compare(Leap::Hand const& hand) const;
    
    // Returns unique names of all gestures.
    std::vector<char> names() const;
    
    // Returns the Leap::Hand objects associated with the given name.
    std::vector<Leap::Hand> hands(char const name) const;
    
private:
    friend std::ostream& operator<<(std::ostream&, Lexicon const&);
    friend std::istream& operator>>(std::istream&, Lexicon&);
    
    std::multimap<char, std::pair<Leap::Hand, bones_directions>> gestures_;
    
    std::string serialized_data_;
};

}