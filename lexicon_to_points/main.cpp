#include "LeapAsl/Lexicon.h"

#include <LeapSDK/Leap.h>

#include <iostream>

using namespace std;

int main()
{
    LeapAsl::Lexicon lexicon;
    cin >> lexicon;

    for(auto const& name : lexicon.names())
    {
        for(auto const& hand : lexicon.hands(name))
        {
            cout << (int)name;
            
            for(auto const& finger : LeapAsl::to_position(hand))
            {
                for(auto const& joint : finger)
                {
                    cout << ',' << joint.x << ',' << joint.y << ',' << joint.z;
                }
            }
            
            cout << '\n';
        }
    }

    return 0;
}
