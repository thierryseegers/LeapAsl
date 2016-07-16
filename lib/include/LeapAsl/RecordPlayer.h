#pragma once

#include <LeapSDK/Leap.h>

#include <iosfwd>
#include <utility>
#include <vector>

namespace LeapAsl
{

class RecordPlayer;
    
class RecordPlayerListener
{
public:
    virtual void on_frame(RecordPlayer const&)
    {}
};

class RecordPlayer
{
    std::istream& record_stream_;
    
    std::vector<std::reference_wrapper<RecordPlayerListener>> listeners_;
    
    Leap::Frame last_frame_ = Leap::Frame::invalid();
    
public:
    RecordPlayer(std::istream& i);
    
    void add_listener(RecordPlayerListener& listener);
    
    // Blocks until the stream is completely read.
    void read();

    // Returns the last frame reported.
    Leap::Frame frame() const;
};

}
