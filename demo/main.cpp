#include "respacer.h"

#include "LeapLearnedGestures/LearnedGestures.h"

#include <LeapSDK/Leap.h>
#include <LeapSDK/LeapMath.h>
#include <LeapSDK/util/LeapUtil.h>
#include <LeapSDK/util/LeapUtilGL.h>
#include <lm/model.hh>
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

class Analyzer
{
public:
    using on_gesture_f = function<void (vector<pair<double, string>> const&, string const&)>;
    
    Analyzer(string const& dictionary_path, string const& language_model_path, on_gesture_f&& on_gesture)
        : dictionary_(dictionary_path)
        , model_(language_model_path.c_str())
        , on_gesture_(on_gesture)
        , restart_(false)
    {}

    void restart()
    {
        restart_ = true;
    }
    
    void on_gesture(map<double, string> const& matches)
    {
        size_t const n_top_matches = min((size_t)5, matches.size());
        
        vector<pair<double, string>> top_matches;
        copy_n(matches.begin(), n_top_matches, back_inserter(top_matches));
        
        vector<size_t> indices(n_top_matches);
        iota(indices.begin(), indices.end(), 0);
        
        if(sentences_.empty() ||
           restart_.exchange(false))
        {
            cout << ">restart" << endl;
            
            dropped_char_indices_.clear();
            sentences_.clear();

            for(int i : indices)
            {
                if(top_matches[i].second != Analyzer::space_symbol) // Skip whitespace at the begininng of a sentence.
                {
                    lm::ngram::State out_state;
                    auto const score = model_.Score(model_.BeginSentenceState(), model_.GetVocabulary().Index(top_matches[i].second), out_state);

                    sentences_.emplace(combined_score{0, score, i + 1.}, make_pair(top_matches[i].second, model_.BeginSentenceState()));
                }
            }
        }
        else
        {
            cout << ">next" << endl;
            
            decltype(sentences_) sentences;
            
            for(auto const& sentence : sentences_)
            {
                auto const last_space_ix = sentence.second.first.rfind(Analyzer::space_symbol);
                auto const last_word = (last_space_ix == string::npos ? sentence.second.first : sentence.second.first.substr(last_space_ix + 1));
            
                for(int i : indices)
                {
                    string const s = top_matches[i].second;

                    if(s == Analyzer::space_symbol)
                    {
                        if(!last_word.empty())  // Do nothing on double spaces.
                        {
                            auto const validity = dictionary_.validate(last_word.begin(), last_word.end());
                            if(validity == detail::trie<char>::correct)
                            {
                                lm::ngram::State out_state;
                                auto const score = model_.Score(sentence.second.second, model_.GetVocabulary().Index(last_word), out_state);
                                
                                sentences.emplace(combined_score{sentence.first.complete_words + score, 0., sentence.first.gesture + (i + 1) * top_matches[i].first},
                                                  make_pair(sentence.second.first + s, out_state));
                            }
                        }
                    }
                    else if(s == Analyzer::period_symbol)
                    {
                        if(!last_word.empty())  // Do nothing on double spaces.
                        {
                            auto const validity = dictionary_.validate(last_word.begin(), last_word.end());
                            if(validity == detail::trie<char>::correct)
                            {
                                lm::ngram::State out_state;
                                auto const score = model_.Score(sentence.second.second, model_.GetVocabulary().Index("</s>"), out_state);
                                
                                sentences.emplace(combined_score{sentence.first.complete_words + score, 0., sentence.first.gesture + (i + 1) * top_matches[i].first},
                                                  make_pair(sentence.second.first + s, out_state));
                            }
                        }
                    }
                    else
                    {
                        auto const word = last_word + s;
                        
                        auto const validity = dictionary_.validate(word.begin(), word.end());
                        if(validity & detail::trie<char>::valid)
                        {
                            lm::ngram::State out_state;
                            auto const score = model_.Score(sentence.second.second, model_.GetVocabulary().Index(word), out_state);

                            sentences.emplace(combined_score{sentence.first.complete_words, score, sentence.first.gesture + (i + 1) * top_matches[i].first},
                                              make_pair(sentence.second.first + s, sentence.second.second));
                        }
                    }
                }
            }

            if(sentences.size() == 0)
            {
                // This means none of the top matches lead to valid words. Record this spot as a dropped character.
                dropped_char_indices_.push_front(sentences_.begin()->second.first.size());
            }
            else
            {
                // Keep the top N results.
                if(sentences.size() > 100)
                {
                    sentences.erase(next(sentences.begin(), 100), sentences.end());
                }
                swap(sentences, sentences_);
            }
        }
        
        auto top_sentence = *sentences_.begin();
        for(auto const i : dropped_char_indices_)
        {
            top_sentence.second.first.insert(i, Analyzer::dropped_symbol);
        }
            
        for(auto const sentence : sentences_)
        {
            cout << "[{" << sentence.first.complete_words << ", " << sentence.first.incomplete_word << ", " << sentence.first.gesture
                << "}, \"" << sentence.second.first << "\"]" << endl;
        }
        
        on_gesture_(top_matches, top_sentence.second.first);
    }

