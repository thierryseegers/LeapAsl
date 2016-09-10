#include "LeapAsl/Predictors.h"

#include "LeapAsl/Labels.h"
#include "LeapAsl/Lexicon.h"

#include <LeapSDK/Leap.h>
#if defined(USE_MLPACK)
	#include <mlpack/core.hpp>
	#include <mlpack/methods/softmax_regression/softmax_regression.hpp>
#endif

#include <fstream>
#include <map>

namespace LeapAsl
{

namespace Predictors
{
    
using namespace std;

Lexicon::Lexicon(ifstream&& lexicon)
{
    lexicon >> lexicon_;
}
    
multimap<double, char> Lexicon::predict(Leap::Hand const& hand) const
{
    return lexicon_.compare(hand);
}

#if defined(USE_MLPACK)
MlpackSoftmaxRegression::MlpackSoftmaxRegression(string const& model_path)
    : softmax_regression_model_(0, 0, true)
{
    mlpack::data::Load(model_path, "softmax_regression_model", softmax_regression_model_, true);
}
    
multimap<double, char> MlpackSoftmaxRegression::predict(Leap::Hand const& hand) const
{
    multimap<double, char> scores;
    
	arma::Mat<double> data(240, 1);
	int i = 0;

	for(auto const& bone : LeapAsl::to_directions(hand))
	{
		data(i++, 0) = bone.origin.x;
		data(i++, 0) = bone.origin.y;
		data(i++, 0) = bone.origin.z;

		data(i++, 0) = bone.xBasis.x;
		data(i++, 0) = bone.xBasis.y;
		data(i++, 0) = bone.xBasis.z;

		data(i++, 0) = bone.yBasis.x;
		data(i++, 0) = bone.yBasis.y;
		data(i++, 0) = bone.yBasis.z;

		data(i++, 0) = bone.zBasis.x;
		data(i++, 0) = bone.zBasis.y;
		data(i++, 0) = bone.zBasis.z;
	}

	arma::mat const hypothesis = arma::exp(softmax_regression_model_.Parameters() * data);
	arma::mat const probabilities = hypothesis / arma::repmat(arma::sum(hypothesis, 0), character_to_label.size(), 1);
    
    for(int i = 0; i != probabilities.n_rows; ++i)
    {
        scores.emplace(probabilities(i, 0), label_to_character.at(i));
    }
    
    return scores;
}
#endif

}
    
}
