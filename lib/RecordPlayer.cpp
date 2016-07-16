#include "LeapAsl/RecordPlayer.h"

#include <LeapSDK/Leap.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace LeapAsl
{

using namespace std;

RecordPlayer::RecordPlayer(istream& i)
    : record_stream_(i)
{}
    
void RecordPlayer::add_listener(RecordPlayerListener& listener)
{
    listeners_.push_back(listener);
}

void RecordPlayer::read()
{
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
}
    
Leap::Frame RecordPlayer::frame() const
{
    return last_frame_;
}
    
}
