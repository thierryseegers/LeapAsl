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
    
Recognizer::Recognizer(Leap::Controller const& controller, Lexicon const& lexicon, on_recognition_f&& on_recognition, duration const& hold_duration, duration const& down_duration, duration const& sample_rate)
    : poll_(true)
{
    poller_ = thread([=, &controller, &lexicon]()
                     {
                         fingers_position anchor;
                         map<string, double> scores;

                         duration wait_time = sample_rate;
                         time_point anchor_sample;
                         
                         unique_lock<mutex> l(m_);
                         while(!cv_.wait_for(l, wait_time, [=, &controller, &lexicon]{ return !poll_; }))
                         {
                             auto const now = chrono::high_resolution_clock::now();

                             // Reject and reset if there is more than one hand in the frame. (A (temporary?) restriction.)
                             if(controller.frame().hands().count() != 1)
                             {
                                 anchor = fingers_position();
                                 scores.clear();
                                 
                                 wait_time = sample_rate;
                                 continue;
                             }

                             auto const& hand = controller.frame().hands()[0];

                             // Reject and reset if low-confidence.
                             if(hand.confidence() < 0.2)
                             {
                                 anchor = fingers_position();
                                 scores.clear();
                                 
                                 wait_time = sample_rate;
                                 continue;
                             }

                             // If we have no anchor, the current position becomes the anchor by which we will compare future positions.
                             // Else if the current position differs too much from the anchor, it becomes the new anchor.
                             if(anchor == fingers_position())
                             {
                                 anchor = to_position(hand);
                                 anchor_sample = now;
                             }
                             else if(difference(anchor, to_position(hand)) > 1000)
                             {
                                 anchor = to_position(hand);
                                 anchor_sample = now;

                                 scores.clear();
                             }

                             // Analyze the one hand present.

                             for(auto const& score : lexicon.compare(hand))
                             {
                                 scores[score.second] += score.first;
                             }

                             // If we have past our \ref hold_duration, report what we have, reset the information and schedule the next sample analysis for \ref down_duration later.
                             // Else, just set the next sample analysis for \ref sample_rate later.
                             if(now - anchor_sample >= hold_duration)
                             {
                                 // Order gesture names by their scores.
                                 multimap<double, string> matches;
                                 for(auto const& score : scores)
                                 {
                                     matches.emplace(score.second / 1000, score.first);
                                 }
                                 on_recognition(matches);
                                 
                                 anchor = fingers_position();
                                 scores.clear();
                                 
                                 wait_time = down_duration;
                             }
                             else
                             {
                                 wait_time = sample_rate;
                             }
                         }
                     });
}

Recognizer::~Recognizer()
{
    {
        lock_guard<mutex> l(m_);
        poll_ = false;
        cv_.notify_one();
    }
    poller_.join();
}

}
