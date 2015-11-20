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
    
Listener::Listener(Trainer const& trainer, duration const& hold_duration, duration const& down_duration, duration const& sample_rate)
    : hold_duration_(hold_duration)
    , down_duration_(down_duration)
    , sample_rate_(sample_rate)
    , next_sample_(chrono::high_resolution_clock::now())
    , poses_(trainer.poses_)
{}

void Listener::onGesture(map<double, string> const& matches)
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

    auto const scores = poses_.compare(hand);
    for(auto const& score : scores)
    {
        scores_[score.second] += score.first;
    }
    
    // If the top current gesture is different from the top recorded one, replace the information we have and start from scratch.
    // Else, if the current gesture is the same as the recorded one and we have past our \ref hold_time, report what we have and reset the information.
    // Else, do nothing and we will keep analyzing new samples.
    if(now - anchor_sample_ >= hold_duration_)
    {
        // Order gesture names by overall scores.
        map<double, string> matches;
        for(auto const& score : scores_)
        {
            matches[score.second] = score.first;
        }
        onGesture(matches);
        
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