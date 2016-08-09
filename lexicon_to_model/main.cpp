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
    cin >> lexicon;

	ofstream data_stream("data.csv"), labels_stream("labels.csv");

#if defined(USE_MLPACK)
    arma::Mat<double> training_data(240, 0);
    arma::Row<size_t> training_labels;
#endif

    for(char name : lexicon.names())
    {
        for(auto const& hand : lexicon.hands(name))
        {
			labels_stream << name << endl;

#if defined(USE_MLPACK)
			training_labels.insert_cols(training_labels.n_cols, 1);
			training_labels(0, training_labels.n_cols - 1) = LeapAsl::character_to_label.at(name);

			training_data.insert_cols(training_data.n_cols, 1);
			int i = 0;
#endif

			bool first = true;
            for(auto const& bone : LeapAsl::to_directions(hand))
            {
				first ? (void)(first = false) : (void)(data_stream << ',');

				data_stream
					<< bone.origin.x << ',' << bone.origin.y << ',' << bone.origin.z << ','
					<< bone.xBasis.x << ',' << bone.xBasis.y << ',' << bone.xBasis.z << ','
					<< bone.yBasis.x << ',' << bone.yBasis.y << ',' << bone.yBasis.z << ','
					<< bone.zBasis.x << ',' << bone.zBasis.y << ',' << bone.zBasis.z;

#if defined(USE_MLPACK)
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
#endif
			}

			data_stream << endl;
        }
    }

#if defined(USE_MLPACK)
    mlpack::regression::SoftmaxRegressionFunction sm_function(training_data, training_labels, LeapAsl::character_to_label.size());
    mlpack::optimization::L_BFGS<mlpack::regression::SoftmaxRegressionFunction> optimizer(sm_function);
    mlpack::regression::SoftmaxRegression<> sm_regression(optimizer);
    
    mlpack::data::Save("mlpack_softmax_regression_model.xml", "softmax_regression_model", sm_regression, true);
#endif
	
    return 0;
}
