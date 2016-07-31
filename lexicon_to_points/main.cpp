#include "LeapAsl/Lexicon.h"

#include <LeapSDK/Leap.h>

#include <fstream>
#include <iostream>
#include <map>

using namespace std;

const map<char, int> character_to_label = {
    {'a', 0},
    {'b', 1},
    {'c', 2},
    {'d', 3},
    {'e', 4},
    {'f', 5},
    {'g', 6},
    {'h', 7},
    {'i', 8},
    {'k', 9},
    {'l', 10},
    {'m', 11},
    {'n', 12},
    {'o', 13},
    {'p', 14},
    {'q', 15},
    {'r', 16},
    {'s', 17},
    {'t', 18},
    {'u', 19},
    {'v', 20},
    {'w', 21},
    {'x', 22},
    {'y', 23},
    {' ', 24},
    {'.', 25}
};

int main()
{
    LeapAsl::Lexicon lexicon;
    cin >> lexicon;

    ofstream data("data.csv"), labels("labels.csv");
    
    for(auto const& name : lexicon.names())
    {
        for(auto const& hand : lexicon.hands(name))
        {
            labels << character_to_label.at(name) << '\n';
            
            for(auto const& finger : LeapAsl::to_position(hand))
            {
                for(auto const& joint : finger)
                {
                    data << joint.x << ',' << joint.y << ',' << joint.z << ',';
                }
            }
            
            data << "\b\n";
        }
    }

    return 0;
}
