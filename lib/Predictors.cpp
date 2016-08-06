#include "LeapAsl/Predictors.h"

#include "LeapAsl/Labels.h"
#include "LeapAsl/Lexicon.h"

#include <LeapSDK/Leap.h>
#include <mlpack/core.hpp>
#include <mlpack/methods/softmax_regression/softmax_regression.hpp>

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

MlpackSoftmaxRegression::MlpackSoftmaxRegression(string const& model_path)
    : softmax_regression_model_(0, 0, true)
{
    mlpack::data::Load(model_path, "softmax_regression_model", softmax_regression_model_, true);
}
    
multimap<double, char> MlpackSoftmaxRegression::predict(Leap::Hand const& hand) const
{
    multimap<double, char> scores;
    
    arma::mat data(60, 1);
    size_t i = 0;
    for(auto const& finger : to_position(hand))
        for(auto const point : finger)
        {
            data(i++, 0) = point.x;
            data(i++, 0) = point.y;
            data(i++, 0) = point.z;
        }
    
    arma::mat const hypothesis = arma::exp(arma::repmat(softmax_regression_model_.Parameters().col(0), 1, data.n_cols) +
                                           softmax_regression_model_.Parameters().cols(1, softmax_regression_model_.Parameters().n_cols - 1) * data);
    arma::mat const probabilities = hypothesis / arma::repmat(arma::sum(hypothesis, 0), 26, 1);
    
    for(int i = 0; i != probabilities.n_rows; ++i)
    {
        scores.emplace(probabilities(i, 0), label_to_character.at(i));
    }
    
    return scores;
}

}
    
}
