#pragma once


#include "../Data/ModelPredictions.hpp"
#include "../Data/ModelParams.hpp"
#include "../TaskRunner/Task.hpp"
#include "../ML/NeuralNetwork.hpp"

#include <thread>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>
#include <sstream>

class TaskPerformPredictions : public Task {
public:

	Mtx FeatureScaling(Model* m, Mtx M, std::map<std::string, ModelParams*> params) {

		Underscore<Mtx::IndexType> _;

		std::vector<std::string> cols = m->InputVariables();
		cols.push_back(m->OutputVariable());


		//the dataset follows the same feature order as M, like: temp1, temp2, etc..
		double scaleMin = 0;
		double scaleMax = 1;
		for (int col = 1; col <= M.numCols(); col++) {

			double valMin = (*params[cols[col - 1] + "_min"]->GetMtx())(1, 1);
			double valMax = (*params[cols[col - 1] + "_max"]->GetMtx())(1, 1);


			for (int row = 1; row <= M.numRows(); row++) {

				M(row, col) = ((M(row, col) - valMin) / (valMax - valMin))
					* (scaleMax - scaleMin) + scaleMin;
			}
		}

		return M;
	}


	Mtx Rescale(Model* m, Mtx result, std::map<std::string, ModelParams*> params) {

		Underscore<Mtx::IndexType> _;

		//the dataset follows the same feature order as M, like: temp1, temp2, etc..

	
		double scaleMin = (*params[m->OutputVariable() + "_min"]->GetMtx())(1, 1);
		double scaleMax = (*params[m->OutputVariable() + "_max"]->GetMtx())(1, 1);


		double valMin = 0;
		double valMax = 1;


		for (int row = 1; row <= result.numRows(); row++) {

			result(row, 1) = ((result(row, 1) - valMin) / (valMax - valMin))
				* (scaleMax - scaleMin) + scaleMin;

		}


		return result;
	}


	void Process(ModelPrediction* pending) {
		std::chrono::high_resolution_clock::time_point start(
			std::chrono::high_resolution_clock::now());

		Model* model = Model::Load(pending->ModelId());
		
		std::map<std::string, ModelParams*> neuralnetworkParameters =
			ModelParams::LoadParameters(model->ModelId());

		//check if Theta1 and Theta2 are present
		if (neuralnetworkParameters.size() > 0) {
			int params = neuralnetworkParameters.count("theta1") +
				neuralnetworkParameters.count("theta2");


			if (params != 2) {
			
				for (auto pair : neuralnetworkParameters) { //if there's only one, because weird reasons
					delete pair.second;
				}

				throw std::string("There are no params for the model being predicted");
			}
			else {
				//InputVariables on model dictates how the network layout is set up
				//so we must use the same order

				//let's create the dataset, varying only the time variable
				Mtx M(pending->AmountOfPredictions(), model->InputVariables().size());


				std::cout << "prediction dataset created" << std::endl;

				//Performance: I don't think it matters if we build an entire column at a time.
				//It doesn't really matter, the number of iterations would be the same... I guess.

				//lets calculate the step value for the time var just once:

				double step = (pending->ToValue() - pending->FromValue()) /
					((double)pending->AmountOfPredictions());


				std::cout << "step: " << step << std::endl;

				for (int i = 1; i <= pending->AmountOfPredictions(); i++) {
					//we must now add the values to the row in the order of the model's input variables
					int j = 1;
					for (auto inputVar : model->InputVariables()) {

						//if this is NOT the time variable, we just add the fixed value
						if (inputVar != pending->InputVar()) {
							double v = pending->InputValues()[inputVar];
							M(i, j) = v;  //value set by the user
						}
						else {
							//this is our time variable, lets calculate the 
							M(i, j) = pending->FromValue() + (step * i);
						}

						j++;
					}
				}

				std::cout << "will feature scale" << std::endl;
				M = FeatureScaling(model, M, neuralnetworkParameters);


				//get the trained params
				Mtx* theta1 = neuralnetworkParameters["theta1"]->GetMtx();
				Mtx* theta2 = neuralnetworkParameters["theta2"]->GetMtx();

				NeuralNetwork nn;

				Mtx result = nn.Predict(M, *theta1, *theta2);

				result = Rescale(model, result, neuralnetworkParameters);

				std::stringstream ss;

				std::cout << "saving result" << std::endl;

				//transform result in array
				//it's gonna be always a vector matrix (i.e. one line multiple columns)
				//so it doesn't really matter how many rows and cols we'll add to the 
				//string

				for (int i = 1; i <= result.numRows(); i++)
				{
					for (int j = 1; j <= result.numCols(); j++)
					{
						ss << result(i, j);
						if (i * j < result.numRows() * result.numCols()) {
							ss << ",";
						}
					}
				}

				pending->SaveResult(ss.str());
				std::chrono::high_resolution_clock::time_point end(
					std::chrono::high_resolution_clock::now());
				
				auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

				model->SetIntColumn("millisecondslastprediction", milliseconds.count());

				for (auto pair : neuralnetworkParameters) { //delete the allocated matrices
					delete pair.second;
				}

			}

		}
		if (model != nullptr) delete model;
	}

	void Run() {


		ModelPrediction* pending = ModelPrediction::GetPending();

		if (pending != nullptr) {
			//std::cout << "Performing prediction" << std::endl;

			Process(pending);
			delete pending;
		}
		else {
			//std::cout << "No predictions to perform" << std::endl;
		}
	}

	~TaskPerformPredictions() {

	}

};