    static string const space_symbol, period_symbol;
    
private:
    static string const dropped_symbol;
    
    detail::trie<char> dictionary_;
    lm::ngram::Model model_;
    on_gesture_f on_gesture_;
    
    atomic<bool> restart_;
    
    struct combined_score
    {
        double complete_words, incomplete_word, gesture;
        
        bool operator<(combined_score const& other) const
        {
            if(abs((complete_words + incomplete_word) - (other.complete_words + other.incomplete_word)) < numeric_limits<double>::epsilon())
            {
                return gesture < other.gesture;
            }
            else
            {
                return (complete_words + incomplete_word) > (other.complete_words + other.incomplete_word);
            }
        }
    };
    
    using sentences_t = multimap<combined_score, pair<string, lm::ngram::State>>;
    sentences_t sentences_;
    
    list<string::size_type> dropped_char_indices_;
};

string const Analyzer::space_symbol = "_", Analyzer::period_symbol = ".", Analyzer::dropped_symbol = "?";

int main()
{
    sf::Font font;
    if(!font.loadFromFile("resources/Inconsolata.otf"))
    {
        return EXIT_FAILURE;
    }
    
    sf::Text asl_letter = sf::Text("", font, 30);
    asl_letter.setColor(sf::Color(255, 255, 255, 170));
    asl_letter.setPosition(20.f, 400.f);
    
    sf::Text asl_sentence = sf::Text("", font, 30);
    asl_sentence.setColor(sf::Color(255, 255, 255, 170));
    asl_sentence.setPosition(400.f, 400.f);
    
    sf::Text replay_character = sf::Text("", font, 50);
    replay_character.setColor(sf::Color(255, 255, 255, 170));
    replay_character.setPosition(600.f, 170.f);
    
    auto const on_gesture = [&asl_letter, &asl_sentence](vector<pair<double, string>> const& top_matches, string const& top_sentence)
    {
        stringstream ss;
        for(auto const& top_match : top_matches)
        {
            ss << '\'' << top_match.second << "' [" << top_match.first << "]\n";
        }
        
        asl_letter.setString(ss.str());
        
        asl_sentence.setString(top_sentence);
    };
    
    Analyzer analyzer("aspell_en_expanded", "romeo_and_juliet_corpus.mmap", on_gesture);

    LearnedGestures::Database database;
    {
        ifstream gestures_data_istream("gestures", ios::binary);
        if(!gestures_data_istream)
        {
            return EXIT_FAILURE;
        }
        
        gestures_data_istream >> database;
    }
    
    Leap::Controller controller;
    LearnedGestures::Recognizer recognizer(controller, database, bind(&Analyzer::on_gesture, ref(analyzer), placeholders::_1));

    Leap::Hand replay_hand;

    // Request a 32-bits depth buffer when creating the window
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 32;
    contextSettings.stencilBits = 8;
    contextSettings.antialiasingLevel = 4;

    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "LeapLearnedGestures Demo", sf::Style::Default, contextSettings);
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
                    analyzer.restart();
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                        sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))
                {
                    if(event.key.code >= sf::Keyboard::A && event.key.code <= sf::Keyboard::Z)
                    {
                        if(replay_character.getString().isEmpty() ||
                           char(event.key.code + 'a' != replay_character.getString()[0]))
                        {
                            replay_character.setString(char(event.key.code + 'a'));
                            replay_hand = database.hand(replay_character.getString());
                        }
                    }
                }
                else if(controller.frame().hands()[0].isValid())
                {
                    if(event.key.code >= sf::Keyboard::A && event.key.code <= sf::Keyboard::Z)
                    {
                        database.capture(string(1, event.key.code + 'a'), controller.frame());
                    }
                    else if(event.key.code == sf::Keyboard::Space)
                    {
                        database.capture(Analyzer::space_symbol, controller.frame());
                    }
                    else if(event.key.code == sf::Keyboard::Period)
                    {
                        database.capture(Analyzer::period_symbol, controller.frame());
                    }
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clean the screen and the depth buffer
        
        window.pushGLStates();
        {
            // Draw the last recognized letter.
            window.draw(asl_letter);
            
            // Draw the current best word.
            window.draw(asl_sentence);
            
            // Draw the replay character.
            if(replay_character.getString() != "" && replay_hand.isValid())
            {
                window.draw(replay_character);
            }
        }
        window.popGLStates();
        
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
        
        // Draw the replay hand.
        if(replay_character.getString() != "" && replay_hand.isValid())
        {
            // Draw the replay hand normalized and offset to the right a tad.
            auto const offset = Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, Leap::Vector{+200, 0, 0}};
            auto const normalized = LearnedGestures::normalized_hand_transform(replay_hand);
            
            drawTransformedSkeletonHand(replay_hand, offset * normalized, LeapUtilGL::GLVector4fv{0, 0, 1, 1});
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
