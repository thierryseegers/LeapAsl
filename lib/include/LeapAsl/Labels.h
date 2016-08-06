#pragma once

#include <map>

static const std::map<char, int> character_to_label = {
    {' ', 0},
    {'.', 1},
    {'a', 2},
    {'b', 3},
    {'c', 4},
    {'d', 5},
    {'e', 6},
    {'f', 7},
    {'g', 8},
    {'h', 9},
    {'i', 10},
    {'k', 11},
    {'l', 12},
    {'m', 13},
    {'n', 14},
    {'o', 15},
    {'p', 16},
    {'q', 17},
    {'r', 18},
    {'s', 19},
    {'t', 20},
    {'u', 21},
    {'v', 22},
    {'w', 23},
    {'x', 24},
    {'y', 25},
};

static auto const& ctl = character_to_label;

static const std::map<int, char> label_to_character = {
    {0, ' '},
    {1, '.'},
    {2, 'a'},
    {3, 'b'},
    {4, 'c'},
    {5, 'd'},
    {6, 'e'},
    {7, 'f'},
    {8, 'g'},
    {9, 'h'},
    {10, 'i'},
    {11, 'k'},
    {12, 'l'},
    {13, 'm'},
    {14, 'n'},
    {15, 'o'},
    {16, 'p'},
    {17, 'q'},
    {18, 'r'},
    {19, 's'},
    {20, 't'},
    {21, 'u'},
    {22, 'v'},
    {23, 'w'},
    {24, 'x'},
    {25, 'y'},
};

static auto const& ltc = label_to_character;
