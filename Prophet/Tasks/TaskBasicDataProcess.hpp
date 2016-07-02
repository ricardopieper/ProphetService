#pragma once

#include "../Data/Upload.hpp"
#include "../Data/Model.hpp"
#include "../Data/ModelDatasets.hpp"
#include "../Data/BasicModelView.hpp"
#include "../FileReaders/CSVReader.hpp"

#include <thread>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>
#include <map>
#include "../TaskRunner/Task.hpp"


class TaskBasicDataProcess {

	static double Avg(Mtx m) {

		double sum = 0;

		for (int i = 1; i <= m.numRows(); i++) {
			for (int j = 1; j <= m.numCols(); j++) {
				sum += m(i, j);
			}
		}

		return sum / (double)(m.numRows() * m.numCols());
	}

public:
	static void Run(Model* model, Mtx* data) {
		Underscore<Mtx::IndexType>  _;
		//the matrix's columns are in the order of model->inputVars() + outputVar

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

		delete bmv;

	}


	~TaskBasicDataProcess() {

	}

};
