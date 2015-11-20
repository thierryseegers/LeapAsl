#pragma once

#include "LeapLearnedGestures/Utility.h"

#include <LeapSDK/Leap.h>

#include <array>
#include <iosfwd>
#include <map>
#include <string>

namespace LearnedGestures { namespace detail
{

class Poses
{
public:
    // Add a pose to the set of known poses. (Overwrites if an existing name is given.)
    void capture(std::string const& name, Leap::Hand const& hand);
    
    // Returns highest scoring gesture.
    std::string match(Leap::Hand const& hand) const;
    
    // Returns sorted scores of all gestures.
    std::multimap<double, std::string> compare(Leap::Hand const& hand) const;
    
private:
    friend std::ostream& operator<<(std::ostream&, Poses const&);
    friend std::istream& operator>>(std::istream&, Poses&);
    
    std::map<std::string, fingers_position> poses_;
};

}}