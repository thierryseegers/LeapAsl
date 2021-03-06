#include "LeapAsl/Lexicon.h"

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
    if(!font.loadFromFile("./resources/arial.ttf"))
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
    
    LeapAsl::Lexicon lexicon;
    {
        ifstream lexicon_data_istream("lexicon", ios::binary);
        if(lexicon_data_istream)
        {
            lexicon_data_istream >> lexicon;
        }
    }
    
    Leap::Controller controller;
    controller.setPolicy(Leap::Controller::POLICY_IMAGES);
    Leap::Hand replay_hand = Leap::Hand::invalid();

    // Request a 32-bits depth buffer when creating the window
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 32;
    contextSettings.stencilBits = 8;
    contextSettings.antialiasingLevel = 4;

    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "LeapAsl Lexicographer", sf::Style::Default, contextSettings);
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
                // Look up a pre-recorded character.
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) ||
                   sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt))
                {
                    try
                    {
                        if(event.key.code >= sf::Keyboard::A && event.key.code <= sf::Keyboard::Z)
                        {
                            replay_character.setString(char(event.key.code + 'a'));
                            replay_hand = lexicon.hands(replay_character.getString()[0])[0];
                        }
                        else if(event.key.code == sf::Keyboard::Space)
                        {
                            replay_character.setString(' ');
                            replay_hand = lexicon.hands(' ')[0];
                        }
                        else if(event.key.code == sf::Keyboard::Period)
                        {
                            replay_character.setString('.');
                            replay_hand = lexicon.hands('.')[0];
                        }
   
                    }
                    catch(out_of_range const& e)
                    {
                        replay_character.setString("<?>");
                        replay_hand = Leap::Hand::invalid();
                    }
                }
                // Record (or re-record) a character.
                else if(controller.frame().hands()[0].isValid())
                {
                    char name = '?';
                    if(event.key.code >= sf::Keyboard::A && event.key.code <= sf::Keyboard::Z)
                    {
                        name = event.key.code + 'a';
                    }
                    else if(event.key.code == sf::Keyboard::Space)
                    {
                        name = ' ';
                    }
                    else if(event.key.code == sf::Keyboard::Period)
                    {
                        name = '.';
                    }
                    
                    lexicon.capture(name, controller.frame());
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
    
    // Save the Lexicon to file.
    try
    {
        string const temp_filename = tmpnam(nullptr);
        
        ofstream lexicon_data_ostream(temp_filename.c_str(), ios::binary);
        if(lexicon_data_ostream)
        {
            lexicon_data_ostream << lexicon;
        }
        
        rename(temp_filename.c_str(), "lexicon");
    }
    catch(exception const& e)
    {
        cout << "Failed to write lexicon to disk" << endl << e.what() << endl;
    }

    return 0;
}
