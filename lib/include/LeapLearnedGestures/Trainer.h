#pragma once

#include "detail/Poses.h"

#include <LeapSDK/Leap.h>

#include <iosfwd>
#include <string>

namespace LearnedGestures
{
    
class Listener;
    
class Trainer
{
public:
    void capture(std::string const& name, Leap::Hand const& hand);
    
private:
    friend Listener;
    
    friend std::ostream& operator<<(std::ostream&, Trainer const&);
    friend std::istream& operator>>(std::istream&, Trainer&);
    
    detail::Poses poses_;
};

}