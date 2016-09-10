#include "levhenstein_distance.h"

#include "LeapAsl/LeapAsl.h"

#include <boost/program_options.hpp>
#include <LeapSDK/Leap.h>

#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

using namespace std;

int main(int argc, char* argv[])
{
	string dictionary_path, language_model_path, lexicon_path;

	boost::program_options::options_description desc{"options"};
	desc.add_options()
	("help,h", "Help screen")
	("dictionary,d", boost::program_options::value<string>(&dictionary_path)->default_value("aspell_en_expanded"), "Path to dictionary file")
	("model,m", boost::program_options::value<string>(&language_model_path)->default_value("romeo_and_juliet_corpus.mmap"), "Path to language model file")
	("lexicon,l", boost::program_options::value<string>(&lexicon_path)->default_value("lexicon.sample"), "Path to lexicon file")
	("predictor,p", boost::program_options::value<string>()->default_value("lexicon")->notifier([](string const& value)
	{
#if !defined(USE_MLPACK)
		if(value == "mlpack-softmax-regression")
		{
			throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value, "predictor", value);
		}
#endif
	}), "Predictor to use, one of: {lexicon"
#if defined(USE_MLPACK)
", mlpack-softmax-regression"
#endif
"}"
	 );

	boost::program_options::variables_map vm;
	try
	{
		boost::program_options::store(po::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);

		if(vm.count("help"))
		{
			cout << desc << "\n";
			return 0;
		}
	}
	catch(const exception & e)
	{
		cerr << e.what() << endl;
		cout << desc << endl;
		return 1;
	}

	Leap::Controller controller;
	
    string last_top_sentence;
    size_t cumulative_distance = 0;
    auto const on_gesture = [&](vector<pair<double, char>> const&, string const& top_sentence)
    {
        cumulative_distance += levenshtein_distance(last_top_sentence, top_sentence);
        last_top_sentence = top_sentence;
    };
    
    LeapAsl::Analyzer analyzer(dictionary_path, language_model_path, on_gesture);
    LeapAsl::RecordPlayer record_player(cin);

	unique_ptr<LeapAsl::Predictors::Predictor> predictor;
	if(vm["predictor"].as<string>() == "mlpack-softmax-regression")
	{
		predictor = make_unique<LeapAsl::Predictors::MlpackSoftmaxRegression>("mlpack_softmax_regression_model.xml");
	}
	else
	{
		predictor = make_unique<LeapAsl::Predictors::Lexicon>(ifstream(lexicon_path));
	}

	LeapAsl::Recognizer recognizer(predictor.get(), bind(&LeapAsl::Analyzer::on_recognition, ref(analyzer), placeholders::_1));
	record_player.add_listener(recognizer);

	record_player.read();
    
    cout << last_top_sentence << '\n' << cumulative_distance << '\n';
    
    return EXIT_SUCCESS;
}
