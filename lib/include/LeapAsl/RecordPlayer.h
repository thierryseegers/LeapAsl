#pragma once

#include <LeapSDK/Leap.h>

#include <condition_variable>
#include <functional>
#include <iosfwd>
#include <mutex>
#include <utility>
#include <thread>
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
    bool read_ = false;
    
    std::istream& record_stream_;
    std::thread reader_;
    std::mutex m_;
    std::condition_variable cv_;
    
    std::vector<std::reference_wrapper<RecordPlayerListener>> listeners_;
    
    Leap::Frame last_frame_ = Leap::Frame::invalid();
    
public:
    RecordPlayer(std::istream& i);
    
    ~RecordPlayer();
    
    void add_listener(RecordPlayerListener& listener);
    
    // Returns the last frame reported.
    Leap::Frame frame() const;
    
    // Start reading stream.
    void start();
    
    // Blocking wait until the stream is completely read.
    void wait();
};

}
