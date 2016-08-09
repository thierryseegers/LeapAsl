#include "LeapAsl/Labels.h"
#include "LeapAsl/Lexicon.h"

#include <LeapSDK/Leap.h>
#if defined(USE_MLPACK)
	#include <mlpack/core.hpp>
	#include <mlpack/methods/softmax_regression/softmax_regression.hpp>
#endif

#include <fstream>
#include <iostream>

using namespace std;

int main()
{
    LeapAsl::Lexicon lexicon;
    ifstream lexicon_stream("lexicon.sample");
    lexicon_stream >> lexicon;
    //cin >> lexicon;

#if defined(USE_MLPACK)
    arma::Mat<double> training_data(240, 0);
    arma::Row<size_t> training_labels;
    
    for(char name : lexicon.names())
    {
        for(auto const& hand : lexicon.hands(name))
        {
			training_labels.insert_cols(training_labels.n_cols, 1);
			training_labels(0, training_labels.n_cols - 1) = LeapAsl::character_to_label.at(name);

            training_data.insert_cols(training_data.n_cols, 1);
			int i = 0;

            for(auto const& bone : LeapAsl::to_directions(hand))
            {
				training_data(i++, training_data.n_cols - 1) = bone.origin.x;
				training_data(i++, training_data.n_cols - 1) = bone.origin.y;
				training_data(i++, training_data.n_cols - 1) = bone.origin.z;

				training_data(i++, training_data.n_cols - 1) = bone.xBasis.x;
				training_data(i++, training_data.n_cols - 1) = bone.xBasis.y;
				training_data(i++, training_data.n_cols - 1) = bone.xBasis.z;

				training_data(i++, training_data.n_cols - 1) = bone.yBasis.x;
				training_data(i++, training_data.n_cols - 1) = bone.yBasis.y;
				training_data(i++, training_data.n_cols - 1) = bone.yBasis.z;

				training_data(i++, training_data.n_cols - 1) = bone.zBasis.x;
				training_data(i++, training_data.n_cols - 1) = bone.zBasis.y;
				training_data(i++, training_data.n_cols - 1) = bone.zBasis.z;
            }
        }
    }

	mlpack::data::Save("labels.csv", training_labels);
	mlpack::data::Save("data.csv", training_data);

    mlpack::regression::SoftmaxRegressionFunction sm_function(training_data, training_labels, LeapAsl::character_to_label.size());
    mlpack::optimization::L_BFGS<mlpack::regression::SoftmaxRegressionFunction> optimizer(sm_function);
    mlpack::regression::SoftmaxRegression<> sm_regression(optimizer);
    
    mlpack::data::Save("softmax_regression_model.xml", "softmax_regression_model", sm_regression, true);
#endif
	
    return 0;
}
