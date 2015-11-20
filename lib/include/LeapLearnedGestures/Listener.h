#pragma once

#include "Trainer.h"

#include "detail/Poses.h"

#include <LeapSDK/Leap.h>

#include <chrono>
#include <map>
#include <string>

namespace LearnedGestures
{

class Listener : public Leap::Listener
{
public:
    using duration = std::chrono::high_resolution_clock::duration;
    using time_point = std::chrono::high_resolution_clock::time_point;
    
    Listener(Trainer const& trainer, duration const& hold_time = std::chrono::milliseconds(1500), duration const& down_time = std::chrono::milliseconds(1000), duration const& sample_rate = std::chrono::milliseconds(100));
    
    virtual void onGesture(std::map<float, std::string> const& matches);
    
private:
    void onFrame(Leap::Controller const& controller);
    
    duration const hold_time_, down_time_, sample_rate_;
    time_point recorded_sample_, next_sample_;
    
    std::string top_gesture_;
    std::map<std::string, float> recorded_gestures_;
    
    detail::Poses poses_;
};
    
}