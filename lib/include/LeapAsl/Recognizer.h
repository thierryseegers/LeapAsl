#pragma once

#include "LeapAsl/Lexicon.h"
#include "LeapAsl/Predictors.h"
#include "LeapAsl/RecordPlayer.h"

#include <LeapSDK/Leap.h>

#include <chrono>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace LeapAsl
{

using namespace std::literals;

template<class Predictor>
class Recognizer : public Leap::Listener, public RecordPlayerListener
{
public:
    using on_recognition_f = std::function<void (std::multimap<double, char> const&)>;
    
    using duration = std::chrono::high_resolution_clock::duration;
    using time_point = std::chrono::high_resolution_clock::time_point;
    
    Recognizer(std::ifstream&& predictor_input_stream, on_recognition_f&& on_recognition, duration const& hold_duration = 1s, duration const& rest_duration = 1s);
    
    Recognizer(std::string const& predictor_input_path, on_recognition_f&& on_recognition, duration const& hold_duration = 1s, duration const& rest_duration = 1s);

    ~Recognizer()
    {}
    
    virtual void onFrame(Leap::Controller const& controller) override
    {
        analyze(controller.frame(), std::chrono::high_resolution_clock::now());
    }
    
    virtual void on_frame(RecordPlayer const& record_player) override
    {
        auto const frame = record_player.frame();
        analyze(frame, time_point{std::chrono::microseconds(frame.timestamp())});
    }
    
private:
    void analyze(Leap::Frame const& frame, time_point const& now)
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
        
        fingers_position fp = to_position(hand);
        
        // If we have no anchor, the current position becomes the anchor by which we will compare future positions.
        // Else if the current position differs too much from the anchor, it becomes the new anchor.
        if(anchor_ == fingers_position())
        {
            anchor_ = fp;
            anchor_sample_ = now;
        }
        else if(difference(anchor_, fp) > 1000)
        {
            anchor_ = fp;
            anchor_sample_ = now;
            
            scores_.clear();
        }
        
        // Analyze the one hand present.
        
        
        for(auto const& score : predictor_->predict(hand))
        {
            scores_[score.second] += score.first;
        }
        
        // If we have past our \ref hold_duration, report what we have, reset the information and schedule the next sample analysis for \ref rest_duration_ later.
        // Else, just set the next sample analysis for as soon as possible.
        if(now - anchor_sample_ >= hold_duration_)
        {
            // Order gesture names by their scores.
            std::multimap<double, char> matches;
            for(auto const& score : scores_)
            {
                matches.emplace(score.second, score.first);
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

    
    std::unique_ptr<Predictor> predictor_;
    
    on_recognition_f const on_recognition_;
    duration const hold_duration_, rest_duration_;
    
    fingers_position anchor_ = fingers_position();
    std::map<char, double> scores_;
    Recognizer::time_point anchor_sample_, next_sample_;

};

template<>
Recognizer<Predictors::Lexicon>::Recognizer(std::ifstream&& predictor_input_stream, on_recognition_f&& on_recognition, duration const& hold_duration, duration const& rest_duration)
    : predictor_(new Predictors::Lexicon(std::forward<std::ifstream>(predictor_input_stream)))
    , on_recognition_(on_recognition)
    , hold_duration_(hold_duration)
    , rest_duration_(rest_duration)
    , next_sample_()
{}

template<>
Recognizer<Predictors::MlpackSoftmaxRegression>::Recognizer(std::string const& predictor_input_path, on_recognition_f&& on_recognition, duration const& hold_duration, duration const& rest_duration)
    : predictor_(new Predictors::MlpackSoftmaxRegression(predictor_input_path))
    , on_recognition_(on_recognition)
    , hold_duration_(hold_duration)
    , rest_duration_(rest_duration)
    , next_sample_()
{}

}
