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
    
    Recognizer::duration analyze(Leap::Frame const& frame, Recognizer::time_point const& now, Lexicon const& lexicon, Recognizer::on_recognition_f const& on_recognition, Recognizer::duration const& sample_rate, Recognizer::duration const& hold_duration, Recognizer::duration const& down_duration)
{
    static fingers_position anchor;
    static map<string, double> scores;
    static Recognizer::time_point anchor_sample;

    // Reject and reset if there is more than one hand in the frame. (A (temporary?) restriction.)
    if(frame.hands().count() != 1)
    {
        anchor = fingers_position();
        scores.clear();
        
        return sample_rate;
    }
    
    auto const& hand = frame.hands()[0];
    
    // Reject and reset if low-confidence.
    if(hand.confidence() < 0.2)
    {
        anchor = fingers_position();
        scores.clear();
        
        return sample_rate;
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
        
        return down_duration;
    }
    else
    {
        return sample_rate;
    }
}

Recognizer::Recognizer(Leap::Controller const& controller, Lexicon const& lexicon, on_recognition_f&& on_recognition, duration const& sample_rate, duration const& hold_duration, duration const& down_duration)
    : read_(true)
{
    reader_ = thread([=, &controller, &lexicon]()
                     {
                         duration wait_time = sample_rate;
                         
                         unique_lock<mutex> l(m_);
                         while(!cv_.wait_for(l, wait_time, [=, &controller, &lexicon]{ return !read_; }))
                         {
                             wait_time = analyze(controller.frame(), chrono::high_resolution_clock::now(), lexicon, on_recognition, sample_rate, hold_duration, down_duration);
                         }
                     });
}

Recognizer::Recognizer(RecordPlayer const& player, Lexicon const& lexicon, on_recognition_f&& on_recognition, duration const& sample_rate, duration const& hold_duration, duration const& down_duration)
    : read_(true)
{
    reader_ = thread([=, &player, &lexicon]()
                     {
                         int64_t last_timestamp = player.frame().timestamp();

                         duration wait_time = sample_rate, elapsed_time;
                         time_point now{chrono::microseconds(last_timestamp)};

                         for(auto frame = player.frame(); read_ && frame.isValid(); frame = player.frame())
                         {
                             auto const frame_duration = chrono::microseconds(frame.timestamp() - last_timestamp);
                             last_timestamp = frame.timestamp();

                             elapsed_time += frame_duration;
                             now += frame_duration;
                             
                             if(elapsed_time >= wait_time)
                             {
                                 wait_time = analyze(frame, now, lexicon, on_recognition, sample_rate, hold_duration, down_duration);
                                 elapsed_time = duration::zero();
                             }
                         }
                     });
}

Recognizer::~Recognizer()
{
    {
        lock_guard<mutex> l(m_);
        read_ = false;
        cv_.notify_one();
    }
    reader_.join();
}

}
