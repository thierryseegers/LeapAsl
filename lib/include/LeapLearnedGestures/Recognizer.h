#pragma once

#include "Utility.h"

#include "Database.h"

#include <LeapSDK/Leap.h>

#include <chrono>
#include <functional>
#include <map>
#include <string>

namespace LearnedGestures
{

class Recognizer : public Leap::Listener
{
public:
    using on_gesture_f = std::function<void (std::map<double, std::string> const&)>;
    
    using duration = std::chrono::high_resolution_clock::duration;
    using time_point = std::chrono::high_resolution_clock::time_point;
    
    Recognizer(Leap::Controller& controller, Database const& database, on_gesture_f&& on_gesture, duration const& hold_duration = std::chrono::milliseconds(1000), duration const& down_duration = std::chrono::milliseconds(1000), duration const& sample_rate = std::chrono::milliseconds(100));
    
private:
    virtual void onFrame(Leap::Controller const& controller) override;
    
    Database const& database_;
    
    on_gesture_f const on_gesture_;
    
    duration const hold_duration_, down_duration_, sample_rate_;
    time_point anchor_sample_, next_sample_;
    
    fingers_position anchor_;
    std::map<std::string, double> scores_;
};
    
}