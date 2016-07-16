#include "LeapAsl/Recognizer.h"

#include "LeapAsl/Lexicon.h"

#include <LeapSDK/Leap.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

namespace LeapAsl
{

using namespace std;
    
Recognizer::Recognizer(Lexicon const& lexicon, on_recognition_f&& on_recognition, duration const& hold_duration, duration const& rest_duration)
    : lexicon_(lexicon)
    , on_recognition_(on_recognition)
    , hold_duration_(hold_duration)
    , rest_duration_(rest_duration)
    , next_sample_()
{}

Recognizer::~Recognizer()
{}

void Recognizer::onFrame(Leap::Controller const& controller)
{
   analyze(controller.frame(), chrono::high_resolution_clock::now());
}

void Recognizer::on_frame(RecordPlayer const& record_player)
{
    auto const frame = record_player.frame();
    analyze(frame, time_point{chrono::microseconds(frame.timestamp())});
}

void Recognizer::analyze(Leap::Frame const& frame, time_point const& now)
{    
    // If we are not scheduled to take a sample yet, return immediately.
    if(now < next_sample_)
    {
        return;
    }
    
    // Reject and reset if there is more than one hand in the frame. (A (temporary?) restriction.)
    if(frame.hands().count() != 1)
    {
        anchor_ = fingers_position();
        scores_.clear();
        
        return;
    }
    
    auto const& hand = frame.hands()[0];
    
    // Reject and reset if low-confidence.
    if(hand.confidence() < 0.2)
    {
        anchor_ = fingers_position();
        scores_.clear();
        
        return;
    }
    
    // If we have no anchor, the current position becomes the anchor by which we will compare future positions.
    // Else if the current position differs too much from the anchor, it becomes the new anchor.
    if(anchor_ == fingers_position())
    {
        anchor_ = to_position(hand);
        anchor_sample_ = now;
    }
    else if(difference(anchor_, to_position(hand)) > 1000)
    {
        anchor_ = to_position(hand);
        anchor_sample_ = now;
        
        scores_.clear();
    }
    
    // Analyze the one hand present.
    
    for(auto const& score : lexicon_.compare(hand))
    {
        scores_[score.second] += score.first;
    }
    
    // If we have past our \ref hold_duration, report what we have, reset the information and schedule the next sample analysis for \ref rest_duration_ later.
    // Else, just set the next sample analysis for as soon as possible.
    if(now - anchor_sample_ >= hold_duration_)
    {
        // Order gesture names by their scores.
        multimap<double, string> matches;
        for(auto const& score : scores_)
        {
            matches.emplace(score.second / 1000, score.first);
        }
        on_recognition_(matches);
        
        anchor_ = fingers_position();
        scores_.clear();
        
        next_sample_ = now + rest_duration_;
    }
    else
    {
        next_sample_ = now;
    }
}
    
}
