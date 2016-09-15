#include "LeapAsl/RecordPlayer.h"

#include <LeapSDK/Leap.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace LeapAsl
{

using namespace std;

RecordPlayer::RecordPlayer(istream& record)
    : record_(record)
{}

void RecordPlayer::add_listener(RecordPlayerListener& listener)
{
    listeners_.push_back(listener);
}

void RecordPlayer::read()
{
    string::size_type serialized_length;

	Leap::Frame frame;
	while(record_ >> serialized_length, record_.ignore())
	{
		string serialized_frame(serialized_length, '\0');
		record_.read(&*serialized_frame.begin(), serialized_length);
		record_.ignore();

		last_frame_.deserialize(serialized_frame);

		for_each(listeners_.begin(), listeners_.end(), [this](auto& listener)
				 {
					 listener.get().on_frame(*this);
				 });
	}
}
    
Leap::Frame RecordPlayer::frame() const
{
    return last_frame_;
}
    
}
