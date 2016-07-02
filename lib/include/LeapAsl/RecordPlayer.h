#pragma once

#include <LeapSDK/Leap.h>

#include <functional>
#include <iosfwd>
#include <utility>

namespace LeapAsl
{

class RecordPlayer
{
    std::istream& record_stream_;
    std::function<void ()> on_end_;
    
public:
    RecordPlayer(std::istream& i, std::function<void ()> on_end);
    
    Leap::Frame frame() const;
};

}
