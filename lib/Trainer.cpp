#include "include/LeapLearnedGestures/Trainer.h"

#include <LeapSDK/Leap.h>

#include <iostream>
#include <string>

namespace LearnedGestures
{

using namespace std;
    
void Trainer::capture(string const& name, Leap::Hand const& hand)
{
    poses_.capture(name, hand);
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
