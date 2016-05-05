#include "LeapAsl/Database.h"

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

void drawTransformedSkeletonHand(Leap::Hand const& hand,
                                 Leap::Matrix const& transformation = Leap::Matrix::identity(),
                                 LeapUtilGL::GLVector4fv const& bone_color = LeapUtilGL::GLVector4fv::One(),
                                 LeapUtilGL::GLVector4fv const& joint_color = LeapUtilGL::GLVector4fv::One())
{
    static float const joint_radius_scale = 0.75f;
    static float const bone_radius_scale = 0.5f;
    static float const palm_radius_scale = 1.15f;
    
    LeapUtilGL::GLAttribScope colorScope(GL_CURRENT_BIT);
    
    Leap::Vector const palm = hand.palmPosition();
    Leap::Vector const palm_direction = hand.direction();
    Leap::Vector const palm_normal = hand.palmNormal();
    Leap::Vector const palm_side = palm_direction.cross(palm_normal).normalized();

    float const thumb_distance = hand.fingers()[Leap::Finger::TYPE_THUMB].bone(Leap::Bone::TYPE_METACARPAL).prevJoint().distanceTo(hand.palmPosition());
    Leap::Vector const wrist = palm - thumb_distance * (palm_direction * 0.90f + (hand.isLeft() ? -1.0f : 1.0f) * palm_side * 0.38f);
    
    Leap::FingerList const& fingers = hand.fingers();
    
    float radius = 0.0f;
    Leap::Vector current_box_base;
    Leap::Vector last_box_base = wrist;
    
    for(int i = 0, ei = fingers.count(); i < ei; i++)
    {
        Leap::Finger const& finger = fingers[i];
        radius = finger.width() * 0.5f;
        
        // draw individual fingers
        for(int j = Leap::Bone::TYPE_METACARPAL; j <= Leap::Bone::TYPE_DISTAL; j++)
        {
            Leap::Bone const& bone = finger.bone(static_cast<Leap::Bone::Type>(j));
            
            // don't draw metacarpal, a box around the metacarpals is draw instead.
            if(j == Leap::Bone::TYPE_METACARPAL)
            {
                // cache the top of the metacarpal for the next step in metacarpal box
                current_box_base = bone.nextJoint();
            }
            else
            {
                glColor4fv(bone_color);
                drawCylinder(LeapUtilGL::kStyle_Solid, transformation.transformPoint(bone.prevJoint()), transformation.transformPoint(bone.nextJoint()), bone_radius_scale * radius);
                glColor4fv(joint_color);
                drawSphere(LeapUtilGL::kStyle_Solid, transformation.transformPoint(bone.nextJoint()), joint_radius_scale * radius);
            }
        }
        
        // draw segment of metacarpal box
        glColor4fv(bone_color);
        drawCylinder(LeapUtilGL::kStyle_Solid, transformation.transformPoint(current_box_base), transformation.transformPoint(last_box_base), bone_radius_scale * radius);
        
        glColor4fv(joint_color);
        drawSphere(LeapUtilGL::kStyle_Solid, transformation.transformPoint(current_box_base), joint_radius_scale*radius);
        
        last_box_base = current_box_base;
    }
    
    // close the metacarpal box
    radius = fingers[Leap::Finger::TYPE_THUMB].width() * 0.5f;
    current_box_base = wrist;
    glColor4fv(bone_color);
    drawCylinder(LeapUtilGL::kStyle_Solid, transformation.transformPoint(current_box_base), transformation.transformPoint(last_box_base), bone_radius_scale * radius);
    
    glColor4fv(joint_color);
    drawSphere(LeapUtilGL::kStyle_Solid, transformation.transformPoint(current_box_base), joint_radius_scale * radius);
    
    // draw palm position
    glColor4fv(joint_color);
    drawSphere(LeapUtilGL::kStyle_Solid, transformation.transformPoint(palm), palm_radius_scale * radius);
}

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
    
    LeapAsl::Database database;
    {
        ifstream gestures_data_istream("gestures", ios::binary);
        if(gestures_data_istream)
        {
            gestures_data_istream >> database;
        }
    }
    
    Leap::Controller controller;
    Leap::Hand replay_hand = Leap::Hand::invalid();

    // Request a 32-bits depth buffer when creating the window
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 32;
    contextSettings.stencilBits = 8;
    contextSettings.antialiasingLevel = 4;

    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "LeapLearnedGestures Recorder", sf::Style::Default, contextSettings);
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
                            replay_hand = database.hand(replay_character.getString());
                        }
                        else if(event.key.code == sf::Keyboard::Space)
                        {
                            replay_character.setString("_");
                            replay_hand = database.hand(" ");
                        }
                        else if(event.key.code == sf::Keyboard::Period)
                        {
                            replay_character.setString(".");
                            replay_hand = database.hand(replay_character.getString());
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
                    if(event.key.code >= sf::Keyboard::A && event.key.code <= sf::Keyboard::Z)
                    {
                        database.capture(string(1, event.key.code + 'a'), controller.frame());
                    }
                    else if(event.key.code == sf::Keyboard::Space)
                    {
                        database.capture(" ", controller.frame());
                    }
                    else if(event.key.code == sf::Keyboard::Period)
                    {
                        database.capture(".", controller.frame());
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
            drawTransformedSkeletonHand(hand, offset, LeapUtilGL::GLVector4fv{1, 0, 0, 1});

            // Draw the hand normalized and at the origin.
            auto const normalized = LeapAsl::normalized_hand_transform(hand);
            auto const centered = Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, -hand.palmPosition()};
            drawTransformedSkeletonHand(hand, normalized * centered, LeapUtilGL::GLVector4fv{0, 1, 0, 1});
        }
        
        // Draw the replay hand.
        if(replay_hand.isValid())
        {
            // Draw the replay hand normalized and offset to the right a tad.
            auto const centered = Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, -replay_hand.palmPosition()};
            auto const normalized = LeapAsl::normalized_hand_transform(replay_hand);
            auto const offset = Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, Leap::Vector{+200, 0, 0}};
            
            drawTransformedSkeletonHand(replay_hand, offset * normalized * centered, LeapUtilGL::GLVector4fv{0, 0, 1, 1});
        }

        // Finally, display the rendered frame on screen.
        window.display();
    }
    
    // Save the database to file.
    try
    {
        string const temp_filename = tmpnam(nullptr);
        
        ofstream gestures_data_ostream(temp_filename.c_str(), ios::binary);
        if(gestures_data_ostream)
        {
            gestures_data_ostream << database;
        }
        
        rename(temp_filename.c_str(), "gestures");
    }
    catch(exception const& e)
    {
        cout << "Failed to write gestures to disk" << endl << e.what() << endl;
    }

    return 0;
}
