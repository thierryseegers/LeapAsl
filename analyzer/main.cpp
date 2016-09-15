#include "levhenstein_distance.h"

#include "LeapAsl/LeapAsl.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
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
	boost::filesystem::path dictionary, input, language_model, lexicon, mlpack_softmax_regression_model;

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
	("dictionary,d", boost::program_options::value<boost::filesystem::path>(&dictionary)->default_value("aspell_en_expanded")->notifier(bind(validate_path, placeholders::_1, "dictionary")), "Path to dictionary file")
	("input,i", boost::program_options::value<boost::filesystem::path>(&input)->notifier(bind(validate_path, placeholders::_1, "input")), "Path to input file, optional, defaults to stdin")
	("lexicon,l", boost::program_options::value<boost::filesystem::path>(&lexicon)->default_value("lexicon.sample"), "Path to lexicon file, implies --predictor=lexicon")
#if defined(ENABLE_MLPACK)
	("mlpack-softmax-regression-model,r", boost::program_options::value<boost::filesystem::path>(&mlpack_softmax_regression_model)->default_value("mlpack_softmax_regression_model.xml"), "Path to mlpack softmax regression model file, implies --predictor=mlpack-softmax-regression")
#endif
	("language-model,m", boost::program_options::value<boost::filesystem::path>(&language_model)->default_value("romeo_and_juliet_corpus.mmap"), "Path to language model file")
	("predictor,p", boost::program_options::value<string>()->default_value("lexicon")->notifier([](string const& value)
	{
		if(value == "mlpack-softmax-regression")
		{
#if !defined(ENABLE_MLPACK)
			throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value, "predictor", value);
#endif
		}
		else if(value != "lexicon")
		{
			throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value, "predictor", value);
		}
	}), "Predictor to use, one of: {lexicon"
#if defined(ENABLE_MLPACK)
		", mlpack-softmax-regression"
#endif
		"}");

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

		if(vm["predictor"].as<string>() == "lexicon")
		{
			validate_path(lexicon, "lexicon");
		}
		else if(vm["predictor"].as<string>() == "mlpack-softmax-regression")
		{
			validate_path(mlpack_softmax_regression_model, "mlpack-softmax-regression-model");
		}
	}
	catch(const exception & e)
	{
		cerr << e.what() << endl;
		cout << options << endl;
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
    
	LeapAsl::Analyzer analyzer(boost::filesystem::ifstream(dictionary), language_model.string(), on_gesture);
	boost::filesystem::ifstream i(input);
	LeapAsl::RecordPlayer record_player(i.is_open() ? i : cin);

	unique_ptr<LeapAsl::Predictors::Predictor> predictor;
	if(vm["predictor"].as<string>() == "mlpack-softmax-regression")
	{
		predictor = make_unique<LeapAsl::Predictors::MlpackSoftmaxRegression>(mlpack_softmax_regression_model.string());
	}
	else
	{
		predictor = make_unique<LeapAsl::Predictors::Lexicon>(boost::filesystem::ifstream(lexicon));
	}

	LeapAsl::Recognizer recognizer(*predictor, bind(&LeapAsl::Analyzer::on_recognition, ref(analyzer), placeholders::_1));
	record_player.add_listener(recognizer);

	record_player.read();
    
    cout << last_top_sentence << '\n' << cumulative_distance << '\n';
    
    return EXIT_SUCCESS;
}
