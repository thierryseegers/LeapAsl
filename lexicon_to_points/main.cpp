#include "LeapAsl/Labels.h"
#include "LeapAsl/Lexicon.h"

#include <LeapSDK/Leap.h>

#include <fstream>
#include <iostream>
#include <map>

using namespace std;

int main()
{
    LeapAsl::Lexicon lexicon;
    cin >> lexicon;

    ofstream data("data.csv"), labels("labels.csv");
    
    for(auto const& name : lexicon.names())
    {
        for(auto const& hand : lexicon.hands(name))
        {
            labels << LeapAsl::character_to_label.at(name) << '\n';
            
            bool first = true;

            for(auto const& finger : LeapAsl::to_position(hand))
            {
                for(auto const& joint : finger)
                {
                    if(first)
                    {
                        first = false;
                    }
                    else
                    {
                        data << ',';
                    }
                    
                    data << joint.x << ',' << joint.y << ',' << joint.z;
                }
            }
            
            data << "\n";
        }
    }

    return 0;
}
