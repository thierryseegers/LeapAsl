#pragma once

#include "LeapLearnedGestures/Utility.h"

#include <LeapSDK/Leap.h>

#include <array>
#include <iosfwd>
#include <map>
#include <string>
#include <utility>

namespace LearnedGestures { namespace detail
{

class Gestures
{
public:
    // Add a pose to the set of known poses. (Overwrites if an existing name is given.)
    void capture(std::string const& name, Leap::Frame const& frame);
    
    // Returns highest scoring gesture.
    std::string match(Leap::Hand const& hand) const;
    
    // Returns sorted scores of all gestures.
    std::multimap<double, std::string> compare(Leap::Hand const& hand) const;
    
    // Returns the Leap::Hand object associated with the given name.
    Leap::Hand hand(std::string const& name) const;
    
private:
    friend std::ostream& operator<<(std::ostream&, Gestures const&);
    friend std::istream& operator>>(std::istream&, Gestures&);
    
    std::map<std::string, std::pair<Leap::Frame, fingers_position>> gestures_;
};

}}