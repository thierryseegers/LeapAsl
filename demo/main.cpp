#include "LeapLearnedGestures/LearnedGestures.h"
#include "LeapLearnedGestures/NormalizedHandTransform.h"

#include <LeapSDK/Leap.h>
#include <LeapSDK/LeapMath.h>
#include <LeapSDK/util/LeapUtil.h>
#include <LeapSDK/util/LeapUtilGL.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

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

class Listener : public LearnedGestures::Listener
{
public:
    Listener(LearnedGestures::Trainer const& trainer, sf::Text& text)
        : LearnedGestures::Listener(trainer)
        , text_(text)
    {}
    
    virtual void onGesture(LearnedGestures::LearnedGesture const& gesture) override
    {
        text_.setString(gesture.name());
        cout << gesture.name() << '\n';
    }
    
private:
    sf::Text& text_;
};

int main()
{
    LearnedGestures::Trainer trainer;
    {
        ifstream gestures_data_istream("gestures", ios::binary);
        if(gestures_data_istream)
        {
            gestures_data_istream >> trainer;
        }
    }
    
    sf::Font font;
    if (!font.loadFromFile("resources/arial.ttf"))
        return EXIT_FAILURE;

    sf::Text asl_letter = sf::Text("", font, 60);
    asl_letter.setColor(sf::Color(255, 255, 255, 170));
    asl_letter.setPosition(400.f, 500.f);

    Listener listener(trainer, asl_letter);
    
    Leap::Controller controller;
    controller.addListener(listener);
    
    // Request a 32-bits depth buffer when creating the window
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 32;
    contextSettings.stencilBits = 8;
    contextSettings.antialiasingLevel = 4;
    
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "LeapGesturesEx Demo", sf::Style::Default, contextSettings);
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
                if(event.key.code >= sf::Keyboard::A && event.key.code <= sf::Keyboard::Z)
                {
                    trainer.capture(string(1, event.key.code + 'A'), controller.frame().hands()[0]);
                }
                /*
                else if(event.key.code == sf::Keyboard::Space)
                {
                    auto const& hand = controller.frame().hands()[0];
                    
                    if(hand.isValid() && hand.confidence() >= 0.2)
                    {
                        auto const& scores = gestures.search(hand);
                        for(auto const& s : scores)
                        {
                            cout << '[' << s.first << ',' << s.second << ']' << endl;
                        }
                        
                        auto const& match = gestures.match(hand);
                        
                        asl_letter.setString(match);
                        cout << match << endl;
                    }
                    else
                    {
                        cout << "?" << endl;
                        asl_letter.setString('?');
                    }
                }
                 */
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clean the screen and the depth buffer
        
        window.pushGLStates();
        {
            Leap::Hand const& hand = controller.frame().hands()[0];
            
            // Draw the hand with its original rotation and scale.
            auto const offset = Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, -hand.palmPosition() + Leap::Vector{-200, 0, 0}};
            drawTransformedSkeletonHand(hand, offset, LeapUtilGL::GLVector4fv{1, 0, 0, 1});

            // Draw the hand normalized.
            auto const normalized = normalized_hand_transform(hand);
            drawTransformedSkeletonHand(hand, normalized, LeapUtilGL::GLVector4fv{0, 1, 0, 1});

            // Draw the last recognized letter.
            window.draw(asl_letter);
        }
        window.popGLStates();

        // Finally, display the rendered frame on screen
        window.display();
    }
    
    ofstream gestures_data_ostream("gestures", ios::binary);
    if(gestures_data_ostream)
    {
        gestures_data_ostream << trainer;
    }
    
    return 0;
}
