#include "levhenstein_distance.h"

#include "LeapAsl/LeapAsl.h"

#include <LeapSDK/Leap.h>
#include <LeapSDK/LeapMath.h>
#include <LeapSDK/util/LeapUtilGL.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

int main()
{
    sf::Font font;
    if(!font.loadFromFile("resources/arial.ttf"))
    {
        return EXIT_FAILURE;
    }
    
    sf::Text asl_character = sf::Text("", font, 30);
    asl_character.setColor(sf::Color(255, 255, 255, 170));
    asl_character.setPosition(20.f, 400.f);
    
    sf::Text asl_sentence = sf::Text("", font, 30);
    asl_sentence.setColor(sf::Color(255, 255, 255, 170));
    asl_sentence.setPosition(400.f, 400.f);
    
    sf::Text replay_character = sf::Text("", font, 50);
    replay_character.setColor(sf::Color(255, 255, 255, 170));
    replay_character.setPosition(600.f, 170.f);
    
    sf::Text current_levhenstein_distance = sf::Text("", font, 30);
    current_levhenstein_distance.setColor(sf::Color(255, 255, 255, 170));
    current_levhenstein_distance.setPosition(400.f, 450.f);
    
    sf::Text cumulative_levhenstein_distance = sf::Text("", font, 30);
    cumulative_levhenstein_distance.setColor(sf::Color(255, 255, 255, 170));
    cumulative_levhenstein_distance.setPosition(400.f, 480.f);

    ofstream capture("capture");
    
    string last_top_sentence;
    size_t cumulative_distance = 0;
    auto const on_gesture = [&](vector<pair<double, string>> const& top_matches, string const& top_sentence)
    {
        stringstream ss;
        for(auto const& top_match : top_matches)
        {
            ss << '\'' << (top_match.second == " " ? "_" : top_match.second) << "' [" << top_match.first << "]\n";
        }
        
        asl_character.setString(ss.str());
        
        sf::String s = top_sentence;
        s.replace(" ", "_");
        asl_sentence.setString(s);
        
        if(!last_top_sentence.empty())
        {
            auto const distance = levenshtein_distance(last_top_sentence, top_sentence);
            cumulative_distance += distance;
            
            current_levhenstein_distance.setString("Error: " + to_string(distance));
            cumulative_levhenstein_distance.setString("Cumulative error: " + to_string(cumulative_distance));
        }
        last_top_sentence = top_sentence;
    };
    
    LeapAsl::Analyzer analyzer("aspell_en_expanded", "romeo_and_juliet_corpus.mmap", on_gesture);

    LeapAsl::Lexicon Lexicon;
    {
        ifstream lexicon_data_istream("lexicon", ios::binary);
        if(!lexicon_data_istream)
        {
            lexicon_data_istream.open("lexicon.sample", ios::binary);
            if(!lexicon_data_istream)
            {
                return EXIT_FAILURE;
            }
        }
        
        lexicon_data_istream >> Lexicon;
    }
    
    Leap::Controller controller;
    LeapAsl::Recognizer recognizer(controller, Lexicon, bind(&LeapAsl::Analyzer::on_gesture, ref(analyzer), placeholders::_1));

    Leap::Hand replay_hand;

    // Request a 32-bits depth buffer when creating the window
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 32;
    contextSettings.stencilBits = 8;
    contextSettings.antialiasingLevel = 4;

    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "LeapAsl Transcriber", sf::Style::Default, contextSettings);
    window.setVerticalSyncEnabled(true);

    // Make the window the active target for OpenGL calls
    // Note: If using sf::Texture or sf::Shader with OpenGL,
    // be sure to call sf::Texture::getMaximumSize() and/or
    // sf::Shader::isAvailable() at least once before calling
    // setActive(), as those functions will cause a context switch
    window.setActive();

    // Enable Z-buffer read and write
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClearDepth(1.f);

    // Disable lighting
    glDisable(GL_LIGHTING);

    // Configure the viewport (the same size as the window)
    glViewport(0, 0, window.getSize().x, window.getSize().y);

    // Setup a perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLfloat const ratio = static_cast<float>(window.getSize().x) / window.getSize().y;
    glFrustum(-ratio, ratio, -1.f, 1.f, 1.f, 500.f);

    // Look here
    gluLookAt(0, 300, 250, 0., 0., 0., 0., 1., 0.);


    // Start game loop
    while(window.isOpen())
    {
        // Process events
        sf::Event event;
        while(window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
            
            // Escape key: exit
            if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
                window.close();
            
            // Adjust the viewport when the window is resized
            if (event.type == sf::Event::Resized)
                glViewport(0, 0, event.size.width, event.size.height);
            
            if(event.type == sf::Event::KeyPressed)
            {
                // Clear current sentence.
                if(event.key.code == sf::Keyboard::Delete ||
                   event.key.code == sf::Keyboard::BackSpace)
                {
                    analyzer.reset();
                    
                    last_top_sentence.clear();
                    cumulative_distance = 0;
                    current_levhenstein_distance.setString("Error: 0");
                    cumulative_levhenstein_distance.setString("Cumulative error: 0");
                    
                    capture.open("capture", ios_base::trunc);
                }
                // Look up a pre-recorded character.
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) ||
                        sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt))
                {
                    try
                    {
                        if(event.key.code >= sf::Keyboard::A && event.key.code <= sf::Keyboard::Z)
                        {
                            replay_character.setString(char(event.key.code + 'a'));
                            replay_hand = Lexicon.hand(replay_character.getString());
                        }
                        else if(event.key.code == sf::Keyboard::Space)
                        {
                            replay_character.setString("_");
                            replay_hand = Lexicon.hand(" ");
                        }
                        else if(event.key.code == sf::Keyboard::Period)
                        {
                            replay_character.setString(".");
                            replay_hand = Lexicon.hand(replay_character.getString());
                        }
                        
                    }
                    catch(out_of_range const& e)
                    {
                        replay_character.setString("<?>");
                        replay_hand = Leap::Hand::invalid();
                    }
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clean the screen and the depth buffer
        
        window.pushGLStates();
        {
            // Draw the last recognized letter.
            window.draw(asl_character);
            
            // Draw the current best word.
            window.draw(asl_sentence);
            
            window.draw(current_levhenstein_distance);
            window.draw(cumulative_levhenstein_distance);
            
            // Draw the replay character.
            if(replay_character.getString() != "")
            {
                window.draw(replay_character);
            }
        }
        window.popGLStates();
        
        auto const& hand = controller.frame().hands()[0];
        if(hand.isValid())
        {
            // Draw the hand with its original rotation and scale but offset to the left of the origin.
            auto const offset = Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, -hand.palmPosition() + Leap::Vector{-200, 0, 0}};
            LeapAsl::draw_skeleton_hand(hand, offset, LeapUtilGL::GLVector4fv{1, 0, 0, 1});
            
            // Draw the hand normalized and at the origin.
            auto const normalized = LeapAsl::normalized_hand_transform(hand);
            auto const centered = Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, -hand.palmPosition()};
            LeapAsl::draw_skeleton_hand(hand, normalized * centered, LeapUtilGL::GLVector4fv{0, 1, 0, 1});
        }
        
        // Draw the replay hand.
        if(replay_hand.isValid())
        {
            // Draw the replay hand normalized and offset to the right a tad.
            auto const centered = Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, -replay_hand.palmPosition()};
            auto const normalized = LeapAsl::normalized_hand_transform(replay_hand);
            auto const offset = Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, Leap::Vector{+200, 0, 0}};
            
            LeapAsl::draw_skeleton_hand(replay_hand, offset * normalized * centered, LeapUtilGL::GLVector4fv{0, 0, 1, 1});
        }

        // Finally, display the rendered frame on screen.
        window.display();
    }

    return 0;
}
