#include "Data/DatabaseSession.hpp"
#include "Tasks/TaskProcessUpload.hpp"
#include "Tasks/TaskTrainModel.hpp"
#include "Tasks/TaskPerformPredictions.hpp"
#include "TaskRunner/TaskRunner.hpp"

//#include "Data/Upload.hpp"
#include <iostream>
//#include <string>

int main(int argc, char** argv) {

	if (argc > 1) {
		DatabaseSession::DefaultSession = argv[1];
	}

	Task *uploads = new TaskProcessUpload()
		 , *trainModel = new TaskTrainModel()
		 , *performPredictions = new TaskPerformPredictions()
		, *test = new TaskTestModel()
		;

	std::vector<TaskRunner*> tasks = {
	 	TaskRunner::RunTask(uploads),
		TaskRunner::RunTask(trainModel),
		TaskRunner::RunTask(performPredictions)
		//TaskRunner::RunTask(test)
	};

	for (auto task : tasks) {
		task->Start();
	}

	for (auto task : tasks) {
		task->Wait();
	}

	for (auto task : tasks) {
		delete task;
	}

	delete uploads;
	delete trainModel;
	delete performPredictions;


}
