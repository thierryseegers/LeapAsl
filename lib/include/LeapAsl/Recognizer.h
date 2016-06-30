#pragma once

#include "LeapAsl/Lexicon.h"
#include "LeapAsl/Utility.h"

#include <LeapSDK/Leap.h>

#include <chrono>
#include <functional>
#include <map>
#include <string>

namespace LeapAsl
{

using namespace std::literals;
    
class Recognizer : public Leap::Listener
{
public:
    using on_gesture_f = std::function<void (std::multimap<double, std::string> const&)>;
    
    using duration = std::chrono::high_resolution_clock::duration;
    using time_point = std::chrono::high_resolution_clock::time_point;
    
    Recognizer(Lexicon const& Lexicon, on_gesture_f&& on_gesture, duration const& hold_duration = 1s, duration const& down_duration = 1s, duration const& sample_rate = 100ms);
    
private:
    virtual void onFrame(Leap::Controller const& controller) override;
    
    Lexicon const& Lexicon_;
    
    on_gesture_f const on_gesture_;
    
    duration const hold_duration_, down_duration_, sample_rate_;
    time_point anchor_sample_, next_sample_;
    
    fingers_position anchor_;
    std::map<std::string, double> scores_;
};
    
}