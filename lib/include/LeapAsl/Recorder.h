#pragma once

#include <LeapSDK/Leap.h>

#include <iosfwd>

namespace LeapAsl
{
    
class Recorder : public Leap::Listener
{
    std::ostream& output_stream;
    
public:
    Recorder(std::ostream& o) : output_stream(o)
    {}
    
    virtual void onFrame(Leap::Controller const& controller) override;
};

}
