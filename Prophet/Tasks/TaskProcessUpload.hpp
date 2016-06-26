#pragma once

#include "../Data/Upload.hpp"
#include "../Data/Model.hpp"
#include "../Data/ModelDatasets.hpp"
#include "../Data/BasicModelView.hpp"
#include "TaskBasicDataProcess.hpp"

#include <thread>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>
#include "../TaskRunner/Task.hpp"

class TaskProcessUpload : public Task {
private:

	bool Contains(std::vector<std::string> vec, std::string elem) {
		auto it = std::find(vec.begin(), vec.end(), elem);

		if (it != vec.end()) {
			return true;
		}
		else return false;

	}

	void Process(Upload* upload) {

		std::string file = upload->File();

		Model* m = Model::Load(upload->ModelId());

		CSVReader csv(file);

		auto fileHeader = csv.GetHeader();

		auto modelColumns = m->InputVariables();

		//consider the output variable also
		modelColumns.push_back(m->OutputVariable());

		//ok, does the file have all the columns of the model?
		//for each col in model, check if exists on file. If doesnt, then exception.

		for (std::string modelCol : modelColumns)
		{

			if (!Contains(fileHeader, modelCol))
			{
				std::string err = "Error while processing: make sure the columns in your CSV"
					" file match the variables that you chose in the model.";

				upload->SetProcessed(true);
				upload->SetResult(err);
				upload->Save();

				throw err;
			}
		}

		try {

			auto mtx = csv.GetMatrix();

			ModelDatasets model;

			model.Save(m, modelColumns, mtx);

			upload->SetProcessed(true);
			upload->SetResult("File loaded successfully");
			upload->Save();

			TaskBasicDataProcess::Run(m);

		}
		catch (std::string err) {
			upload->SetProcessed(true);
			upload->SetResult(err);
			upload->Save();
		}

		delete m;
	}


public:
	void Run() {
		//get a single job from cassandra
		Upload* unprocessed = Upload::GetPending();

		if (unprocessed != nullptr) {
			//std::cout << "Processing upload" << std::endl;

			Process(unprocessed);
			delete unprocessed;
		}
		else {
			//std::cout << "No uploads to process" << std::endl;
		}
	}


	~TaskProcessUpload() {

	}

};
