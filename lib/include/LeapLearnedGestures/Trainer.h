#pragma once

#include "Utility.h"

#include "detail/Gestures.h"

#include <LeapSDK/Leap.h>

#include <iosfwd>
#include <string>

namespace LearnedGestures
{

class Listener;
    
class Trainer
{
public:
    void capture(std::string const& name, Leap::Frame const& frame);
    
    Leap::Hand hand(std::string const& name) const;
    
private:
    friend Listener;
    
    friend std::ostream& operator<<(std::ostream&, Trainer const&);
    friend std::istream& operator>>(std::istream&, Trainer&);
    
    detail::Gestures gestures_;
};

}