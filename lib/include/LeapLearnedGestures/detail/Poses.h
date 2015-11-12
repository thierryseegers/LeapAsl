#pragma once

#include "Pose.h"

#include <iosfwd>
#include <map>
#include <string>

namespace LearnedGestures { namespace detail
{

class Poses
{
public:
    void capture(std::string const& name, Leap::Hand const& hand);
    
    // Returns highest scoring gesture.
    std::string match(Leap::Hand const& hand) const;
    
    // Returns sorted scores of all gestures.
    std::map<float, std::string> search(Leap::Hand const& hand) const;
    
private:
    friend std::ostream& operator<<(std::ostream&, Poses const&);
    friend std::istream& operator>>(std::istream&, Poses&);
    
    std::map<std::string, Pose> poses_;
};

}}