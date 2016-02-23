#include "respacer.h"

#include "LeapLearnedGestures/LearnedGestures.h"
#include "LeapLearnedGestures/Utility.h"

#include <LeapSDK/Leap.h>
#include <LeapSDK/LeapMath.h>
#include <LeapSDK/util/LeapUtil.h>
#include <LeapSDK/util/LeapUtilGL.h>
#include <lm/model.hh>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
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

class Listener : public LearnedGestures::Listener
{
public:
    Listener(string const& dictionary_path, string const& language_model_path, LearnedGestures::Trainer const& trainer, sf::Text& letter, sf::Text& top_sentence, bool& restart)
        : LearnedGestures::Listener(trainer)
        , letter_(letter)
        , top_sentence_(top_sentence)
        , restart_(restart)
        , dictionary_(dictionary_path)
        , model_(language_model_path.c_str())
    {}

    virtual void onGesture(map<double, string> const& matches) override
    {
        static auto const indices = {0, 1, 2, 3, 4};
        static stringstream ss;

        array<pair<double, string>, 5> top_matches;
        copy_n(matches.begin(), 5, top_matches.begin());
        
        ss.str("");
        for(auto const& top_match : top_matches)
        {
            ss << '\'' << top_match.second << "' [" << top_match.first << "]\n";
        }

        letter_.setString(ss.str());
        
        
        if(sentences_.empty() ||
           restart_)
        {
            cout << ">restart" << endl;
            
            sentences_.clear();
            for(int i : indices)
            {
                sentences_.emplace(combined_score{i + 1., numeric_limits<double>::min()}, make_pair(string() + (char)::tolower(top_matches[i].second[0]), model_.BeginSentenceState()));
            }
            
            restart_ = false;
        }
        else
        {
            cout << ">next" << endl;
            
            decltype(sentences_) sentences;
            
            for(auto const& sentence : sentences_)
            {
                auto const last_space_ix = sentence.second.first.rfind(' ');
                auto const last_word = (last_space_ix == string::npos ? sentence.second.first : sentence.second.first.substr(last_space_ix + 1));
            
                for(int i : indices)
                {
                    char const c = (char)::tolower(top_matches[i].second[0]);

                    if(c == ' ')
                    {
                        if(!last_word.empty())  // Do nothing on double spaces.
                        {
                            auto const validity = dictionary_.validate(last_word.begin(), last_word.end());
                            if(validity == detail::trie<char>::correct)
                            {
                                lm::ngram::State const& in_state = model_.BeginSentenceState();
                                lm::ngram::State out_state;
                                
                                auto const score = model_.Score(in_state, model_.GetVocabulary().Index(last_word), out_state);
                                
                                sentences.emplace(combined_score{sentence.first.gesture_score + (i + 1) * top_matches[i].first, score},
                                                   make_pair(sentence.second.first + c, out_state));
                            }
                        }
                    }
                    else if(c == '.')
                    {
                        if(!last_word.empty())  // Do nothing on double spaces.
                        {
                            auto const validity = dictionary_.validate(last_word.begin(), last_word.end());
                            if(validity == detail::trie<char>::correct)
                            {
                                lm::ngram::State const& in_state = model_.BeginSentenceState();
                                lm::ngram::State out_state;
                                
                                auto const score = model_.Score(in_state, model_.GetVocabulary().Index("</s>"), out_state);
                                
                                sentences.emplace(combined_score{sentence.first.gesture_score + (i + 1) * top_matches[i].first, score},
                                                   make_pair(sentence.second.first + c, out_state));
                            }
                        }
                    }
                    else
                    {
                        auto const word = last_word + c;
                        
                        auto const validity = dictionary_.validate(word.begin(), word.end());
                        if(validity & detail::trie<char>::valid)
                        {
                            auto const score = combined_score{sentence.first.gesture_score + (i + 1) * top_matches[i].first, sentence.first.language_model_score};
                            sentences.emplace(score, make_pair(sentence.second.first + c, sentence.second.second));
                        }
                    }
                }
            }

            // Keep the top N results.
            if(sentences.size() > 50)
            {
                sentences.erase(next(sentences.begin(), 50));
            }
            swap(sentences, sentences_);

            auto const& top_sentence = *sentences_.begin();
            cout << "[{" << top_sentence.first.gesture_score << ", " << top_sentence.first.language_model_score
                << "}, \"" << top_sentence.second.first << "\"]" << endl;
            
            top_sentence_.setString(top_sentence.second.first);
 
        }
    }

private:
    sf::Text& letter_, &top_sentence_;
    bool& restart_;
    
    detail::trie<char> dictionary_;
    lm::ngram::Model model_;

public:
    struct combined_score
    {
        double gesture_score, language_model_score;
        
        bool operator<(combined_score const& other) const
        {
            return gesture_score * language_model_score < other.gesture_score * other.language_model_score;
        }
    };
    
private:
    using sentences_t = multimap<combined_score, pair<string, lm::ngram::State>>;
    sentences_t sentences_;
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
    if (!font.loadFromFile("resources/Inconsolata.otf"))
        return EXIT_FAILURE;

    sf::Text asl_letter = sf::Text("", font, 30);
    asl_letter.setColor(sf::Color(255, 255, 255, 170));
    asl_letter.setPosition(20.f, 400.f);
    
    sf::Text asl_word = sf::Text("", font, 30);
    asl_word.setColor(sf::Color(255, 255, 255, 170));
    asl_word.setPosition(400.f, 400.f);
    
    bool restart = false;

    Listener listener("../aspell_en_expanded", "../romeo_and_juliet.mmap", trainer, asl_letter, asl_word, restart);
    
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
                if(event.key.code == sf::Keyboard::Delete ||
                   event.key.code == sf::Keyboard::BackSpace)
                {
                    restart = true;
                }
                else if(controller.frame().hands()[0].isValid())
                {
                    if(event.key.code >= sf::Keyboard::A && event.key.code <= sf::Keyboard::Z)
                    {
                        trainer.capture(string(1, event.key.code + 'A'), controller.frame().hands()[0]);
                    }
                    else if(event.key.code == sf::Keyboard::Space)
                    {
                        trainer.capture(string(1, ' '), controller.frame().hands()[0]);
                    }
                    else if(event.key.code == sf::Keyboard::Period)
                    {
                        trainer.capture(string(1, '.'), controller.frame().hands()[0]);
                    }
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clean the screen and the depth buffer
        
        window.pushGLStates();
        {
            auto const& hand = controller.frame().hands()[0];
            
            if(hand.isValid())
            {
                // Draw the hand with its original rotation and scale but offset to the left a tad.
                auto const offset = Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, -hand.palmPosition() + Leap::Vector{-200, 0, 0}};
                drawTransformedSkeletonHand(hand, offset, LeapUtilGL::GLVector4fv{1, 0, 0, 1});

                // Draw the hand normalized.
                auto const normalized = LearnedGestures::normalized_hand_transform(hand);
                drawTransformedSkeletonHand(hand, normalized, LeapUtilGL::GLVector4fv{0, 1, 0, 1});
            }
            
            // Draw the last recognized letter.
            window.draw(asl_letter);
            
            // Draw the current best word.
            window.draw(asl_word);
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
