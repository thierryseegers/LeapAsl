#pragma once

#include "Utility.h"

#include "Trainer.h"

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
    
    Listener(Trainer const& trainer, duration const& hold_duration = std::chrono::milliseconds(1000), duration const& down_duration = std::chrono::milliseconds(1000), duration const& sample_rate = std::chrono::milliseconds(100));
    
    virtual void onGesture(std::map<double, std::string> const& matches);
    
private:
    virtual void onFrame(Leap::Controller const& controller) override;
    
    duration const hold_duration_, down_duration_, sample_rate_;
    time_point anchor_sample_, next_sample_;
    
    fingers_position anchor_;
    std::map<std::string, double> scores_;
    
    Trainer const& trainer_;
};
    
}