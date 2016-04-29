#include "LeapLearnedGestures/Recognizer.h"

#include "LeapLearnedGestures/Database.h"

#include <LeapSDK/Leap.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <string>
#include <utility>

namespace LearnedGestures
{

using namespace std;
    
Recognizer::Recognizer(Leap::Controller& controller, Database const& database, on_gesture_f&& on_gesture, duration const& hold_duration, duration const& down_duration, duration const& sample_rate)
    : database_(database)
    , on_gesture_(on_gesture)
    , hold_duration_(hold_duration)
    , down_duration_(down_duration)
    , sample_rate_(sample_rate)
    , next_sample_(chrono::high_resolution_clock::now())
{
    controller.addListener(*this);
}

void Recognizer::onFrame(Leap::Controller const& controller)
{
    auto const now = chrono::high_resolution_clock::now();
    
    // If we're not scheduled to take a sample yet, return immediately.
    if(now < next_sample_)
    {
        return;
    }
    
    // Reject and reset if there is more than one hand in the frame. (A (temporary?) restriction.)
    if(controller.frame().hands().count() != 1)
    {
        anchor_ = fingers_position();
        scores_.clear();
        
        next_sample_ = now + sample_rate_;
        
        return;
    }
    
    auto const& hand = controller.frame().hands()[0];
    
    // Reject and reset if low-confidence.
    if(hand.confidence() < 0.2)
    {
        anchor_ = fingers_position();
        scores_.clear();
        
        next_sample_ = now + sample_rate_;
        
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

    auto const scores = database_.compare(hand);
    for(auto const& score : scores)
    {
        scores_[score.second] += score.first;
    }
    
    
    // If we have past our \ref hold_duration_, report what we have, reset the information and schedule the next sample analysis for \ref down_duration_ later.
    // Else, just set the next sample analysis for \ref sample_rate_ later.
    if(now - anchor_sample_ >= hold_duration_)
    {
        // Order gesture names by their scores.
        multimap<double, string> matches;
        for(auto const& score : scores_)
        {
            matches.emplace(score.second / 1000, score.first);
        }
        on_gesture_(matches);
        
        anchor_ = fingers_position();
        scores_.clear();
        
        next_sample_ = now + down_duration_;
    }
    else
    {
        next_sample_ = now + sample_rate_;
    }
}
    
}