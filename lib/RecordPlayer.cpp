#include "LeapAsl/RecordPlayer.h"

#include <LeapSDK/Leap.h>

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace LeapAsl
{

using namespace std;

RecordPlayer::RecordPlayer(istream& i)
    : record_stream_(i)
{
    reader_ = thread([this]
    {
        {
            unique_lock<mutex> ul(m_);
            cv_.wait(ul, [this]{ return read_; });
        }
        
        string::size_type serialized_length;

        while(record_stream_)
        {
            record_stream_ >> serialized_length;
            record_stream_.ignore();

            if(record_stream_)
            {
                string serialized_frame(serialized_length, '\0');
                record_stream_.read(&*serialized_frame.begin(), serialized_length);
                record_stream_.ignore();
                
                last_frame_.deserialize(serialized_frame);
                
                for_each(listeners_.begin(), listeners_.end(), [this](auto& listener){ listener.get().on_frame(*this); });
            }
        }
        
        {
            lock_guard<mutex> lg(m_);
            read_ = false;
        }
        cv_.notify_all();
    });
}

RecordPlayer::~RecordPlayer()
{
    reader_.join();
}
    
void RecordPlayer::add_listener(RecordPlayerListener& listener)
{
    listeners_.push_back(listener);
}

void RecordPlayer::start()
{
    {
        lock_guard<mutex> lg(m_);
        read_ = true;
    }
    cv_.notify_one();
}

Leap::Frame RecordPlayer::frame() const
{
    return last_frame_;
}

void RecordPlayer::wait()
{
    unique_lock<mutex> ul(m_);
    cv_.wait(ul, [this]{ return !read_; });
}
    
}
