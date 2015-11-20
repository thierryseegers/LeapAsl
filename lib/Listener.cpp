#include "LeapLearnedGestures/Listener.h"

#include "LeapLearnedGestures/Trainer.h"
#include "LeapLearnedGestures/detail/Poses.h"

#include <LeapSDK/Leap.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <string>
#include <utility>

namespace LearnedGestures
{

using namespace std;
    
Listener::Listener(Trainer const& trainer, duration const& hold_time, duration const& down_time, duration const& sample_rate)
    : hold_time_(hold_time)
    , down_time_(down_time)
    , sample_rate_(sample_rate)
    , next_sample_(chrono::high_resolution_clock::now())
    , poses_(trainer.poses_)
{}

void Listener::onGesture(map<float, string> const& matches)
{}
    
void Listener::onFrame(Leap::Controller const& controller)
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
        top_gesture_.clear();
        recorded_gestures_.clear();
        
        recorded_sample_ = now;
        
        return;
    }
    
    auto const& hand = controller.frame().hands()[0];
    
    // Reject and reset if low-confidence.
    if(hand.confidence() < 0.2)
    {
        top_gesture_.clear();
        recorded_gestures_.clear();
        
        recorded_sample_ = now;
        
        return;
    }
    
    // Analyze the one hand present.
    auto const scores = poses_.compare(hand);
    for(auto const& score : scores)
    {
        recorded_gestures_[score.second] += score.first;
    }
    
    // If the top current gesture is different from the top recorded one, replace the information we have and start from scratch.
    // Else, if the current gesture is the same as the recorded one and we have past our \ref hold_time, report what we have and reset the information.
    // Else, do nothing and we will keep analyzing new samples.
    if(top_gesture_ != scores.begin()->second)
    {
        top_gesture_ = scores.begin()->second;
        
        recorded_gestures_.clear();
        recorded_sample_ = now;
    }
    else if(now - recorded_sample_ >= hold_time_)
    {
        map<float, string> matches;
        for(auto const& recorded_search : recorded_gestures_)
        {
            matches[recorded_search.second] = recorded_search.first;
        }
        onGesture(matches);
        
        top_gesture_.clear();
        recorded_gestures_.clear();
        next_sample_ = now + down_time_;
    }
}
    
}