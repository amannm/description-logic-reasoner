#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <string>

struct Job {
	std::string name;
	std::mutex queueAccess; 
	std::queue<std::function<void()>> taskQueue;
	std::condition_variable newTask;

	std::mutex saturationStatus;
	int numActive;
	bool finished;

	Job(const std::string&);
	void start(int);
	void addTask(std::function<void ()> func);
};