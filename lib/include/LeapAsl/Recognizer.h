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
    
class Recognizer
{
public:
    using on_recognition_f = std::function<void (std::multimap<double, std::string> const&)>;
    
    using duration = std::chrono::high_resolution_clock::duration;
    using time_point = std::chrono::high_resolution_clock::time_point;
    
    Recognizer(Leap::Controller const& controller, Lexicon const& lexicon, on_recognition_f&& on_recognition, duration const& sample_rate = 100ms, duration const& hold_duration = 1s, duration const& down_duration = 1s);
    
    Recognizer(RecordPlayer const& player, Lexicon const& lexicon, on_recognition_f&& on_recognition, duration const& sample_rate = 100ms, duration const& hold_duration = 1s, duration const& down_duration = 1s);

    ~Recognizer();
    
private:
    std::thread reader_;
    std::atomic<bool> read_;
    std::mutex m_;
    std::condition_variable cv_;
};
    
}