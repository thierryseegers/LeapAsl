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
    void capture(std::string const& name, Leap::Frame const& frame);
    
    // Returns highest scoring gesture.
    std::string match(Leap::Hand const& hand) const;
    
    // Returns sorted scores of all gestures.
    std::multimap<double, std::string> compare(Leap::Hand const& hand) const;
    
    // Returns the Leap::Hand objects associated with the given name.
    std::vector<Leap::Hand> hands(std::string const& name) const;
    
private:
    friend std::ostream& operator<<(std::ostream&, Lexicon const&);
    friend std::istream& operator>>(std::istream&, Lexicon&);
    
    std::multimap<std::string, std::pair<Leap::Hand, fingers_position>> gestures_;
    
    std::string serialized_data_;
};

}