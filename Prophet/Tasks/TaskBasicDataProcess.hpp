#pragma once

#include "../Data/Upload.hpp"
#include "../Data/Model.hpp"
#include "../Data/ModelDatasets.hpp"
#include "../Data/BasicModelView.hpp"
#include "../FileReaders/CSVReader.hpp"
#include "../Data/ModelParams.hpp"

#include <thread>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>
#include <map>
#include "../TaskRunner/Task.hpp"
#include "../ML/Functions.hpp"

class TaskBasicDataProcess {



	static double Avg(Mtx m) {

		double avg = 0;
		int c = 0;
		for (int i = 1; i <= m.numRows(); i++) {
			for (int j = 1; j <= m.numCols(); j++) {

				avg = (avg * c + m(i,j)) / (c + 1);
			
				//WHAT IS THE BEST LANGUAGE EVER?
				c++;
			}
		}

		return avg;
	}

public:
	/*
	static void SaveMinMax(Model* model, Mtx* data){
		Underscore<Mtx::IndexType>  _;
		std::vector<std::string> cols = model->InputVariables();
		cols.push_back(model->OutputVariable());

		std::map<std::string, double> avgs;

		int colIdx = 1;
		for (std::string col : cols) {
			Mtx column = (*data)(_, _(colIdx, colIdx));
			double valMin = Functions::Min(column);
			double valMax = Functions::Max(column);
			
			Mtx mtxMin(1, 1);
			mtxMin(1,1) = valMin;


			Mtx mtxMax;
			mtxMax(1,1) = valMax;

			std::string varmin = col+"_min";
			std::string varmax = col+"_max";

			ModelParams::Save(*model, varmin, mtxMin);
			ModelParams::Save(*model, varmax, mtxMax);
		
		}


	}*/

	static void Averages(Model* model) {

		Underscore<Mtx::IndexType>  _;
		//the matrix's columns are in the order of model->inputVars() + outputVar
		Mtx* data = model->GetDataset();
		//ok we have all columns
		std::vector<std::string> cols = model->InputVariables();
		cols.push_back(model->OutputVariable());

		std::map<std::string, double> avgs;

		int colIdx = 1;
		for (std::string col : cols) {
			Mtx column = (*data)(_, _(colIdx, colIdx));

			double avg = Avg(column);

			avgs.emplace("avg"+col, avg);

			colIdx++;
		}

		BasicModelView* bmv = BasicModelView::Get(model->ModelId());

		if (bmv != nullptr) {
			bmv->SetAverages(avgs);
			bmv->Update();
		}
		else {
			bmv = new BasicModelView();
			bmv->SetId(model->ModelId());
			bmv->SetAverages(avgs);
			bmv->Insert();
		}
		delete data;
		delete bmv;

	}


	~TaskBasicDataProcess() {

	}

};
