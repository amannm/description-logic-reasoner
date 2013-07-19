#include "Job.h"
#include "Logger.h"
namespace {
	class Worker {
		Job& job;
	public:
		Worker(Job& j) : 
			job(j) {}
		void operator()() {
			while(true) {
				//Working room
				while(true) {
					job.queueAccess.lock();
					if(job.taskQueue.size() != 0) {
						auto task = job.taskQueue.front();
						job.taskQueue.pop();
						job.queueAccess.unlock();
						task();
					}
					else {
						job.queueAccess.unlock();
						break;
					}
				}
				//Waiting room
				std::unique_lock<std::mutex> waitingStatus(job.saturationStatus);
				if(--job.numActive == 0) {
					job.finished = true;
					waitingStatus.unlock();
					job.newTask.notify_all();
					break;
				}
				else {
					job.newTask.wait(waitingStatus);
					if(job.finished) {
						break;
					}
				}
				++job.numActive;
			}
		}
	};
}


Job::Job(const std::string& n) :
	name(n),
	numActive(0),
	finished(false) {
}
void Job::start(int numThreads) {
	if(taskQueue.size() > 0) {
		Logger::Section section(name + " (" + std::to_string(numThreads) + " threads)"); 
		numActive = numThreads;
		std::vector<std::thread> workers;
		while(numThreads-- > 0) {
			workers.push_back(std::thread(Worker(*this)));
		}
		for(auto& worker : workers) {
			worker.join();
		}
		numActive = 0;
		finished = false;
	}
}
void Job::addTask(std::function<void ()> func) {
	queueAccess.lock();
	taskQueue.push(func);
	queueAccess.unlock();
	newTask.notify_one();
}