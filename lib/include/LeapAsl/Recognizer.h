#pragma once

#include "LeapAsl/Lexicon.h"
#include "LeapAsl/RecordPlayer.h"

#include <LeapSDK/Leap.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>

namespace LeapAsl
{

using namespace std::literals;
    
class Recognizer : public Leap::Listener, public RecordPlayerListener
{
public:
    using on_recognition_f = std::function<void (std::multimap<double, std::string> const&)>;
    
    using duration = std::chrono::high_resolution_clock::duration;
    using time_point = std::chrono::high_resolution_clock::time_point;
    
    Recognizer(Lexicon const& lexicon, on_recognition_f&& on_recognition, duration const& hold_duration = 1s, duration const& rest_duration = 1s);

    ~Recognizer();
    
    virtual void onFrame(Leap::Controller const&) override;
    
    virtual void on_frame(RecordPlayer const&) override;
    
private:
    void analyze(Leap::Frame const& frame, time_point const& now);
    
    Lexicon const& lexicon_;
    on_recognition_f const on_recognition_;
    duration const hold_duration_, rest_duration_;
    
    fingers_position anchor_ = fingers_position();
    std::map<std::string, double> scores_;
    Recognizer::time_point anchor_sample_, next_sample_;

};
    
}