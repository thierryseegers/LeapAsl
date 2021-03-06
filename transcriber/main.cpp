#include "levhenstein_distance.h"

#include "LeapAsl/LeapAsl.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
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

int main(int argc, char *argv[])
{
	boost::filesystem::path capture, dictionary, language_model, lexicon, mlpack_softmax_regression_model;

	auto const validate_path = [](boost::filesystem::path const& path, string const& option_name)
	{
		if(!boost::filesystem::exists(path) ||
		   !boost::filesystem::is_regular_file(path))
		{
			throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value, option_name, path.string());
		}
	};

	boost::program_options::options_description options;
	options.add_options()
	("help,h", "Help screen")
	("capture,c", boost::program_options::value<boost::filesystem::path>(&capture)->default_value("capture"), "Path to capture file")
	("dictionary,d", boost::program_options::value<boost::filesystem::path>(&dictionary)->default_value("aspell_en_expanded")->notifier(bind(validate_path, placeholders::_1, "dictionary")), "Path to dictionary file")
	("lexicon,l", boost::program_options::value<boost::filesystem::path>(&lexicon)->default_value("lexicon.sample")->notifier(bind(validate_path, placeholders::_1, "lexicon")), "Path to lexicon file")
#if defined(ENABLE_MLPACK)
	("mlpack-softmax-regression-model,r", boost::program_options::value<boost::filesystem::path>(&mlpack_softmax_regression_model)->default_value("mlpack_softmax_regression_model.xml")->notifier(bind(validate_path, placeholders::_1, "mlpack-softmax-regression-model")), "Path to mlpack softmax regression model file")
#endif
	("language-model,m", boost::program_options::value<boost::filesystem::path>(&language_model)->default_value("romeo_and_juliet_corpus.mmap"), "Path to language model file");
	
	boost::program_options::variables_map vm;
	try
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), vm);
		boost::program_options::notify(vm);
		
		if(vm.count("help"))
		{
			cout << options << "\n";
			return 0;
		}
	}
	catch(const exception & e)
	{
		cerr << e.what() << endl;
		cout << options << endl;
		return 1;
	}

    sf::Font font;
    if(!font.loadFromFile("resources/arial.ttf"))
    {
        return EXIT_FAILURE;
    }
    
	sf::Text predictor_name = sf::Text("", font, 30);
	predictor_name.setColor(sf::Color(255, 255, 255, 170));
	predictor_name.setPosition(20.f, 350.f);

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

    string last_top_sentence;
    size_t cumulative_distance = 0;
    auto const on_gesture = [&](vector<pair<double, char>> const& top_matches, string const& top_sentence)
    {
        stringstream ss;
        for(auto const& top_match : top_matches)
        {
            ss << '\'' << (top_match.second == ' ' ? '_' : top_match.second) << "' [" << top_match.first << "]\n";
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
    
	 LeapAsl::Analyzer analyzer(boost::filesystem::ifstream(dictionary), language_model.string(), on_gesture);
    
	boost::filesystem::ofstream record(capture);
    LeapAsl::Recorder recorder(record);
    
    Leap::Controller controller;
    controller.addListener(recorder);

	// Instantiate predictors.
	map<string, unique_ptr<LeapAsl::Predictors::Predictor const>> predictors;


	predictors["Lexicon"] = make_unique<LeapAsl::Predictors::Lexicon>(boost::filesystem::ifstream(lexicon));
#if defined(ENABLE_MLPACK)
	predictors["MlpackSoftmaxRegression"] = make_unique<LeapAsl::Predictors::MlpackSoftmaxRegression>(mlpack_softmax_regression_model.string());
#endif
	auto const default_predictor = predictors.cbegin();

	predictor_name.setString(default_predictor->first);

	// Instantiate recognizer.
	LeapAsl::Recognizer recognizer(*default_predictor->second, bind(&LeapAsl::Analyzer::on_recognition, ref(analyzer), placeholders::_1));
    controller.addListener(recognizer);
    
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
                    
                    asl_character.setString("");
                    asl_sentence.setString("");
                    
                    last_top_sentence.clear();
                    cumulative_distance = 0;
                    current_levhenstein_distance.setString("Error: 0");
                    cumulative_levhenstein_distance.setString("Cumulative error: 0");
                    
                    record.open(capture, ios_base::trunc);
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
                            //replay_hand = lexicon.hands(replay_character.getString()[0])[0];
                        }
                        else if(event.key.code == sf::Keyboard::Space)
                        {
                            replay_character.setString('_');
                            //replay_hand = lexicon.hands(' ')[0];
                        }
                        else if(event.key.code == sf::Keyboard::Period)
                        {
                            replay_character.setString('.');
                            //replay_hand = lexicon.hands('.')[0];
                        }
                        
                    }
                    catch(out_of_range const& e)
                    {
                        replay_character.setString("<?>");
                        replay_hand = Leap::Hand::invalid();
                    }
                }
				else if(event.key.code >= sf::Keyboard::Num0 && event.key.code - sf::Keyboard::Num0 < predictors.size())
				{
					auto const p = next(predictors.cbegin(), event.key.code - sf::Keyboard::Num0);

					predictor_name.setString(p->first);
					recognizer.predictor() = *p->second;
				}
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clean the screen and the depth buffer
        
        window.pushGLStates();
        {
			window.draw(predictor_name);
			
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
