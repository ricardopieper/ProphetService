#pragma once

#include "../Data/Upload.hpp"
#include "../Data/Model.hpp"
#include "../Data/ModelDatasets.hpp"
#include "../Data/BasicModelView.hpp"
#include "../Data/UploadChunks.hpp"
#include "TaskBasicDataProcess.hpp"

#include <thread>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>
#include <map>
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

		CSVReader* csv = nullptr;
		Model* m = nullptr;
		Mtx* mtx = nullptr;

		try {
			std::chrono::high_resolution_clock::time_point start(
				std::chrono::high_resolution_clock::now());

			std::cout << "Processing Upload" << std::endl;

			std::string file = UploadChunks::LoadFile(*upload);

			std::cout << "Loaded file: " << file.size() << std::endl;

			m = Model::Load(upload->ModelId());

			std::cout << "Loaded Model: " << m->Name() << std::endl;

			csv = new CSVReader(file);

			std::cout << "Loaded CSV Reader" << std::endl;

			auto fileHeader = csv->GetHeader();

			std::cout << "Loaded CSV Header: " << fileHeader.size() << std::endl;

			auto modelColumns = m->InputVariables();

			//consider the output variable also
			modelColumns.push_back(m->OutputVariable());

			//ok, does the file have all the columns of the model?
			std::cout << "model columns: " << std::endl;
			for (std::string modelCol : modelColumns) {
				std::cout << modelCol << std::endl;
			}
			std::cout << std::endl;
			std::cout << "file columns: " << std::endl;
			for (std::string fileCol : fileHeader) {
				std::cout << fileCol << std::endl;
			}


			//for each col in model, check if exists on file. If doesnt, then exception.

			std::map<std::string, int> indexDefs;

			//FLENS is 1-indexed by default
			int index = 1;
			for (std::string modelCol : modelColumns)
			{

				if (!Contains(fileHeader, modelCol))
				{

					std::string err = "Error while processing: make sure the columns in your CSV"
						" file match the variables that you chose in the model. Column not found in your file: "+ modelCol;
					
					throw err;
				}

				indexDefs.emplace(modelCol, index);
				index++;
			}


			mtx = csv->GetMatrix(indexDefs);


			std::cout << "Loaded matrix from csv" << std::endl;

			ModelDatasets modelDatasets;

			std::cout << "Saving model..." << std::endl;

			modelDatasets.Save(m, modelColumns, mtx);

			upload->SetProcessed(true);
			upload->SetResult("File loaded successfully");
			upload->Save();

			std::chrono::high_resolution_clock::time_point end(
				std::chrono::high_resolution_clock::now());

			auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

			m->SetIntColumn("millisecondsupload", milliseconds.count());

			TaskBasicDataProcess::Averages(m);

		}
		catch (std::string err) {
			upload->SetProcessed(true);
			upload->SetResult(err);
			upload->Save();
			std::cout << err;
		}

		delete mtx;
		delete m;
		delete csv;
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
