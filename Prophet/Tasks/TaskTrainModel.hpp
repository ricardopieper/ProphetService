#pragma once
#include "../ML/FlensDefs.hpp"
#include "../ML/NeuralNetwork.hpp"
#include "../TaskRunner/Task.hpp"

#include "../Data/Model.hpp"
#include "../Data/ModelParams.hpp"
#include "../ML/Functions.hpp"
#include "../FileReaders/FlensCsv.hpp"

#include <thread>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>

class TaskTrainModel : public Task {
private:

	void Process(Model* unprocessed) {

		//Mtx::IndexVariable i, j;
		Underscore<Mtx::IndexType> _;

		
		std::chrono::high_resolution_clock::time_point start(
			std::chrono::high_resolution_clock::now());

		Mtx* ds = unprocessed->GetDataset();

		if (ds != nullptr) {
			//delete all first
			ModelParams::DeleteAll(*unprocessed);

		
			//feature scaling
			Mtx dataset = *ds;

			double scaleMin = 0;
			double scaleMax = 1;
			
			std::vector<std::string> cols = unprocessed->InputVariables();
			cols.push_back(unprocessed->OutputVariable());

			for (int column = 1; column <= dataset.numCols(); column++) {

				Mtx mColuna = dataset(_, _(column, column));
		
				double valMin = Functions::Min(mColuna);
				double valMax = Functions::Max(mColuna);

				for (int row = 1; row <= dataset.numRows(); row++) {

					dataset(row, column) = ((dataset(row, column) - valMin) / (valMax - valMin))
						* (scaleMax - scaleMin) + scaleMin;

				}

				std::cout << "Saving min and max params";

				Mtx mtxMin(1, 1);
				mtxMin(1, 1) = valMin;
				std::cout << "min: "<<mtxMin<<std::endl;


				Mtx mtxMax(1, 1);
				mtxMax(1, 1) = valMax;
				std::cout << "max: " << mtxMin << std::endl;

				std::string varmin = cols[column-1] + "_min";
				std::string varmax = cols[column - 1] + "_max";

				ModelParams::Save(*unprocessed, varmin, mtxMin);
				ModelParams::Save(*unprocessed, varmax, mtxMax);
				
			}

			Mtx input = dataset(_, _(1, dataset.numCols() - 1));
			Mtx output = dataset(_, _(dataset.numCols(), dataset.numCols()));

			//number of hidden nodes = sqrt(cols(input)) * 2;

			NeuralNetwork nn((int)sqrt((double)input.numCols()) * 2, 0.1);



			std::cout << "Performing Training" <<std::endl;

			auto trainedParams = nn.Train(NeuralNetwork::ErrorType::MeanSquaredError,
				&input, &output);

			std::cout<<"Performing Training - Complete" <<std::endl;

		
			
			for (auto p : trainedParams) {
				std::cout<<"Saving param "<<p.Name<<std::endl;
				ModelParams::Save(*unprocessed, p.Name, p.Params);
			}
			
			std::chrono::high_resolution_clock::time_point end(
				std::chrono::high_resolution_clock::now());

			auto microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		
			unprocessed->SetProcessed(microseconds.count());
			std::cout<<"Training finished"<<std::endl;

			delete ds;
		} else{
			std::cout<<"Dataset is null"<<std::endl;
		}
	}
	bool logTrainingModel = true;
public:
	void Run() {
		try {
			Model* unprocessed = Model::GetPending();

			if (unprocessed != nullptr) {

				std::cout << "Training model" << std::endl;
				logTrainingModel = true;
				Process(unprocessed);
				delete unprocessed;
			}
			else {

				if (logTrainingModel)
				{
					std::cout << "No models to train" << std::endl;
					logTrainingModel = false;
				}

			}
		}
		catch (std::string& e) {
			std::cout << e << std::endl;
		}
	}

	~TaskTrainModel() {

	}

};

class TaskTestModel : public Task {
private:


	Mtx Rescale(Mtx result, Mtx* originalDataset) {

		Underscore<Mtx::IndexType> _;

		//last column
		Mtx lastCol = (*originalDataset)(_,
			_(originalDataset->numCols(),
				originalDataset->numCols()));

		//ok
		//the dataset follows the same feature order as M, like: temp1, temp2, etc..

		Mtx original = *originalDataset;
		double scaleMin = Functions::Min(lastCol);
		double scaleMax = Functions::Max(lastCol);


		double valMin = 0;
		double valMax = 1;


		for (int row = 1; row <= result.numRows(); row++) {

			result(row, 1) = ((result(row, 1) - valMin) / (valMax - valMin))
				* (scaleMax - scaleMin) + scaleMin;

		}


		return result;
	}

	void Process(Mtx* ds) {

		//Mtx::IndexVariable i, j;
		Underscore<Mtx::IndexType> _;

		//Mtx dataset = *(new Mtx());

		if (ds != nullptr) {
			//feature scaling

			Mtx dataset = *ds;
			Mtx copy = dataset;
			double scaleMin = 0;
			double scaleMax = 1;

			for (int column = 1; column <= dataset.numCols(); column++) {

				Mtx mColuna = dataset(_, _(column, column));

				double valMin = Functions::Min(mColuna);
				double valMax = Functions::Max(mColuna);

				for (int row = 1; row <= dataset.numRows(); row++) {

					dataset(row, column) = ((dataset(row, column) - valMin) / (valMax - valMin))
						* (scaleMax - scaleMin) + scaleMin;

				}

			}

			Mtx input = dataset(_, _(1, dataset.numCols() - 1));
			Mtx output = dataset(_, _(dataset.numCols(), dataset.numCols()));


			ofstream o("dataset.txt");
			o << "input" << std::endl;
			o << input;
			o << "output" << std::endl;
			o << output;
			o.close();

			//number of hidden nodes = sqrt(cols(input)) * 2;

			NeuralNetwork nn((int)sqrt((double)input.numCols()) * 2, 0.1);

			std::chrono::high_resolution_clock::time_point start(
				std::chrono::high_resolution_clock::now());

			auto trainedParams = nn.Train(NeuralNetwork::ErrorType::MeanSquaredError,
				&input, &output);

			std::chrono::high_resolution_clock::time_point end(
				std::chrono::high_resolution_clock::now());

			Mtx result = nn.Predict(input, trainedParams[0].Params, trainedParams[1].Params);
			ofstream o2("result.txt");
			o2 << "result" << std::endl;
			o2 << result;

			o2.close();

			result = Rescale(result, &copy);
			ofstream o3("result2.txt");
			o3 << "result" << std::endl;
			o3 << result;

			o3.close();
			//ModelParams::DeleteAll(*unprocessed);
			//for (auto p : trainedParams) {
//				ModelParams::Save(*unprocessed, p.Name, p.Params);
	//		}

		//	auto microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			//unprocessed->SetProcessed(microseconds.count());





			delete ds;
		}
	}

public:
	bool ran = false;
	void Run() {
		if (!ran) {
			try {
				FlensCsv v;

				Mtx::IndexVariable i, j;
				Underscore<Mtx::IndexType> _;

				Mtx* raw = v.LoadMatrix("C:/Users/Ricardo/Desktop/csvFile.txt");
				Process(raw);
			}
			catch (std::string& e) {
				std::cout << e << std::endl;
			}
		}
		ran = true;
	}
};
