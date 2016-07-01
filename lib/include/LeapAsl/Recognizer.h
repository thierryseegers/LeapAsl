#pragma once

#include "LeapAsl/Lexicon.h"
#include "LeapAsl/Utility.h"

#include <LeapSDK/Leap.h>

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
    
class Recognizer : public Leap::Listener
{
public:
    using on_recognition_f = std::function<void (std::multimap<double, std::string> const&)>;
    
    using duration = std::chrono::high_resolution_clock::duration;
    using time_point = std::chrono::high_resolution_clock::time_point;
    
    Recognizer(Leap::Controller const& controller, Lexicon const& Lexicon, on_recognition_f&& on_recognition, duration const& hold_duration = 1s, duration const& down_duration = 1s, duration const& sample_rate = 100ms);
    
    ~Recognizer();
    
private:
    std::thread poller_;
    bool poll_;
    std::mutex m_;
    std::condition_variable cv_;
};
    
}