#pragma once

#include "detail/Poses.h"

#include <LeapSDK/Leap.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <string>
#include <utility>

namespace LearnedGestures
{

class LearnedGesture : public Leap::Gesture
{
public:
    enum Type
    {
        TYPE_INVALID    = Leap::Gesture::TYPE_INVALID,
        TYPE_POSE,
        TYPE_GESTURE,
        TYPE_ALL        = TYPE_POSE | TYPE_GESTURE
    };

    LearnedGesture(Type const type = TYPE_INVALID, std::string const& name = std::string())
    : type_(type)
    , name_(name)
    {}
    
    Type type() const
    {
        return type_;
    }
    
    std::string name() const
    {
        return name_;
    }
    
private:
    Type const type_;
    
    std::string name_;
};

class Listener;

class Trainer
{
public:
    void capture(std::string const& name, Leap::Hand const& hand)
    {
        poses_.capture(name, hand);
    }
    
private:
    friend Listener;
    
    friend std::ostream& operator<<(std::ostream&, Trainer const&);
    friend std::istream& operator>>(std::istream&, Trainer&);
    
    detail::Poses poses_;
};

std::ostream& operator<<(std::ostream& o, Trainer const& t)
{
    return o << t.poses_;
}

std::istream& operator>>(std::istream& i, Trainer& t)
{
    return i >> t.poses_;
}
    
class Listener : public Leap::Listener
{
public:
    using duration = std::chrono::high_resolution_clock::duration;
    using time_point = std::chrono::high_resolution_clock::time_point;

    Listener(Trainer const& trainer, int const types = LearnedGesture::TYPE_ALL, duration const& hold_time = std::chrono::milliseconds(2000), duration const& sample_rate = std::chrono::milliseconds(100))
    : hold_time(hold_time)
    , sample_rate(sample_rate)
    , next_sample(std::chrono::high_resolution_clock::now())
    , poses_(trainer.poses_)
    {}
    
    virtual void onGesture(LearnedGesture const&)
    {}

    virtual void onGesture(std::array<std::pair<float, std::string>, 3> const& matches)
    {}

private:
    virtual void onFrame(Leap::Controller const& controller)
    {
        auto const now = std::chrono::high_resolution_clock::now();
        
        // If we're not scheduled to take a sample yet, return immediately.
        if(now < next_sample)
        {
            return;
        }
        
        // There must be a single hand in the frame.
        if(controller.frame().hands().count() != 1)
        {
            return;
        }
        
        // Analyze the one hand present.
        //std::string const gesture = poses_.match(controller.frame().hands()[0]);
        auto const& searches_ = poses_.search(controller.frame().hands()[0]);
        
        
        // If the current gesture is invalid or it is different from the recorded one, replace the information we have and start from scratch.
        // Else, if the current gesture is the same as the recorded one and we have past our \ref hold_time, report what we have and reset the information.
        // Else, do nothing and we will keep analyzing new samples.
        if(//!gesture.valid() ||
           //gesture != recorded_gesture)
           //top_searches != top_searches_)
           searches_.begin()->second != recorded_searches_[0].second &&
           (++searches_.begin())->second != recorded_searches_[1].second &&
           (++++searches_.begin())->second != recorded_searches_[2].second)
        {
            //recorded_gesture = gesture;
            std::copy(searches_.begin(), ++++++searches_.begin(), recorded_searches_.begin());
            recorded_sample = now;
        }
        else if(now - recorded_sample >= hold_time)
        {
            //onGesture(LearnedGesture(LearnedGesture::TYPE_POSE, gesture));
            //onGesture(LearnedGesture(LearnedGesture::TYPE_POSE, top_searches_[0]));
            recorded_searches_[0].first /= 100;
            recorded_searches_[1].first /= 100;
            recorded_searches_[2].first /= 100;
            
            onGesture(recorded_searches_);
            
            recorded_gesture.clear();// = gesture;
            recorded_sample = now;
        }
        else
        {
            recorded_searches_[0].first += searches_.begin()->first;
            recorded_searches_[1].first += (++searches_.begin())->first;
            recorded_searches_[2].first += (++++searches_.begin())->first;
        }
    }
    
    duration const hold_time, sample_rate;
    time_point next_sample;
    
    std::string recorded_gesture;
    time_point recorded_sample;
    
    std::array<std::pair<float, std::string>, 3> recorded_searches_;
    
    detail::Poses poses_;
};

}