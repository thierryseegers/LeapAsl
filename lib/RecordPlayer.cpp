#include "LeapAsl/RecordPlayer.h"

#include <LeapSDK/Leap.h>

#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace LeapAsl
{

using namespace std;

RecordPlayer::RecordPlayer(istream& i, function<void ()> on_end)
    : record_stream_(i)
    , on_end_(on_end)
{}

Leap::Frame RecordPlayer::frame() const
{
    string::size_type serialized_length;
    record_stream_ >> serialized_length;
    record_stream_.ignore();
    
    if(record_stream_)
    {
        string serialized_frame(serialized_length, '\0');
        record_stream_.read(&*serialized_frame.begin(), serialized_length);
        record_stream_.ignore();
        
        Leap::Frame frame;
        frame.deserialize(serialized_frame);
        
        return frame;
    }
    else
    {
        on_end_();
        return Leap::Frame::invalid();
    }
}
    
}
