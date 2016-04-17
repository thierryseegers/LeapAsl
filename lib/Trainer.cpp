#include "include/LeapLearnedGestures/Trainer.h"

#include <LeapSDK/Leap.h>

#include <iostream>
#include <string>

namespace LearnedGestures
{

using namespace std;
    
void Trainer::capture(string const& name, Leap::Frame const& frame)
{
    poses_.capture(name, frame);
}

Leap::Hand Trainer::hand(string const& name) const
{
    return poses_.hand(name);
}

ostream& operator<<(ostream& o, LearnedGestures::Trainer const& t)
{
    return o << t.poses_;
}

istream& operator>>(istream& i, LearnedGestures::Trainer& t)
{
    return i >> t.poses_;
}

}
