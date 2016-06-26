#include "Task.hpp"

class TaskRunner {
private:
	bool stop = false;
	std::thread* m_thread = nullptr;
	Task* m_task;

	void task() {
		while (!stop) {

			try {
				m_task->Run();
			}
			catch (std::string& err) {
				std::cout << "Error: " << err << std::endl;
			}
			std::this_thread::sleep_for(2s);
		}
	}

public:


	static TaskRunner* RunTask(Task* task) {
		return new TaskRunner(task);
	}

	TaskRunner(Task* task) {
		m_task = task;
	}

	void Start() {
		m_thread = new std::thread([this]() { task(); });
	}

	void Stop() {
		stop = true;
	}

	void Wait() {
		m_thread->join();
	}

	~TaskRunner() {
		delete m_thread;
	}

};