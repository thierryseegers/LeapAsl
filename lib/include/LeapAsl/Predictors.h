#pragma once

#include "LeapAsl/Lexicon.h"

#include <LeapSDK/Leap.h>
#if defined(ENABLE_MLPACK)
	#include <mlpack/methods/softmax_regression/softmax_regression.hpp>
#endif

#include <iosfwd>
#include <map>
#include <string>

namespace LeapAsl
{
    
namespace Predictors
{

class Predictor
{
public:
    virtual std::multimap<double, char> predict(Leap::Hand const& hand) const = 0;
};
    
class Lexicon : public Predictor
{
    LeapAsl::Lexicon lexicon_;
    
public:
    Lexicon(std::ifstream&& lexicon);
    
    virtual std::multimap<double, char> predict(Leap::Hand const& hand) const override;
};

#if defined(ENABLE_MLPACK)
class MlpackSoftmaxRegression : public Predictor
{
	mlpack::regression::SoftmaxRegression<> softmax_regression_model_;

public:
	MlpackSoftmaxRegression(std::string const& model_path);
	
	virtual std::multimap<double, char> predict(Leap::Hand const& hand) const override;
};
#endif

}
    
}
